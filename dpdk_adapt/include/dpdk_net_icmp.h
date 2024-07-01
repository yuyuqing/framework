

#ifndef _DPDK_NET_ICMP_H_
#define _DPDK_NET_ICMP_H_


#include "dpdk_net_interface.h"


class CIcmpStack : public CNetStack
{
public :
    CIcmpStack ();
    virtual ~CIcmpStack();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);
};


#endif


