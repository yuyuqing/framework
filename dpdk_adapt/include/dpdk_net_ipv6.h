

#ifndef _DPDK_NET_IPV6_H_
#define _DPDK_NET_IPV6_H_


#include "dpdk_net_interface.h"


class CIPv6Stack : public CNetStack
{
public :
    CIPv6Stack ();
    virtual ~CIPv6Stack();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

protected :
    CNetStack  *m_pIcmpStack;
    CNetStack  *m_pUdpStack;
    CNetStack  *m_pSctpStack;
    CNetStack  *m_pTcpStack;
};


#endif


