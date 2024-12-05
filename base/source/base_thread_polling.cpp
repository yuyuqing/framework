

#include "base_thread_polling.h"


CPollingThread::CPollingThread (const T_ThreadParam &rtParam)
    : CBaseThread(rtParam)
{
    m_eType           = RT_THREAD;
    m_eClass          = E_THREAD_POLLING;
    m_dwCBNum         = rtParam.dwCBNum;
    m_dwPacketCBNum   = rtParam.dwPacketCBNum;
    m_dwMultiCBNum    = rtParam.dwMultiCBNum;
    m_dwTimerThresh   = rtParam.dwTimerThresh;
    m_dwCyclePer100NS = CYCLE_NUM_PER_1US / 10;
    m_dwAppNum        = 0;
    memset(m_atAppCtx, 0x00, sizeof(m_atAppCtx));
}


CPollingThread::~CPollingThread()
{
    m_dwCBNum         = 0;
    m_dwPacketCBNum   = 0;
    m_dwMultiCBNum    = 0;
    m_dwTimerThresh   = 0;
    m_dwCyclePer100NS = CYCLE_NUM_PER_1US / 10;
    m_dwAppNum        = 0;
    memset(m_atAppCtx, 0x00, sizeof(m_atAppCtx));
}


/* 创建线程实例后执行初始化(在主线程栈空间执行) */
WORD32 CPollingThread::Initialize()
{
    TRACE_STACK("CPollingThread::Initialize()");

    WORD32 dwResult = CBaseThread::Initialize();
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    m_MsgRingH.Initialize();
    m_MsgRingL.Initialize();
    m_MsgRingD.Initialize();
    m_EventMap.Initialize();

    return SUCCESS;
}


/* 在创建线程实例时加载App */
WORD32 CPollingThread::LoadApp(T_AppInfo *ptAppInfo)
{
    TRACE_STACK("CPollingThread::LoadApp()");

    if ((m_dwAppNum >= MAX_APP_NUM_PER_THREAD) || (NULL == ptAppInfo))
    {
        return FAIL;
    }

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_DEBUG, TRUE, 
               "m_dwThreadID : %d; AppInfo : "
               "dwAppID = %d, dwThreadID = %d, dwStartEventID = %d\n",
               m_dwThreadID,
               ptAppInfo->dwAppID,
               ptAppInfo->dwThreadID,
               ptAppInfo->dwStartEventID);

    m_atAppCtx[m_dwAppNum].pAppInfo   = ptAppInfo;
    m_atAppCtx[m_dwAppNum].bTimerFlag = FALSE;

    m_dwAppNum++;

    return SUCCESS;
}


VOID CPollingThread::Run(CBaseThread *pArg)
{
    TRACE_STACK("CPollingThread::Run()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_DEBUG, TRUE, 
               "m_dwSelfThreadID : %d, m_dwThreadID : %d\n", 
               m_dwSelfThreadID,
               m_dwThreadID);

    /* 注册线程Cancel处理消息 */
    Register(EV_BASE_THREAD_CANCEL_ID, 
             (CCBObject *)this, 
             (PCBFUNC)(&CPollingThread::ProcCancelMsg));

    /* 注册ResetApp处理消息 */
    Register(EV_BASE_THREAD_RESET_APP_ID, 
             (CCBObject *)this, 
             (PCBFUNC)(&CPollingThread::ProcResetAppMsg));

    StartApp();

    DoRun();
}


WORD32 CPollingThread::Register(WORD32     dwMsgID,
                                CCBObject *pObj,
                                PCBFUNC    pFunc)
{
    if ((NULL == pObj) || (NULL == pFunc))
    {
        return FAIL;
    }

    return m_EventMap.RegisterProcessor(dwMsgID, pObj, pFunc);
}


VOID CPollingThread::ProcCancelMsg(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("CPollingThread::ProcCancelMsg()");

    SetCancel(TRUE);
}


VOID CPollingThread::ProcResetAppMsg(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("CPollingThread::ProcResetAppMsg()");

    T_BaseMessage *ptMsg = (T_BaseMessage *)pIn;

    T_AppContext *ptAppCtx = FindAppCtx(ptMsg->dwDstAppID);
    if (NULL == ptAppCtx)
    {
        return ;
    }

    BYTE          *pAddr  = ptAppCtx->pAppInfo->aucAppState;
    CAppState     *pState = ptAppCtx->pAppInfo->pAppState;
    CAppInterface *pApp   = pState->GetAppInst();

    if (APP_STATE_EXIT != pState->GetState())
    {
        return ;
    }

    (*(ptAppCtx->pAppInfo->pResetFunc))(ptAppCtx->pAppInfo->pMem);

    pApp->Initialize();
    pApp->PreInit(this, ptAppCtx->pAppInfo);

    /* 恢复状态 */
    pState = new (pAddr) CAppState(pApp);
}


CAppInterface * CPollingThread::GetAppInst(E_AppClass eClass, WORD32 dwAssocID)
{
    CAppInterface *pAppInst = NULL;

    for (WORD32 dwIndex = 0; dwIndex < m_dwAppNum; dwIndex++)
    {
        pAppInst = m_atAppCtx[dwIndex].pAppInfo->pAppState->GetAppInst();

        if (pAppInst->IsAssociate(eClass, dwAssocID))
        {
            return pAppInst;
        }
    }

    return NULL;
}


VOID CPollingThread::Dump()
{
    TRACE_STACK("CPollingThread::Dump()");

    CBaseThread::Dump();

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "m_dwCBNum : %d, m_dwPacketCBNum : %d, "
               "m_dwMultiCBNum : %d, m_dwTimerThresh : %d, m_dwAppNum : %d\n",
               m_dwCBNum,
               m_dwPacketCBNum,
               m_dwMultiCBNum,
               m_dwTimerThresh,
               m_dwAppNum);

    for (WORD32 dwIndex = 0; dwIndex < m_dwAppNum; dwIndex++)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                   "bTimerFlag : %d, dwAppID : %3d, dwMemSize : %9d, "
                   "bAssocFlag : %d, dwAssocNum : %d\n",
                   m_atAppCtx[dwIndex].bTimerFlag,
                   m_atAppCtx[dwIndex].pAppInfo->dwAppID,
                   m_atAppCtx[dwIndex].pAppInfo->dwMemSize,
                   m_atAppCtx[dwIndex].pAppInfo->bAssocFlag,
                   m_atAppCtx[dwIndex].pAppInfo->dwAssocNum);        
    }
}


/* 线程启动后启动App(在线程栈空间执行, 在进入线程主循环之前调用) */
WORD32 CPollingThread::StartApp()
{
    TRACE_STACK("CPollingThread::StartApp()");

    for (WORD32 dwIndex = 0; dwIndex < m_dwAppNum; dwIndex++)
    {
        m_atAppCtx[dwIndex].pAppInfo->pAppState->Start();
    }

    return SUCCESS;
}


VOID CPollingThread::DoRun()
{
    TRACE_STACK("CPollingThread::DoRun()");

    WORD32 dwProcHNum       = 0;
    WORD32 dwProcLNum       = 0;
    WORD32 dwProcDNum       = 0;
    WORD32 dwRemainLNum     = 0;
    WORD32 dwRemainDNum     = 0;
    WORD64 lwTimeStampStart = 0;
    WORD64 lwTimeStampProcH = 0;
    WORD64 lwTimeStampProcL = 0;
    WORD64 lwTimeStampProcD = 0;
    WORD64 lwTimeStampEnd   = 0;

    while (TRUE)
    {
        lwTimeStampStart  = GetCycle();
        m_dwCyclePer100NS = g_pGlobalClock->GetCyclePer100NS();

        /* 优先处理完所有高优先级队列中的消息 */
        dwProcHNum = m_MsgRingH.Dequeue(this, &CPollingThread::ProcMessageHP);

        lwTimeStampProcH = GetCycle();

        if (likely(m_MsgRingL.isEmpty()))
        {
            dwRemainLNum = 0;
            dwProcLNum   = 0;
        }
        else
        {
            /* 每次循环固定从低优先级队列中取指定数量的消息 */
            dwRemainLNum = m_MsgRingL.Dequeue(m_dwCBNum, 
                                              this, 
                                              &CPollingThread::ProcMessageLP, 
                                              dwProcLNum);
        }

        lwTimeStampProcL = GetCycle();

        /* 从数据队列接收用户面报文 */
        dwRemainDNum = m_MsgRingD.Dequeue(m_dwPacketCBNum, dwProcDNum);

        lwTimeStampProcD = GetCycle();

        /* 轮询处理 */
        Polling();

        lwTimeStampEnd = GetCycle();

        Statistic(dwProcHNum, dwProcLNum, dwProcDNum, 
                  dwRemainLNum, dwRemainDNum, 
                  lwTimeStampStart, 
                  lwTimeStampProcH, lwTimeStampProcL, lwTimeStampProcD, 
                  lwTimeStampEnd);

        /* 线程退出 */
        if (unlikely(isCancel()))
        {
            LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_DEBUG, TRUE, 
                       "Thread Cancel\n"); 
            break ;
        }
    }
}


