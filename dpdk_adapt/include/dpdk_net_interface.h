

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

    /* ���ձ��Ĵ��� */
    virtual WORD32 RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead) = 0;

    /* ��װ��̫������ͷ; pPkt : ��̫��ͷ��ַ */
    WORD16 EncodeEthPacket(BYTE   *pPkt,
                           BYTE   *pSrcMacAddr,
                           BYTE   *pDstMacAddr,
                           WORD32  dwDeviceID,
                           WORD32  dwVlanID,
                           WORD16  wEthType);

    /* ��װIPv4����ͷ
     * pPkt      : IPv4ͷ��ַ
     * wTotalLen : ��IPͷ��IP���ĳ���
     * wProto    : �ϲ�Э��(ICMP/TCP/UDP/SCTP)
     */
    WORD16 EncodeIpv4Packet(BYTE   *pPkt,
                            WORD16  wTotalLen,
                            WORD16  wProto,
                            WORD32  dwSrcIP,
                            WORD32  dwDstIP);

    /* ��װIPv6����ͷ
     * pPkt      : IPv6ͷ��ַ
     * wTotalLen : ����IPv6ͷ���غɳ���(����IPv6��չͷ��, �������չͷ��)
     * wProto    : �ϲ�Э��(ICMP/TCP/UDP/SCTP)
     */
    WORD16 EncodeIpv6Packet(BYTE     *pPkt,
                            WORD16    wTotalLen,
                            WORD16    wProto,
                            T_IPAddr &rtSrcIP,
                            T_IPAddr &rtDstIP);

    /* ����ICMPУ��� */
    WORD16 CalcIcmpCheckSum(WORD16 *pwAddr, WORD32 dwCount);

    /* ����ICMPv6У��� */
    WORD16 CalcIcmpv6CheckSum(WORD16   *pwAddr,
                              WORD32    dwCount,
                              T_IPAddr &rtSrcAddr,
                              T_IPAddr &rtDstAddr);

protected :
    CCentralMemPool  *m_pMemInterface;
    CVlanTable       *m_pVlanTable;
};


/* ��������ӿھ�� */
class CNetIntfHandler : public CNetStack
{
public :
    CNetIntfHandler ();
    virtual ~CNetIntfHandler();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

    WORD32 InitArpTable();

    /* ������̫�����Ĵ���; pHead : ��̫��ͷ */
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
    std::atomic<WORD16>  m_wIPIdentity;  /* IP����ͷ�е�ID��ʶ */

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


