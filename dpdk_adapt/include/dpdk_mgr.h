

#ifndef _DPDK_MGR_H_
#define _DPDK_MGR_H_


#include "dpdk_device.h"
#include "dpdk_vlan_table.h"


#define EAL_ARG_NUM             ((WORD32)(8))


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

    CIPTable   & GetIPTable();
    CVlanTable & GetVlanTable();

    VOID Dump();

protected :
    WORD32 InitDpdkEal(const CHAR *pArg0,
                       CHAR       *pArgCore,
                       CHAR       *pArgMem,
                       CHAR       *pArgChannel,
                       CHAR       *pArgFilePrefix,
                       CHAR       *pArgProcType,
                       CHAR       *pArgIovaMode,
                       CHAR       *pArgVirtNet);

    WORD32 InitDevice(T_DpdkJsonCfg &rtCfg);

    T_DeviceInfo * CreateInfo(WORD32            dwDeviceID,
                              WORD32            dwPortID,
                              WORD32            dwQueueNum,
                              T_ProductDefInfo *ptDefInfo);

protected :
    CCentralMemPool    *m_pMemInterface;
    WORD32              m_dwDevNum;
    T_DeviceInfo        m_atDevInfo[MAX_DEV_PORT_NUM];

    CIPTable            m_cIPTable;
    CVlanTable          m_cVlanTable;
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


inline CIPTable & CDpdkMgr::GetIPTable()
{
    return m_cIPTable;
}


inline CVlanTable & CDpdkMgr::GetVlanTable()
{
    return m_cVlanTable;
}


extern CDpdkMgr *g_pDpdkMgr;


#endif


