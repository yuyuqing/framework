

#ifndef _DPDK_MGR_H_
#define _DPDK_MGR_H_


#include "dpdk_device.h"
#include "dpdk_net_interface.h"


class CDpdkMgr : public CSingleton<CDpdkMgr>, public CBaseData
{
public :
    CDpdkMgr();
    virtual ~CDpdkMgr();

    WORD32 Initialize(CCentralMemPool *pMemInterface,
                      const CHAR      *pArg0,
                      T_DpdkJsonCfg   &rtCfg);

    T_DeviceInfo * FindDevInfo(WORD32 dwDeviceID);
    CBaseDevice  * FindDevice(WORD32 dwDeviceID);
    CBaseDevice  * FindDevice(E_DeviceType eType, WORD16 wPortID);

    WORD32 GetDeviceNum();

    /* 获取指定类型的所有设备实例 */
    WORD32 GetDevices(E_DeviceType eType, CBaseDevice **pDevices);

    VOID Dump();

protected :
    WORD32 InitDpdkEal(T_DpdkJsonCfg &rtCfg);

    WORD32 InitDevice(T_DpdkJsonCfg &rtCfg);

    T_DeviceInfo * CreateInfo(T_DpdkDevJsonCfg &rtDevCfg,
                              T_ProductDefInfo *ptDefInfo);

protected :
    CCentralMemPool    *m_pMemInterface;
    WORD32              m_dwDevNum;
    T_DeviceInfo        m_atDevInfo[MAX_DEV_PORT_NUM];

    CNetIntfHandler     m_cNetHandler;
};


inline T_DeviceInfo * CDpdkMgr::FindDevInfo(WORD32 dwDeviceID)
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwDevNum; dwIndex++)
    {
        if (dwDeviceID == m_atDevInfo[dwIndex].dwDeviceID)
        {
            return &(m_atDevInfo[dwIndex]);
        }
    }

    return NULL;
}


inline CBaseDevice * CDpdkMgr::FindDevice(WORD32 dwDeviceID)
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwDevNum; dwIndex++)
    {
        if (dwDeviceID == m_atDevInfo[dwIndex].dwDeviceID)
        {
            return m_atDevInfo[dwIndex].pDevice;
        }
    }

    return NULL;
}


inline CBaseDevice * CDpdkMgr::FindDevice(E_DeviceType eType, WORD16 wPortID)
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwDevNum; dwIndex++)
    {
        if ( (wPortID == m_atDevInfo[dwIndex].dwPortID)
          && (eType == m_atDevInfo[dwIndex].pDevice->GetType()))
        {
            return m_atDevInfo[dwIndex].pDevice;
        }
    }

    return NULL;
}


inline WORD32 CDpdkMgr::GetDeviceNum()
{
    return m_dwDevNum;
}


/* 获取指定类型的所有设备实例 */
inline WORD32 CDpdkMgr::GetDevices(E_DeviceType eType, CBaseDevice **pDevices)
{
    WORD32 dwDevNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < m_dwDevNum; dwIndex++)
    {
        if (eType == m_atDevInfo[dwIndex].pDevice->GetType())
        {
            pDevices[dwDevNum++] = m_atDevInfo[dwIndex].pDevice;
        }
    }

    return dwDevNum;
}


extern CDpdkMgr *g_pDpdkMgr;


#endif


