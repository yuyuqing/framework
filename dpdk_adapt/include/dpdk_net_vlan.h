

#ifndef _DPDK_NET_VLAN_H_
#define _DPDK_NET_VLAN_H_


#include "dpdk_net_interface.h"


class CVlanStack : public CNetStack
{
public :
    CVlanStack ();
    virtual ~CVlanStack();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

    virtual WORD32 RecvEthPacket(CAppInterface *pApp,
                                 WORD16         wProto,
                                 WORD32         dwDevID,
                                 WORD32         dwPortID,
                                 WORD32         dwQueueID,
                                 T_MBuf        *pMBuf,
                                 T_EthHead     *ptEthHead);

protected :
    CNetStack  *m_pIPv4Stack;
    CNetStack  *m_pIPv6Stack;
};


#endif


