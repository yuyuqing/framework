

#ifndef _DPDK_NET_INTERFACE_H_
#define _DPDK_NET_INTERFACE_H_


#include "dpdk_device.h"
#include "dpdk_vlan_table.h"

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

    /* 接收以太网报文处理; pHead : 以太网头 */
    virtual WORD32 RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead);

    WORD16 FetchAddIPIdentity();

protected :
    std::atomic<WORD16>  m_wIPIdentity;  /* IP报文头中的ID标识 */

    CNetStack           *m_pArpStack;
    CNetStack           *m_pIPv4Stack;
    CNetStack           *m_pIPv6Stack;
};


inline WORD16 CNetIntfHandler::FetchAddIPIdentity()
{
    return m_wIPIdentity.fetch_add(1, std::memory_order_relaxed);
}


extern CNetIntfHandler *g_pNetIntfHandler;


#endif


