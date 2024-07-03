

#ifndef _DPDK_NET_INTERFACE_H_
#define _DPDK_NET_INTERFACE_H_


#include "dpdk_device.h"
#include "dpdk_vlan_table.h"

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

    /* ����̫�����Ĵ���ӿ�; wProto : �Ͳ�Э��ջ����(0 : EtherNet) */
    virtual WORD32 RecvEthPacket(CAppInterface *pApp,
                                 WORD16         wProto,
                                 WORD32         dwDevID,
                                 WORD32         dwPortID,
                                 WORD32         dwQueueID,
                                 T_MBuf        *pMBuf,
                                 T_EthHead     *ptEthHead);

    /* ��VLAN���Ĵ���ӿ�; ptHead : ȥ��VLAN��̫����ͷ�ľ���ͷָ�� */
    virtual WORD32 RecvVlanPacket(CAppInterface *pApp,
                                  CVlanInst     *pVlanInst,
                                  WORD32         dwDevID,
                                  WORD32         dwPortID,
                                  WORD32         dwQueueID,
                                  T_MBuf        *pMBuf,
                                  CHAR          *ptHead);

protected :
    CCentralMemPool  *m_pMemInterface;
};


/* ��������ӿھ�� */
class CNetIntfHandler : public CNetStack
{
public :
    CNetIntfHandler ();
    virtual ~CNetIntfHandler();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

    /* wProto : �Ͳ�Э��ջ����(0 : EtherNet) */
    virtual WORD32 RecvEthPacket(CAppInterface *pApp,
                                 WORD16         wProto,
                                 WORD32         dwDevID,
                                 WORD32         dwPortID,
                                 WORD32         dwQueueID,
                                 T_MBuf        *pMBuf,
                                 T_EthHead     *ptEthHead);

protected :
    CNetStack  *m_pArpStack;
    CNetStack  *m_pIPv4Stack;
    CNetStack  *m_pIPv6Stack;
    CNetStack  *m_pVlanStack;
};


extern CNetIntfHandler *g_pNetIntfHandler;


#endif


