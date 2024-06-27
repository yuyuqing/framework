

#ifndef _DPDK_DEVICE_H_
#define _DPDK_DEVICE_H_


#include <rte_version.h>
#include <rte_ethdev.h>
#include <rte_eal.h>

#include "pub_typedef.h"

#include "base_call_back.h"
#include "base_factory_tpl.h"
#include "base_config_file.h"
#include "base_mem_pool.h"


#define MAX_DEV_QUEUE_NUM        ((WORD32)(4))
#define MBUF_BURST_NUM           ((WORD32)(32))
#define RX_DESC_DEFAULT          ((WORD32)(1024))
#define TX_DESC_DEFAULT          ((WORD32)(1024))
#define MBUF_NAME_LEN            ((WORD32)(64))
#define MBUF_NUM_PER_PORT        ((WORD32)(16384))
#define MBUF_CACHE_SIZE          ((WORD32)(128))
#define MBUF_PRIV_SIZE           ((WORD32)(0))
#define MBUF_DATA_ROOM_SIZE      ((WORD32)(2048))


typedef struct rte_mbuf          T_MBuf;
typedef enum rte_eth_event_type  E_EthEventType;


/* 报文回调处理函数 */
using PMBufCallBack = WORD32 (*)(VOID   *pArg,
                                 WORD32  dwDevID,
                                 WORD32  dwPortID,
                                 WORD32  dwQueueID,
                                 T_MBuf *pMBuf);


typedef enum tagE_DeviceType
{
    E_DEV_INVALID = 0,

    E_ETH_DEVICE,       /* 以太网设备 */
    E_BB_DEVICE,        /* 基带设备 */
}E_DeviceType;


typedef struct tagT_DeviceParam
{
    WORD32           dwDeviceID;             /* 设备ID */
    WORD32           dwPortID;               /* DPDK PortID */
    WORD32           dwQueueNum;             /* DPDK队列数量 */
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

    WORD32 Initialize(WORD16 wPortID, WORD16 wQueueID);

    /* 一次性处理设备队列中指定数量的报文 */
    WORD32 RecvPacket(WORD32 dwNum, VOID *pObj, PMBufCallBack pFunc);

    /* 发送单条报文 */
    WORD32 SendPacket(T_MBuf *pBuf);

    /* 发送多条报文 */
    WORD32 SendPacket(WORD32 dwNum, T_MBuf **pBufs);

protected :
    CBaseDevice         *m_pDev;

    WORD32               m_dwDeviceID;
    WORD16               m_wPortID;
    WORD16               m_wQueueID;

    struct rte_mempool  *m_pMbufPool;
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

    E_DeviceType GetType();

    virtual VOID Dump();

protected :
    WORD32 Initialize(rte_eth_dev_cb_fn pFunc);

    CDevQueue * CreateQueue(WORD16  wQueueID,
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


