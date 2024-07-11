

#include "dpdk_bb_handler.h"
#include "base_log.h"


CBBHandler *g_pBBHandler = NULL;


CBBHandler::CBBHandler ()
{
    g_pBBHandler = this;
    m_dwCellNum  = 0;

    for (WORD32 dwIndex = 0; dwIndex < MAX_CELL_PER_GNB; dwIndex++)
    {
        m_atCellMap[dwIndex].wCellID      = INVALID_WORD;
        m_atCellMap[dwIndex].wDeviceID    = INVALID_WORD;
        m_atCellMap[dwIndex].wQueueID     = INVALID_WORD;
        m_atCellMap[dwIndex].wFapiCellID  = INVALID_WORD;
        m_atCellMap[dwIndex].pDataTraffic = NULL;
        m_atCellMap[dwIndex].pCtrlTraffic = NULL;
    }
}


CBBHandler::~CBBHandler()
{
    g_pBBHandler = NULL;
    m_dwCellNum  = 0;

    for (WORD32 dwIndex = 0; dwIndex < MAX_CELL_PER_GNB; dwIndex++)
    {
        m_atCellMap[dwIndex].wCellID      = INVALID_WORD;
        m_atCellMap[dwIndex].wDeviceID    = INVALID_WORD;
        m_atCellMap[dwIndex].wQueueID     = INVALID_WORD;
        m_atCellMap[dwIndex].wFapiCellID  = INVALID_WORD;
        m_atCellMap[dwIndex].pDataTraffic = NULL;
        m_atCellMap[dwIndex].pCtrlTraffic = NULL;
    }
}


WORD32 CBBHandler::Initialize()
{
    TRACE_STACK("CBBHandler::Initialize()");
    return SUCCESS;
}


WORD32 CBBHandler::RegistCellTraffic(WORD16        wCellID,
                                     WORD16        wDeviceID,
                                     WORD16        wQueueID,
                                     WORD16        wFapiCellID,
                                     CBaseTraffic *pDataTraffic,
                                     CBaseTraffic *pCtrlTraffic)
{
    TRACE_STACK("CBBHandler::RegistCellTraffic()");

    if ((0 == wCellID) || (wCellID > MAX_CELL_PER_GNB))
    {
        assert(0);
    }

    T_CellTrafficMap &rtCell = m_atCellMap[wCellID - 1];

    if (INVALID_WORD == rtCell.wCellID)
    {
        m_dwCellNum++;  /* 第一次注册时++, 后续不再++ */

        rtCell.wCellID = wCellID;
    }
    else
    {
        assert(wCellID == rtCell.wCellID);
    }

    if (INVALID_WORD == rtCell.wDeviceID)
    {
        rtCell.wDeviceID = wDeviceID;
    }
    else
    {
        assert(wDeviceID == rtCell.wDeviceID);
    }

    if (INVALID_WORD == rtCell.wQueueID)
    {
        rtCell.wQueueID = wQueueID;
    }
    else
    {
        assert(wQueueID == rtCell.wQueueID);
    }

    if (INVALID_WORD == rtCell.wFapiCellID)
    {
        rtCell.wFapiCellID = wFapiCellID;
    }
    else
    {
        assert(wFapiCellID == rtCell.wFapiCellID);
    }

    if (NULL == rtCell.pDataTraffic)
    {
        rtCell.pDataTraffic = pDataTraffic;
    }

    if (NULL == rtCell.pCtrlTraffic)
    {
        rtCell.pCtrlTraffic = pCtrlTraffic;
    }

    return SUCCESS;
}


WORD32 CBBHandler::Polling()
{
    for (WORD32 dwIndex = 0; dwIndex < MAX_CELL_PER_GNB; dwIndex++)
    {
        if (NULL == m_atCellMap[dwIndex].pCtrlTraffic)
        {
            continue ;
        }

#ifdef PICOCOM_FAPI
        pcxxCtrlRecv(m_atCellMap[dwIndex].wDeviceID, m_atCellMap[dwIndex].wFapiCellID);
#endif
    }

    return SUCCESS;
}


