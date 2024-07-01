

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

    /* wProto : 低层协议栈类型(0 : EtherNet) */
    virtual WORD32 RecvEthPacket(CAppInterface *pApp,
                                 WORD16         wProto,
                                 WORD32         dwDevID,
                                 WORD32         dwPortID,
                                 WORD32         dwQueueID,
                                 T_MBuf        *pMBuf,
                                 T_EthHead     *ptEthHead);

    /* 主动向目的IP发Arp请求, 用于查询对端MAC地址 */
    WORD32 SendArpRequest(CDevQueue *pQueue, WORD32 dwDstIP);

protected :
    WORD32 ProcArpRequest(CAppInterface *pApp,
                          WORD32         dwDevID,
                          WORD32         dwPortID,
                          WORD32         dwQueueID,
                          T_ArpHead     *pArpHead);

    WORD32 ProcArpReply(T_ArpHead *pArpHead, WORD32 dwDevID);

    /* dwDevID : 指示从相应设备收到ARP报文, 记录该设备信息 */
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
    CSpinLock     m_cLock;      /* 针对单个线程(持有CArpStack的线程) */
    CIPTable     *m_pIPTable;
    CArpTable    *m_pArpTable;
};


#endif


