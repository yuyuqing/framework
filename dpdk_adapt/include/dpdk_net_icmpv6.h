

#ifndef _DPDK_NET_ICMPV6_H_
#define _DPDK_NET_ICMPV6_H_


#include "dpdk_net_interface.h"


class CIcmpV6Stack : public CNetStack
{
public :
    CIcmpV6Stack ();
    virtual ~CIcmpV6Stack();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

    /* 接收报文处理 */
    virtual WORD32 RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead);
};


#endif


