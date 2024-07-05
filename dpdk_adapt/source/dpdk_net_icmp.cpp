

#include "dpdk_net_icmp.h"
#include "dpdk_app_eth.h"
#include "dpdk_device_eth.h"


CIcmpStack::CIcmpStack ()
{
}


CIcmpStack::~CIcmpStack()
{
}


WORD32 CIcmpStack::Initialize(CCentralMemPool *pMemInterface)
{
    CNetStack::Initialize(pMemInterface);

    return SUCCESS;
}


/* 接收报文处理; pHead : ICMP报文头 */
WORD32 CIcmpStack::RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead)
{
    TRACE_STACK("CIcmpStack::RecvPacket()");

    WORD32      dwResult   = INVALID_DWORD;
    T_EthHead  *ptEthHead  = (T_EthHead *)(pHead - rtInfo.wL3Len - rtInfo.wL2Len);
    T_Ipv4Head *ptIpv4Head = (T_Ipv4Head *)(pHead - rtInfo.wL3Len);
    T_IcmpHead *ptIcmpHead = (T_IcmpHead *)pHead;

    switch (ptIcmpHead->icmp_type)
    {
    case RTE_IP_ICMP_ECHO_REQUEST :
        {
#if RTE_VERSION >= RTE_VERSION_NUM(21, 11, 1, 0)
            dwResult = ProcIcmpRequest(ptEthHead->src_addr.addr_bytes,
                                       ptIpv4Head->src_addr,
                                       ptIpv4Head->dst_addr,
                                       ptIpv4Head,
                                       ptIcmpHead,
                                       rtInfo.dwDeviceID,
                                       rtInfo.dwVlanID,
                                       pApp);
#else
            dwResult = ProcIcmpRequest(ptEthHead->s_addr.addr_bytes,
                                       ptIpv4Head->src_addr,
                                       ptIpv4Head->dst_addr,
                                       ptIpv4Head,
                                       ptIcmpHead,
                                       rtInfo.dwDeviceID,
                                       rtInfo.dwVlanID,
                                       pApp);
#endif
        }
        break ;

    case RTE_IP_ICMP_ECHO_REPLY :
        {
            /* 忽略, 丢弃 */
            dwResult = SUCCESS;
        }
        break ;

    default :
        break ;
    }

    return dwResult;
}


WORD32 CIcmpStack::ProcIcmpRequest(BYTE          *pSrcMacAddr,
                                   WORD32         dwSrcIP,
                                   WORD32         dwDstIP,
                                   T_Ipv4Head    *ptIpv4Head,
                                   T_IcmpHead    *ptIcmpHead,
                                   WORD32         dwDeviceID,
                                   WORD32         dwVlanID,
                                   CAppInterface *pApp)
{
    TRACE_STACK("CIcmpStack::ProcIcmpRequest()");

    CEthApp            *pEthApp  = (CEthApp *)pApp;
    CDevQueue          *pQueue   = pEthApp->GetQueue();
    CEthDevice         *pDevice  = (CEthDevice *)(pQueue->GetDevice());
    struct rte_mempool *pMemPool = pQueue->GetTxMemPool();

    /* 目的IP不属于本设备 */
    if (FALSE == pDevice->IsMatch(dwDstIP))
    {
        return FAIL;
    }

    if (0 != dwVlanID)
    {
        CVlanInst *pVlanInst = m_pVlanTable->FindVlan(dwDeviceID, dwVlanID);
        if (NULL == pVlanInst)
        {
            return FAIL;
        }
    }

    WORD16 wIcmpPayLen = ntohs(ptIpv4Head->total_length)
                       - sizeof(struct rte_ipv4_hdr)
                       - sizeof(struct rte_icmp_hdr);

    T_MBuf *pIcmpReply = EncodeIcmpReply(pDevice->GetMacAddr(),
                                         pSrcMacAddr,
                                         dwDeviceID,
                                         dwVlanID,
                                         dwDstIP,
                                         dwSrcIP,
                                         ptIcmpHead->icmp_ident,
                                         ptIcmpHead->icmp_seq_nb,
                                         (BYTE *)(ptIcmpHead + 1),
                                         wIcmpPayLen,
                                         pMemPool);
    if (NULL == pIcmpReply)
    {
        return FAIL;
    }

    pQueue->SendPacket(pIcmpReply);

    return SUCCESS;
}


T_MBuf * CIcmpStack::EncodeIcmpReply(BYTE               *pSrcMacAddr,
                                     BYTE               *pDstMacAddr,
                                     WORD32              dwDeviceID,
                                     WORD32              dwVlanID,
                                     WORD32              dwSrcIP,
                                     WORD32              dwDstIP,
                                     WORD16              wIdentify,
                                     WORD16              wSeqNum,
                                     BYTE               *pIcmpPayLoad,
                                     WORD16              wPayLoadLen,
                                     struct rte_mempool *pMBufPool)
{
    TRACE_STACK("CIcmpStack::EncodeIcmpReply()");

    BYTE       *pPkt      = NULL;
    T_IcmpHead *pIcmpHead = NULL;

    T_MBuf *pMBuf = rte_pktmbuf_alloc(pMBufPool);
    if (NULL == pMBuf)
    {
        return NULL;
    }

    pPkt = rte_pktmbuf_mtod(pMBuf, BYTE *);

    WORD16 wEthLen = EncodeEthPacket(pPkt,
                                     pSrcMacAddr,
                                     pDstMacAddr,
                                     dwDeviceID,
                                     dwVlanID,
                                     RTE_ETHER_TYPE_IPV4);
    WORD16 wIPv4Len = EncodeIpv4Packet((pPkt + wEthLen),
                                       (wPayLoadLen + sizeof(T_IcmpHead) + sizeof(T_Ipv4Head)),
                                       IPPROTO_ICMP,
                                       dwSrcIP,
                                       dwDstIP);

    WORD16 wTotalLen = wEthLen + wIPv4Len + sizeof(T_IcmpHead) + wPayLoadLen;

    pMBuf->data_len = wTotalLen;
    pMBuf->pkt_len  = wTotalLen;

    pIcmpHead = (T_IcmpHead *)(pPkt + wEthLen + wIPv4Len);
    pIcmpHead->icmp_type   = RTE_IP_ICMP_ECHO_REPLY;
    pIcmpHead->icmp_code   = 0;
    pIcmpHead->icmp_cksum  = 0;
    pIcmpHead->icmp_ident  = wIdentify;
    pIcmpHead->icmp_seq_nb = wSeqNum;

    rte_memcpy((BYTE *)(pIcmpHead + 1), pIcmpPayLoad, wPayLoadLen);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"

    pIcmpHead->icmp_cksum = CalcIcmpCheckSum((WORD16 *)pIcmpHead, sizeof(T_IcmpHead) + wPayLoadLen);

#pragma GCC diagnostic pop

    CHAR aucSrcMacAddr[24] = {0,};
    CHAR aucDstMacAddr[24] = {0,};

    sprintf(aucSrcMacAddr,
            "%02x:%02x:%02x:%02x:%02x:%02x",
            pSrcMacAddr[0], pSrcMacAddr[1], pSrcMacAddr[2],
            pSrcMacAddr[3], pSrcMacAddr[4], pSrcMacAddr[5]);
    sprintf(aucDstMacAddr,
            "%02x:%02x:%02x:%02x:%02x:%02x",
            pDstMacAddr[0], pDstMacAddr[1], pDstMacAddr[2],
            pDstMacAddr[3], pDstMacAddr[4], pDstMacAddr[5]);

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "SrcIP : %u, SrcMac : %s, DstIP : %u, DstMac : %s\n",
               dwSrcIP, aucSrcMacAddr,
               dwDstIP, aucDstMacAddr);

    return pMBuf;
}


WORD16 CIcmpStack::CalcIcmpCheckSum(WORD16 *pwAddr, WORD32 dwCount)
{
    register long lwSum = 0;

    while (dwCount > 1) 
    {
        lwSum   += *(WORD16 *)pwAddr++;
        dwCount -= 2;
    }

    if (dwCount > 0)
    {
        lwSum += *(WORD16 *)pwAddr;
    }

    while (lwSum >> 16)
    {
        lwSum = (lwSum & 0xffff) + (lwSum >> 16);
    }

    return ~lwSum;
}


