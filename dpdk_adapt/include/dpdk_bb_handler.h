

#ifndef _DPDK_BB_HANDLER_H_
#define _DPDK_BB_HANDLER_H_


#include "dpdk_bb_traffic.h"


typedef struct tagT_CellTrafficMap
{
    WORD16         wCellID;       /* 高层小区ID(1/2/3/4) */
    WORD16         wDeviceID;     /* PCI设备ID(与base.json配置一致) */
    WORD16         wQueueID;      /* DPDK队列ID, 固定为0 */
    WORD16         wFapiCellID;   /* FAPI小区ID, 每个PCI设备内唯一编号(0/1) */
    CBaseTraffic  *pDataTraffic;  /* FAPI数据业务通道 */
    CBaseTraffic  *pCtrlTraffic;  /* FAPI控制业务通道 */
}T_CellTrafficMap;


class CBBHandler : public CCBObject
{
public :
    CBBHandler ();
    virtual ~CBBHandler();

    virtual WORD32 Initialize();

    WORD32 RegistCellTraffic(WORD16        wCellID,
                             WORD16        wDeviceID,
                             WORD16        wQueueID,
                             WORD16        wFapiCellID,
                             CBaseTraffic *pDataTraffic,
                             CBaseTraffic *pCtrlTraffic);

    WORD32 Polling();

    /* 根据DeviceID + FapiCellID查找对应的高层小区ID */
    WORD16 GetCellID(WORD16 wDeviceID, WORD16 wFapiCellID);

    /* 根据高层小区ID查找对应的FAPI数据业务通道 */
    CBaseTraffic * GetDataTraffic(WORD16 wCellID);

    /* 根据高层小区ID查找对应的FAPI控制业务通道 */
    CBaseTraffic * GetCtrlTraffic(WORD16 wCellID);

    VOID Dump();

protected :
    WORD32              m_dwCellNum;
    T_CellTrafficMap    m_atCellMap[MAX_CELL_PER_GNB];
};


/* 根据DeviceID + FapiCellID查找对应的高层小区ID */
inline WORD16 CBBHandler::GetCellID(WORD16 wDeviceID, WORD16 wFapiCellID)
{
    for (WORD32 dwIndex = 0; dwIndex < MAX_CELL_PER_GNB; dwIndex++)
    {
        if ( (wDeviceID   == m_atCellMap[dwIndex].wDeviceID)
          && (wFapiCellID == m_atCellMap[dwIndex].wFapiCellID))
        {
            return m_atCellMap[dwIndex].wCellID;
        }
    }

    return INVALID_WORD;
}


/* 根据高层小区ID查找对应的FAPI数据业务通道 */
inline CBaseTraffic * CBBHandler::GetDataTraffic(WORD16 wCellID)
{
    for (WORD32 dwIndex = 0; dwIndex < MAX_CELL_PER_GNB; dwIndex++)
    {
        if (wCellID == m_atCellMap[dwIndex].wCellID)
        {
            return m_atCellMap[dwIndex].pDataTraffic;
        }
    }

    return NULL;
}


/* 根据高层小区ID查找对应的FAPI控制业务通道 */
inline CBaseTraffic * CBBHandler::GetCtrlTraffic(WORD16 wCellID)
{
    for (WORD32 dwIndex = 0; dwIndex < MAX_CELL_PER_GNB; dwIndex++)
    {
        if (wCellID == m_atCellMap[dwIndex].wCellID)
        {
            return m_atCellMap[dwIndex].pCtrlTraffic;
        }
    }

    return NULL;
}


extern CBBHandler *g_pBBHandler;


#endif


