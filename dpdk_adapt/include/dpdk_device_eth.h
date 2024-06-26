

#ifndef _DPDK_DEVICE_ETH_H_
#define _DPDK_DEVICE_ETH_H_


#include "dpdk_device.h"


typedef enum tagE_EthLinkType
{
    E_ETH_LINK_ACCESS = 0,    /* ������������Access�˿�(�շ����Ĳ���VLAN��ǩ) */
    E_ETH_LINK_TRUNK,         /* ������������Trunk�˿�(�շ����ı����VLAN��ǩ) */
    E_ETH_LINK_HYBRID,        /* ������������Hybrid�˿�(�շ����Ŀ���Я��VLAN��ǩ, Ҳ���Բ�Я��VLAN��ǩ) */
}E_EthLinkType;


#define MAX_ETH_QUEUE_NUM        ((WORD32)(4))
#define MBUF_NAME_LEN            ((WORD32)(64))
#define MBUF_NUM_PER_PORT        ((WORD32)(16384))
#define MBUF_CACHE_SIZE          ((WORD32)(128))
#define MBUF_PRIV_SIZE           ((WORD32)(0))
#define MBUF_DATA_ROOM_SIZE      ((WORD32)(2048))


class CEthDevice;


class CEthDevQueue : public CBaseData
{
public :
    friend class CEthDevice;

public :
    CEthDevQueue(CEthDevice &rEthDev);
    virtual ~CEthDevQueue();

    WORD32 Initialize(WORD16 wPortID, WORD16 wQueueID);

    /* һ���Դ��������������еı��� */
    WORD32 RecvPacket(VOID *pObj, PMBufCallBack pFunc);

    /* һ���Դ�������������ָ�������ı��� */
    WORD32 RecvPacket(WORD32 dwNum, VOID *pObj, PMBufCallBack pFunc);

    /* ���͵������� */
    WORD32 SendPacket(T_MBuf *pBuf);

    /* ���Ͷ������� */
    WORD32 SendPacket(WORD32 dwNum, T_MBuf **pBufs);

protected :
    CEthDevice          &m_rEthDev;

    WORD32               m_dwDeviceID;
    WORD16               m_wPortID;
    WORD16               m_wQueueID;

    struct rte_mempool  *m_pMbufPool;
};


/* һ���Դ��������������еı��� */
inline WORD32 CEthDevQueue::RecvPacket(VOID *pObj, PMBufCallBack pFunc)
{
    T_MBuf *apMBuf[MBUF_BURST_NUM] = {NULL, };

    WORD32  dwRecvNum  = 0;
    WORD32  dwTotalNum = 0;

    do
    {
        dwRecvNum = rte_eth_rx_burst(m_wPortID,
                                     m_wQueueID,
                                     apMBuf,
                                     MBUF_BURST_NUM);
        for (WORD32 dwIndex = 0; dwIndex < dwRecvNum; dwIndex++)
        {
            (*pFunc)(pObj, m_dwDeviceID, m_wPortID, m_wQueueID, apMBuf[dwIndex]);

            rte_pktmbuf_free(apMBuf[dwIndex]);  /* �ͷ��ڴ� */
        }

        dwTotalNum += dwRecvNum;
    } while(dwRecvNum == MBUF_BURST_NUM);

    return dwTotalNum;
}


/* һ���Դ�������������ָ�������ı��� */
inline WORD32 CEthDevQueue::RecvPacket(WORD32 dwNum, VOID *pObj, PMBufCallBack pFunc)
{
    T_MBuf *apMBuf[dwNum] = {NULL, };

    WORD32 dwRecvNum = rte_eth_rx_burst(m_wPortID,
                                        m_wQueueID,
                                        apMBuf,
                                        dwNum);
    for (WORD32 dwIndex = 0; dwIndex < dwRecvNum; dwIndex++)
    {
        (*pFunc)(pObj, m_dwDeviceID, m_wPortID, m_wQueueID, apMBuf[dwIndex]);

        rte_pktmbuf_free(apMBuf[dwIndex]);  /* �ͷ��ڴ� */
    }

    return dwRecvNum;
}


/* ���͵������� */
inline WORD32 CEthDevQueue::SendPacket(T_MBuf *pBuf)
{
    WORD32 dwSendNum = rte_eth_tx_burst(m_wPortID,
                                        m_wQueueID,
                                        &pBuf,
                                        1);
    if (unlikely(dwSendNum < 1))
    {
        rte_pktmbuf_free(pBuf);
    }

    return dwSendNum;
}


/* ���Ͷ������� */
inline WORD32 CEthDevQueue::SendPacket(WORD32 dwNum, T_MBuf **pBufs)
{
    WORD32 dwSendNum = rte_eth_tx_burst(m_wPortID,
                                        m_wQueueID,
                                        pBufs,
                                        dwNum);
    if (unlikely(dwSendNum < dwNum))
    {
        do
        {
            rte_pktmbuf_free(pBufs[dwSendNum]);
        } while(++dwSendNum < dwNum);
    }

    return dwSendNum;
}


class CEthDevice : public CBaseDevice
{
public :
    enum { E_MAX_MTU         = 1500 };
    enum { E_RX_DESC_DEFAULT = 1024 };
    enum { E_TX_DESC_DEFAULT = 1024 };

public :
    CEthDevice (const T_DeviceParam &rtParam);
    virtual ~CEthDevice();

    WORD32 Initialize();

protected :
    CEthDevQueue * CreateEthQueue(WORD16 wQueueID, SWORD32 iSocketID);

protected :
    BYTE                       m_ucLinkType;
    BYTE                       m_ucVlanNum;
    BYTE                       m_ucIPNum;

    WORD16                     m_wRxDescNum;
    WORD16                     m_wTxDescNum;

    struct rte_eth_conf        m_tEthConf;
    struct rte_eth_dev_info    m_tDevInfo;
    struct rte_ether_addr      m_tEthAddr;
    struct rte_eth_rxconf      m_tRxConf;
    struct rte_eth_txconf      m_tTxConf;

    CEthDevQueue              *m_apEthQueue[MAX_ETH_QUEUE_NUM];
};


#endif


