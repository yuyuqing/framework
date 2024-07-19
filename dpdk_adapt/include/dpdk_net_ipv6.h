

#ifndef _DPDK_NET_IPV6_H_
#define _DPDK_NET_IPV6_H_


#include "dpdk_net_interface.h"


class CIPv6Stack : public CNetStack
{
public :
    CIPv6Stack ();
    virtual ~CIPv6Stack();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

    /* 接收报文处理; pHead : IPv6头 */
    virtual WORD32 RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead);

    CNetStack * GetIcmpStack();

protected :
    CNetStack  *m_pIcmpStack;
    CNetStack  *m_pIpSecStack;
    CNetStack  *m_pUdpStack;
    CNetStack  *m_pSctpStack;
    CNetStack  *m_pTcpStack;
};


inline CNetStack * CIPv6Stack::GetIcmpStack()
{
    return m_pIcmpStack;
}


#endif


