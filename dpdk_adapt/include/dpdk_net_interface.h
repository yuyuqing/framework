

#ifndef _DPDK_NET_INTERFACE_H_
#define _DPDK_NET_INTERFACE_H_


#include "dpdk_device.h"
#include "dpdk_net_vlan_table.h"
#include "dpdk_net_arp_table.h"

#include "base_app_interface.h"


class CNetStack : public CCBObject, public CBaseData
{
public :
    enum { IP_HEAD_TTL = 64 };

    static WORD32 RecvPacket(VOID   *pArg,
                             WORD32  dwDevID,
                             WORD32  dwPortID,
                             WORD32  dwQueueID,
                             T_MBuf *pMBuf);

    static WORD32 ParseEthernet(T_EthHead *ptEthHead, T_OffloadInfo &rtInfo);

    static WORD32 ParseIpv4(T_Ipv4Head *ptIpv4Head, T_OffloadInfo &rtInfo);
    static WORD32 ParseIpv6(T_Ipv6Head *ptIpv6Head, T_OffloadInfo &rtInfo);

public :
    CNetStack ();
    virtual ~CNetStack();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

    /* 接收报文处理 */
    virtual WORD32 RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead) = 0;

    /* 封装以太网报文头; pPkt : 以太网头地址 */
    WORD16 EncodeEthPacket(BYTE   *pPkt,
                           BYTE   *pSrcMacAddr,
                           BYTE   *pDstMacAddr,
                           WORD32  dwDeviceID,
                           WORD32  dwVlanID,
                           WORD16  wEthType);

    /* 封装IPv4报文头
     * pPkt      : IPv4头地址
     * wTotalLen : 含IP头的IP报文长度
     * wProto    : 上层协议(ICMP/TCP/UDP/SCTP)
     */
    WORD16 EncodeIpv4Packet(BYTE   *pPkt,
                            WORD16  wTotalLen,
                            WORD16  wProto,
                            WORD32  dwSrcIP,
                            WORD32  dwDstIP);

    /* 封装IPv6报文头
     * pPkt      : IPv6头地址
     * wTotalLen : 不含IPv6头的载荷长度(如有IPv6扩展头部, 则包含扩展头部)
     * wProto    : 上层协议(ICMP/TCP/UDP/SCTP)
     */
    WORD16 EncodeIpv6Packet(BYTE     *pPkt,
                            WORD16    wTotalLen,
                            WORD16    wProto,
                            T_IPAddr &rtSrcIP,
                            T_IPAddr &rtDstIP);

    /* 计算ICMP校验和 */
    WORD16 CalcIcmpCheckSum(WORD16 *pwAddr, WORD32 dwCount);

    /* 计算ICMPv6校验和 */
    WORD16 CalcIcmpv6CheckSum(WORD16   *pwAddr,
                              WORD32    dwCount,
                              T_IPAddr &rtSrcAddr,
                              T_IPAddr &rtDstAddr);

protected :
    CCentralMemPool  *m_pMemInterface;
    CVlanTable       *m_pVlanTable;
};


/* 本地网络接口句柄 */
class CNetIntfHandler : public CNetStack
{
public :
    CNetIntfHandler ();
    virtual ~CNetIntfHandler();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

    WORD32 InitArpTable();

    /* 接收以太网报文处理; pHead : 以太网头 */
    virtual WORD32 RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead);

    CIPTable   & GetIPTable();
    CVlanTable & GetVlanTable();
    CArpTable  & GetArpTable();

    CNetStack * GetIPv6Stack();

    WORD16 FetchAddIPIdentity();

    VOID Dump();

protected :
    std::atomic<WORD16>  m_wIPIdentity;  /* IP报文头中的ID标识 */

    CIPTable             m_cIPTable;
    CVlanTable           m_cVlanTable;
    CArpTable            m_cArpTable;

    CNetStack           *m_pArpStack;
    CNetStack           *m_pIPv4Stack;
    CNetStack           *m_pIPv6Stack;
};


inline CIPTable & CNetIntfHandler::GetIPTable()
{
    return m_cIPTable;
}


inline CVlanTable & CNetIntfHandler::GetVlanTable()
{
    return m_cVlanTable;
}


inline CArpTable & CNetIntfHandler::GetArpTable()
{
    return m_cArpTable;
}


inline CNetStack * CNetIntfHandler::GetIPv6Stack()
{
    return m_pIPv6Stack;
}


inline WORD16 CNetIntfHandler::FetchAddIPIdentity()
{
    return m_wIPIdentity.fetch_add(1, std::memory_order_relaxed);
}


extern CNetIntfHandler *g_pNetIntfHandler;


#endif


