

#ifndef _DPDK_NET_TCP_H_
#define _DPDK_NET_TCP_H_


#include "dpdk_net_interface.h"


class CTcpStack : public CNetStack
{
public :
    CTcpStack ();
    virtual ~CTcpStack();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

    /* 接收报文处理; pHead : TCP头 */
    virtual WORD32 RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead);
};


#endif


