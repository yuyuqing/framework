

#ifndef _DPDK_NET_SCTP_H_
#define _DPDK_NET_SCTP_H_


#include "dpdk_net_interface.h"


class CSctpStack : public CNetStack
{
public :
    CSctpStack ();
    virtual ~CSctpStack();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);
};


#endif


