

#ifndef _DPDK_DEVICE_BB_H_
#define _DPDK_DEVICE_BB_H_


#include "dpdk_device.h"
#include "dpdk_bb_traffic.h"


class CBBDevice : public CBaseDevice
{
public :
    static SWORD32 LsiEventCallBack(WORD16          wPortID,
                                    E_EthEventType  eType,
                                    VOID           *pParam,
                                    VOID           *pRetParam);

public :
    CBBDevice (const T_DeviceParam &rtParam);
    virtual ~CBBDevice();

    /* 调用派生类设置RxConf属性 */
    virtual WORD32 SetRxConf();

    /* 调用派生类设置TxConf属性 */
    virtual WORD32 SetTxConf();

    virtual WORD32 Initialize();

    T_TrafficInfo * FindTrafficInfo(WORD32 dwTrafficID);

protected :
    WORD32 InitTraffic(T_DpdkBBDevJsonCfg &rtCfg);

    T_TrafficInfo * CreateInfo(WORD32                      dwQueueID,
                               T_DpdkBBFapiTrafficJsonCfg &rtCfg,
                               T_ProductDefInfo           *ptDefInfo);

protected :
    WORD32           m_dwTrafficNum;
    T_TrafficInfo    m_atTrafficInfo[MAX_BB_TRAFFIC_NUM];
};


inline T_TrafficInfo * CBBDevice::FindTrafficInfo(WORD32 dwTrafficID)
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwTrafficNum; dwIndex++)
    {
        if (dwTrafficID == m_atTrafficInfo[dwIndex].dwTrafficID)
        {
            return &(m_atTrafficInfo[dwIndex]);
        }
    }

    return NULL;
}


#endif


