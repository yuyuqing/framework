

#include "dpdk_app_eth.h"
#include "dpdk_net_arp.h"
#include "dpdk_net_ipv4.h"
#include "dpdk_net_ipv6.h"
#include "dpdk_net_ipsec.h"
#include "dpdk_net_udp.h"
#include "dpdk_net_sctp.h"
#include "dpdk_net_tcp.h"

#include "base_log.h"


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


/* 封装IPv6报文头
 * pPkt      : IPv6头地址
 * wTotalLen : 不含IPv6头的载荷长度(如有IPv6扩展头部, 则包含扩展头部)
 * wProto    : 上层协议(ICMP/TCP/UDP/SCTP)
 */
/********************************************************************
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
WORD16 CNetStack::EncodeIpv6Packet(BYTE     *pPkt,
                                   WORD16    wTotalLen,
                                   WORD16    wProto,
                                   T_IPAddr &rtSrcIP,
                                   T_IPAddr &rtDstIP)
{
    WORD16      wIPv6Len  = sizeof(T_Ipv6Head);
    T_Ipv6Head *pIPv6Head = (T_Ipv6Head *)pPkt;

    pIPv6Head->vtc_flow    = HTONW(0x60000000);
    pIPv6Head->payload_len = HTONS(wTotalLen);
    pIPv6Head->proto       = wProto;
    pIPv6Head->hop_limits  = IP_HEAD_TTL;

    memcpy(pIPv6Head->src_addr, rtSrcIP.tIPv6.aucIPAddr, IPV6_ADDR_LEN);
    memcpy(pIPv6Head->dst_addr, rtDstIP.tIPv6.aucIPAddr, IPV6_ADDR_LEN);

    return wIPv6Len;
}


/* 计算ICMP校验和 */
WORD16 CNetStack::CalcIcmpCheckSum(WORD16 *pwAddr, WORD32 dwCount)
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


/* 计算ICMPv6校验和 */
WORD16 CNetStack::CalcIcmpv6CheckSum(WORD16   *pwAddr,
                                     WORD32    dwCount,
                                     T_IPAddr &rtSrcAddr,
                                     T_IPAddr &rtDstAddr)
{
    WORD32 dwAcc     = 0;
    WORD32 dwAddr    = 0;
    WORD16 wProtoLen = (WORD16)dwCount;
    WORD16 wProto    = IPPROTO_ICMPV6;
    WORD16 wCheckSum = CalcIcmpCheckSum(pwAddr, dwCount);

    for (WORD32 dwIndex = 0; dwIndex < 4; dwIndex++)
    {
        dwAddr = rtSrcAddr.tIPv6.adwIPAddr[dwIndex];
        dwAcc  = dwAcc + (dwAddr & 0xFFFFUL);
        dwAcc  = dwAcc + ((dwAddr >> 16) & 0xFFFFUL);

        dwAddr = rtDstAddr.tIPv6.adwIPAddr[dwIndex];
        dwAcc  = dwAcc + (dwAddr & 0xFFFFUL);
        dwAcc  = dwAcc + ((dwAddr >> 16) & 0xFFFFUL);
    }

    dwAcc = (dwAcc >> 16) + (dwAcc & 0x0000FFFFUL);
    dwAcc = (dwAcc >> 16) + (dwAcc & 0x0000FFFFUL);

    dwAcc += (WORD16)(~wCheckSum);
    dwAcc  = (dwAcc >> 16) + (dwAcc & 0x0000FFFFUL);

    dwAcc += (WORD32)(HTONS(wProto));
    dwAcc += (WORD32)(HTONS(wProtoLen));

    dwAcc  = (dwAcc >> 16) + (dwAcc & 0x0000FFFFUL);
    dwAcc  = (dwAcc >> 16) + (dwAcc & 0x0000FFFFUL);

    return (WORD16)(~(dwAcc & 0xFFFFUL));
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

    if (NULL != m_pIpSecStack)
    {
        delete m_pIpSecStack;
        m_pMemInterface->Free((BYTE *)m_pIpSecStack);
    }

    if (NULL != m_pUdpStack)
    {
        delete m_pUdpStack;
        m_pMemInterface->Free((BYTE *)m_pUdpStack);
    }

    if (NULL != m_pSctpStack)
    {
        delete m_pSctpStack;
        m_pMemInterface->Free((BYTE *)m_pSctpStack);
    }

    if (NULL != m_pTcpStack)
    {
        delete m_pTcpStack;
        m_pMemInterface->Free((BYTE *)m_pTcpStack);
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
    m_cNdpTable.Initialize();

    BYTE *pArpMem   = m_pMemInterface->Malloc(sizeof(CArpStack));
    BYTE *pIPv4Mem  = m_pMemInterface->Malloc(sizeof(CIPv4Stack));
    BYTE *pIPv6Mem  = m_pMemInterface->Malloc(sizeof(CIPv6Stack));
    BYTE *pIpSecMem = m_pMemInterface->Malloc(sizeof(CIpSecStack));
    BYTE *pUdpMem   = m_pMemInterface->Malloc(sizeof(CUdpStack));
    BYTE *pSctpMem  = m_pMemInterface->Malloc(sizeof(CSctpStack));
    BYTE *pTcpMem   = m_pMemInterface->Malloc(sizeof(CTcpStack));

    if ( (NULL == pArpMem)
      || (NULL == pIPv4Mem)
      || (NULL == pIPv6Mem)
      || (NULL == pIpSecMem)
      || (NULL == pUdpMem)
      || (NULL == pSctpMem)
      || (NULL == pTcpMem))
    {
        assert(0);
    }

    m_pArpStack   = new (pArpMem) CArpStack();
    m_pIPv4Stack  = new (pIPv4Mem) CIPv4Stack();
    m_pIPv6Stack  = new (pIPv6Mem) CIPv6Stack();
    m_pIpSecStack = new (pIpSecMem) CIpSecStack();
    m_pUdpStack   = new (pUdpMem)  CUdpStack();
    m_pSctpStack  = new (pSctpMem) CSctpStack();
    m_pTcpStack   = new (pTcpMem)  CTcpStack();

    m_pArpStack->Initialize(pMemInterface);
    m_pIpSecStack->Initialize(pMemInterface);
    m_pUdpStack->Initialize(pMemInterface);
    m_pSctpStack->Initialize(pMemInterface);
    m_pTcpStack->Initialize(pMemInterface);

    CIPv4Stack *pIpv4Stack = (CIPv4Stack *)m_pIPv4Stack;
    CIPv6Stack *pIpv6Stack = (CIPv6Stack *)m_pIPv6Stack;

    pIpv4Stack->Initialize(pMemInterface, m_pIpSecStack, m_pUdpStack, m_pSctpStack, m_pTcpStack);
    pIpv6Stack->Initialize(pMemInterface, m_pIpSecStack, m_pUdpStack, m_pSctpStack, m_pTcpStack);

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


VOID CNetIntfHandler::Dump()
{
    TRACE_STACK("CNetIntfHandler::Dump()");

    m_cIPTable.Dump();
    m_cVlanTable.Dump();
    m_cArpTable.Dump();
}


