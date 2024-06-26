

#include "dpdk_device.h"
#include "base_log.h"


CBaseDevice::CBaseDevice (E_DeviceType eType, const T_DeviceParam &rtParam)
    : m_rCentralMemPool(*(rtParam.pMemPool))
{
    m_dwDeviceID = rtParam.dwDeviceID;
    m_wPortID    = (WORD16)(rtParam.dwPortID);
    m_ucQueueNum = (BYTE)(rtParam.dwQueueNum);
    m_ucDevType  = (BYTE)(eType);
}


CBaseDevice::~CBaseDevice()
{
    m_dwDeviceID = INVALID_DWORD;
    m_wPortID    = INVALID_WORD;
    m_ucQueueNum = 0;
    m_ucDevType  = 0;
}


WORD32 CBaseDevice::Initialize()
{
    return SUCCESS;
}


VOID CBaseDevice::Dump()
{
    TRACE_STACK("CBaseDevice::Dump()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "m_dwDeviceID : %d, m_wPortID : %d, m_ucQueueNum : %d, "
               "m_ucDevType : %d\n",
               m_dwDeviceID,
               m_wPortID,
               m_ucQueueNum,
               m_ucDevType);
}


VOID CFactoryDevice::Dump()
{
    TRACE_STACK("CFactoryDevice::Dump()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE, 
               "m_dwDefNum : %2d\n",
               m_dwDefNum);

    for (WORD32 dwIndex = 0; dwIndex < m_dwDefNum; dwIndex++)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE, 
                   "pCreateFunc : %p, pResetFunc : %p, pDestroyFunc : %p, aucName : %s\n",
                   m_atDefInfo[dwIndex].pCreateFunc,
                   m_atDefInfo[dwIndex].pResetFunc,
                   m_atDefInfo[dwIndex].pDestroyFunc,
                   m_atDefInfo[dwIndex].aucName);
    }
}


