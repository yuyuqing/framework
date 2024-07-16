

#include "dpdk_app_eth.h"
#include "dpdk_net_arp.h"
#include "dpdk_net_ipv4.h"
#include "dpdk_net_ipv6.h"


CNetIntfHandler *g_pNetIntfHandler = NULL;


/********************************************************************
 **************************** Ethernet ******************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                Destination Hardware Address                  | *
 * |                              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ *
 * |                              |                               | *
 * |                   Source Hardware Address                    | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |         Length/Type          |             Data              | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 **************************** Ethernet ******************************

 ****************************** VLAN ********************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                Destination Hardware Address                  | *
 * |                              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ *
 * |                              |                               | *
 * |                   Source Hardware Address                    | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |              TPID            | PRI |CFI|       VID           | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |         Length/Type          |             Data              | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 ****************************** VLAN ********************************

 ******************************* ARP ********************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |          Hardware Type       |        Protocol Type          | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * | Hard Addr Len | ProtAddrLen  |           Operation           | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                   Sender Hardware Address                    | *
 * |                              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ *
 * |                              |      Sender IP Address        | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                   Target Hardware Address                    | *
 * |                              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ *
 * |                              |      Target IP Address        | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |           Identifier         |        Sequence Number        | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 ******************************* ARP ********************************

 ****************************** IPv4 ********************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |Version|  IHL |Type of Service|          Total Length         | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |         Identification       |Flags|      Fragment Offset    | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * | Time to Live |    Protocol   |         Header Checksum       | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                       Source Address                         | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                    Destination Address                       | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                    Options                    |    Padding   | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 ****************************** IPv4 ********************************

 ****************************** ICMP ********************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 7 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |     Type     |     Code      |          Checksum             | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |          Identifier          |        Sequence Number        | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                            Data                              | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 ****************************** ICMP ********************************

 ******************************* UDP ********************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 7 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |          Source Port         |       Destination Port        | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |            Length            |           Checksum            | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                         data octets                          | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 ******************************* UDP ********************************

 ******************************* TCP ********************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 7 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |          Source Port         |       Destination Port        | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                       Sequence Number                        | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                   Acknowledgment Number                      | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |  Data |          |U|A|P|R|S|F|                               | *
 * | Offset| Reserved |R|C|S|S|Y|I|            Window             | *
 * |       |          |G|K|H|T|N|N|                               | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |           Checksum           |         Urgent Pointer        | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                   Options                    |    Padding    | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                            data                              | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 ******************************* TCP ********************************

 ****************************** IPv6 ********************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |Version|  TrafficClass |               Flow Label             | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |         PayloadLength        |  NextHeader  |    HopLimit    | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                   Source Address(128bit)                     | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                Destination Address(128bit)                   | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 ****************************** IPv6 ********************************
 *******************************************************************/
WORD32 CNetStack::RecvPacket(VOID   *pArg,
                             WORD32  dwDevID,
                             WORD32  dwPortID,
                             WORD32  dwQueueID,
                             T_MBuf *pMBuf)
{
    T_OffloadInfo  tInfo;
    memset(&tInfo, 0x00, sizeof(tInfo));

    tInfo.dwDeviceID = dwDevID;
    tInfo.dwPortID   = dwPortID;
    tInfo.dwQueueID  = dwQueueID;

    CEthApp   *pApp      = (CEthApp *)pArg;
    T_EthHead *ptEthHead = rte_pktmbuf_mtod(pMBuf, T_EthHead *);

    ParseEthernet(ptEthHead, tInfo);

    return g_pNetIntfHandler->RecvPacket(pApp, tInfo, pMBuf, (CHAR *)ptEthHead);
}


WORD32 CNetStack::ParseEthernet(T_EthHead *ptEthHead, T_OffloadInfo &rtInfo)
{
    T_Ipv4Head *ptIpv4Head = NULL;
    T_Ipv6Head *ptIpv6Head = NULL;
    T_VlanHead *ptVlanHead = NULL;
    WORD32      dwVlanID   = 0;

    rtInfo.wL2Len   = sizeof(T_EthHead);
    rtInfo.wEthType = ptEthHead->ether_type;

    while ( (rtInfo.wEthType == HTONS(RTE_ETHER_TYPE_VLAN))
         || (rtInfo.wEthType == HTONS(RTE_ETHER_TYPE_QINQ)))
    {
        ptVlanHead = (T_VlanHead *)(((CHAR *)(ptEthHead)) + rtInfo.wL2Len);
        dwVlanID   = (dwVlanID << 12) | ((rte_be_to_cpu_16(ptVlanHead->vlan_tci)) & 0x0FFF);

        rtInfo.wL2Len   += sizeof(T_VlanHead);
        rtInfo.wEthType  = ptVlanHead->eth_proto;
    }

    rtInfo.dwVlanID = dwVlanID;

    switch (rtInfo.wEthType)
    {
    case HTONS(RTE_ETHER_TYPE_IPV4) :
        {
            ptIpv4Head = (T_Ipv4Head *)(((CHAR *)(ptEthHead)) + rtInfo.wL2Len);
            ParseIpv4(ptIpv4Head, rtInfo);
        }
        break ;

    case HTONS(RTE_ETHER_TYPE_IPV6) :
        {
            ptIpv6Head = (T_Ipv6Head *)(((CHAR *)(ptEthHead)) + rtInfo.wL2Len);
            ParseIpv6(ptIpv6Head, rtInfo);
        }
        break ;

    default :
        {
        }
        break ;
    }

    return SUCCESS;
}


WORD32 CNetStack::ParseIpv4(T_Ipv4Head *ptIpv4Head, T_OffloadInfo &rtInfo)
{
    T_TcpHead *ptTcpHead = NULL;

    rtInfo.wL3Len    = rte_ipv4_hdr_len(ptIpv4Head);
    rtInfo.ucL4Proto = ptIpv4Head->next_proto_id;

    switch (rtInfo.ucL4Proto)
    {
    case IPPROTO_UDP :
        {
            rtInfo.wL4Len = sizeof(T_UdpHead);
        }
        break ;

    case IPPROTO_TCP :
        {
            ptTcpHead     = (T_TcpHead *)(((CHAR *)(ptIpv4Head)) + rtInfo.wL3Len);
            rtInfo.wL4Len = (ptTcpHead->data_off & 0xF0) >> 2;
        }
        break ;

    case IPPROTO_SCTP :
        {
        }
        break ;

    default :
        {
        }
        break ;
    }

    return SUCCESS;
}


WORD32 CNetStack::ParseIpv6(T_Ipv6Head *ptIpv6Head, T_OffloadInfo &rtInfo)
{
    T_TcpHead *ptTcpHead = NULL;

    rtInfo.wL3Len    = sizeof(T_Ipv6Head);
    rtInfo.ucL4Proto = ptIpv6Head->proto;

    switch (rtInfo.ucL4Proto)
    {
    case IPPROTO_UDP :
        {
            rtInfo.wL4Len = sizeof(T_UdpHead);
        }
        break ;

    case IPPROTO_TCP :
        {
            ptTcpHead     = (T_TcpHead *)(((CHAR *)(ptIpv6Head)) + rtInfo.wL3Len);
            rtInfo.wL4Len = (ptTcpHead->data_off & 0xF0) >> 2;
        }
        break ;

    case IPPROTO_SCTP :
        {
        }
        break ;

    default :
        {
        }
        break ;
    }

    return SUCCESS;
}


CNetStack::CNetStack ()
{
    m_pMemInterface = NULL;
    m_pVlanTable    = NULL;
}


CNetStack::~CNetStack()
{
    m_pMemInterface = NULL;
    m_pVlanTable    = NULL;
}


WORD32 CNetStack::Initialize(CCentralMemPool *pMemInterface)
{
    m_pMemInterface = pMemInterface;
    m_pVlanTable    = &(g_pNetIntfHandler->GetVlanTable());

    return SUCCESS;
}


/********************************************************************
 **************************** Ethernet ******************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                Destination Hardware Address                  | *
 * |                              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ *
 * |                              |                               | *
 * |                   Source Hardware Address                    | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |         Length/Type          |             Data              | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 **************************** Ethernet ******************************

 ****************************** VLAN ********************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                Destination Hardware Address                  | *
 * |                              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ *
 * |                              |                               | *
 * |                   Source Hardware Address                    | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |              TPID            | PRI |CFI|       VID           | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |         Length/Type          |             Data              | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 ****************************** VLAN ********************************
*******************************************************************/
WORD16 CNetStack::EncodeEthPacket(BYTE   *pPkt,
                                  BYTE   *pSrcMacAddr,
                                  BYTE   *pDstMacAddr,
                                  WORD32  dwDeviceID,
                                  WORD32  dwVlanID,
                                  WORD16  wEthType)
{
    WORD16     wEthLen  = sizeof(T_EthHead);
    T_EthHead *pEthHead = (T_EthHead *)pPkt;

#if RTE_VERSION >= RTE_VERSION_NUM(21, 11, 1, 0)
    rte_memcpy(pEthHead->dst_addr.addr_bytes, pDstMacAddr, RTE_ETHER_ADDR_LEN);
    rte_memcpy(pEthHead->src_addr.addr_bytes, pSrcMacAddr, RTE_ETHER_ADDR_LEN);
#else
    rte_memcpy(pEthHead->d_addr.addr_bytes, pDstMacAddr, RTE_ETHER_ADDR_LEN);
    rte_memcpy(pEthHead->s_addr.addr_bytes, pSrcMacAddr, RTE_ETHER_ADDR_LEN);
#endif

    if (0 != dwVlanID)
    {
        pEthHead->ether_type = HTONS(RTE_ETHER_TYPE_VLAN);

        /* 在外面已经校验过VLANID, 因此pVlanInst必然不为NULL */
        CVlanInst  *pVlanInst  = m_pVlanTable->FindVlan(dwDeviceID, dwVlanID);
        T_VlanHead *ptVlanHead = (T_VlanHead *)(pEthHead + 1);
        WORD16      wVlanTag   = 0;
        WORD16      wVlanID    = (WORD16)dwVlanID;
        WORD16      wPriority  = (WORD16)(pVlanInst->GetPriority());

        wVlanTag = ((wPriority & 0x07) << 13) | (1 << 12) | (wVlanID & 0x0FFF);

        ptVlanHead->vlan_tci  = RTE_BE16(wVlanTag);
        ptVlanHead->eth_proto = HTONS(wEthType);

        wEthLen += sizeof(T_VlanHead);
    }
    else
    {
        pEthHead->ether_type = HTONS(wEthType);
    }

    return wEthLen;
}


/* 封装IPv4报文头
 * pPkt      : IPv4头地址
 * wTotalLen : 含IP头的IP报文长度
 * wProto    : 上层协议(ICMP/TCP/UDP/SCTP)
 */
/********************************************************************
 *******************************  IP ********************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |Version|  IHL |Type of Service|          Total Length         | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |         Identification       |Flags|      Fragment Offset    | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * | Time to Live |    Protocol   |         Header Checksum       | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                       Source Address                         | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                    Destination Address                       | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                    Options                    |    Padding   | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 *******************************  IP ********************************
*******************************************************************/
WORD16 CNetStack::EncodeIpv4Packet(BYTE   *pPkt,
                                   WORD16  wTotalLen,
                                   WORD16  wProto,
                                   WORD32  dwSrcIP,
                                   WORD32  dwDstIP)
{
    WORD16      wIPIdent  = g_pNetIntfHandler->FetchAddIPIdentity();
    WORD16      wIPv4Len  = sizeof(T_Ipv4Head);
    T_Ipv4Head *pIPv4Head = (T_Ipv4Head *)pPkt;

    pIPv4Head->version_ihl     = 0x45;
    pIPv4Head->type_of_service = 0;
    pIPv4Head->total_length    = HTONS(wTotalLen);
    pIPv4Head->packet_id       = HTONS(wIPIdent);
    pIPv4Head->fragment_offset = 0;
    pIPv4Head->time_to_live    = IP_HEAD_TTL;
    pIPv4Head->next_proto_id   = wProto;
    pIPv4Head->hdr_checksum    = 0;
    pIPv4Head->src_addr        = dwSrcIP;
    pIPv4Head->dst_addr        = dwDstIP;
    pIPv4Head->hdr_checksum    = rte_ipv4_cksum(pIPv4Head);

    return wIPv4Len;
}


CNetIntfHandler::CNetIntfHandler ()
{
    m_pArpStack  = NULL;
    m_pIPv4Stack = NULL;
    m_pIPv6Stack = NULL;

    g_pNetIntfHandler = this;
}


CNetIntfHandler::~CNetIntfHandler()
{
    if (NULL != m_pArpStack)
    {
        delete m_pArpStack;
        m_pMemInterface->Free((BYTE *)m_pArpStack);
    }

    if (NULL != m_pIPv4Stack)
    {
        delete m_pIPv4Stack;
        m_pMemInterface->Free((BYTE *)m_pIPv4Stack);
    }

    if (NULL != m_pIPv6Stack)
    {
        delete m_pIPv6Stack;
        m_pMemInterface->Free((BYTE *)m_pIPv6Stack);
    }

    m_pArpStack  = NULL;
    m_pIPv4Stack = NULL;
    m_pIPv6Stack = NULL;

    g_pNetIntfHandler = NULL;
}


WORD32 CNetIntfHandler::Initialize(CCentralMemPool *pMemInterface)
{
    TRACE_STACK("CNetIntfHandler::Initialize()");

    CNetStack::Initialize(pMemInterface);

    m_cIPTable.Initialize();
    m_cVlanTable.Initialize();

    BYTE *pArpMem  = m_pMemInterface->Malloc(sizeof(CArpStack));
    BYTE *pIPv4Mem = m_pMemInterface->Malloc(sizeof(CIPv4Stack));
    BYTE *pIPv6Mem = m_pMemInterface->Malloc(sizeof(CIPv6Stack));

    if ( (NULL == pArpMem)
      || (NULL == pIPv4Mem)
      || (NULL == pIPv6Mem))
    {
        assert(0);
    }

    m_pArpStack  = new (pArpMem) CArpStack();
    m_pIPv4Stack = new (pIPv4Mem) CIPv4Stack();
    m_pIPv6Stack = new (pIPv6Mem) CIPv6Stack();

    m_pArpStack->Initialize(pMemInterface);
    m_pIPv4Stack->Initialize(pMemInterface);
    m_pIPv6Stack->Initialize(pMemInterface);

    return SUCCESS;
}


WORD32 CNetIntfHandler::InitArpTable()
{
    TRACE_STACK("CNetIntfHandler::InitArpTable()");

    m_cArpTable.Initialize(m_cIPTable);

    return SUCCESS;
}


/* 接收以太网报文处理; pHead : 以太网头 */
WORD32 CNetIntfHandler::RecvPacket(CAppInterface *pApp,
                                   T_OffloadInfo &rtInfo,
                                   T_MBuf        *pMBuf,
                                   CHAR          *pHead)
{
    WORD32 dwResult = INVALID_DWORD;
    WORD16 wEthType = rte_be_to_cpu_16(rtInfo.wEthType);

    switch (wEthType)
    {
    case RTE_ETHER_TYPE_ARP :
        {
            dwResult = m_pArpStack->RecvPacket(pApp,
                                               rtInfo,
                                               pMBuf,
                                               (pHead + rtInfo.wL2Len));
        }
        break ;

    case RTE_ETHER_TYPE_IPV4 :
        {
            dwResult = m_pIPv4Stack->RecvPacket(pApp,
                                                rtInfo,
                                                pMBuf,
                                                (pHead + rtInfo.wL2Len));
        }
        break ;

    case RTE_ETHER_TYPE_IPV6 :
        {
            dwResult = m_pIPv6Stack->RecvPacket(pApp,
                                                rtInfo,
                                                pMBuf,
                                                (pHead + rtInfo.wL2Len));
        }
        break ;

    default :
        {
        }
        break ;
    }

    return dwResult;
}


