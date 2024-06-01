

#include "base_mem_mgr.h"
#include "base_thread_log.h"
#include "base_thread_pool.h"


DEFINE_APP(COamApp);


COamCBNode::COamCBNode()
{
    m_dwInstID   = INVALID_DWORD;
    m_dwTaskID   = INVALID_DWORD;
    m_pObj       = NULL;
    m_pFunc      = NULL;
    m_lwUserData = 0;
    m_dwTimerID  = INVALID_DWORD;
}


COamCBNode::~COamCBNode()
{
    m_dwInstID   = INVALID_DWORD;
    m_dwTaskID   = INVALID_DWORD;
    m_pObj       = NULL;
    m_pFunc      = NULL;
    m_lwUserData = 0;
    m_dwTimerID  = INVALID_DWORD;
}


WORD32 COamCBNode::Initialize(WORD32     dwInstID,
                              WORD32     dwTaskID,
                              CCBObject *pObj,
                              PCBFUNC    pFunc,
                              WORD64     lwUserData)
{
    m_dwInstID   = dwInstID;
    m_dwTaskID   = dwTaskID;
    m_pObj       = pObj;
    m_pFunc      = pFunc;
    m_lwUserData = lwUserData;

    return SUCCESS;
}


COamApp::COamApp ()
    : CAppInterface(E_APP_OAM)
{
    g_pOamApp      = this;
    m_ucPos        = 0;
    m_ucMeasMinute = CLogInfo::s_awPeriodMinute[E_LOG_PERIOD_05_MINUTE];
    m_wSwitchPrd   = CLogInfo::s_awPeriodMinute[E_LOG_PERIOD_05_MINUTE];
    m_ucThrdNum    = 0;
    m_ucAppNum     = 0;

    for (WORD32 dwIndex = 0; dwIndex < MAX_WORKER_NUM; dwIndex++)
    {
        m_apThread[dwIndex] = NULL;
    }

    for (WORD32 dwIndex = 0; dwIndex < MAX_APP_NUM; dwIndex++)
    {
        m_apApp[dwIndex] = NULL;
    }

    memset(&m_tMemPoolMeasure, 0x00, sizeof(m_tMemPoolMeasure));
}


COamApp::~COamApp()
{
}


WORD32 COamApp::InitApp()
{
    TRACE_STACK("COamApp::InitApp()");

    m_cCBTable.Initialize();

    RegisterProcessor(EV_BASE_LOG_SET_GLOBAL_SWITCH_ID,
                      (CCBObject *)this,
                      (PCBFUNC)(&COamApp::ProcGlobalSwitchMsg));

    RegisterProcessor(EV_BASE_LOG_SET_WRITE_PERIOD_ID,
                      (CCBObject *)this,
                      (PCBFUNC)(&COamApp::ProcWritePeriodMsg));

    RegisterProcessor(EV_BASE_LOG_SET_MODULE_SWITCH_ID,
                      (CCBObject *)this,
                      (PCBFUNC)(&COamApp::ProcModuleSwitchMsg));

    RegisterProcessor(EV_BASE_LOG_REGIST_CALLBACK_ID,
                      (CCBObject *)this,
                      (PCBFUNC)(&COamApp::ProcCBRegistMsg));

    RegisterProcessor(EV_BASE_LOG_REMOVE_CALLBACK_ID,
                      (CCBObject *)this,
                      (PCBFUNC)(&COamApp::ProcCBRemoveMsg));

    return SUCCESS;
}


WORD32 COamApp::Start()
{
    TRACE_STACK("COamApp::Start()");

    g_dwLogAppID = m_dwAppID;

    WORD32 dwNum = m_pOwner->SendHPMsgToApp(m_dwAppID,
                                            m_dwAppID,
                                            EV_BASE_APP_STARTUP_ID,
                                            0, NULL);

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "g_dwLogAppID : %d, m_dwAppID : %d, m_dwThreadID : %d, "
               "m_ucPos : %d, m_ucMeasMinute : %d, m_wSwitchPrd : %d, "
               "dwNum : %d\n",
               g_dwLogAppID,
               m_dwAppID,
               m_dwThreadID,
               m_ucPos,
               m_ucMeasMinute,
               m_wSwitchPrd,
               dwNum);

    return SUCCESS;
}


WORD32 COamApp::Init()
{
    TRACE_STACK("COamApp::Init()");

    WORD64 lwSeconds  = 0;
    WORD64 lwMicroSec = 0;
    WORD64 lwCycle    = 0;
    WORD32 dwPeriod   = 0;
    WORD32 dwResult   = 0;

    m_ucPos        = (BYTE)(g_pLogger->GetPos());
    m_ucMeasMinute = (BYTE)(g_pLogger->GetLogMeasure());
    m_wSwitchPrd   = g_pLogger->GetPeriod();

    dwPeriod = m_ucMeasMinute;
    dwPeriod = dwPeriod * 60000;

    /* 发送周期性输出系统维测任务的SYNC消息 */
    dwResult = SendRegistCBMsg(E_OAM_TASK_SYNC_ID,
                               dwPeriod,
                               (CCBObject *)this,
                               (PCBFUNC)(&COamApp::SyncClock),
                               NULL);
    if (SUCCESS != dwResult)
    {
        assert(0);
    }

    g_pGlobalClock->GetTime(lwSeconds, lwMicroSec, lwCycle);

    dwResult = DoLogTask(lwSeconds, lwMicroSec, lwCycle, m_wSwitchPrd);
    if (SUCCESS != dwResult)
    {
        assert(0);
    }

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "lwSeconds : %lu, lwMicroSec : %lu, lwCycle : %lu, "
               "m_ucPos : %d, m_ucMeasMinute : %d, m_wSwitchPrd : %d\n",
               lwSeconds,
               lwMicroSec,
               lwCycle,
               m_ucPos,
               m_ucMeasMinute,
               m_wSwitchPrd);

    return SUCCESS;
}


WORD32 COamApp::DeInit()
{
    TRACE_STACK("COamApp::DeInit()");

    m_pOwner->SendLPMsgToApp(m_dwAppID,
                             m_dwAppID,
                             EV_BASE_APP_SHUTDOWN_ID,
                             0, NULL);

    return SUCCESS;
}


WORD32 COamApp::Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen)
{
    TRACE_STACK("COamApp::Exit()");

    m_pOwner->SendLPMsgToApp(m_dwAppID,
                             m_dwAppID,
                             EV_BASE_THREAD_CANCEL_ID,
                             0, NULL);

    return SUCCESS;
}


/* 通知其它APP上电, 在main进程中调用 */
WORD32 COamApp::InitAllApps()
{
    TRACE_STACK("COamApp::InitAllApps()");

    CAppCntrl *pAppCntrl = CAppCntrl::GetInstance();
    WORD32     dwAppNum  = pAppCntrl->GetAppNum();

    for (WORD32 dwIndex = 0; dwIndex < dwAppNum; dwIndex++)
    {
        T_AppInfo *ptAppInfo = (*pAppCntrl)[dwIndex];
        if (m_dwAppID == ptAppInfo->dwAppID)
        {
            /* 跳过自己 */
            continue ;
        }

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE, 
                   "Notify App init; AppID : %d, ThreadID : %d, Name : %s\n",
                   ptAppInfo->dwAppID,
                   ptAppInfo->dwThreadID,
                   ptAppInfo->aucName);

        SendHighPriorMsgToApp(ptAppInfo->dwThreadID,
                              ptAppInfo->dwAppID,
                              0,
                              EV_BASE_APP_STARTUP_ID,
                              0, NULL);
    }

    return SUCCESS;
}


VOID COamApp::ProcGlobalSwitchMsg(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("COamApp::ProcGlobalSwitchMsg()");

    if ((NULL == pIn) || (dwLen != sizeof(T_LogSetGlobalSwitch)))
    {
        return ;
    }

    T_LogSetGlobalSwitch *ptMsg = (T_LogSetGlobalSwitch *)pIn;
    if (ptMsg->dwProcID != g_pLogger->GetProcID())
    {
        return ;
    }

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE, 
               "bSwitch : %d\n",
               ptMsg->bSwitch);

    g_pLogger->SetGlobalSwitch(ptMsg->bSwitch);

    /* 更新线程池每个线程的日志文件配置 */
    CBaseThread *pWorker     = NULL;
    WORD32       dwThreadNum = g_pThreadPool->GetThreadNum();

    for (WORD32 dwIndex = 0; dwIndex < dwThreadNum; dwIndex++)
    {
        pWorker = (*g_pThreadPool)[dwIndex];
        if (NULL != pWorker)
        {
            pWorker->UpdateGlobalSwitch(ptMsg->bSwitch);
        }
    }
}


VOID COamApp::ProcWritePeriodMsg(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("COamApp::ProcWritePeriodMsg()");

    if ((NULL == pIn) || (dwLen != sizeof(T_LogSetWritePeriod)))
    {
        return ;
    }

    T_LogSetWritePeriod *ptMsg = (T_LogSetWritePeriod *)pIn;
    if (ptMsg->ucPeriod >= E_LOG_PERIOD_NUM)
    {
        return ;
    }

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE, 
               "ucPeriod : %d\n",
               ptMsg->ucPeriod);

    /* 仅更新g_pLogger的配置, 原因:其它日志文件不切换 */
    g_pLogger->SetPeriod((E_LogFilePeriod)(ptMsg->ucPeriod));

    /* 当周期发生更新后, 需要立即重新计算当前写的日志文件 */
    g_pLogger->SetLogFile();

    /* 同步更新本地存储的内容 */    
    m_ucPos      = g_pLogger->GetPos();
    m_wSwitchPrd = g_pLogger->GetPeriod();

    /* 更新线程池每个线程的日志文件配置 */
    CBaseThread *pWorker     = NULL;
    WORD32       dwThreadNum = g_pThreadPool->GetThreadNum();

    for (WORD32 dwIndex = 0; dwIndex < dwThreadNum; dwIndex++)
    {
        pWorker = (*g_pThreadPool)[dwIndex];
        if (NULL != pWorker)
        {
            pWorker->UpdatePeriod((E_LogFilePeriod)(ptMsg->ucPeriod));
        }
    }
}


VOID COamApp::ProcModuleSwitchMsg(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("COamApp::ProcModuleSwitchMsg()");

    if ((NULL == pIn) || (dwLen != sizeof(T_LogSetModuleSwitch)))
    {
        return ;
    }

    T_LogSetModuleSwitch *ptMsg = (T_LogSetModuleSwitch *)pIn;

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "ModuleID : %3u, CellNum : %u, LevelNum : %u, Switch : %d\n",
               ptMsg->ucModuleID,
               ptMsg->ucCellNum,
               ptMsg->ucLevelNum,
               ptMsg->bSwitch);

    BOOL bGlobalSwitch = g_pLogger->GetGlobalSwitch();

    /* 在设置模块级别的日志配置时, 先关闭日志总开关, 待模块级设置完成再恢复 */
    if (bGlobalSwitch)
    {
        g_pLogger->SetGlobalSwitch(FALSE);
    }

    g_pLogger->SetModuleMask((WORD32)(ptMsg->ucModuleID), ptMsg->bSwitch);
    g_pLogger->ClearExtModuleMask((WORD32)(ptMsg->ucModuleID));

    for (WORD32 dwIndex1 = 0; dwIndex1 < ptMsg->ucCellNum; dwIndex1++)
    {
        g_pLogger->SetExtModuleMask((WORD32)(ptMsg->ucModuleID),
                                    (WORD32)(ptMsg->awCellID[dwIndex1]),
                                    TRUE);
    }

    for (WORD32 dwIndex1 = 0; dwIndex1 < ptMsg->ucLevelNum; dwIndex1++)
    {
        g_pLogger->SetLevelMask((WORD32)(ptMsg->ucModuleID),
                                (WORD32)(ptMsg->aucLevelID[dwIndex1]),
                                TRUE);
    }

    /* 恢复日志总开关配置 */
    if (bGlobalSwitch)
    {
        g_pLogger->SetGlobalSwitch(bGlobalSwitch);
    }

    /* 更新线程池每个线程的日志文件配置 */
    CBaseThread *pWorker     = NULL;
    WORD32       dwThreadNum = g_pThreadPool->GetThreadNum();

    for (WORD32 dwIndex = 0; dwIndex < dwThreadNum; dwIndex++)
    {
        pWorker = (*g_pThreadPool)[dwIndex];
        if (NULL != pWorker)
        {
            pWorker->UpdateModuleSwitch(*ptMsg);
        }
    }
}


/* 处理注册定时回调函数的消息 */
VOID COamApp::ProcCBRegistMsg(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("COamApp::ProcCBRegistMsg()");

    if ((NULL == pIn) || (dwLen != sizeof(T_OamRegistCallBack)))
    {
        return ;
    }

    T_OamRegistCallBack *ptMsg = (T_OamRegistCallBack *)pIn;

    WORD32 dwInstID  = INVALID_DWORD;
    WORD32 dwTaskID  = ptMsg->dwTaskID;
    WORD32 dwPeriod  = ptMsg->dwPeriod;
    WORD64 lwUsrData = ptMsg->lwUsrData;

    COamCBNode *pNode = m_cCBTable.Find(dwTaskID);
    if (NULL != pNode)
    {
        return ;
    }

    pNode = m_cCBTable.Create(ptMsg->dwTaskID, dwInstID);
    if (NULL == pNode)
    {
        return ;
    }

    pNode->Initialize(dwInstID,
                      dwTaskID,
                      ptMsg->pObjAddr,
                      ptMsg->pFuncAddr,
                      lwUsrData);

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "InstID : %u, TaskID : %u, Period : %u, UsrData : %lu, "
               "ObjAddr : %lu\n",
               dwInstID, dwTaskID, dwPeriod, lwUsrData,
               (WORD64)(ptMsg->pObjAddr));

    pNode->m_dwTimerID = RegisterTimer(dwPeriod,
                                       (CCBObject *)this,
                                       (PCBFUNC)(&COamApp::CallBack),
                                       dwInstID, dwTaskID, dwPeriod,
                                       (VOID *)(pNode),
                                       (VOID *)(lwUsrData));
}


/* 处理去注册定时回调函数的消息 */
VOID COamApp::ProcCBRemoveMsg(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("COamApp::ProcCBRemoveMsg()");

    if ((NULL == pIn) || (dwLen != sizeof(T_OamRemoveCallBack)))
    {
        return ;
    }

    T_OamRemoveCallBack *ptMsg = (T_OamRemoveCallBack *)pIn;

    WORD32 dwTaskID  = ptMsg->dwTaskID;
    WORD32 dwTimerID = INVALID_DWORD;

    COamCBNode *pNode = m_cCBTable.Find(dwTaskID);
    if (NULL == pNode)
    {
        return ;
    }

    dwTimerID = pNode->m_dwTimerID;

    KillTimer(dwTimerID);
    m_cCBTable.Remove(dwTaskID);
}


VOID COamApp::CallBack(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("COamApp::CallBack()");

    if (unlikely((NULL == pIn) || (dwLen != sizeof(T_TimerParam))))
    {
        return ;
    }

    T_TimerParam *ptParam = (T_TimerParam *)pIn;

    WORD32      dwInstID  = ptParam->dwID;
    WORD32      dwTaskID  = ptParam->dwExtendID;
    WORD32      dwPeriod  = (WORD32)(ptParam->lwTransID);
    COamCBNode *pNode     = (COamCBNode *)(ptParam->pContext);
    WORD64      lwUsrData = (WORD64)(ptParam->pUserData);
    CCBObject  *pObj      = (CCBObject *)(pNode->m_pObj);
    PCBFUNC     pFunc     = (PCBFUNC)(pNode->m_pFunc);

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "InstID : %u, TaskID : %u, Period : %u, UsrData : %lu, "
               "Obj : %lu, InstID : %u, TaskID : %u, UsrData : %lu\n",
               dwInstID, dwTaskID, dwPeriod, lwUsrData,
               (WORD64)pObj,
               pNode->m_dwInstID,
               pNode->m_dwTaskID,
               pNode->m_lwUserData);

    pNode->m_dwTimerID = RegisterTimer(dwPeriod,
                                       (CCBObject *)this,
                                       (PCBFUNC)(&COamApp::CallBack),
                                       dwInstID, dwTaskID, dwPeriod,
                                       (VOID *)(pNode),
                                       (VOID *)(lwUsrData));

    (pObj->*(pFunc)) ((VOID *)lwUsrData, sizeof(WORD64));
}


WORD32 COamApp::SendRegistCBMsg(WORD32     dwTaskID,
                                WORD32     dwPeriod,
                                CCBObject *pObj,
                                PCBFUNC    pFunc,
                                VOID      *pUsrData)
{
    if ((NULL == pObj) || (NULL == pFunc))
    {
        return FAIL;
    }

    T_OamRegistCallBack  tMsg;

    tMsg.dwTaskID  = dwTaskID;
    tMsg.dwPeriod  = dwPeriod;
    tMsg.pObjAddr  = pObj;
    tMsg.pFuncAddr = pFunc;
    tMsg.lwUsrData = (WORD64)(pUsrData);

    WORD32 dwNum = m_pOwner->SendLPMsgToApp(m_dwAppID,
                                            m_dwAppID,
                                            EV_BASE_LOG_REGIST_CALLBACK_ID,
                                            sizeof(tMsg),
                                            &tMsg);
    if (0 == dwNum)
    {
        return FAIL;
    }

    return SUCCESS;
}


WORD32 COamApp::SendRemoveCBMsg(WORD32 dwTaskID)
{
    T_OamRemoveCallBack  tMsg;

    tMsg.dwTaskID = dwTaskID;

    WORD32 dwNum = m_pOwner->SendLPMsgToApp(m_dwAppID,
                                            m_dwAppID,
                                            EV_BASE_LOG_REMOVE_CALLBACK_ID,
                                            sizeof(tMsg),
                                            &tMsg);
    if (0 == dwNum)
    {
        return FAIL;
    }

    return SUCCESS;
}


VOID COamApp::SyncClock(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("COamApp::SyncClock()");

    WORD64 lwSeconds     = 0;
    WORD64 lwMicroSec    = 0;
    WORD64 lwCycle       = 0;
    WORD64 lwCentralSize = 0;
    WORD64 lwCentralUsed = 0;
    WORD64 lwCentralFree = 0;

    if (unlikely(0 == m_ucThrdNum))
    {
        m_apThread[m_ucThrdNum] = g_pLogThread;
        m_ucThrdNum++;

        for (WORD32 dwIndex = 0; dwIndex < MAX_WORKER_NUM; dwIndex++)
        {
            CBaseThread *pThread = (*g_pThreadPool)[dwIndex];
            if (NULL == pThread)
            {
                break ;
            }

            m_apThread[m_ucThrdNum] = pThread;
            m_ucThrdNum++;
        }
    }

    if (unlikely(0 == m_ucAppNum))
    {
        CAppCntrl *pAppCntrl = CAppCntrl::GetInstance();
        for (WORD32 dwIndex = 0; dwIndex < MAX_APP_NUM; dwIndex++)
        {
            T_AppInfo *ptAppInfo = (*pAppCntrl)[dwIndex];
            if (NULL == ptAppInfo)
            {
                break ;
            }

            if (NULL == ptAppInfo->pAppState)
            {
                break ;
            }

            m_apApp[m_ucAppNum] = ptAppInfo->pAppState->GetAppInst();
            m_ucAppNum++;
        }
    }

    RegisterTimer(TIMER_TIMEOUT_INTERVAL,
                  (CCBObject *)this,
                  (PCBFUNC)(&COamApp::TimeOutDumpThreadMeas),
                  0);

    RegisterTimer((2 * TIMER_TIMEOUT_INTERVAL),
                  (CCBObject *)this,
                  (PCBFUNC)(&COamApp::TimeOutDumpAppMeas),
                  0);

    RegisterTimer((3 * TIMER_TIMEOUT_INTERVAL),
                  (CCBObject *)this,
                  (PCBFUNC)(&COamApp::TimeOutDumpMsgQueueMeas),
                  0);

    RegisterTimer((4 * TIMER_TIMEOUT_INTERVAL),
                  (CCBObject *)this,
                  (PCBFUNC)(&COamApp::TimeOutDumpMsgMemPoolMeas),
                  0);

    RegisterTimer((5 * TIMER_TIMEOUT_INTERVAL),
                  (CCBObject *)this,
                  (PCBFUNC)(&COamApp::TimeOutDumpMemPoolsMeas),
                  0);

    RegisterTimer((6 * TIMER_TIMEOUT_INTERVAL),
                  (CCBObject *)this,
                  (PCBFUNC)(&COamApp::TimeOutMemMgrDump),
                  0);

    RegisterTimer((7 * TIMER_TIMEOUT_INTERVAL),
                  (CCBObject *)this,
                  (PCBFUNC)(&COamApp::TimeOutLogMeas),
                  0);

    RegisterTimer((8 * TIMER_TIMEOUT_INTERVAL),
                  (CCBObject *)this,
                  (PCBFUNC)(&COamApp::TimeOutLogFlush),
                  0);

    for (WORD32 dwIndex = 0; dwIndex < m_ucThrdNum; dwIndex++)
    {
        m_apThread[dwIndex]->GetMeasure(m_atThrdMeasure[dwIndex]);
        m_apThread[dwIndex]->GetRingMeasure(m_atThreadRingMeasure[dwIndex]);
    }

    for (WORD32 dwIndex = 0; dwIndex < m_ucAppNum; dwIndex++)
    {
        m_apApp[dwIndex]->GetMeasure(m_atAppMeasure[dwIndex]);
    }

    CCentralMemPool *pCentralMemPool = CMemMgr::GetInstance()->GetCentralMemPool();
    pCentralMemPool->GetUsage(lwCentralSize, lwCentralUsed, lwCentralFree);

    CMemPools *pMemPool = CMemPools::GetInstance();
    if (NULL != pMemPool)
    {
        pMemPool->GetMeasure(m_tMemPoolMeasure);
    }

    g_pGlobalClock->GetTime(lwSeconds, lwMicroSec, lwCycle);

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "Seconds : %lu, MicroSec : %lu, Cycle : %lu, "
               "m_ucThrdNum : %d, m_ucAppNum : %d, "
               "CentralMemSize : %ld, CentralUsed : %ld, CentralFree : %ld\n",
               lwSeconds, lwMicroSec, lwCycle,
               m_ucThrdNum, m_ucAppNum,
               lwCentralSize, lwCentralUsed, lwCentralFree);
}


VOID COamApp::TimeOutDumpThreadMeas(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("ThreadMeasure");

    for (WORD32 dwIndex = 0; dwIndex < m_ucThrdNum; dwIndex++)
    {
        T_ThreadMeasure &rtThreadMeas = m_atThrdMeasure[dwIndex];

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                   "OS_Thrd:%u  Thrd:%d  Class:%d  CoreID:%d  Loop:%lu  "
                   "HNum:%lu  LNum:%lu  DNum:%lu  MaxRL:%u  MaxRD:%u  "
                   "TimeUsed(HLDTA:0.1us)[%lu, %lu, %lu, %lu, %lu]\n",
                   rtThreadMeas.dwThreadID,
                   rtThreadMeas.wThreadID,
                   rtThreadMeas.ucThreadClass,
                   rtThreadMeas.ucLogicalID,
                   rtThreadMeas.lwLoop,
                   rtThreadMeas.lwProcHNum,
                   rtThreadMeas.lwProcLNum,
                   rtThreadMeas.lwProcDNum,
                   rtThreadMeas.dwMaxRemainLNum,
                   rtThreadMeas.dwMaxRemainDNum,
                   rtThreadMeas.lwTimeUsedTotalH,
                   rtThreadMeas.lwTimeUsedTotalL,
                   rtThreadMeas.lwTimeUsedTotalD,
                   rtThreadMeas.lwTimeUsedTotalT,
                   rtThreadMeas.lwTimeUsedTotalA);

        for (WORD32 dwIndex1 = 0; dwIndex1 < BIT_NUM_OF_WORD32; dwIndex1++)
        {
            LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                       "%10d  %15lu  %15lu  %15lu  %15lu  %15lu\n",
                       (1 << dwIndex1),
                       rtThreadMeas.alwStatH[dwIndex1],
                       rtThreadMeas.alwStatL[dwIndex1],
                       rtThreadMeas.alwStatD[dwIndex1],
                       rtThreadMeas.alwStatT[dwIndex1],
                       rtThreadMeas.alwStatA[dwIndex1]);
        }

        T_ThreadRingMeasure &rtRingMeas = m_atThreadRingMeasure[dwIndex];

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                   "HProdHead : %u, HProdTail : %u, "
                   "HConsHead : %u, HConsTail : %u, "
                   "LProdHead : %u, LProdTail : %u, "
                   "LConsHead : %u, LConsTail : %u, "
                   "DRingNum : %d\n",
                   rtRingMeas.tRingH.tProd.dwHead,
                   rtRingMeas.tRingH.tProd.dwTail,
                   rtRingMeas.tRingH.tCons.dwHead,
                   rtRingMeas.tRingH.tCons.dwTail,
                   rtRingMeas.tRingL.tProd.dwHead,
                   rtRingMeas.tRingL.tProd.dwTail,
                   rtRingMeas.tRingL.tCons.dwHead,
                   rtRingMeas.tRingL.tCons.dwTail,
                   rtRingMeas.tRingD.dwRingNum);

        for (WORD32 dwIndex1 = 0; dwIndex1 < rtRingMeas.tRingD.dwRingNum; dwIndex1++)
        {
            LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                       "DProdHead : %10u, DProdTail : %10u, "
                       "DConsHead : %10u, DConsTail : %10u\n",
                       rtRingMeas.tRingD.atProd[dwIndex1].dwHead,
                       rtRingMeas.tRingD.atProd[dwIndex1].dwTail,
                       rtRingMeas.tRingD.atCons[dwIndex1].dwHead,
                       rtRingMeas.tRingD.atCons[dwIndex1].dwTail);
        }
    }
}


VOID COamApp::TimeOutDumpAppMeas(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("AppMeasure(Unit : 0.1us)");

    for (WORD32 dwIndex = 0; dwIndex < m_ucAppNum; dwIndex++)
    {
        T_AppMeasure &rtMeas = m_atAppMeasure[dwIndex];

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                   "AppID:%d  Class:%d  Num:%lu  [MsgID:%4d  MaxTime:%d]  TotalTime:%lu\n",
                   rtMeas.dwAppID,
                   rtMeas.dwAppClass,
                   rtMeas.lwProcNum,
                   rtMeas.dwMsgID,
                   rtMeas.dwMaxTimeUsed,
                   rtMeas.lwTimeUsedTotal);

        for (WORD32 dwIndex1 = 0; dwIndex1 < BIT_NUM_OF_WORD32; dwIndex1++)
        {
            LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                       "%10d  %15u\n",
                       (1 << dwIndex1),
                       rtMeas.adwStat[dwIndex1]);
        }
    }
}


VOID COamApp::TimeOutDumpMsgQueueMeas(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("MsgQueueTimeUsed(Unit : 0.1us)");

    for (WORD32 dwIndex = 0; dwIndex < m_ucThrdNum; dwIndex++)
    {
        T_ThreadMeasure &rtThreadMeas = m_atThrdMeasure[dwIndex];

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                   "Thrd:%d  "
                   "HCount:%lu  [HQMaxMsgID:%d, HMaxTime:%d]  HTotalTime:%lu  "
                   "LCount:%lu  [LQMaxMsgID:%d, LMaxTime:%d]  LTotalTime:%lu  "
                   "MCount:%lu  [MQMaxMsgID:%d, MMaxTime:%d]  MTotalTime:%lu\n",
                   rtThreadMeas.wThreadID,
                   rtThreadMeas.tHPMsgQStat.lwMsgCount,
                   rtThreadMeas.tHPMsgQStat.dwQMaxMsgID,
                   rtThreadMeas.tHPMsgQStat.dwMsgQMaxUsed,
                   rtThreadMeas.tHPMsgQStat.lwMsgQTotalTime,
                   rtThreadMeas.tLPMsgQStat.lwMsgCount,
                   rtThreadMeas.tLPMsgQStat.dwQMaxMsgID,
                   rtThreadMeas.tLPMsgQStat.dwMsgQMaxUsed,
                   rtThreadMeas.tLPMsgQStat.lwMsgQTotalTime,
                   rtThreadMeas.tMultiMsgQStat.lwMsgCount,
                   rtThreadMeas.tMultiMsgQStat.dwQMaxMsgID,
                   rtThreadMeas.tMultiMsgQStat.dwMsgQMaxUsed,
                   rtThreadMeas.tMultiMsgQStat.lwMsgQTotalTime);

        for (WORD32 dwIndex1 = 0; dwIndex1 < BIT_NUM_OF_WORD32; dwIndex1++)
        {
            LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                       "%10d      HP:%15u     LP:%15u     Multi:%15u\n",
                       (1 << dwIndex1),
                       rtThreadMeas.tHPMsgQStat.adwStat[dwIndex1],
                       rtThreadMeas.tLPMsgQStat.adwStat[dwIndex1],
                       rtThreadMeas.tMultiMsgQStat.adwStat[dwIndex1]);
        }
    }
}


VOID COamApp::TimeOutDumpMsgMemPoolMeas(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("ThreadPool_MsgMemPool");

    for (WORD32 dwIndex = 0; dwIndex < m_ucThrdNum; dwIndex++)
    {
        T_ThreadMeasure &rtThreadMeas = m_atThrdMeasure[dwIndex];

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                   "Thrd:%d  \n",
                   rtThreadMeas.wThreadID);

        for (WORD32 dwIndex1 = 0;
             dwIndex1 < m_atThrdMeasure[dwIndex].tMsgMemStat.dwTypeNum;
             dwIndex1++)
        {
            LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                       "Type : %2d, Size : %9ld, UsedCount : %15lu, FreeCount : %15lu\n",
                       dwIndex1,
                       rtThreadMeas.tMsgMemStat.alwTotalSize[dwIndex1],
                       rtThreadMeas.tMsgMemStat.alwUsedCount[dwIndex1],
                       rtThreadMeas.tMsgMemStat.alwFreeCount[dwIndex1]);
        }
    }
}


VOID COamApp::TimeOutDumpMemPoolsMeas(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("BlockMemPool_Point");

    for (WORD32 dwIndex = 0; dwIndex < E_MP_MALLOC_POINT_NUM; dwIndex++)
    {
        if ( (0 == m_tMemPoolMeasure.alwMallocStat[dwIndex]) 
          && (0 == m_tMemPoolMeasure.alwFreeStat[dwIndex]))
        {
            continue ;
        }

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                   "Pool1 : Point = %4d, Malloc = %15lu, Free = %15lu\n",
                   dwIndex,
                   m_tMemPoolMeasure.alwMallocStat[dwIndex],
                   m_tMemPoolMeasure.alwFreeStat[dwIndex]);
    }

    CMemPools *pMemPool = CMemPools::GetInstance();
    if (NULL != pMemPool)
    {
        pMemPool->Dump();
    }
}


VOID COamApp::TimeOutMemMgrDump(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("COamApp::TimeOutMemMgrDump()");

    CMemMgr::GetInstance()->Dump();
}


VOID COamApp::TimeOutLogMeas(const VOID *pIn, WORD32 dwLen)
{
    Dump();
}


VOID COamApp::TimeOutLogFlush(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("COamApp::TimeOutLogFlush()");

    CLogThread *pThread = (CLogThread *)g_pLogThread;
    pThread->Flush();
}


VOID COamApp::TimeOutSwitch(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("COamApp::TimeOutSwitch()");

    WORD64 lwSeconds  = 0;
    WORD64 lwMicroSec = 0;
    WORD64 lwCycle    = 0;
    WORD32 dwResult   = 0;
    BYTE   ucPos      = (m_ucPos + 1) % CLogInfo::LOG_DUAL_FILE_NUM;

    g_pGlobalClock->GetTime(lwSeconds, lwMicroSec, lwCycle);

    dwResult = DoLogTask(lwSeconds, lwMicroSec, lwCycle, m_wSwitchPrd);
    if (SUCCESS != dwResult)
    {
        assert(0);
    }

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "lwSeconds : %lu, lwMicroSec : %lu, lwCycle : %lu, ucPos : %d, "
               "m_ucPos : %d, m_ucMeasMinute : %d, m_wSwitchPrd : %d\n",
               lwSeconds,
               lwMicroSec,
               lwCycle,
               ucPos,
               m_ucPos,
               m_ucMeasMinute,
               m_wSwitchPrd);

    /* 设置其它日志文件切换; 同时文件重新从开始的位置记录 */
    g_pLogger->SetPos(ucPos);
    m_ucPos = g_pLogger->GetPos();

    /* 切换日志线程及线程池的日志文件切换 */
    //SwitchLogFile(ucPos);
}


WORD32 COamApp::CalcTick(WORD64 lwSeconds, WORD64 lwMicroSec, WORD32 dwPeriod)
{
    WORD64 lwPrdSec  = dwPeriod * 60;     /* 将分钟转换为秒 */
    WORD32 dwPrdMill = dwPeriod * 60000;  /* 将分钟转换为毫秒 */

    time_t  tSec = lwSeconds;
    tm      tTM;
    LocalTime(&tSec, tTM);

    /* 当前时间(增加100ms延迟, 预留100ms)相对于0:0:0.000000的时间偏移 */
    WORD64 lwTimeShift = ((((tTM.tm_hour * 60) + tTM.tm_min) * 60) + tTM.tm_sec);
    WORD64 lwMultiple  = (((lwTimeShift + 1) / lwPrdSec) + 1) * lwPrdSec;
    WORD64 lwDiffSec   = lwMultiple - lwTimeShift;
    WORD64 lwDiffMicro = (lwDiffSec * 1000000) - (lwMicroSec % 1000000) - (LOG_PERIOD_AHEAD * 1000);
    WORD64 lwDiffMill  = lwDiffMicro / 1000;

    WORD32 dwTick = (lwDiffMill) ? ((WORD32)lwDiffMill) : dwPrdMill;

    dwTick = ((dwTick / dwPrdMill) <= 1) ? (dwTick) : (dwPrdMill + (dwTick % dwPrdMill));

    return dwTick;
}


/* 计算切换日志文件的时间点, 并启动绝对定时器 */
WORD32 COamApp::DoLogTask(WORD64 lwSeconds,
                          WORD64 lwMicroSec,
                          WORD64 lwCycle,
                          WORD32 dwSwitchPrd)
{
    WORD32 dwTick    = CalcTick(lwSeconds, lwMicroSec, dwSwitchPrd);
    WORD32 dwTimerID = RegisterTimer(dwTick,
                                     (CCBObject *)this,
                                     (PCBFUNC)(&COamApp::TimeOutSwitch),
                                     0);
    if (INVALID_DWORD == dwTimerID)
    {
        return FAIL;
    }

    return SUCCESS;
}


WORD32 COamApp::SwitchLogFile(BYTE ucPos)
{
    CLogInfo *pLogger = m_pOwner->GetLogger();
    if (pLogger != g_pLogger)
    {
        pLogger->SetPos(ucPos);
    }

    WORD32 dwThreadNum = g_pThreadPool->GetThreadNum();
    for (WORD32 dwIndex = 0; dwIndex < dwThreadNum; dwIndex++)
    {
        CBaseThread *pThread = (*g_pThreadPool)[dwIndex];
        if (NULL == pThread)
        {
            continue ;
        }

        pLogger = NULL;
        pLogger = pThread->GetLogger();
        if ((NULL != pLogger) && (pLogger != g_pLogger))
        {
            pLogger->SetPos(ucPos);
        }
    }
}


