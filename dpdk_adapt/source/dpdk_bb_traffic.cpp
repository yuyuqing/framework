

#include "dpdk_bb_traffic.h"
#include "base_log.h"


CBaseTraffic::CBaseTraffic (E_TrafficType eType, const T_TrafficParam &rtParam)
{
    m_eType        = eType;
    m_dwDeviceID   = rtParam.dwDeviceID;
    m_dwPortID     = rtParam.dwPortID;
    m_dwQueueID    = rtParam.dwQueueID;
    m_dwTrafficID  = rtParam.dwTrafficID;
    m_dwFAPICellID = rtParam.dwFAPICellID;
    m_dwBindCellID = rtParam.dwBindCellID;
}


CBaseTraffic::~CBaseTraffic()
{
}


WORD32 CBaseTraffic::Initialize()
{
    return SUCCESS;
}


VOID CBaseTraffic::Dump()
{
    TRACE_STACK("CBaseTraffic::Dump()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "m_dwDeviceID : %d, m_dwPortID : %d, m_dwQueueID : %d, "
               "m_dwTrafficID : %d, m_dwFAPICellID : %d, m_dwBindCellID : %d\n",
               m_dwDeviceID,
               m_dwPortID,
               m_dwQueueID,
               m_dwTrafficID,
               m_dwFAPICellID,
               m_dwBindCellID);
}


VOID CFactoryTraffic::Dump()
{
    TRACE_STACK("CFactoryTraffic::Dump()");

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


