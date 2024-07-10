

#ifndef _DPDK_DEVICE_H_
#define _DPDK_DEVICE_H_


#include "dpdk_common.h"


typedef struct tagT_DeviceParam
{
    WORD32           dwDeviceID;             /* 设备ID */
    WORD32           dwPortID;               /* DPDK PortID */
    WORD32           dwQueueNum;             /* DPDK队列数量 */
    WORD32           dwMBufNum;
    WORD32           dwMBufCacheSize;
    WORD32           dwMBufPrivSize;
    WORD32           dwMBufRoomSize;
    WORD32           dwRxDescNum;
    WORD32           dwTxDescNum;
    CCentralMemPool *pMemPool;
}T_DeviceParam;


class CBaseDevice;


class CDevQueue : public CBaseData
{
public :
    friend class CBaseDevice;

public :
    CDevQueue(CBaseDevice *pDevice);
    virtual ~CDevQueue();

    WORD32 Initialize(WORD16 wPortID,
                      WORD16 wQueueID,
                      WORD32 dwMBufNum,
                      WORD32 dwMBufCacheSize,
                      WORD32 dwMBufPrivSize,
                      WORD32 dwMBufRoomSize);

    /* 一次性处理设备队列中指定数量的报文 */
    WORD32 RecvPacket(WORD32 dwNum, VOID *pObj, PMBufCallBack pFunc);

    /* 发送单条报文 */
    WORD32 SendPacket(T_MBuf *pBuf);

    /* 发送多条报文 */
    WORD32 SendPacket(WORD32 dwNum, T_MBuf **pBufs);

    CBaseDevice * GetDevice();

    WORD32 GetDeviceID();

    struct rte_mempool * GetTxMemPool();
    struct rte_mempool * GetRxMemPool();

protected :
    CBaseDevice         *m_pDev;

    WORD32               m_dwDeviceID;
    WORD16               m_wPortID;
    WORD16               m_wQueueID;

    struct rte_mempool  *m_pTxMbufPool;
    struct rte_mempool  *m_pRxMbufPool;
};


/* 一次性处理设备队列中指定数量的报文 */
inline WORD32 CDevQueue::RecvPacket(WORD32 dwNum, VOID *pObj, PMBufCallBack pFunc)
{
    T_MBuf *apMBuf[dwNum] = {NULL, };

    WORD32 dwRecvNum = rte_eth_rx_burst(m_wPortID,
                                        m_wQueueID,
                                        apMBuf,
                                        dwNum);
    for (WORD32 dwIndex = 0; dwIndex < dwRecvNum; dwIndex++)
    {
        (*pFunc)(pObj, m_dwDeviceID, m_wPortID, m_wQueueID, apMBuf[dwIndex]);

        rte_pktmbuf_free(apMBuf[dwIndex]);  /* 释放内存 */
    }

    return dwRecvNum;
}


/* 发送单条报文 */
inline WORD32 CDevQueue::SendPacket(T_MBuf *pBuf)
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


/* 发送多条报文 */
inline WORD32 CDevQueue::SendPacket(WORD32 dwNum, T_MBuf **pBufs)
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


inline CBaseDevice * CDevQueue::GetDevice()
{
    return m_pDev;
}


inline WORD32 CDevQueue::GetDeviceID()
{
    return m_dwDeviceID;
}


inline struct rte_mempool * CDevQueue::GetTxMemPool()
{
    return m_pTxMbufPool;
}


inline struct rte_mempool * CDevQueue::GetRxMemPool()
{
    return m_pRxMbufPool;
}


class CBaseDevice : public CCBObject
{
public :
    /* 在派生类构造方法中设置m_tEthConf(以满足不同设备的不同配置需求) */
    CBaseDevice (E_DeviceType eType, const T_DeviceParam &rtParam);
    virtual ~CBaseDevice();

    /* 调用派生类设置RxConf属性 */
    virtual WORD32 SetRxConf() = 0;

    /* 调用派生类设置TxConf属性 */
    virtual WORD32 SetTxConf() = 0;

    virtual WORD32 Initialize() = 0;

    CDevQueue * GetQueue(WORD16 wQueueID);

    WORD32 GetDeviceID();
    WORD16 GetPortID();

    BYTE GetQueueNum();

    E_DeviceType GetType();

    virtual VOID Dump();

protected :
    WORD32 Initialize(rte_eth_dev_cb_fn pFunc);

    WORD32 DevStart();

    CDevQueue * CreateQueue(WORD16  wQueueID,
                            WORD32  dwMBufNum,
                            WORD32  dwMBufCacheSize,
                            WORD32  dwMBufPrivSize,
                            WORD32  dwMBufRoomSize,
                            SWORD32 iSocketID,
                            WORD16  wRxDescNum,
                            WORD16  wTxDescNum,
                            struct rte_eth_rxconf &rtRxConf,
                            struct rte_eth_txconf &rtTxConf);

protected :
    CCentralMemPool         &m_rCentralMemPool;

    WORD32                   m_dwDeviceID;
    WORD16                   m_wPortID;
    BYTE                     m_ucQueueNum;
    BYTE                     m_ucDevType;

    WORD32                   m_dwMBufNum;
    WORD32                   m_dwMBufCacheSize;
    WORD32                   m_dwMBufPrivSize;
    WORD32                   m_dwMBufRoomSize;

    WORD16                   m_wRxDescNum;
    WORD16                   m_wTxDescNum;

    CDevQueue               *m_apQueue[MAX_DEV_QUEUE_NUM];

    struct rte_eth_conf      m_tEthConf;
    struct rte_eth_dev_info  m_tDevInfo;
    struct rte_eth_rxconf    m_tRxConf;
    struct rte_eth_txconf    m_tTxConf;
};


inline CDevQueue * CBaseDevice::GetQueue(WORD16 wQueueID)
{
    if (unlikely(wQueueID >= m_ucQueueNum))
    {
        return NULL;
    }

    return m_apQueue[wQueueID];
}


inline WORD32 CBaseDevice::GetDeviceID()
{
    return m_dwDeviceID;
}


inline WORD16 CBaseDevice::GetPortID()
{
    return m_wPortID;
}


inline BYTE CBaseDevice::GetQueueNum()
{
    return m_ucQueueNum;
}


inline E_DeviceType CBaseDevice::GetType()
{
    return (E_DeviceType)m_ucDevType;
}


typedef struct tagT_DeviceInfo
{
    CHAR             aucName[DEV_NAME_LEN];
    WORD32           dwDeviceID;             /* 设备ID */
    WORD32           dwPortID;               /* DPDK PortID */
    WORD32           dwQueueNum;             /* DPDK队列数量 */
    WORD32           dwMBufNum;
    WORD32           dwMBufCacheSize;
    WORD32           dwMBufPrivSize;
    WORD32           dwMBufRoomSize;
    WORD32           dwRxDescNum;
    WORD32           dwTxDescNum;
    WORD32           dwMemSize;              /* 设备实例大小 */
    PCreateProduct   pCreateFunc;
    PDestroyProduct  pDestroyFunc;
    BYTE            *pMem;
    CBaseDevice     *pDevice;
}T_DeviceInfo;


class CFactoryDevice : public CFactoryTpl<CFactoryDevice>
{
public :
    CFactoryDevice () {}
    virtual ~CFactoryDevice() {}

    VOID Dump();
};


#define DEFINE_DEVICE(V)    \
    WORD32 __attribute__((used)) __dwDev_##V##_ = CFactoryDevice::DefineProduct<V, T_DeviceParam>(#V)


#endif


