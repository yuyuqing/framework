

#include "dpdk_net_arp.h"
#include "dpdk_mgr.h"
#include "dpdk_app_eth.h"
#include "dpdk_device_eth.h"


CArpStack::CArpStack ()
{
    m_pIPTable  = NULL;
    m_pArpTable = NULL;
}


CArpStack::~CArpStack()
{
    m_pIPTable  = NULL;
    m_pArpTable = NULL;
}


WORD32 CArpStack::Initialize(CCentralMemPool *pMemInterface)
{
    CNetStack::Initialize(pMemInterface);

    m_pIPTable  = &(g_pDpdkMgr->GetIPTable());
    m_pArpTable = &(g_pDpdkMgr->GetArpTable());

    return SUCCESS;
}


WORD32 CArpStack::RecvEthPacket(CAppInterface *pApp,
                                WORD32         dwDevID,
                                WORD32         dwPortID,
                                WORD32         dwQueueID,
                                T_MBuf        *pMBuf,
                                T_EthHead     *ptEthHead)
{
    TRACE_STACK("CArpStack::RecvEthPacket()");

    T_ArpHead *pArpHead = rte_pktmbuf_mtod_offset(pMBuf,
                                                  struct rte_arp_hdr *,
                                                  sizeof(struct rte_ether_hdr));
    WORD32 dwResult   = INVALID_DWORD;
    WORD16 wArpOpCode = rte_be_to_cpu_16(pArpHead->arp_opcode);
    switch (wArpOpCode)
    {
    case RTE_ARP_OP_REQUEST :
        {
            dwResult = ProcArpRequest(pApp, dwDevID, dwPortID, dwQueueID, pArpHead);
        }
        break ;

    case RTE_ARP_OP_REPLY :
        {
            dwResult = ProcArpReply(pArpHead, dwDevID);
        }
        break ;

    default :
        {
            /* 错误报文 */
            dwResult = FAIL;
        }
        break ;
    }

    return dwResult;
}


/* 主动向目的IP发Arp请求, 用于查询对端MAC地址 */
WORD32 CArpStack::SendArpRequest(CDevQueue *pQueue, WORD32 dwDstIP)
{
    return SUCCESS;
}


WORD32 CArpStack::ProcArpRequest(CAppInterface *pApp,
                                 WORD32         dwDevID,
                                 WORD32         dwPortID,
                                 WORD32         dwQueueID,
                                 T_ArpHead     *pArpHead)
{
    WORD32  dwSrcIP = pArpHead->arp_data.arp_sip;
    WORD32  dwDstIP = pArpHead->arp_data.arp_tip;
    BYTE   *pSrcMac = pArpHead->arp_data.arp_sha.addr_bytes;

    UpdateArpTable(dwDevID, dwSrcIP, pSrcMac);

    /* 根据目的IP查找IP表, 获取本地接口信息 */
    CIPInst *pIPInst = m_pIPTable->FindByIPv4(dwDstIP);
    if (NULL == pIPInst)
    {
        return FAIL;
    }

    if (dwDevID != pIPInst->GetDeviceID())
    {
        return FAIL;
    }

    CEthApp            *pEthApp  = (CEthApp *)pApp;
    CDevQueue          *pQueue   = pEthApp->GetQueue();
    CEthDevice         *pDevice  = (CEthDevice *)(pQueue->GetDevice());
    struct rte_mempool *pMemPool = pQueue->GetMemPool();

    T_MBuf *pArpReply = EncodeArpReply(pDevice->GetMacAddr(),
                                       pArpHead->arp_data.arp_sha.addr_bytes,
                                       pIPInst->GetIPAddr().dwIPv4,
                                       pArpHead->arp_data.arp_sip,
                                       pMemPool);
    if (NULL == pArpReply)
    {
        return FAIL;
    }

    pQueue->SendPacket(pArpReply);

    return SUCCESS;
}


WORD32 CArpStack::ProcArpReply(T_ArpHead *pArpHead, WORD32 dwDevID)
{
    WORD32  dwSrcIP = pArpHead->arp_data.arp_sip;
    WORD32  dwDstIP = pArpHead->arp_data.arp_tip;
    BYTE   *pSrcMac = pArpHead->arp_data.arp_sha.addr_bytes;
    BYTE   *pDstMac = pArpHead->arp_data.arp_tha.addr_bytes;

    UpdateArpTable(dwDevID, dwSrcIP, pSrcMac);
    UpdateArpTable(dwDevID, dwDstIP, pDstMac);

    return SUCCESS;
}


WORD32 CArpStack::UpdateArpTable(WORD32 dwDevID, WORD32 dwIP, BYTE *pMacAddr)
{
    CArpInst *pArpInst = m_pArpTable->FindArp(dwIP);
    if (NULL != pArpInst)
    {
        m_cLock.Lock();

        pArpInst->Update(dwDevID);

        if ((*pArpInst) == pMacAddr)
        {
            /* 无需更新 */
        }
        else
        {
            pArpInst->Update(pMacAddr);
        }

        m_cLock.UnLock();
    }
    else
    {
        T_IPAddr  tIPAddr;
        T_MacAddr tMacAddr;

        tIPAddr.dwIPv4 = dwIP;
        memcpy(tMacAddr.aucMacAddr, pMacAddr, ARP_MAC_ADDR_LEN);

        pArpInst = m_pArpTable->RegistArp(dwDevID, E_IPV4_TYPE, tIPAddr, tMacAddr);
        if (NULL == pArpInst)
        {
            return FAIL;
        }
    }

    pArpInst->Dump();

    return SUCCESS;
}


T_MBuf * CArpStack::EncodeArpReply(BYTE               *pSrcMacAddr,
                                   BYTE               *pDstMacAddr,
                                   WORD32              dwSrcIP,
                                   WORD32              dwDstIP,
                                   struct rte_mempool *pMBufPool)
{
    TRACE_STACK("CArpStack::EncodeArpReply()");

    BYTE      *pPkt     = NULL;
    T_EthHead *pEthHead = NULL;
    T_ArpHead *pArpHead = NULL;

    T_MBuf *pMBuf = rte_pktmbuf_alloc(pMBufPool);
    if (NULL == pMBuf)
    {
        return NULL;
    }

    WORD16 wTotalLen = sizeof(T_EthHead) + sizeof(T_ArpHead);

    pMBuf->data_len = wTotalLen;
    pMBuf->pkt_len  = wTotalLen;

    pPkt     = rte_pktmbuf_mtod(pMBuf, BYTE *);
    pEthHead = (T_EthHead *)pPkt;

    rte_memcpy(pEthHead->dst_addr.addr_bytes, pDstMacAddr, RTE_ETHER_ADDR_LEN);
    rte_memcpy(pEthHead->src_addr.addr_bytes, pSrcMacAddr, RTE_ETHER_ADDR_LEN);
    pEthHead->ether_type = htons(RTE_ETHER_TYPE_ARP);

    pArpHead               = (T_ArpHead *)(pEthHead + 1);
    pArpHead->arp_hardware = htons(RTE_ARP_HRD_ETHER);
    pArpHead->arp_protocol = htons(RTE_ETHER_TYPE_IPV4);
    pArpHead->arp_hlen     = RTE_ETHER_ADDR_LEN;
    pArpHead->arp_plen     = sizeof(WORD32);
    pArpHead->arp_opcode   = htons(RTE_ARP_OP_REPLY);

    rte_memcpy(pArpHead->arp_data.arp_sha.addr_bytes, pSrcMacAddr, RTE_ETHER_ADDR_LEN);
    pArpHead->arp_data.arp_sip = dwSrcIP;

    rte_memcpy(pArpHead->arp_data.arp_tha.addr_bytes, pDstMacAddr, RTE_ETHER_ADDR_LEN);
    pArpHead->arp_data.arp_tip = dwDstIP;

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
               "ArpSrcIP : %u, ArpSrcMac : %s, ArpDstIP : %u, ArpDstMac : %s\n",
               dwSrcIP, aucSrcMacAddr,
               dwDstIP, aucDstMacAddr);

    return pMBuf;
}


