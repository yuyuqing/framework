

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


/* wProto : 低层协议栈类型(0 : EtherNet; 此时取值应为RTE_ETHER_TYPE_IPV4) */
WORD32 CIcmpStack::RecvEthPacket(CAppInterface *pApp,
                                 WORD16         wProto,
                                 WORD32         dwDevID,
                                 WORD32         dwPortID,
                                 WORD32         dwQueueID,
                                 T_MBuf        *pMBuf,
                                 T_EthHead     *ptEthHead)
{
    TRACE_STACK("CIcmpStack::RecvEthPacket()");

    if (unlikely(RTE_ETHER_TYPE_IPV4 != wProto))
    {
        return FAIL;
    }

    WORD32      dwResult   = INVALID_DWORD;
    T_Ipv4Head *ptIpv4Head = rte_pktmbuf_mtod_offset(pMBuf, struct rte_ipv4_hdr *, sizeof(struct rte_ether_hdr));
    T_IcmpHead *ptIcmpHead = (T_IcmpHead *)(ptIpv4Head + 1);

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
                                       pApp);
#else
            dwResult = ProcIcmpRequest(ptEthHead->s_addr.addr_bytes,
                                       ptIpv4Head->src_addr,
                                       ptIpv4Head->dst_addr,
                                       ptIpv4Head,
                                       ptIcmpHead,
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
                                   CAppInterface *pApp)
{
    TRACE_STACK("CIcmpStack::RecvEthPacket()");

    CEthApp            *pEthApp  = (CEthApp *)pApp;
    CDevQueue          *pQueue   = pEthApp->GetQueue();
    CEthDevice         *pDevice  = (CEthDevice *)(pQueue->GetDevice());
    struct rte_mempool *pMemPool = pQueue->GetMemPool();

    /* 目的IP不属于本设备 */
    if (FALSE == pDevice->IsMatch(dwDstIP))
    {
        return FAIL;
    }

    WORD16 wIcmpPayLen = ntohs(ptIpv4Head->total_length)
                       - sizeof(struct rte_ipv4_hdr)
                       - sizeof(struct rte_icmp_hdr);

    T_MBuf *pIcmpReply = EncodeIcmpReply(pDevice->GetMacAddr(),
                                         pSrcMacAddr,
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
    T_EthHead  *pEthHead  = NULL;
    T_Ipv4Head *pIpv4Head = NULL;
    T_IcmpHead *pIcmpHead = NULL;

    T_MBuf *pMBuf = rte_pktmbuf_alloc(pMBufPool);
    if (NULL == pMBuf)
    {
        return NULL;
    }

    WORD16 wTotalLen = sizeof(T_EthHead) + sizeof(T_Ipv4Head) + sizeof(T_IcmpHead) + wPayLoadLen;

    pMBuf->data_len = wTotalLen;
    pMBuf->pkt_len  = wTotalLen;

    pPkt     = rte_pktmbuf_mtod(pMBuf, BYTE *);
    pEthHead = (T_EthHead *)pPkt;

#if RTE_VERSION >= RTE_VERSION_NUM(21, 11, 1, 0)
    rte_memcpy(pEthHead->dst_addr.addr_bytes, pDstMacAddr, RTE_ETHER_ADDR_LEN);
    rte_memcpy(pEthHead->src_addr.addr_bytes, pSrcMacAddr, RTE_ETHER_ADDR_LEN);
#else
    rte_memcpy(pEthHead->d_addr.addr_bytes, pDstMacAddr, RTE_ETHER_ADDR_LEN);
    rte_memcpy(pEthHead->s_addr.addr_bytes, pSrcMacAddr, RTE_ETHER_ADDR_LEN);
#endif

    pEthHead->ether_type = htons(RTE_ETHER_TYPE_IPV4);

    pIpv4Head = (T_Ipv4Head *)(pEthHead + 1);
    pIpv4Head->version_ihl     = 0x45;
    pIpv4Head->type_of_service = 0;
    pIpv4Head->total_length    = htons(wPayLoadLen + sizeof(T_IcmpHead) + sizeof(T_Ipv4Head));
    pIpv4Head->packet_id       = 0;
    pIpv4Head->fragment_offset = 0;
    pIpv4Head->time_to_live    = 64;
    pIpv4Head->next_proto_id   = IPPROTO_ICMP;
    pIpv4Head->hdr_checksum    = 0;
    pIpv4Head->src_addr        = dwSrcIP;
    pIpv4Head->dst_addr        = dwDstIP;
    pIpv4Head->hdr_checksum    = rte_ipv4_cksum(pIpv4Head);

    pIcmpHead = (T_IcmpHead *)(pIpv4Head + 1);
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


