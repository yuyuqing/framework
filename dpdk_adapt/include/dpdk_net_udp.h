

#ifndef _DPDK_NET_UDP_H_
#define _DPDK_NET_UDP_H_


#include "dpdk_net_interface.h"


class CUdpStack : public CNetStack
{
public :
    CUdpStack ();
    virtual ~CUdpStack();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

    /* ���ձ��Ĵ���; pHead : UDPͷ */
    virtual WORD32 RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead);
};


#endif


