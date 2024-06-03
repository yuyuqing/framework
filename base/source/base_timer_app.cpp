

#include "base_mem_mgr.h"
#include "base_thread_log.h"
#include "base_timer_app.h"


DEFINE_APP(CTimerApp);


CTimerApp::CTimerApp ()
    : CAppInterface(E_APP_TIMER)
{
}


CTimerApp::~CTimerApp()
{
}


WORD32 CTimerApp::InitApp()
{
    TRACE_STACK("CTimerApp::InitApp()");

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
    return SUCCESS;
}


WORD32 CTimerApp::Init()
{
    TRACE_STACK("CTimerApp::Init()");
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


/* ����������ʱ����Ϣ */
VOID CTimerApp::ProcStartTimer(const VOID *pIn, WORD32 dwLen)
{
}


/* ����ֹͣ��ʱ����Ϣ */
VOID CTimerApp::ProcStopTimer(const VOID *pIn, WORD32 dwLen)
{
}


/* �������ö�ʱ����Ϣ */
VOID CTimerApp::ProcResetTimer(const VOID *pIn, WORD32 dwLen)
{
}


/* ����SlotInd��Ϣ */
VOID CTimerApp::ProcSlotInd(const VOID *pIn, WORD32 dwLen)
{
}


/* ��CTimerApp����������ʱ����Ϣ */
WORD32 CTimerApp::CreateTimer(WORD32          dwTick,
                              PTimerCallBack  pFunc,
                              WORD32          dwID,
                              WORD32          dwExtendID,
                              WORD32          dwTransID,
                              WORD32          dwResvID,
                              VOID           *pContext,
                              VOID           *pUserData)
{
    CMsgMemPool *pMemPool = m_pOwner->GetMsgMemPool();

    BYTE *pBuf = pMemPool->Malloc(E_MemSizeType_256);
    if (NULL == pBuf)
    {
        return INVALID_DWORD;
    }

    T_StartTimerMessage *ptMsg = (T_StartTimerMessage *)(pBuf + sizeof(T_BaseMessage));

    WORD64 lwMicroSec = 0;
    WORD64 lwCycle    = 0;
    WORD64 lwNewUs    = 0;
    WORD32 dwInnerID  = m_dwSelfRingID << 24;

    g_pGlobalClock->GetTime3(lwMicroSec, lwCycle);

    dwInnerID |= ((m_dwSelfTimerInnerID++) & 0xFFFFFF);

    lwNewUs = lwMicroSec + (dwTick * 1000);

    ptMsg->lwSeconds  = lwNewUs / 1000000;
    ptMsg->dwMicroSec = (WORD32)(lwNewUs % 1000000);
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
        return INVALID_DWORD;
    }
}


/* ��CTimerApp����ֹͣ��ʱ����Ϣ */
WORD32 CTimerApp::RemoveTimer(WORD32 dwTimerID)
{
    CMsgMemPool *pMemPool = m_pOwner->GetMsgMemPool();

    BYTE *pBuf = pMemPool->Malloc(E_MemSizeType_256);
    if (NULL == pBuf)
    {
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
        return FAIL;
    }
}


/* ��CTimerApp�������ö�ʱ����Ϣ */
WORD32 CTimerApp::ResetTimer(WORD32 dwTimerID, WORD32 dwTick)
{
    CMsgMemPool *pMemPool = m_pOwner->GetMsgMemPool();

    BYTE *pBuf = pMemPool->Malloc(E_MemSizeType_256);
    if (NULL == pBuf)
    {
        return FAIL;
    }

    T_ResetTimerMessage *ptMsg = (T_ResetTimerMessage *)(pBuf + sizeof(T_BaseMessage));

    WORD64 lwMicroSec = 0;
    WORD64 lwCycle    = 0;
    WORD64 lwNewUs    = 0;

    g_pGlobalClock->GetTime3(lwMicroSec, lwCycle);

    lwNewUs = lwMicroSec + (dwTick * 1000);

    ptMsg->lwSeconds  = lwNewUs / 1000000;
    ptMsg->dwMicroSec = (WORD32)(lwNewUs % 1000000);
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
        return FAIL;
    }
}


/* ��CTimerApp����SlotTti��Ϣ */
WORD32 CTimerApp::NotifySlotInd(WORD16 wSFN, BYTE ucSlot)
{
    CMsgMemPool *pMemPool = m_pOwner->GetMsgMemPool();

    BYTE *pBuf = pMemPool->Malloc(E_MemSizeType_256);
    if (NULL == pBuf)
    {
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
        return FAIL;
    }
}


