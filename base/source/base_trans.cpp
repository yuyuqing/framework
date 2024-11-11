

#include "base_trans.h"
#include "base_thread_pool.h"


CBaseTrans::CBaseTrans (CAppInterface *pApp,
                        WORD32         dwUpperID,
                        WORD32         dwExtendID,
                        WORD64         lwTransID,
                        CCBObject     *pObj,
                        PCBFUNC        pFunc)
{
    m_pApp         = pApp;
    m_dwUpperID    = dwUpperID;
    m_dwExtendID   = dwExtendID;
    m_lwTransID    = lwTransID;
    m_pObj         = pObj;
    m_pTimeOutFunc = pFunc;
    m_dwTimerID    = INVALID_DWORD;
    m_eTransState  = E_TRANS_INVALID;
    m_dwCurStepID  = 0;
    m_dwStepNum    = 0;

    for (WORD32 dwIndex = 0; dwIndex < MAX_TRANS_STEP_NUM; dwIndex++)
    {
        m_apStep[dwIndex] = NULL;
    }
}


CBaseTrans::~CBaseTrans()
{
    CBaseTrans *pTrans = (CBaseTrans *)(this);

    if (E_TRANS_COMPLETE == m_eTransState)
    {
        pTrans->Finish();
    }
    else
    {
        pTrans->Abort();
    }

    if (INVALID_DWORD != m_dwTimerID)
    {
        m_pApp->KillTimer(m_dwTimerID);
    }

    m_pApp         = NULL;
    m_dwUpperID    = INVALID_DWORD;
    m_dwExtendID   = INVALID_DWORD;
    m_lwTransID    = INVALID_LWORD;
    m_pObj         = NULL;
    m_pTimeOutFunc = NULL;
    m_dwTimerID    = INVALID_DWORD;
    m_eTransState  = E_TRANS_INVALID;
    m_dwCurStepID  = 0;
    m_dwStepNum    = 0;

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


WORD32 CBaseTrans::Execute()
{
    WORD32          dwResult   = INVALID_DWORD;
    WORD32          dwTransIDH = 0;
    WORD32          dwTransIDL = 0;
    CBaseTransStep *pCurStep   = NULL;

    if (m_dwCurStepID >= m_dwStepNum)
    {
        m_eTransState = E_TRANS_COMPLETE;
        return SUCCESS;
    }

    pCurStep = m_apStep[m_dwCurStepID];
    if (NULL == pCurStep)
    {
        m_eTransState = E_TRANS_FAIL;
        return FAIL;
    }

    dwResult = pCurStep->Execute();
    if (SUCCESS != dwResult)
    {
        m_eTransState = E_TRANS_FAIL;
        return FAIL;
    }

    if (INVALID_DWORD != m_dwTimerID)
    {
        m_pApp->KillTimer(m_dwTimerID);
    }

    dwTransIDH = (WORD32)(m_lwTransID >> 32);
    dwTransIDL = (WORD32)(m_lwTransID & 0x00FFFFFFFFUL);

    m_dwTimerID = m_pApp->RegisterTimer(TRANS_STEP_WAIT_TICK,
                                        m_pObj,
                                        m_pTimeOutFunc,
                                        m_dwUpperID,
                                        m_dwExtendID,
                                        dwTransIDH, dwTransIDL,
                                        (VOID *)this,
                                        (VOID *)pCurStep);
    if (INVALID_DWORD == m_dwTimerID)
    {
        m_eTransState = E_TRANS_FAIL;
        return FAIL;
    }

    m_eTransState = E_TRANS_WAIT;

    return SUCCESS;
}


WORD32 CBaseTrans::WaitTimeOut(WORD64 lwTransID, CBaseTransStep *pStep)
{
    CBaseTransStep *pCurStep = GetCurStep();

    if ((lwTransID != m_lwTransID) || (pStep != pCurStep))
    {
        return FAIL;
    }

    m_dwTimerID = INVALID_DWORD;

    return SUCCESS;
}


WORD32 CBaseTrans::RecvMsg(WORD32 dwMsgID, VOID *pIn, WORD32 dwMsgLen)
{
    WORD32          dwResult = INVALID_DWORD;
    CBaseTransStep *pCurStep = NULL;

    if (m_dwCurStepID >= m_dwStepNum)
    {
        return FAIL;
    }

    pCurStep = m_apStep[m_dwCurStepID];
    if (NULL == pCurStep)
    {
        return FAIL;
    }

    /* 先清除定时器 */
    if (INVALID_DWORD != m_dwTimerID)
    {
        m_pApp->KillTimer(m_dwTimerID);
        m_dwTimerID = INVALID_DWORD;
    }

    if ( (dwMsgID != pCurStep->m_dwRecvMsgID)
      || (E_TRANS_WAIT != m_eTransState))
    {
        dwResult = FAIL;
    }
    else
    {
        dwResult = pCurStep->ProcMsg(pIn, dwMsgLen);
    }

    if (SUCCESS != dwResult)
    {
        m_eTransState = E_TRANS_FAIL;
        return FAIL;
    }

    m_dwCurStepID++;

    if (m_dwCurStepID == m_dwStepNum)
    {
        m_eTransState = E_TRANS_COMPLETE;
        return SUCCESS;
    }
    else
    {
        m_eTransState = E_TRANS_INIT;

        return Execute();
    }
}


CBaseTransStep::CBaseTransStep (CBaseTrans *pTrans,
                                WORD32      dwStepID,
                                WORD32      dwSendMsgID,
                                WORD32      dwRecvMsgID)
    : m_pTrans(pTrans)
{
    m_dwStepID    = dwStepID;
    m_dwSendMsgID = dwSendMsgID;
    m_dwRecvMsgID = dwRecvMsgID;
}


CBaseTransStep::~CBaseTransStep()
{
    m_pTrans      = NULL;
    m_dwStepID    = INVALID_DWORD;
    m_dwSendMsgID = INVALID_DWORD;
    m_dwRecvMsgID = INVALID_DWORD;
}


