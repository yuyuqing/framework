

#include "base_util.h"
#include "base_sort.h"
#include "base_app_interface.h"


WORD32 CAppState::Enter(E_AppState  eState,
                        WORD32      dwMsgID, 
                        VOID       *pIn, 
                        WORD16      wMsgLen, 
                        CAppState **pState,
                        BYTE       *pData)
{
    if (unlikely((NULL == pIn)
              || (NULL == pData)))
    {
        return FAIL;
    }

    CAppInterface *pAppInst         = (*pState)->m_pAppInst;
    WORD32         dwResult         = INVALID_DWORD;
    WORD64         lwTimeStampStart = 0;
    WORD64         lwTimeStampEnd   = 0;

    switch (eState)
    {
    case APP_STATE_NULL:
    case APP_STATE_IDLE:
        {
            /* 收到系统第一条上电消息, 执行Init操作 */
            (*pState) = new (pData) CAppIdleState(pAppInst);
            dwResult  = (*pState)->Init();
            if (SUCCESS == dwResult)
            {
                (*pState) = new (pData) CAppWorkState(pAppInst);
            }
        }
        break ;

    case APP_STATE_WORK:
        {
            if (unlikely(EV_BASE_APP_SHUTDOWN_ID == dwMsgID))
            {
                dwResult = (*pState)->DeInit();
                (*pState) = new (pData) CAppExitState(pAppInst);
            }
            else
            {
                lwTimeStampStart = GetCycle();
                pAppInst->Proc(dwMsgID, pIn, wMsgLen);
                lwTimeStampEnd = GetCycle();

                pAppInst->Statistic(dwMsgID, lwTimeStampStart, lwTimeStampEnd);
            }
        }
        break ;

    case APP_STATE_EXIT:
        {
            (*pState)->Exit(dwMsgID, pIn, wMsgLen);
        }
        break ;

    default :
        break ;
    }

    return SUCCESS;
}


CAppState::CAppState (CAppInterface *pAppInst)
    : m_pAppInst(pAppInst)
{
    m_eState = APP_STATE_NULL;
}


CAppState::~CAppState()
{
    m_eState = APP_STATE_NULL;

    if (NULL != m_pAppInst)
    {
        m_pAppInst = NULL;
    }
}


WORD32 CAppState::Start()
{
    if (APP_STATE_NULL != m_eState)
    {
        return FAIL;
    }

    if (NULL == m_pAppInst)
    {
        return FAIL;
    }

    return m_pAppInst->Start();
}


WORD32 CAppState::Init()
{
    if (APP_STATE_IDLE != m_eState)
    {
        return FAIL;
    }

    return SUCCESS;
}


WORD32 CAppState::DeInit()
{
    if (APP_STATE_WORK != m_eState)
    {
        return FAIL;
    }

    return SUCCESS;
}


WORD32 CAppState::Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen)
{
    if (APP_STATE_EXIT != m_eState)
    {
        return FAIL;
    }

    return SUCCESS;
}


VOID CAppState::Dump()
{
    TRACE_STACK("CAppState::Dump()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "m_eState : %d\n",
               m_eState);

    m_pAppInst->Dump();
}


CAppIdleState::CAppIdleState (CAppInterface *pAppInst)
    : CAppState(pAppInst)
{
    m_eState = APP_STATE_IDLE;
}


CAppIdleState::~CAppIdleState()
{
}


WORD32 CAppIdleState::Init()
{
    if (NULL == m_pAppInst)
    {
        return FAIL;
    }

    return m_pAppInst->Init();
}


CAppWorkState::CAppWorkState (CAppInterface *pAppInst)
    : CAppState(pAppInst)
{
    m_eState = APP_STATE_WORK;
}


CAppWorkState::~CAppWorkState()
{
}


WORD32 CAppWorkState::DeInit()
{
    if (NULL == m_pAppInst)
    {
        return FAIL;
    }

    TRACE_STACK("CAppWorkState::DeInit()");

    T_AppMeasure tMeasure;

    /* 输出App级别维测数据 */
    m_pAppInst->GetMeasure(tMeasure);

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "AppID:%d  Class:%d  Num:%ld  [MsgID:%4d  MaxTime:%d]  TotalProcTime(0.1us):%lu\n",
               tMeasure.dwAppID,
               tMeasure.dwAppClass,
               tMeasure.lwProcNum,
               tMeasure.dwMsgID,
               tMeasure.dwMaxTimeUsed,
               tMeasure.lwTimeUsedTotal);

    for (WORD32 dwIndex1 = 0; dwIndex1 < BIT_NUM_OF_WORD32; dwIndex1++)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                   "%10d  %15lu\n",
                   (1 << dwIndex1),
                   tMeasure.alwStat[dwIndex1]);
    }

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "AppID:%d  Count:%ld  [QMaxMsgID:%d, MaxTime:%d]  TotalWaitTime(0.1us):%lu\n",
               tMeasure.dwAppID,
               tMeasure.tMsgQStat.lwMsgCount,
               tMeasure.tMsgQStat.dwQMaxMsgID,
               tMeasure.tMsgQStat.dwMsgQMaxUsed,
               tMeasure.tMsgQStat.lwMsgQTotalTime);

    for (WORD32 dwIndex1 = 0; dwIndex1 < BIT_NUM_OF_WORD32; dwIndex1++)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                   "%10d  %15lu\n",
                   (1 << dwIndex1),
                   tMeasure.tMsgQStat.alwStat[dwIndex1]);
    }

    return m_pAppInst->DeInit();
}


CAppExitState::CAppExitState (CAppInterface *pAppInst)
    : CAppState(pAppInst)
{
    m_eState = APP_STATE_EXIT;
}


CAppExitState::~CAppExitState()
{
}


WORD32 CAppExitState::Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen)
{
    if (NULL == m_pAppInst)
    {
        return FAIL;
    }

    return m_pAppInst->Exit(dwMsgID, pIn, wMsgLen);
}


