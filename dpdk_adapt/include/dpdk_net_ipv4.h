

#ifndef _DPDK_NET_IPV4_H_
#define _DPDK_NET_IPV4_H_


#include "dpdk_net_interface.h"


class CIPv4Stack : public CNetStack
{
public :
    CIPv4Stack ();
    virtual ~CIPv4Stack();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

    /* 接收报文处理; pHead : IPv4头 */
    virtual WORD32 RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead);

protected :
    CNetStack  *m_pIcmpStack;
    CNetStack  *m_pUdpStack;
    CNetStack  *m_pSctpStack;
    CNetStack  *m_pTcpStack;
};


#endif


