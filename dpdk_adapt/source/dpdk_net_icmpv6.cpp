

#include "dpdk_net_icmpv6.h"
#include "dpdk_app_eth.h"
#include "dpdk_device_eth.h"


CIcmpV6Stack::CIcmpV6Stack ()
{
    m_pArpTable = NULL;
    m_pNdpTable = NULL;
}


CIcmpV6Stack::~CIcmpV6Stack()
{
    m_pArpTable = NULL;
    m_pNdpTable = NULL;
}


WORD32 CIcmpV6Stack::Initialize(CCentralMemPool *pMemInterface)
{
    CNetStack::Initialize(pMemInterface);

    m_pArpTable = &(g_pNetIntfHandler->GetArpTable());
    m_pNdpTable = &(g_pNetIntfHandler->GetNdpTable());

    return SUCCESS;
}


/* 接收报文处理 */
WORD32 CIcmpV6Stack::RecvPacket(CAppInterface *pApp,
                                T_OffloadInfo &rtInfo,
                                T_MBuf        *pMBuf,
                                CHAR          *pHead)
{
    TRACE_STACK("CIcmpV6Stack::RecvPacket()");

    WORD32      dwResult   = INVALID_DWORD;
    T_EthHead  *ptEthHead  = (T_EthHead *)(pHead - rtInfo.wL3Len - rtInfo.wL2Len);
    T_Ipv6Head *ptIpv6Head = (T_Ipv6Head *)(pHead - rtInfo.wL3Len);
    T_IcmpHead *ptIcmpHead = (T_IcmpHead *)pHead;

    switch (ptIcmpHead->icmp_type)
    {
    case E_ICMP6_TYPE_NS :
        {
            dwResult = ProcNeighborSolicatation(pApp,
                                                rtInfo,
                                                ptEthHead,
                                                ptIpv6Head,
                                                ptIcmpHead);
        }
        break ;

    case E_ICMP6_TYPE_NA :
        {
            dwResult = ProcNeighborAdvertisement(ptEthHead,
                                                 ptIpv6Head,
                                                 rtInfo.dwDeviceID);
        }
        break ;

    case E_ICMP6_TYPE_EREQ :
        {
            dwResult = ProcEchoRequest(pApp,
                                       rtInfo,
                                       ptEthHead,
                                       ptIpv6Head,
                                       ptIcmpHead);
        }
        break ;

    case E_ICMP6_TYPE_EREP :
        {
            /* 忽略, 丢弃 */
            dwResult = SUCCESS;
        }
        break ;

    case E_ICMP6_TYPE_RA :
        {
            /* 用于IP自动配置 或 路由器信息获取, 暂不支持 */
        }
        break ;

    default :
        break ;
    }

    return dwResult;
}


/* 主动向目的IP发NS请求, 用于查询TargetIP对应的MAC地址 */
WORD32 CIcmpV6Stack::SendNeighborSolication(CDevQueue *pQueue,
                                            WORD64     lwOption,
                                            T_IPAddr  &rtTargetIP,
                                            T_IPAddr  &rtSrcIP,
                                            T_IPAddr  &rtDstIP,
                                            T_MacAddr &rtDstMac,
                                            WORD32     dwVlanID)
{
    TRACE_STACK("CIcmpV6Stack::SendNeighborSolication()");

    WORD32              dwDeviceID = pQueue->GetDeviceID();
    CEthDevice         *pDevice    = (CEthDevice *)(pQueue->GetDevice());
    struct rte_mempool *pMemPool   = pQueue->GetTxMemPool();

    T_MBuf *pNS = EncodeNeighborSolicitation(pDevice->GetMacAddr(),
                                             rtDstMac.aucMacAddr,
                                             dwDeviceID,
                                             dwVlanID,
                                             rtSrcIP,
                                             rtDstIP,
                                             rtTargetIP,
                                             lwOption,
                                             pMemPool);
    if (NULL == pNS)
    {
        return FAIL;
    }

    pQueue->SendPacket(pNS);

    return SUCCESS;
}


/* 主动向目的IP发RS请求, 用于查询路由器信息 */
WORD32 CIcmpV6Stack::SendRouterSolication(CDevQueue *pQueue,
                                          WORD64     lwOption,
                                          T_IPAddr  &rtSrcIP,
                                          T_IPAddr  &rtDstIP,
                                          T_MacAddr &rtSrcMac,
                                          T_MacAddr &rtDstMac)
{
    TRACE_STACK("CIcmpV6Stack::SendRouterSolication()");

    WORD32              dwDeviceID = pQueue->GetDeviceID();
    struct rte_mempool *pMemPool   = pQueue->GetTxMemPool();

    T_MBuf *pNS = EncodeRouterSolicitation(rtSrcMac.aucMacAddr,
                                           rtDstMac.aucMacAddr,
                                           dwDeviceID,
                                           0,
                                           rtSrcIP,
                                           rtDstIP,
                                           lwOption,
                                           pMemPool);
    if (NULL == pNS)
    {
        return FAIL;
    }

    pQueue->SendPacket(pNS);

    return SUCCESS;
}


T_MBuf * CIcmpV6Stack::EncodeNeighborSolicitation(BYTE     *pSrcMacAddr,
                                                  BYTE     *pDstMacAddr,
                                                  WORD32    dwDeviceID,
                                                  WORD32    dwVlanID,
                                                  T_IPAddr &rtSrcIP,
                                                  T_IPAddr &rtDstIP,
                                                  T_IPAddr &rtTargetIP,
                                                  WORD64    lwOption,
                                                  struct rte_mempool *pMBufPool)
{
    TRACE_STACK("CIcmpV6Stack::EncodeNeighborSolicitation()");

    BYTE       *pPkt      = NULL;
    CHAR       *pOption   = NULL;
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
                                     RTE_ETHER_TYPE_IPV6);

    WORD16 wIPv6Len = EncodeIpv6Packet((pPkt + wEthLen),
                                       (24 + sizeof(T_IcmpHead)),
                                       IPPROTO_ICMPV6,
                                       rtSrcIP,
                                       rtDstIP);

    WORD16 wTotalLen = wEthLen + wIPv6Len + sizeof(T_IcmpHead) + 24;

    pMBuf->data_len = wTotalLen;
    pMBuf->pkt_len  = wTotalLen;

    pIcmpHead              = (T_IcmpHead *)(pPkt + wEthLen + wIPv6Len);
    pIcmpHead->icmp_type   = E_ICMP6_TYPE_NS;
    pIcmpHead->icmp_code   = 0;
    pIcmpHead->icmp_cksum  = 0;
    pIcmpHead->icmp_ident  = 0;
    pIcmpHead->icmp_seq_nb = 0;

    memcpy((CHAR *)(pIcmpHead + 1), rtTargetIP.tIPv6.aucIPAddr, IPV6_ADDR_LEN);

    pOption = (CHAR *)(pPkt + wEthLen + wIPv6Len + sizeof(T_IcmpHead) + IPV6_ADDR_LEN);
    memcpy(pOption, &lwOption, sizeof(WORD64));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
    pIcmpHead->icmp_cksum = CalcIcmpv6CheckSum((WORD16 *)pIcmpHead,
                                               sizeof(T_IcmpHead) + 24,
                                               rtSrcIP,
                                               rtDstIP);
#pragma GCC diagnostic pop

    return pMBuf;
}


T_MBuf * CIcmpV6Stack::EncodeRouterSolicitation(BYTE     *pSrcMacAddr,
                                                BYTE     *pDstMacAddr,
                                                WORD32    dwDeviceID,
                                                WORD32    dwVlanID,
                                                T_IPAddr &rtSrcIP,
                                                T_IPAddr &rtDstIP,
                                                WORD64    lwOption,
                                                struct rte_mempool *pMBufPool)
{
    TRACE_STACK("CIcmpV6Stack::EncodeRouterSolicitation()");

    BYTE       *pPkt      = NULL;
    CHAR       *pOption   = NULL;
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
                                     RTE_ETHER_TYPE_IPV6);

    WORD16 wIPv6Len = EncodeIpv6Packet((pPkt + wEthLen),
                                       (sizeof(WORD64) + sizeof(T_IcmpHead)),
                                       IPPROTO_ICMPV6,
                                       rtSrcIP,
                                       rtDstIP);

    WORD16 wTotalLen = wEthLen + wIPv6Len + sizeof(T_IcmpHead) + sizeof(WORD64);

    pMBuf->data_len = wTotalLen;
    pMBuf->pkt_len  = wTotalLen;

    pIcmpHead              = (T_IcmpHead *)(pPkt + wEthLen + wIPv6Len);
    pIcmpHead->icmp_type   = E_ICMP6_TYPE_RS;
    pIcmpHead->icmp_code   = 0;
    pIcmpHead->icmp_cksum  = 0;
    pIcmpHead->icmp_ident  = 0;
    pIcmpHead->icmp_seq_nb = 0;

    pOption = (CHAR *)(pPkt + wEthLen + wIPv6Len + sizeof(T_IcmpHead));
    memcpy(pOption, &lwOption, sizeof(WORD64));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
    pIcmpHead->icmp_cksum = CalcIcmpv6CheckSum((WORD16 *)pIcmpHead,
                                               sizeof(T_IcmpHead) + sizeof(WORD64),
                                               rtSrcIP,
                                               rtDstIP);
#pragma GCC diagnostic pop

    return pMBuf;
}


WORD32 CIcmpV6Stack::ProcNeighborSolicatation(CAppInterface  *pApp,
                                              T_OffloadInfo  &rtInfo,
                                              T_EthHead      *ptEthHead,
                                              T_Ipv6Head     *ptIpv6Head,
                                              T_IcmpHead     *ptIcmpHead)
{
    TRACE_STACK("CIcmpV6Stack::ProcNeighborSolicatation()");

    CHAR               *pTargetIP = (((CHAR *)(ptIcmpHead)) + sizeof(T_IcmpHead));
    CEthApp            *pEthApp   = (CEthApp *)pApp;
    CDevQueue          *pQueue    = pEthApp->GetQueue();
    CEthDevice         *pDevice   = (CEthDevice *)(pQueue->GetDevice());
    struct rte_mempool *pMemPool  = pQueue->GetTxMemPool();

    T_IPAddr tSrcIP;     /* IPv6头中的源IP */
    T_IPAddr tDstIP;     /* IPv6头中的目的IP, 此IP可能是组播IP(被请求节点组播IP) */
    T_IPAddr tTargetIP;  /* ICMPv6报文中的目的IP */

    tSrcIP.eType    = E_IPV6_TYPE;
    tDstIP.eType    = E_IPV6_TYPE;
    tTargetIP.eType = E_IPV6_TYPE;

    memcpy(tSrcIP.tIPv6.aucIPAddr, ptIpv6Head->src_addr, IPV6_ADDR_LEN);
    memcpy(tDstIP.tIPv6.aucIPAddr, ptIpv6Head->dst_addr, IPV6_ADDR_LEN);
    memcpy(tTargetIP.tIPv6.aucIPAddr, pTargetIP, IPV6_ADDR_LEN);

#if RTE_VERSION >= RTE_VERSION_NUM(21, 11, 1, 0)
    BYTE *pSrcMac = ptEthHead->src_addr.addr_bytes;
    BYTE *pDstMac = ptEthHead->dst_addr.addr_bytes;
#else
    BYTE *pSrcMac = ptEthHead->s_addr.addr_bytes;
    BYTE *pDstMac = ptEthHead->d_addr.addr_bytes;
#endif

    UpdateNeighbor(rtInfo.dwDeviceID, tSrcIP, pSrcMac);

    ProcNDP(rtInfo.dwDeviceID, tSrcIP.tIPv6, pSrcMac);

    /* IPv6没有广播, 二层地址必须与单播或组播地址匹配 */
    if ( (FALSE == IsMultiCastAddr(pDstMac))
      && (FALSE == pDevice->IsMatch(pDstMac)))
    {
        return FAIL;
    }

    /* 被请求节点组播IP是否与本节点组播地址匹配 */
    if (FALSE == pDevice->IsMatch(rtInfo.dwVlanID, tDstIP))
    {
        CString<IPV6_STRING_LEN> cIPAddr;
        tDstIP.toStr(cIPAddr);

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_WARN, TRUE,
                   "DstAddr is not match; DeviceID : %d, PortID : %d, "
                   "QueueID : %d, VlanID : %d, DstIPAddr : %s\n",
                   rtInfo.dwDeviceID,
                   rtInfo.dwPortID,
                   rtInfo.dwQueueID,
                   rtInfo.dwVlanID,
                   cIPAddr.toChar());

        return FAIL;
    }

    /* 目的IP是否与本节点Link-Local或Global-Unicast地址匹配 */
    if (FALSE == pDevice->IsMatch(rtInfo.dwVlanID, tTargetIP))
    {
        CString<IPV6_STRING_LEN> cIPAddr;
        tTargetIP.toStr(cIPAddr);

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_WARN, TRUE,
                   "DstAddr is not match; DeviceID : %d, PortID : %d, "
                   "QueueID : %d, VlanID : %d, TargetIPAddr : %s\n",
                   rtInfo.dwDeviceID,
                   rtInfo.dwPortID,
                   rtInfo.dwQueueID,
                   rtInfo.dwVlanID,
                   cIPAddr.toChar());

        return FAIL;
    }

    /* IPv6头部PayloadLength不包含IPv6头部长度, 包含IPv6扩展头部长度 + 载荷长度 */
    /* ICMPv6载荷数据包括 : TargetIPAddr + ICMPv6Option */
    T_MBuf *pNA = EncodeNeighborAdvertisement(pDevice->GetMacAddr(),
                                              pSrcMac,
                                              rtInfo.dwDeviceID,
                                              rtInfo.dwVlanID,
                                              tTargetIP,
                                              tSrcIP,
                                              pMemPool);
    if (NULL == pNA)
    {
        return FAIL;
    }

    pQueue->SendPacket(pNA);

    /* 被动发起NS请求 */
    PassiveNS(pQueue,
              pDevice->GetMacAddr(),
              pSrcMac,
              rtInfo.dwDeviceID,
              rtInfo.dwVlanID,
              tTargetIP,
              tSrcIP,
              pMemPool);

    return SUCCESS;
}


T_MBuf * CIcmpV6Stack::EncodeNeighborAdvertisement(BYTE      *pSrcMacAddr,
                                                   BYTE      *pDstMacAddr,
                                                   WORD32     dwDeviceID,
                                                   WORD32     dwVlanID,
                                                   T_IPAddr  &rtSrcIP,
                                                   T_IPAddr  &rtDstIP,
                                                   struct rte_mempool *pMBufPool)
{
    TRACE_STACK("CIcmpV6Stack::EncodeNeighborAdvertisement()");

    BYTE       *pPkt      = NULL;
    CHAR       *pOption   = NULL;
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
                                     RTE_ETHER_TYPE_IPV6);

    WORD16 wIPv6Len = EncodeIpv6Packet((pPkt + wEthLen),
                                       (24 + sizeof(T_IcmpHead)),
                                       IPPROTO_ICMPV6,
                                       rtSrcIP,
                                       rtDstIP);

    WORD16 wTotalLen = wEthLen + wIPv6Len + sizeof(T_IcmpHead) + 24;

    pMBuf->data_len = wTotalLen;
    pMBuf->pkt_len  = wTotalLen;

    pIcmpHead              = (T_IcmpHead *)(pPkt + wEthLen + wIPv6Len);
    pIcmpHead->icmp_type   = E_ICMP6_TYPE_NA;
    pIcmpHead->icmp_code   = 0;
    pIcmpHead->icmp_cksum  = 0;
    pIcmpHead->icmp_ident  = HTONS(0x6000);
    pIcmpHead->icmp_seq_nb = 0;

    memcpy((CHAR *)(pIcmpHead + 1), rtSrcIP.tIPv6.aucIPAddr, IPV6_ADDR_LEN);

    pOption    = (CHAR *)(pPkt + wEthLen + wIPv6Len + sizeof(T_IcmpHead) + IPV6_ADDR_LEN);
    pOption[0] = 0x02;
    pOption[1] = 0x01;
    memcpy((CHAR *)(pOption + 2), pSrcMacAddr, ARP_MAC_ADDR_LEN);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
    pIcmpHead->icmp_cksum = CalcIcmpv6CheckSum((WORD16 *)pIcmpHead,
                                               sizeof(T_IcmpHead) + 24,
                                               rtSrcIP,
                                               rtDstIP);
#pragma GCC diagnostic pop

    return pMBuf;
}


WORD32 CIcmpV6Stack::ProcNeighborAdvertisement(T_EthHead  *ptEthHead,
                                               T_Ipv6Head *ptIpv6Head,
                                               WORD32      dwDevID)
{
    TRACE_STACK("CIcmpV6Stack::ProcNeighborAdvertisement()");

    T_IPAddr tSrcIP;
    T_IPAddr tDstIP;

    tSrcIP.eType = E_IPV6_TYPE;
    tDstIP.eType = E_IPV6_TYPE;

    memcpy(tSrcIP.tIPv6.aucIPAddr, ptIpv6Head->src_addr, IPV6_ADDR_LEN);
    memcpy(tDstIP.tIPv6.aucIPAddr, ptIpv6Head->dst_addr, IPV6_ADDR_LEN);

#if RTE_VERSION >= RTE_VERSION_NUM(21, 11, 1, 0)
    BYTE *pSrcMac = ptEthHead->src_addr.addr_bytes;
    BYTE *pDstMac = ptEthHead->dst_addr.addr_bytes;
#else
    BYTE *pSrcMac = ptEthHead->s_addr.addr_bytes;
    BYTE *pDstMac = ptEthHead->d_addr.addr_bytes;
#endif

    UpdateNeighbor(dwDevID, tSrcIP, pSrcMac);
    UpdateNeighbor(dwDevID, tDstIP, pDstMac);

    ProcNDP(dwDevID, tSrcIP.tIPv6, pSrcMac);

    return SUCCESS;
}


WORD32 CIcmpV6Stack::UpdateNeighbor(WORD32    dwDevID,
                                    T_IPAddr &rtIP,
                                    BYTE     *pMacAddr)
{
    CArpInst *pArpInst = m_pArpTable->FindArp(dwDevID, rtIP);
    if (NULL != pArpInst)
    {
        m_cLock.Lock();

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
        T_MacAddr tMacAddr;
        memcpy(tMacAddr.aucMacAddr, pMacAddr, ARP_MAC_ADDR_LEN);

        pArpInst = m_pArpTable->RegistArp(dwDevID, rtIP, tMacAddr);
        if (NULL == pArpInst)
        {
            return FAIL;
        }
    }

    pArpInst->Dump();

    return SUCCESS;
}


WORD32 CIcmpV6Stack::ProcNDP(WORD32 dwDevID, T_IPv6Addr &rtIP, BYTE *pMacAddr)
{
    TRACE_STACK("CIcmpV6Stack::ProcNDP()");

    /* 如果是多播地址, 忽略 */
    if ((0x33 == pMacAddr[0]) || (0xFF == rtIP.aucIPAddr[0]))
    {
        return FAIL;
    }

    CDevNdpTable *pDevNdpTable = m_pNdpTable->GetDevTable(dwDevID);
    if (NULL == pDevNdpTable)
    {
        return FAIL;
    }

    CNdpInst *pNdpInst = pDevNdpTable->FindNdpInst(rtIP);
    if (NULL != pNdpInst)
    {
        if (FALSE == pNdpInst->IsMatch(pMacAddr))
        {
            pNdpInst->UpdateMacAddr(pMacAddr);
        }

        return SUCCESS;
    }

    pNdpInst = pDevNdpTable->CreateNdpInst(dwDevID, rtIP, pMacAddr);
    if (NULL == pNdpInst)
    {
        return FAIL;
    }

    return SUCCESS;
}


WORD32 CIcmpV6Stack::PassiveNS(CDevQueue *pQueue,
                               BYTE      *pSrcMacAddr,
                               BYTE      *pDstMacAddr,
                               WORD32     dwDeviceID,
                               WORD32     dwVlanID,
                               T_IPAddr  &rtSrcIP,
                               T_IPAddr  &rtDstIP,
                               struct rte_mempool *pMBufPool)
{
    TRACE_STACK("CIcmpV6Stack::PassiveNS()");

    T_IcmpV6Option tOption;
    T_IPAddr       tMultiCastIP;

    tOption.aucOption[0] = 0x01;
    tOption.aucOption[1] = 0x01;
    memcpy(&(tOption.aucOption[2]), pSrcMacAddr, ARP_MAC_ADDR_LEN);

    tMultiCastIP.eType = E_IPV6_TYPE;

    /* 目的IP是被请求节点组播IP */
    memset(tMultiCastIP.tIPv6.aucIPAddr, 0x00, IPV6_ADDR_LEN);
    tMultiCastIP.tIPv6.aucIPAddr[0]  = 0xFF;
    tMultiCastIP.tIPv6.aucIPAddr[1]  = 0x02;
    tMultiCastIP.tIPv6.aucIPAddr[11] = 0x01;
    tMultiCastIP.tIPv6.aucIPAddr[12] = 0xFF;
    tMultiCastIP.tIPv6.aucIPAddr[13] = rtDstIP.tIPv6.aucIPAddr[13];
    tMultiCastIP.tIPv6.aucIPAddr[14] = rtDstIP.tIPv6.aucIPAddr[14];
    tMultiCastIP.tIPv6.aucIPAddr[15] = rtDstIP.tIPv6.aucIPAddr[15];

    T_MBuf *pNS = EncodeNeighborSolicitation(pSrcMacAddr,
                                             pDstMacAddr,
                                             dwDeviceID,
                                             dwVlanID,
                                             rtSrcIP,
                                             rtDstIP,
                                             rtDstIP,
                                             tOption.lwOption,
                                             pMBufPool);
    if (NULL == pNS)
    {
        return FAIL;
    }

    pQueue->SendPacket(pNS);

    return SUCCESS;
}


WORD32 CIcmpV6Stack::ProcEchoRequest(CAppInterface  *pApp,
                                     T_OffloadInfo  &rtInfo,
                                     T_EthHead      *ptEthHead,
                                     T_Ipv6Head     *ptIpv6Head,
                                     T_IcmpHead     *ptIcmpHead)
{
    TRACE_STACK("CIcmpV6Stack::ProcEchoRequest()");

    CEthApp            *pEthApp   = (CEthApp *)pApp;
    CDevQueue          *pQueue    = pEthApp->GetQueue();
    CEthDevice         *pDevice   = (CEthDevice *)(pQueue->GetDevice());
    struct rte_mempool *pMemPool  = pQueue->GetTxMemPool();

    T_IPAddr tSrcIP;  /* IPv6头中的源IP */
    T_IPAddr tDstIP;  /* IPv6头中的目的IP, 此IP必须是单播地址 */

    tSrcIP.eType = E_IPV6_TYPE;
    tDstIP.eType = E_IPV6_TYPE;

    memcpy(tSrcIP.tIPv6.aucIPAddr, ptIpv6Head->src_addr, IPV6_ADDR_LEN);
    memcpy(tDstIP.tIPv6.aucIPAddr, ptIpv6Head->dst_addr, IPV6_ADDR_LEN);

#if RTE_VERSION >= RTE_VERSION_NUM(21, 11, 1, 0)
    BYTE *pSrcMac = ptEthHead->src_addr.addr_bytes;
    BYTE *pDstMac = ptEthHead->dst_addr.addr_bytes;
#else
    BYTE *pSrcMac = ptEthHead->s_addr.addr_bytes;
    BYTE *pDstMac = ptEthHead->d_addr.addr_bytes;
#endif

    /* 二层地址必须与MAC地址匹配 */
    if (FALSE == pDevice->IsMatch(pDstMac))
    {
        return FAIL;
    }

    /* 三层地址必须是单播地址 */
    if (IsMultiCastIP(tDstIP))
    {
        return FAIL;
    }

    /* VLAN + IP 必须匹配 */
    if (FALSE == pDevice->IsMatch(rtInfo.dwVlanID, tDstIP))
    {
        CString<IPV6_STRING_LEN> cIPAddr;
        tDstIP.toStr(cIPAddr);

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_WARN, TRUE,
                   "DstAddr is not match; DeviceID : %d, PortID : %d, "
                   "QueueID : %d, VlanID : %d, DstIPAddr : %s\n",
                   rtInfo.dwDeviceID,
                   rtInfo.dwPortID,
                   rtInfo.dwQueueID,
                   rtInfo.dwVlanID,
                   cIPAddr.toChar());

        return FAIL;
    }

    /* IPv6头部PayloadLength不包含IPv6头部长度, 包含IPv6扩展头部长度 + 载荷长度 */
    WORD16 wIcmpPayLen = NTOHS(ptIpv6Head->payload_len)
                       - sizeof(T_IcmpHead);

    T_MBuf *pIcmpReply = EncodeReply(pDevice->GetMacAddr(),
                                     pSrcMac,
                                     rtInfo.dwDeviceID,
                                     rtInfo.dwVlanID,
                                     tDstIP,
                                     tSrcIP,
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


T_MBuf * CIcmpV6Stack::EncodeReply(BYTE      *pSrcMacAddr,
                                   BYTE      *pDstMacAddr,
                                   WORD32     dwDeviceID,
                                   WORD32     dwVlanID,
                                   T_IPAddr  &rtSrcIP,
                                   T_IPAddr  &rtDstIP,
                                   WORD16     wIdentify,
                                   WORD16     wSeqNum,
                                   BYTE      *pIcmpPayLoad,
                                   WORD16     wPayLoadLen,
                                   struct rte_mempool *pMBufPool)
{
    TRACE_STACK("CIcmpV6Stack::EncodeReply()");

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
                                     RTE_ETHER_TYPE_IPV6);

    WORD16 wIPv6Len = EncodeIpv6Packet((pPkt + wEthLen),
                                       (wPayLoadLen + sizeof(T_IcmpHead)),
                                       IPPROTO_ICMPV6,
                                       rtSrcIP,
                                       rtDstIP);

    WORD16 wTotalLen = wEthLen + wIPv6Len + sizeof(T_IcmpHead) + wPayLoadLen;

    pMBuf->data_len = wTotalLen;
    pMBuf->pkt_len  = wTotalLen;

    pIcmpHead              = (T_IcmpHead *)(pPkt + wEthLen + wIPv6Len);
    pIcmpHead->icmp_type   = E_ICMP6_TYPE_EREP;
    pIcmpHead->icmp_code   = 0;
    pIcmpHead->icmp_cksum  = 0;
    pIcmpHead->icmp_ident  = wIdentify;
    pIcmpHead->icmp_seq_nb = wSeqNum;

    rte_memcpy((BYTE *)(pIcmpHead + 1), pIcmpPayLoad, wPayLoadLen);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
    pIcmpHead->icmp_cksum = CalcIcmpv6CheckSum((WORD16 *)pIcmpHead,
                                               sizeof(T_IcmpHead) + wPayLoadLen,
                                               rtSrcIP,
                                               rtDstIP);
#pragma GCC diagnostic pop

    return pMBuf;
}


