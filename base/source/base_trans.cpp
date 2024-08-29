

#include "base_trans.h"
#include "base_thread_pool.h"


CBaseTrans::CBaseTrans (CAppInterface *pApp, WORD64 lwTransID)
{
    m_pApp        = pApp;
    m_lwTransID   = lwTransID;
    m_dwTimerID   = INVALID_DWORD;
    m_eTransState = E_TRANS_INVALID;
    m_dwCurStepID = 0;
    m_dwStepNum   = 0;

    for (WORD32 dwIndex = 0; dwIndex < MAX_TRANS_STEP_NUM; dwIndex++)
    {
        m_apStep[dwIndex] = NULL;
    }
}


CBaseTrans::~CBaseTrans()
{
    if (INVALID_DWORD != m_dwTimerID)
    {
        m_pApp->KillTimer(m_dwTimerID);
    }

    m_lwTransID   = INVALID_LWORD;
    m_dwTimerID   = INVALID_DWORD;
    m_eTransState = E_TRANS_INVALID;
    m_dwCurStepID = 0;
    m_dwStepNum   = 0;

    for (WORD32 dwIndex = 0; dwIndex < MAX_TRANS_STEP_NUM; dwIndex++)
    {
        m_apStep[dwIndex] = NULL;
    }
}


WORD32 CBaseTrans::Initialize()
{
    m_eTransState = E_TRANS_INIT;
    m_cList.Initialize();

    return SUCCESS;
}


WORD64 CBaseTrans::GetTransID()
{
    return m_lwTransID;
}


WORD32 CBaseTrans::GetStepNum()
{
    return m_dwStepNum;
}


WORD32 CBaseTrans::RegisterStep(CBaseTransStep *pStep)
{
    if (m_dwStepNum >= MAX_TRANS_STEP_NUM)
    {
        return FAIL;
    }

    m_apStep[m_dwStepNum++] = pStep;

    return SUCCESS;
}


CBaseTransStep * CBaseTrans::GetCurStep()
{
    if (m_dwCurStepID >= m_dwStepNum)
    {
        return NULL;
    }

    return m_apStep[m_dwCurStepID];
}


WORD32 CBaseTrans::Wait(WORD32 dwStepID)
{
    if (dwStepID != m_dwCurStepID)
    {
        assert(0);
    }

    if (INVALID_DWORD != m_dwTimerID)
    {
        m_pApp->KillTimer(m_dwTimerID);
    }

    CBaseTransStep *pCurStep  = GetCurStep();
    WORD32          dwTransID = pCurStep->GetTransID();
    WORD32          dwTick    = pCurStep->GetWaitTick();

    m_dwTimerID = m_pApp->RegisterTimer(dwTick,
                                        (CCBObject *)this,
                                        (PCBFUNC)(&CBaseTrans::WaitTimeOut),
                                        dwStepID,
                                        dwTransID,
                                        (WORD32)(m_lwTransID >> 32),
                                        (WORD32)m_lwTransID,
                                        (VOID *)pCurStep);
    if (INVALID_DWORD == m_dwTimerID)
    {
        assert(0);
    }

    m_eTransState = E_TRANS_WAIT;

    return SUCCESS;
}


WORD32 CBaseTrans::NextStep(WORD32 dwStepID)
{
    if (dwStepID != m_dwCurStepID)
    {
        assert(0);
    }

    if (INVALID_DWORD != m_dwTimerID)
    {
        m_pApp->KillTimer(m_dwTimerID);
        m_dwTimerID = INVALID_DWORD;
    }

    if ((m_dwCurStepID + 1) == m_dwStepNum)
    {
        m_eTransState = E_TRANS_COMPLETE;
    }
    else
    {
        m_dwCurStepID++;
        m_eTransState = E_TRANS_INIT;
    }

    return SUCCESS;
}


WORD32 CBaseTrans::Fail(WORD32 dwStepID)
{
    if (dwStepID != m_dwCurStepID)
    {
        assert(0);
    }

    if (E_TRANS_WAIT != m_eTransState)
    {
        /* 尚未发起消息流程, 或消息发送失败, 此时不作任何处理 */
        return FAIL;
    }

    CBaseTransStep *pCurStep = GetCurStep();
    pCurStep->ProcFail();

    m_eTransState = E_TRANS_FAIL;

    return SUCCESS;
}


/* 等待响应消息超时 */
VOID CBaseTrans::WaitTimeOut(const VOID *pIn, WORD32 dwLen)
{
    if ((NULL == pIn) || (sizeof(T_TimerParam) != dwLen))
    {
        return ;
    }

    T_TimerParam *ptParam = (T_TimerParam *)(pIn);

    WORD32          dwStepID  = ptParam->dwID;
    WORD32          dwTransID = ptParam->dwExtendID;
    WORD64          lwTransID = ptParam->dwTransID;
    WORD32          dwResvID  = ptParam->dwResvID;
    CBaseTransStep *pStep     = (CBaseTransStep *)ptParam->pContext;
    CBaseTransStep *pCurStep  = GetCurStep();

    lwTransID  = lwTransID << 32;
    lwTransID += dwResvID;

    if ( (lwTransID != m_lwTransID)
      || (dwStepID  != m_dwCurStepID)
      || (pCurStep  != pStep)
      || (dwTransID != pCurStep->GetTransID()))
    {
        assert(0);
    }

    m_dwTimerID = INVALID_DWORD;

    pCurStep->WaitTimeOut();

    m_eTransState = E_TRANS_FAIL;
}


BOOL CBaseTrans::IsFinish()
{
    return (E_TRANS_COMPLETE == m_eTransState);
}


/* 分配内存用于实例化Step; 无需考虑内存回收(伴随Trans销毁自动回收) */
BYTE * CBaseTrans::Mallc()
{
    T_TransStepValue *ptValue = m_cList.Malloc();
    if (NULL == ptValue)
    {
        return NULL;
    }

    return ptValue->aucStep;
}


CBaseTransStep::CBaseTransStep (CBaseTrans *pTrans,
                                WORD32      dwStepID,
                                WORD32      dwSendMsgID,
                                WORD32      dwRecvMsgID,
                                WORD32      dwWaitTick)
    : m_pTrans(pTrans)
{
    m_dwTransID   = (WORD32)(pTrans->GetTransID());
    m_dwStepID    = dwStepID;
    m_eState      = E_TRANS_STEP_INVALID;
    m_dwSendMsgID = dwSendMsgID;
    m_dwRecvMsgID = dwRecvMsgID;
    m_dwWaitTick  = dwWaitTick;
}


CBaseTransStep::~CBaseTransStep()
{
    m_pTrans      = NULL;
    m_dwTransID   = INVALID_DWORD;
    m_dwStepID    = INVALID_DWORD;
    m_eState      = E_TRANS_STEP_INVALID;
    m_dwSendMsgID = INVALID_DWORD;
    m_dwRecvMsgID = INVALID_DWORD;
    m_dwWaitTick  = TRANS_STEP_WAIT_TICK;
}


WORD32 CBaseTransStep::SendMsg(E_AppClass  eDstClass,
                               WORD32      dwDstAssocID,
                               WORD16      wLen,
                               const VOID *ptMsg)
{
    m_eState                = E_TRANS_STEP_INIT;
    m_pTrans->m_eTransState = E_TRANS_INIT;

    WORD32 dwResult = SendLowPriorMsg(eDstClass,
                                      dwDstAssocID,
                                      0,
                                      m_dwSendMsgID,
                                      wLen,
                                      ptMsg);
    if (SUCCESS != dwResult)
    {
        m_pTrans->Fail(m_dwStepID);
    }
    else
    {
        m_eState = E_TRANS_STEP_WAIT;
        m_pTrans->Wait(m_dwStepID);
    }

    return dwResult;
}


WORD32 CBaseTransStep::RecvMsg(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen)
{
    WORD32 dwResult = INVALID_DWORD;

    if ((dwMsgID != m_dwRecvMsgID) || (E_TRANS_STEP_WAIT != m_eState))
    {
        dwResult = FAIL;
    }
    else
    {
        m_eState = E_TRANS_STEP_RECVED;
        dwResult = this->ProcMsg(pIn, wMsgLen);
    }

    if (SUCCESS != dwResult)
    {
        m_pTrans->Fail(m_dwStepID);
    }
    else
    {
        m_eState = E_TRANS_STEP_COMPLETE;
        m_pTrans->NextStep(m_dwStepID);
    }

    return dwResult;
}


WORD32 CBaseTransStep::GetTransID()
{
    return m_dwTransID;
}


WORD32 CBaseTransStep::GetStepID()
{
    return m_dwStepID;
}


WORD32 CBaseTransStep::GetWaitTick()
{
    return m_dwWaitTick;
}


