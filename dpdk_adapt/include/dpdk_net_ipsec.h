

#ifndef _DPDK_NET_IPSEC_H_
#define _DPDK_NET_IPSEC_H_


#include "dpdk_net_interface.h"


class CIpSecStack : public CNetStack
{
public :
    CIpSecStack ();
    virtual ~CIpSecStack();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

    /* 接收报文处理 */
    virtual WORD32 RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead);
};


#endif


