

#ifndef _DPDK_NET_ARP_H_
#define _DPDK_NET_ARP_H_


#include "dpdk_net_interface.h"
#include "dpdk_arp_table.h"


class CArpStack : public CNetStack
{
public :
    CArpStack ();
    virtual ~CArpStack();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

    /* wProto : �Ͳ�Э��ջ����(0 : EtherNet) */
    virtual WORD32 RecvEthPacket(CAppInterface *pApp,
                                 WORD16         wProto,
                                 WORD32         dwDevID,
                                 WORD32         dwPortID,
                                 WORD32         dwQueueID,
                                 T_MBuf        *pMBuf,
                                 T_EthHead     *ptEthHead);

    /* ������Ŀ��IP��Arp����, ���ڲ�ѯ�Զ�MAC��ַ */
    WORD32 SendArpRequest(CDevQueue *pQueue, WORD32 dwDstIP);

protected :
    WORD32 ProcArpRequest(CAppInterface *pApp,
                          WORD32         dwDevID,
                          WORD32         dwPortID,
                          WORD32         dwQueueID,
                          T_ArpHead     *pArpHead);

    WORD32 ProcArpReply(T_ArpHead *pArpHead, WORD32 dwDevID);

    /* dwDevID : ָʾ����Ӧ�豸�յ�ARP����, ��¼���豸��Ϣ */
    WORD32 UpdateArpTable(WORD32 dwDevID, WORD32 dwIP, BYTE *pMacAddr);

    T_MBuf * EncodeArpReply(BYTE               *pSrcMacAddr,
                            BYTE               *pDstMacAddr,
                            WORD32              dwSrcIP,
                            WORD32              dwDstIP,
                            struct rte_mempool *pMBufPool);

    T_MBuf * EncodeArpRequest(BYTE               *pSrcMacAddr,
                              WORD32              dwSrcIP,
                              WORD32              dwDstIP,
                              struct rte_mempool *pMBufPool);

protected :
    CSpinLock     m_cLock;      /* ��Ե����߳�(����CArpStack���߳�) */
    CIPTable     *m_pIPTable;
    CArpTable    *m_pArpTable;
};


#endif


