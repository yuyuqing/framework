

#ifndef _DPDK_NET_IPV4_H_
#define _DPDK_NET_IPV4_H_


#include "dpdk_net_interface.h"


class CIPv4Stack : public CNetStack
{
public :
    CIPv4Stack ();
    virtual ~CIPv4Stack();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

    /* wProto : 低层协议栈类型(0 : EtherNet) */
    virtual WORD32 RecvEthPacket(CAppInterface *pApp,
                                 WORD16         wProto,
                                 WORD32         dwDevID,
                                 WORD32         dwPortID,
                                 WORD32         dwQueueID,
                                 T_MBuf        *pMBuf,
                                 T_EthHead     *ptEthHead);

protected :
    CNetStack  *m_pIcmpStack;
    CNetStack  *m_pUdpStack;
    CNetStack  *m_pSctpStack;
    CNetStack  *m_pTcpStack;
};


#endif


