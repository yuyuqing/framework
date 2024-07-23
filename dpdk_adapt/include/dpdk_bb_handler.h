

#ifndef _DPDK_BB_HANDLER_H_
#define _DPDK_BB_HANDLER_H_


#include "dpdk_bb_traffic.h"


typedef struct tagT_CellTrafficMap
{
    WORD16         wCellID;       /* �߲�С��ID(1/2/3/4) */
    WORD16         wDeviceID;     /* PCI�豸ID(��base.json����һ��) */
    WORD16         wQueueID;      /* DPDK����ID, �̶�Ϊ0 */
    WORD16         wFapiCellID;   /* FAPIС��ID, ÿ��PCI�豸��Ψһ���(0/1) */
    CBaseTraffic  *pDataTraffic;  /* FAPI����ҵ��ͨ�� */
    CBaseTraffic  *pCtrlTraffic;  /* FAPI����ҵ��ͨ�� */
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

    /* ����DeviceID + FapiCellID���Ҷ�Ӧ�ĸ߲�С��ID */
    WORD16 GetCellID(WORD16 wDeviceID, WORD16 wFapiCellID);

    /* ���ݸ߲�С��ID���Ҷ�Ӧ��FAPI����ҵ��ͨ�� */
    CBaseTraffic * GetDataTraffic(WORD16 wCellID);

    /* ���ݸ߲�С��ID���Ҷ�Ӧ��FAPI����ҵ��ͨ�� */
    CBaseTraffic * GetCtrlTraffic(WORD16 wCellID);

    VOID Dump();

protected :
    WORD32              m_dwCellNum;
    T_CellTrafficMap    m_atCellMap[MAX_CELL_PER_GNB];
};


/* ����DeviceID + FapiCellID���Ҷ�Ӧ�ĸ߲�С��ID */
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


/* ���ݸ߲�С��ID���Ҷ�Ӧ��FAPI����ҵ��ͨ�� */
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


/* ���ݸ߲�С��ID���Ҷ�Ӧ��FAPI����ҵ��ͨ�� */
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


