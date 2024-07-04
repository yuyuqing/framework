

#ifndef _DPDK_NET_SCTP_H_
#define _DPDK_NET_SCTP_H_


#include "dpdk_net_interface.h"


class CSctpStack : public CNetStack
{
public :
    CSctpStack ();
    virtual ~CSctpStack();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

    /* 接收报文处理; pHead : SCTP头 */
    virtual WORD32 RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead);
};


#endif


