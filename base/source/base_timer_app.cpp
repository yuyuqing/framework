

#include "base_mem_mgr.h"
#include "base_thread_log.h"
#include "base_timer_app.h"
#include "base_init_component.h"


DEFINE_APP(CTimerApp);


CTimerTreeNode::CTimerTreeNode ()
{
    m_dwTimerID = INVALID_DWORD;
    m_dwKey     = INVALID_DWORD;
    m_pTimer    = NULL;
    m_pCBFunc   = NULL;
}


CTimerTreeNode::~CTimerTreeNode()
{
    if (INVALID_DWORD != m_dwTimerID)
    {
        g_pTimerApp->KillTimer(m_dwTimerID);
    }

    m_dwTimerID = INVALID_DWORD;
    m_dwKey     = INVALID_DWORD;
    m_pTimer    = NULL;
    m_pCBFunc   = NULL;
}


WORD32 CTimerTreeNode::Initialize(WORD32          dwTimerID,
                                  WORD32          dwKey,
                                  CTimerNode     *pTimer,
                                  PTimerCallBack  pFunc)
{
    m_dwTimerID = dwTimerID;
    m_dwKey     = dwKey;
    m_pTimer    = pTimer;
    m_pCBFunc   = pFunc;

    return SUCCESS;
}


VOID CTimerTreeNode::TimeOut(const VOID *pIn, WORD32 dwLen)
{
    if (unlikely((NULL == pIn) || (dwLen != sizeof(T_TimerParam))))
    {
        return ;
    }

    m_dwTimerID = INVALID_DWORD;

    T_TimerParam *ptParam = (T_TimerParam *)pIn;

    if (NULL != m_pCBFunc)
    {
        (*m_pCBFunc) (m_dwKey, ptParam);
    }

    g_pTimerApp->m_tMeas.lwTimeOutCount++;
    g_pTimerApp->InnerDelete(m_dwKey);
}


CTimerApp::CTimerApp ()
    : CAppInterface(E_APP_TIMER)
{
    g_pTimerApp          = this;
    m_pRegistMemPoolFunc = NULL;
    m_lwSlotCount        = 0;
    m_wSFN               = INVALID_WORD;
    m_ucSlot             = INVALID_BYTE;
    m_ucMeasMinute       = CLogInfo::s_awPeriodMinute[E_LOG_PERIOD_05_MINUTE];

    memset((BYTE *)(&m_tMeas), 0x00, sizeof(m_tMeas));
}


CTimerApp::~CTimerApp()
{
}


WORD32 CTimerApp::InitApp()
{
    TRACE_STACK("CTimerApp::InitApp()");

    m_cTree.Initialize();

    m_pRegistMemPoolFunc = CInitList::GetInstance()->m_pRegistMemPoolFunc;

    RegisterProcessor(EV_TIMER_START_TIMER_ID,
                      (CCBObject *)this,
                      (PCBFUNC)(&CTimerApp::ProcStartTimer));

    RegisterProcessor(EV_TIMER_STOP_TIMER_ID,
                      (CCBObject *)this,
                      (PCBFUNC)(&CTimerApp::ProcStopTimer));

    RegisterProcessor(EV_TIMER_RESET_TIMER_ID,
                      (CCBObject *)this,
                      (PCBFUNC)(&CTimerApp::ProcResetTimer));

    RegisterProcessor(EV_TIMER_SLOT_TTI_ID,
                      (CCBObject *)this,
                      (PCBFUNC)(&CTimerApp::ProcSlotInd));

    return SUCCESS;
}


WORD32 CTimerApp::Start()
{
    TRACE_STACK("CTimerApp::Start()");

    m_ucMeasMinute = (BYTE)(g_pLogger->GetLogMeasure());

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "m_dwAppID : %d, m_dwThreadID : %d, m_ucMeasMinute : %d\n",
               m_dwAppID,
               m_dwThreadID,
               m_ucMeasMinute);

    return SUCCESS;
}


WORD32 CTimerApp::Init()
{
    TRACE_STACK("CTimerApp::Init()");

    if (NULL != m_pRegistMemPoolFunc)
    {
        (*m_pRegistMemPoolFunc) ();
    }

    WORD32 dwPeriod = m_ucMeasMinute;
    dwPeriod = dwPeriod * 60000;

    g_pOamApp->SendRegistCBMsg(E_TIMER_MEASURE_ID,
                               dwPeriod,
                               (CCBObject *)this,
                               (PCBFUNC)(&CTimerApp::DumpMeasure),
                               NULL);

    return SUCCESS;
}


WORD32 CTimerApp::DeInit()
{
    TRACE_STACK("CTimerApp::DeInit()");
    return SUCCESS;
}


WORD32 CTimerApp::Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen)
{
    TRACE_STACK("CTimerApp::Exit()");
    return SUCCESS;
}


/* 处理启动定时器消息 */
VOID CTimerApp::ProcStartTimer(const VOID *pIn, WORD32 dwLen)
{
    if (unlikely((NULL == pIn) || (sizeof(T_StartTimerMessage) != dwLen)))
    {
        return ;
    }

    m_tMeas.lwStartMsgCount++;

    T_StartTimerMessage *ptMsg = (T_StartTimerMessage *)pIn;

    WORD32 dwKey     = ptMsg->dwTimerID;
    WORD32 dwTimerID = INVALID_DWORD;

    /* 创建定时器节点 */
    dwTimerID = InnerCreate(dwKey,
                            ptMsg->lwMicroSec,
                            ptMsg->dwTick,
                            ptMsg->pFunc,
                            ptMsg->dwID,
                            ptMsg->dwExtendID,
                            ptMsg->dwTransID,
                            ptMsg->dwResvID,
                            ptMsg->pContext,
                            ptMsg->pUserData);
    if (INVALID_DWORD == dwTimerID)
    {
        m_tMeas.lwCreateFailCount++;
    }
}


/* 处理停止定时器消息 */
VOID CTimerApp::ProcStopTimer(const VOID *pIn, WORD32 dwLen)
{
    if (unlikely((NULL == pIn) || (sizeof(T_StopTimerMessage) != dwLen)))
    {
        return ;
    }

    m_tMeas.lwStopMsgCount++;

    T_StopTimerMessage *ptMsg = (T_StopTimerMessage *)pIn;

    WORD32 dwResult = InnerDelete(ptMsg->dwTimerID);
    if (SUCCESS != dwResult)
    {
        m_tMeas.lwNotFindCount++;
    }
}


/* 处理重置定时器消息 */
VOID CTimerApp::ProcResetTimer(const VOID *pIn, WORD32 dwLen)
{
    if (unlikely((NULL == pIn) || (sizeof(T_ResetTimerMessage) != dwLen)))
    {
        return ;
    }

    m_tMeas.lwResetMsgCount++;

    T_ResetTimerMessage *ptMsg = (T_ResetTimerMessage *)pIn;

    WORD32 dwKey = ptMsg->dwTimerID;

    CTimerTreeNode *pNode = m_cTree.Find(dwKey);
    if (NULL == pNode)
    {
        m_tMeas.lwResetNotFindCount++;
        return ;
    }

    CTimerNode *pTimer = pNode->GetTimer();
    if (NULL == pTimer)
    {
        assert(0);
    }

    CTimerRepo *pTimerRepo = m_pOwner->GetTimerRepo();
    pTimerRepo->Remove(pTimer);
    pTimer->UpdateClock(ptMsg->lwMicroSec, ptMsg->dwTick);
    pTimerRepo->Insert(pTimer);
}


/* 处理SlotInd消息 */
VOID CTimerApp::ProcSlotInd(const VOID *pIn, WORD32 dwLen)
{
    if (unlikely((NULL == pIn) || (sizeof(T_TimerSlotIndMessage) != dwLen)))
    {
        return ;
    }

    m_lwSlotCount++;
    m_tMeas.lwSlotMsgCount++;

    T_TimerSlotIndMessage *ptMsg = (T_TimerSlotIndMessage *)pIn;

    WORD16 wSFN   = ptMsg->wSFN;
    BYTE   ucSlot = ptMsg->ucSlot;

    if (INVALID_WORD == m_wSFN)
    {
        m_wSFN   = wSFN;
        m_ucSlot = ucSlot;
        return ;
    }

    /* 校验Slot消息是否存在丢弃 */
    if (0 == ucSlot)
    {
        if ( (ucSlot != ((m_ucSlot + 1) % 20))
          || (wSFN != ((m_wSFN + 1) % 1024)))
        {
            m_tMeas.lwSlotMsgMissCount++;

            FAST_LOG_PRINTF(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_DEBUG, TRUE,
                            "Missed SlotTTI Message; CurSFN : %d, CurSlot : %d, "
                            "m_wSFN : %d, m_ucSlot : %d\n",
                            wSFN,
                            ucSlot,
                            m_wSFN,
                            m_ucSlot);
        }
    }
    else
    {
        if ( (ucSlot != (m_ucSlot + 1))
          || (wSFN != m_wSFN))
        {
            m_tMeas.lwSlotMsgMissCount++;

            FAST_LOG_PRINTF(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_DEBUG, TRUE,
                            "Missed SlotTTI Message; CurSFN : %d, CurSlot : %d, "
                            "m_wSFN : %d, m_ucSlot : %d\n",
                            wSFN,
                            ucSlot,
                            m_wSFN,
                            m_ucSlot);
        }
    }

    m_wSFN   = wSFN;
    m_ucSlot = ucSlot;
}


/* 向CTimerApp发送启动定时器消息 */
WORD32 CTimerApp::CreateTimer(WORD32          dwTick,
                              PTimerCallBack  pFunc,
                              WORD32          dwID,
                              WORD32          dwExtendID,
                              WORD32          dwTransID,
                              WORD32          dwResvID,
                              VOID           *pContext,
                              VOID           *pUserData)
{
    m_tMeas.lwStartCount++;

    CMsgMemPool *pMemPool = m_pOwner->GetMsgMemPool();

    BYTE *pBuf = pMemPool->Malloc(E_MemSizeType_256);
    if (NULL == pBuf)
    {
        m_tMeas.lwStartMFail++;
        return INVALID_DWORD;
    }

    T_StartTimerMessage *ptMsg = (T_StartTimerMessage *)(pBuf + sizeof(T_BaseMessage));

    WORD64 lwMicroSec = 0;
    WORD64 lwCycle    = 0;
    WORD32 dwInnerID  = m_dwSelfRingID << 24;

    g_pGlobalClock->GetTime3(lwMicroSec, lwCycle);

    dwInnerID |= ((m_dwSelfTimerInnerID++) & 0xFFFFFF);

    ptMsg->lwMicroSec = lwMicroSec;
    ptMsg->dwTick     = dwTick;
    ptMsg->dwTimerID  = dwInnerID;
    ptMsg->pFunc      = pFunc;
    ptMsg->dwID       = dwID;
    ptMsg->dwExtendID = dwExtendID;
    ptMsg->dwTransID  = dwTransID;
    ptMsg->dwResvID   = dwResvID;
    ptMsg->pContext   = pContext;
    ptMsg->pUserData  = pUserData;

    WORD32 dwNum = m_pOwner->SendLPMsg(m_dwAppID,
                                       m_dwAppID,
                                       EV_TIMER_START_TIMER_ID,
                                       sizeof(T_StartTimerMessage),
                                       ptMsg);
    if (1 == dwNum)
    {
        return dwInnerID;
    }
    else
    {
        m_tMeas.lwStartQFail++;
        return INVALID_DWORD;
    }
}


/* 向CTimerApp发送停止定时器消息 */
WORD32 CTimerApp::RemoveTimer(WORD32 dwTimerID)
{
    m_tMeas.lwStopCount++;

    CMsgMemPool *pMemPool = m_pOwner->GetMsgMemPool();

    BYTE *pBuf = pMemPool->Malloc(E_MemSizeType_256);
    if (NULL == pBuf)
    {
        m_tMeas.lwStopMFail++;
        return FAIL;
    }

    T_StopTimerMessage *ptMsg = (T_StopTimerMessage *)(pBuf + sizeof(T_BaseMessage));

    ptMsg->dwTimerID = dwTimerID;

    WORD32 dwNum = m_pOwner->SendLPMsg(m_dwAppID,
                                       m_dwAppID,
                                       EV_TIMER_STOP_TIMER_ID,
                                       sizeof(T_StopTimerMessage),
                                       ptMsg);
    if (1 == dwNum)
    {
        return SUCCESS;
    }
    else
    {
        m_tMeas.lwStopQFail++;
        return FAIL;
    }
}


/* 向CTimerApp发送重置定时器消息 */
WORD32 CTimerApp::ResetTimer(WORD32 dwTimerID, WORD32 dwTick)
{
    m_tMeas.lwResetCount++;

    CMsgMemPool *pMemPool = m_pOwner->GetMsgMemPool();

    BYTE *pBuf = pMemPool->Malloc(E_MemSizeType_256);
    if (NULL == pBuf)
    {
        m_tMeas.lwResetMFail++;
        return FAIL;
    }

    T_ResetTimerMessage *ptMsg = (T_ResetTimerMessage *)(pBuf + sizeof(T_BaseMessage));

    WORD64 lwMicroSec = 0;
    WORD64 lwCycle    = 0;

    g_pGlobalClock->GetTime3(lwMicroSec, lwCycle);

    ptMsg->lwMicroSec = lwMicroSec;
    ptMsg->dwTick     = dwTick;
    ptMsg->dwTimerID  = dwTimerID;

    WORD32 dwNum = m_pOwner->SendLPMsg(m_dwAppID,
                                       m_dwAppID,
                                       EV_TIMER_RESET_TIMER_ID,
                                       sizeof(T_ResetTimerMessage),
                                       ptMsg);
    if (1 == dwNum)
    {
        return SUCCESS;
    }
    else
    {
        m_tMeas.lwResetQFail++;
        return FAIL;
    }
}


/* 向CTimerApp发送SlotTti消息 */
WORD32 CTimerApp::NotifySlotInd(WORD16 wSFN, BYTE ucSlot)
{
    m_tMeas.lwSlotCount++;

    CMsgMemPool *pMemPool = m_pOwner->GetMsgMemPool();

    BYTE *pBuf = pMemPool->Malloc(E_MemSizeType_256);
    if (NULL == pBuf)
    {
        m_tMeas.lwSlotMFail++;
        return FAIL;
    }

    T_TimerSlotIndMessage *ptMsg = (T_TimerSlotIndMessage *)(pBuf + sizeof(T_BaseMessage));

    ptMsg->wSFN   = wSFN;
    ptMsg->ucSlot = ucSlot;

    WORD32 dwNum = m_pOwner->SendHPMsg(m_dwAppID,
                                       m_dwAppID,
                                       EV_TIMER_SLOT_TTI_ID,
                                       sizeof(T_TimerSlotIndMessage),
                                       ptMsg);
    if (1 == dwNum)
    {
        return SUCCESS;
    }
    else
    {
        m_tMeas.lwSlotQFail++;
        return FAIL;
    }
}


/* 周期性输出系统维测 */
VOID CTimerApp::DumpMeasure(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("CTimerApp::DumpMeasure()");

    WORD64 lwStartCount = m_tMeas.lwStartCount.load(std::memory_order_relaxed);
    WORD64 lwStartMFail = m_tMeas.lwStartMFail.load(std::memory_order_relaxed);
    WORD64 lwStartQFail = m_tMeas.lwStartQFail.load(std::memory_order_relaxed);
    WORD64 lwStopCount  = m_tMeas.lwStopCount.load(std::memory_order_relaxed);
    WORD64 lwStopMFail  = m_tMeas.lwStopMFail.load(std::memory_order_relaxed);
    WORD64 lwStopQFail  = m_tMeas.lwStopQFail.load(std::memory_order_relaxed);
    WORD64 lwResetCount = m_tMeas.lwResetCount.load(std::memory_order_relaxed);
    WORD64 lwResetMFail = m_tMeas.lwResetMFail.load(std::memory_order_relaxed);
    WORD64 lwResetQFail = m_tMeas.lwResetQFail.load(std::memory_order_relaxed);
    WORD64 lwSlotCount  = m_tMeas.lwSlotCount.load(std::memory_order_relaxed);
    WORD64 lwSlotMFail  = m_tMeas.lwSlotMFail.load(std::memory_order_relaxed);
    WORD64 lwSlotQFail  = m_tMeas.lwSlotQFail.load(std::memory_order_relaxed);

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "StartNum : %lu, StartMFail : %lu, StartQFail : %lu, "
               "StopNum : %lu, StopMFail : %lu, StopQFail : %lu, "
               "ResetNum : %lu, ResetMFail : %lu, ResetQFail : %lu, "
               "SlotNum : %lu, SlotMFail : %lu, SlotQFail : %lu\n",
               lwStartCount, lwStartMFail, lwStartQFail,
               lwStopCount,  lwStopMFail,  lwStopQFail,
               lwResetCount, lwResetMFail, lwResetQFail,
               lwSlotCount,  lwSlotMFail,  lwSlotQFail);

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "StartMsgNum : %lu, CreateFail : %lu, "
               "StopMsgNum : %lu, StopNotFind : %lu, "
               "ResetMsgNum : %lu, ResetNotFind : %lu, "
               "SlotMsgNum : %lu, SlotMissNum : %lu, TimeOutNum : %lu\n",
               m_tMeas.lwStartMsgCount,
               m_tMeas.lwCreateFailCount,
               m_tMeas.lwStopMsgCount,
               m_tMeas.lwNotFindCount,
               m_tMeas.lwResetMsgCount,
               m_tMeas.lwResetNotFindCount,
               m_tMeas.lwSlotMsgCount,
               m_tMeas.lwSlotMsgMissCount,
               m_tMeas.lwTimeOutCount);
}


WORD32 CTimerApp::InnerDelete(WORD32 dwKey)
{
    /* 在删除树叶节点时会自动删除定时器节点 */
    return m_cTree.Remove(dwKey);
}


WORD32 CTimerApp::InnerCreate(WORD32          dwKey,
                              WORD64          lwMicroSec,
                              WORD32          dwTick,
                              PTimerCallBack  pFunc,
                              WORD32          dwID,
                              WORD32          dwExtendID,
                              WORD32          dwTransID,
                              WORD32          dwResvID,
                              VOID           *pContext,
                              VOID           *pUserData)
{
    CTimerRepo *pTimerRepo = m_pOwner->GetTimerRepo();

    WORD32 dwInstID  = INVALID_DWORD;

    CTimerNode     *pTimer = NULL;
    CTimerTreeNode *pNode  = m_cTree.Create(dwKey, dwInstID);
    if (NULL == pNode)
    {
        return FAIL;
    }

    WORD32 dwTimerID = pTimerRepo->RegisterTimer(dwTick,
                                                 &pTimer,
                                                 (CCBObject *)pNode,
                                                 (PCBFUNC)(&CTimerTreeNode::TimeOut),
                                                 dwID,
                                                 dwExtendID,
                                                 dwTransID,
                                                 dwResvID,
                                                 pContext,
                                                 pUserData);
    if (INVALID_DWORD == dwTimerID)
    {
        m_cTree.RemoveByInstID(dwInstID);
        return FAIL;
    }

    pNode->Initialize(dwTimerID, dwKey, pTimer, pFunc);

    return dwTimerID;
}


