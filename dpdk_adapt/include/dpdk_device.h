

#ifndef _DPDK_DEVICE_H_
#define _DPDK_DEVICE_H_


#include <rte_ethdev.h>
#include <rte_eal.h>

#include "pub_typedef.h"

#include "base_call_back.h"
#include "base_factory_tpl.h"
#include "base_config_file.h"
#include "base_mem_pool.h"


#define MBUF_BURST_NUM   ((WORD32)(32))


typedef struct rte_mbuf  T_MBuf;


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


class CBaseDevice : public CCBObject
{
public :
    CBaseDevice (E_DeviceType eType, const T_DeviceParam &rtParam);
    virtual ~CBaseDevice();

    virtual WORD32 Initialize();

    WORD32 GetDeviceID();
    WORD16 GetPortID();

    E_DeviceType GetType();

    virtual VOID Dump();

protected :
    CCentralMemPool    &m_rCentralMemPool;

    WORD32              m_dwDeviceID;
    WORD16              m_wPortID;
    BYTE                m_ucQueueNum;
    BYTE                m_ucDevType;
};


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


