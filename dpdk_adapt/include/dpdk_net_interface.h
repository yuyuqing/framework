

#ifndef _DPDK_NET_INTERFACE_H_
#define _DPDK_NET_INTERFACE_H_


#include "dpdk_device.h"

#include "base_app_interface.h"


class CNetStack : public CCBObject, public CBaseData
{
public :
    static WORD32 RecvPacket(VOID   *pArg,
                             WORD32  dwDevID,
                             WORD32  dwPortID,
                             WORD32  dwQueueID,
                             T_MBuf *pMBuf);

public :
    CNetStack ();
    virtual ~CNetStack();

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
    CCentralMemPool  *m_pMemInterface;
};


/* 本地网络接口句柄 */
class CNetIntfHandler : public CNetStack
{
public :
    CNetIntfHandler ();
    virtual ~CNetIntfHandler();

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
    CNetStack  *m_pArpStack;
    CNetStack  *m_pVlanStack;
    CNetStack  *m_pIPv4Stack;
    CNetStack  *m_pIPv6Stack;
};


extern CNetIntfHandler *g_pNetIntfHandler;


#endif


