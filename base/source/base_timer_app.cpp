

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

    memset((BYTE *)(&m_tMeas),           0x00, sizeof(m_tMeas));
    memset((BYTE *)(&m_tPhyRecvMeasure), 0x00, sizeof(m_tPhyRecvMeasure));
    memset((BYTE *)(&m_tClAppMeasure),   0x00, sizeof(m_tClAppMeasure));
    memset((BYTE *)(&m_tSchAppMeasure),  0x00, sizeof(m_tSchAppMeasure));
    memset((BYTE *)(&m_tUlRecvMeasure),  0x00, sizeof(m_tUlRecvMeasure));
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

    /* 向NGP内存池注册(原因 : 定时器回调业务函数时, 需要从NGP的内存池中申请内存) */
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

    RemoveProcessor(EV_TIMER_START_TIMER_ID);
    RemoveProcessor(EV_TIMER_STOP_TIMER_ID);
    RemoveProcessor(EV_TIMER_RESET_TIMER_ID);
    RemoveProcessor(EV_TIMER_SLOT_TTI_ID);

    return SUCCESS;
}


WORD32 CTimerApp::Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen)
{
    TRACE_STACK("CTimerApp::Exit()");

    /* 定时器尚未清除(对FT场景存在影响, 正常业务场景无影响) */

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
                            (WORD64)(wSFN),
                            (WORD64)(ucSlot),
                            (WORD64)(m_wSFN),
                            (WORD64)(m_ucSlot));
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
                            (WORD64)(wSFN),
                            (WORD64)(ucSlot),
                            (WORD64)(m_wSFN),
                            (WORD64)(m_ucSlot));
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

    WORD32 dwTick   = COamApp::TIMER_TIMEOUT_INTERVAL;
    WORD32 dwProcID = m_pOwner->GetProcID();

    RegisterTimer(dwTick,
                  (CCBObject *)this,
                  (PCBFUNC)(&CTimerApp::TimeOutDumpTimerMeas),
                  0);

    switch (dwProcID)
    {
    case E_PROC_DU :
        {
            RegisterTimer(2 * dwTick,
                          (CCBObject *)this,
                          (PCBFUNC)(&CTimerApp::TimeOutDumpPhyRecv),
                          0);

            RegisterTimer(3 * dwTick,
                          (CCBObject *)this,
                          (PCBFUNC)(&CTimerApp::TimeOutDumpClApp),
                          0);

            RegisterTimer(4 * dwTick,
                          (CCBObject *)this,
                          (PCBFUNC)(&CTimerApp::TimeOutDumpUlRecv),
                          0);

            RegisterTimer(5 * dwTick,
                          (CCBObject *)this,
                          (PCBFUNC)(&CTimerApp::TimeOutDumpSchApp),
                          0);
        }
        break ;

    default :
        break ;
    }
}


VOID CTimerApp::TimeOutDumpTimerMeas(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("CTimerApp::DumpTimerMeas()");

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


VOID CTimerApp::TimeOutDumpPhyRecv(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("CTimerApp::TimeOutDumpPhyRecv()");

    T_PhyRecvMeasure        tMeasure;
    T_PhyRecvAtomicMeasure &rtMeasAtomic = m_tPhyRecvMeasure;

    for (WORD32 dwIndex = 0; dwIndex < MAX_CELL_PER_GNB; dwIndex++)
    {
        T_PhyRecvCellMeasure       *ptCell       = &(tMeasure.atCell[dwIndex]);
        T_PhyRecvCellAtomicMeasure *ptCellAtomic = &(rtMeasAtomic.atCell[dwIndex]);

        ptCell->lwSlotIndNum    = ptCellAtomic->lwSlotIndNum.load(std::memory_order_relaxed);
        ptCell->lwTotalTimeUsed = ptCellAtomic->lwTotalTimeUsed.load(std::memory_order_relaxed);

        for (WORD32 dwIndex1 = 0; dwIndex1 < BIT_NUM_OF_WORD32; dwIndex1++)
        {
            ptCell->alwJitter[dwIndex1] = ptCellAtomic->alwJitter[dwIndex1].load(std::memory_order_relaxed);
        }

        for (WORD32 dwIndex1 = 0; dwIndex1 < SLOT_NUM_PER_HALF_SFN; dwIndex1++)
        {
            T_PhyRecvCellMeasItem       *ptItem       = &(ptCell->atMeas[dwIndex1]);
            T_PhyRecvCellAtomicMeasItem *ptItemAtomic = &(ptCellAtomic->atMeas[dwIndex1]);

            ptItem->lwSchInvalidPdcchNum = ptItemAtomic->lwSchInvalidPdcchNum.load(std::memory_order_relaxed);
            ptItem->lwSchInvalidPdschNum = ptItemAtomic->lwSchInvalidPdschNum.load(std::memory_order_relaxed);
            ptItem->lwDlActvUeNum        = ptItemAtomic->lwDlActvUeNum.load(std::memory_order_relaxed);
            ptItem->lwUlPdcchNum         = ptItemAtomic->lwUlPdcchNum.load(std::memory_order_relaxed);
            ptItem->lwDlPdcchNum         = ptItemAtomic->lwDlPdcchNum.load(std::memory_order_relaxed);
            ptItem->lwCsiRsNum           = ptItemAtomic->lwCsiRsNum.load(std::memory_order_relaxed);
            ptItem->lwBchNum             = ptItemAtomic->lwBchNum.load(std::memory_order_relaxed);
            ptItem->lwPduNum             = ptItemAtomic->lwPduNum.load(std::memory_order_relaxed);
            ptItem->lwSibNum             = ptItemAtomic->lwSibNum.load(std::memory_order_relaxed);
            ptItem->lwPagingNum          = ptItemAtomic->lwPagingNum.load(std::memory_order_relaxed);
            ptItem->lwMsg2Num            = ptItemAtomic->lwMsg2Num.load(std::memory_order_relaxed);
            ptItem->lwMsg4Num            = ptItemAtomic->lwMsg4Num.load(std::memory_order_relaxed);
            ptItem->lwTBNum              = ptItemAtomic->lwTBNum.load(std::memory_order_relaxed);
            ptItem->lwTBSize             = ptItemAtomic->lwTBSize.load(std::memory_order_relaxed);
            ptItem->lwPuschNum           = ptItemAtomic->lwPuschNum.load(std::memory_order_relaxed);
            ptItem->lwPucchFrmt1Num      = ptItemAtomic->lwPucchFrmt1Num.load(std::memory_order_relaxed);
            ptItem->lwHqCsiSrNum         = ptItemAtomic->lwHqCsiSrNum.load(std::memory_order_relaxed);
            ptItem->lwCsiSrNum           = ptItemAtomic->lwCsiSrNum.load(std::memory_order_relaxed);
            ptItem->lwHqCsiNum           = ptItemAtomic->lwHqCsiNum.load(std::memory_order_relaxed);
            ptItem->lwHqSrNum            = ptItemAtomic->lwHqSrNum.load(std::memory_order_relaxed);
            ptItem->lwHarqNum            = ptItemAtomic->lwHarqNum.load(std::memory_order_relaxed);
            ptItem->lwCsiNum             = ptItemAtomic->lwCsiNum.load(std::memory_order_relaxed);
            ptItem->lwSrNum              = ptItemAtomic->lwSrNum.load(std::memory_order_relaxed);
            ptItem->lwSrsNum             = ptItemAtomic->lwSrsNum.load(std::memory_order_relaxed);
            ptItem->lwRachNum            = ptItemAtomic->lwRachNum.load(std::memory_order_relaxed);
        }
    }

    for (WORD32 dwIndex = 0; dwIndex < MAX_CELL_PER_GNB; dwIndex++)
    {
        T_PhyRecvCellMeasure *ptCell = &(tMeasure.atCell[dwIndex]);

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                   "CellID : %d, lwSlotIndNum : %lu, lwTotalTimeUsed(0.1us) : %lu\n",
                   dwIndex,
                   ptCell->lwSlotIndNum,
                   ptCell->lwTotalTimeUsed);

        for (WORD32 dwIndex1 = 0; dwIndex1 < BIT_NUM_OF_WORD32; dwIndex1++)
        {
            LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                       "%10d   %15lu\n",
                       (1 << dwIndex1),
                       ptCell->alwJitter[dwIndex1]);
        }

        {
            TRACE_STACK("PDCCH    InvalidPdcchNum  InvalidPdschNum  "
                        "ActvUeNum       UlPdcchNum       "
                        "DlPdcchNum         CsiRsNum    ");

            for (WORD32 dwIndex1 = 0; dwIndex1 < SLOT_NUM_PER_HALF_SFN; dwIndex1++)
            {
                T_PhyRecvCellMeasItem *ptItem = &(ptCell->atMeas[dwIndex1]);
                LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                           "%d  %15lu  %15lu  %15lu  %15lu  %15lu  %15lu\n",
                           dwIndex1,
                           ptItem->lwSchInvalidPdcchNum,
                           ptItem->lwSchInvalidPdschNum,
                           ptItem->lwDlActvUeNum,
                           ptItem->lwUlPdcchNum,
                           ptItem->lwDlPdcchNum,
                           ptItem->lwCsiRsNum);
            }
        }

        {
            TRACE_STACK("PDSCH       BchNum           PduNum           "
                        "SibNum        PagingNum          Msg2Num          "
                        "Msg4Num            TBNum           TBSize     ");

            for (WORD32 dwIndex1 = 0; dwIndex1 < SLOT_NUM_PER_HALF_SFN; dwIndex1++)
            {
                T_PhyRecvCellMeasItem *ptItem = &(ptCell->atMeas[dwIndex1]);
                LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                           "%d  %15lu  %15lu  %15lu  %15lu  "
                           "%15lu  %15lu  %15lu  %15lu\n",
                           dwIndex1,
                           ptItem->lwBchNum,
                           ptItem->lwPduNum,
                           ptItem->lwSibNum,
                           ptItem->lwPagingNum,
                           ptItem->lwMsg2Num,
                           ptItem->lwMsg4Num,
                           ptItem->lwTBNum,
                           ptItem->lwTBSize);
            }
        }

        {
            TRACE_STACK("PUSCH     PuschNum    PucchFrmt1Num       "
                        "HqCsiSrNum         CsiSrNum         HqCsiNum          "
                        "HqSrNum          HarqNum           CsiNum            "
                        "SrNum           SrsNum          RachNum     ");

            for (WORD32 dwIndex1 = 0; dwIndex1 < SLOT_NUM_PER_HALF_SFN; dwIndex1++)
            {
                T_PhyRecvCellMeasItem *ptItem = &(ptCell->atMeas[dwIndex1]);
                LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                           "%d  %15lu  %15lu  %15lu  %15lu  %15lu  %15lu  "
                           "%15lu  %15lu  %15lu  %15lu  %15lu\n",
                           dwIndex1,
                           ptItem->lwPuschNum,
                           ptItem->lwPucchFrmt1Num,
                           ptItem->lwHqCsiSrNum,
                           ptItem->lwCsiSrNum,
                           ptItem->lwHqCsiNum,
                           ptItem->lwHqSrNum,
                           ptItem->lwHarqNum,
                           ptItem->lwCsiNum,
                           ptItem->lwSrNum,
                           ptItem->lwSrsNum,
                           ptItem->lwRachNum);
            }
        }
    }
}


VOID CTimerApp::TimeOutDumpClApp(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("CTimerApp::TimeOutDumpClApp()");

    T_ClAppDlFapiMeasure        tMeasure;
    T_ClAppDlFapiAtomicMeasure &rtMeasAtomic = m_tClAppMeasure;

    for (WORD32 dwIndex = 0; dwIndex < MAX_CELL_PER_GNB; dwIndex++)
    {
        T_ClAppDlFapiCellMeasure       *ptCell       = &(tMeasure.atCell[dwIndex]);
        T_ClAppDlFapiCellAtomicMeasure *ptCellAtomic = &(rtMeasAtomic.atCell[dwIndex]);

        ptCell->lwSlotIndNum     = ptCellAtomic->lwSlotIndNum.load(std::memory_order_relaxed);
        ptCell->lwDiscontinueNum = ptCellAtomic->lwDiscontinueNum.load(std::memory_order_relaxed);
        ptCell->lwTotalTimeUsed  = ptCellAtomic->lwTotalTimeUsed.load(std::memory_order_relaxed);

        for (WORD32 dwIndex1 = 0; dwIndex1 < BIT_NUM_OF_WORD32; dwIndex1++)
        {
            ptCell->alwJitter[dwIndex1] = ptCellAtomic->alwJitter[dwIndex1].load(std::memory_order_relaxed);
        }

        for (WORD32 dwIndex1 = 0; dwIndex1 < SLOT_NUM_PER_HALF_SFN; dwIndex1++)
        {
            T_ClAppMeasItem       &rtItem       = ptCell->atMeas[dwIndex1];
            T_ClAppAtomicMeasItem &rtItemAtomic = ptCellAtomic->atMeas[dwIndex1];

            rtItem.dwDlCfgMsgLen  = rtItemAtomic.dwDlCfgMsgLen.load(std::memory_order_relaxed);
            rtItem.dwTxReqMsgLen  = rtItemAtomic.dwTxReqMsgLen.load(std::memory_order_relaxed);
            rtItem.dwUlDciMsgLen  = rtItemAtomic.dwUlDciMsgLen.load(std::memory_order_relaxed);
            rtItem.dwUlCfgMsgLen  = rtItemAtomic.dwUlCfgMsgLen.load(std::memory_order_relaxed);

            rtItem.lwBchNum       = rtItemAtomic.lwBchNum.load(std::memory_order_relaxed);
            rtItem.lwDlSchNum     = rtItemAtomic.lwDlSchNum.load(std::memory_order_relaxed);
            rtItem.lwDlDciNum     = rtItemAtomic.lwDlDciNum.load(std::memory_order_relaxed);
            rtItem.lwCsiRsNum     = rtItemAtomic.lwCsiRsNum.load(std::memory_order_relaxed);
            rtItem.lwTxPduNum     = rtItemAtomic.lwTxPduNum.load(std::memory_order_relaxed);
            rtItem.lwUlDciPduNum  = rtItemAtomic.lwUlDciPduNum.load(std::memory_order_relaxed);
            rtItem.lwPucchNum     = rtItemAtomic.lwPucchNum.load(std::memory_order_relaxed);
            rtItem.lwPuschNum     = rtItemAtomic.lwPuschNum.load(std::memory_order_relaxed);
            rtItem.lwSrsNum       = rtItemAtomic.lwSrsNum.load(std::memory_order_relaxed);
            rtItem.lwRachNum      = rtItemAtomic.lwRachNum.load(std::memory_order_relaxed);
        }
    }

    for (WORD32 dwIndex = 0; dwIndex < MAX_CELL_PER_GNB; dwIndex++)
    {
        T_ClAppDlFapiCellMeasure *ptCell = &(tMeasure.atCell[dwIndex]);

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                   "CellID : %d; lwSlotIndNum : %lu, lwDiscontinueNum : %lu, lwTotalTimeUsed : %lu\n",
                   dwIndex,
                   ptCell->lwSlotIndNum,
                   ptCell->lwDiscontinueNum,
                   ptCell->lwTotalTimeUsed);

        for (WORD32 dwIndex1 = 0; dwIndex1 < BIT_NUM_OF_WORD32; dwIndex1++)
        {
            LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                       "%10d   %15lu\n",
                       (1 << dwIndex1),
                       ptCell->alwJitter[dwIndex1]);
        }

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                   "Slot    DlCfgLen    TxReqLen    UlDciLen     "
                   "UlCfgLen       BchNum         DlSchNum         "
                   "DlDciNum         CsiRsNum         TxPduNum      "
                   "UlDciPduNum         PucchNum         "
                   "PuschNum           SrsNum          RachNum\n");

        for (WORD32 dwIndex1 = 0; dwIndex1 < SLOT_NUM_PER_HALF_SFN; dwIndex1++)
        {
            T_ClAppMeasItem &rtItem = ptCell->atMeas[dwIndex1];

            LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                       "%d  %10u  %10u  %10u  %10u  "
                       "%15lu  %15lu  %15lu  %15lu  %15lu  "
                       "%15lu  %15lu  %15lu  %15lu  %15lu\n",
                       dwIndex1,
                       rtItem.dwDlCfgMsgLen,
                       rtItem.dwTxReqMsgLen,
                       rtItem.dwUlDciMsgLen,
                       rtItem.dwUlCfgMsgLen,
                       rtItem.lwBchNum,
                       rtItem.lwDlSchNum,
                       rtItem.lwDlDciNum,
                       rtItem.lwCsiRsNum,
                       rtItem.lwTxPduNum,
                       rtItem.lwUlDciPduNum,
                       rtItem.lwPucchNum,
                       rtItem.lwPuschNum,
                       rtItem.lwSrsNum,
                       rtItem.lwRachNum);
        }
    }
}


VOID CTimerApp::TimeOutDumpUlRecv(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("CTimerApp::TimeOutDumpUlRecv()");

    T_UlRecvMeasure        tMeasure;
    T_UlRecvAtomicMeasure &rtMeasAtomic = m_tUlRecvMeasure;

    for (WORD32 dwIndex = 0; dwIndex < MAX_CELL_PER_GNB; dwIndex++)
    {
        T_UlRecvCellMeas       *ptCell       = &(tMeasure.atCell[dwIndex]);
        T_UlRecvCellAtomicMeas *ptCellAtomic = &(rtMeasAtomic.atCell[dwIndex]);

        for (WORD32 dwIndex1 = 0; dwIndex1 < SLOT_NUM_PER_HALF_SFN; dwIndex1++)
        {
            T_UlRecvCellMeasItem       *ptItem       = &(ptCell->atMeas[dwIndex1]);
            T_UlRecvCellAtomicMeasItem *ptItemAtomic = &(ptCellAtomic->atMeas[dwIndex1]);

            ptItem->lwUciRcvNum  = ptItemAtomic->lwUciRcvNum.load(std::memory_order_relaxed);
            ptItem->lwCrcRcvNum  = ptItemAtomic->lwCrcRcvNum.load(std::memory_order_relaxed);
            ptItem->lwCrcSuccNum = ptItemAtomic->lwCrcSuccNum.load(std::memory_order_relaxed);
            ptItem->lwCrcFailNum = ptItemAtomic->lwCrcFailNum.load(std::memory_order_relaxed);
            ptItem->lwSrsRcvNum  = ptItemAtomic->lwSrsRcvNum.load(std::memory_order_relaxed);
            ptItem->lwRachRcvNum = ptItemAtomic->lwRachRcvNum.load(std::memory_order_relaxed);

            for (WORD32 dwIndex2 = 0; dwIndex2 < SLOT_NUM_PER_HALF_SFN; dwIndex2++)
            {
                ptItem->alwUeUciSlot[dwIndex2]  = ptItemAtomic->alwUeUciSlot[dwIndex2].load(std::memory_order_relaxed);
                ptItem->alwUeCrcSlot[dwIndex2]  = ptItemAtomic->alwUeCrcSlot[dwIndex2].load(std::memory_order_relaxed);
                ptItem->alwUeSrsSlot[dwIndex2]  = ptItemAtomic->alwUeSrsSlot[dwIndex2].load(std::memory_order_relaxed);
                ptItem->alwUeRachSlot[dwIndex2] = ptItemAtomic->alwUeRachSlot[dwIndex2].load(std::memory_order_relaxed);
            }
        }
    }

    for (WORD32 dwIndex = 0; dwIndex < MAX_CELL_PER_GNB; dwIndex++)
    {
        T_UlRecvCellMeas *ptCell = &(tMeasure.atCell[dwIndex]);

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                   "CellID : %d\n",
                   dwIndex);

        TRACE_STACK("CurSlot    UciRcvNum      CrcRcvNum        "
                    "CrcSuccNum        CrcFailNum        "
                    "SrsRcvNum         RachRcvNum        "
                    "UciSlot7         UciSlot8         UciSlot9         "
                    "CrcSlot7         CrcSlot8         CrcSlot9         "
                    "SrsSlot7         SrsSlot8         SrsSlot9         "
                    "RachSlot7        RachSlot8        RachSlot9    ");

        for (WORD32 dwIndex1 = 0; dwIndex1 < SLOT_NUM_PER_HALF_SFN; dwIndex1++)
        {
            T_UlRecvCellMeasItem *ptItem = &(ptCell->atMeas[dwIndex1]);

            LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                       "%d  %15lu  %15lu  %15lu  %15lu  %15lu  %15lu  "
                       "%15lu  %15lu  %15lu  %15lu  %15lu  %15lu  "
                       "%15lu  %15lu  %15lu  %15lu  %15lu  %15lu\n",
                       dwIndex1,
                       ptItem->lwUciRcvNum,
                       ptItem->lwCrcRcvNum,
                       ptItem->lwCrcSuccNum,
                       ptItem->lwCrcFailNum,
                       ptItem->lwSrsRcvNum,
                       ptItem->lwRachRcvNum,
                       ptItem->alwUeUciSlot[7],
                       ptItem->alwUeUciSlot[8],
                       ptItem->alwUeUciSlot[9],
                       ptItem->alwUeCrcSlot[7],
                       ptItem->alwUeCrcSlot[8],
                       ptItem->alwUeCrcSlot[9],
                       ptItem->alwUeSrsSlot[7],
                       ptItem->alwUeSrsSlot[8],
                       ptItem->alwUeSrsSlot[9],
                       ptItem->alwUeRachSlot[7],
                       ptItem->alwUeRachSlot[8],
                       ptItem->alwUeRachSlot[9]);
        }
    }
}


VOID CTimerApp::TimeOutDumpSchApp(const VOID *pIn, WORD32 dwLen)
{
    TRACE_STACK("CTimerApp::TimeOutDumpSchApp()");

    T_SchAppMeasure        tMeasure;
    T_SchAppAtomicMeasure &rtMeasAtomic = m_tSchAppMeasure;

    for (WORD32 dwIndex = 0; dwIndex < MAX_CELL_PER_GNB; dwIndex++)
    {
        T_SchAppCellMeasure       *ptCell       = &(tMeasure.atCell[dwIndex]);
        T_SchAppCellAtomicMeasure *ptCellAtomic = &(rtMeasAtomic.atCell[dwIndex]);

        ptCell->lwCcchReqNum = ptCellAtomic->lwCcchReqNum.load(std::memory_order_relaxed);

        for (WORD32 dwIndex1 = 0; dwIndex1 < SLOT_NUM_PER_HALF_SFN; dwIndex1++)
        {
            T_SchAppCellMeasItem       *ptItem       = &(ptCell->atMeas[dwIndex1]);
            T_SchAppCellAtomicMeasItem *ptItemAtomic = &(ptCellAtomic->atMeas[dwIndex1]);

            ptItem->lwSlotIndNum = ptItemAtomic->lwSlotIndNum.load(std::memory_order_relaxed);
            ptItem->lwInvalidNum = ptItemAtomic->lwInvalidNum.load(std::memory_order_relaxed);
            ptItem->lwUciIndNum  = ptItemAtomic->lwUciIndNum.load(std::memory_order_relaxed);
            ptItem->lwCrcIndNum  = ptItemAtomic->lwCrcIndNum.load(std::memory_order_relaxed);
            ptItem->lwSrsIndNum  = ptItemAtomic->lwSrsIndNum.load(std::memory_order_relaxed);

            for (WORD32 dwIndex2 = 0; dwIndex2 < MAX_DIFF_SLOT_SCH2RLC; dwIndex2++)
            {
                ptItem->alwDiffTtiNum[dwIndex2] = ptItemAtomic->alwDiffTtiNum[dwIndex2].load(std::memory_order_relaxed);
            }

            for (WORD32 dwIndex2 = 0; dwIndex2 < SLOT_NUM_PER_HALF_SFN; dwIndex2++)
            {
                ptItem->alwDiffUciNum[dwIndex2] = ptItemAtomic->alwDiffUciNum[dwIndex2].load(std::memory_order_relaxed);
                ptItem->alwDiffCrcNum[dwIndex2] = ptItemAtomic->alwDiffCrcNum[dwIndex2].load(std::memory_order_relaxed);
                ptItem->alwDiffSrsNum[dwIndex2] = ptItemAtomic->alwDiffSrsNum[dwIndex2].load(std::memory_order_relaxed);
            }
        }
    }

    for (WORD32 dwIndex = 0; dwIndex < MAX_CELL_PER_GNB; dwIndex++)
    {
        T_SchAppCellMeasure *ptCell = &(tMeasure.atCell[dwIndex]);

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                   "CellID : %d,  lwCcchReqNum : %lu\n",
                   dwIndex,
                   ptCell->lwCcchReqNum);
        TRACE_STACK("CurSlot    SlotIndNum     InvalidNum       "
                    "UciIndNum        "
                    "CrcIndNum        SrsIndNum         TtiDiff0         "
                    "TtiDiff1         TtiDiff2         TtiDiff3         "
                    "UciDiff7         UciDiff8         UciDiff9         "
                    "CrcDiff7         CrcDiff8         CrcDiff9         "
                    "SrsDiff7         SrsDiff8         SrsDiff9         ");

        for (WORD32 dwIndex1 = 0; dwIndex1 < SLOT_NUM_PER_HALF_SFN; dwIndex1++)
        {
            T_SchAppCellMeasItem *ptItem = &(ptCell->atMeas[dwIndex1]);
            LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                       "%d  %15lu  %15lu  %15lu  %15lu  %15lu  %15lu  %15lu  "
                       "%15lu  %15lu  %15lu  %15lu  %15lu  %15lu  %15lu  "
                       "%15lu  %15lu  %15lu  %15lu\n",
                       dwIndex1,
                       ptItem->lwSlotIndNum,
                       ptItem->lwInvalidNum,
                       ptItem->lwUciIndNum,
                       ptItem->lwCrcIndNum,
                       ptItem->lwSrsIndNum,
                       ptItem->alwDiffTtiNum[0],
                       ptItem->alwDiffTtiNum[1],
                       ptItem->alwDiffTtiNum[2],
                       ptItem->alwDiffTtiNum[3],
                       ptItem->alwDiffUciNum[7],
                       ptItem->alwDiffUciNum[8],
                       ptItem->alwDiffUciNum[9],
                       ptItem->alwDiffCrcNum[7],
                       ptItem->alwDiffCrcNum[8],
                       ptItem->alwDiffCrcNum[9],
                       ptItem->alwDiffSrsNum[7],
                       ptItem->alwDiffSrsNum[8],
                       ptItem->alwDiffSrsNum[9]);
        }
    }
}


WORD32 CTimerApp::InnerDelete(WORD32 dwKey)
{
    /* 在删除树叶节点时会自动删除定时器节点 */
    return m_cTree.Delete(dwKey);
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
    CTimerTreeNode *pNode  = m_cTree.Create(dwInstID, dwKey);
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
        m_cTree.DeleteByInstID(dwInstID);
        return FAIL;
    }

    pNode->Initialize(dwTimerID, dwKey, pTimer, pFunc);

    return dwTimerID;
}


