

#include "base_variable.h"
#include "base_timer.h"


CTimerNode::CTimerNode ()
{
    m_pNext       = NULL;
    m_pPrev       = NULL;
    m_lwTimeoutUs = INVALID_LWORD;
    m_pObj        = NULL;
    m_pFunc       = NULL;
    memset(&m_tCBParam, 0x00, sizeof(m_tCBParam));
}


/* 相对定时器, dwTick单位 : 1ms */
CTimerNode::CTimerNode (WORD32     dwTick,
                        CCBObject *pObj,
                        PCBFUNC    pFunc,
                        WORD32     dwID,
                        WORD32     dwExtendID,
                        WORD32     dwTransID,
                        WORD32     dwResvID,
                        VOID      *pContext,
                        VOID      *pUserData)
{
    WORD64 lwMicroSec = 0;
    WORD64 lwCycle    = 0;
    WORD64 lwTick     = dwTick;

    g_pGlobalClock->GetTime3(lwMicroSec, lwCycle);

    m_pNext       = NULL;
    m_pPrev       = NULL;
    m_lwTimeoutUs = lwMicroSec + (lwTick * 1000);
    m_pObj        = pObj;
    m_pFunc       = pFunc;

    m_tCBParam.dwID       = dwID;
    m_tCBParam.dwExtendID = dwExtendID;
    m_tCBParam.dwTransID  = dwTransID;
    m_tCBParam.dwResvID   = dwResvID;
    m_tCBParam.pContext   = pContext;
    m_tCBParam.pUserData  = pUserData;
}


CTimerNode::CTimerNode (BYTE       ucHour,
                        BYTE       ucMinute,
                        BYTE       ucSecond,
                        WORD16     wMillSec,
                        CCBObject *pObj,
                        PCBFUNC    pFunc,
                        WORD32     dwID,
                        WORD32     dwExtendID,
                        WORD32     dwTransID,
                        WORD32     dwResvID,
                        VOID      *pContext,
                        VOID      *pUserData)
{
    tm     tTime;
    WORD64 lwMicroSec = 0;
    WORD64 lwCycle    = 0;

    g_pGlobalClock->GetTime3(lwMicroSec, lwCycle);

    time_t  tSec = lwMicroSec / 1000000;
    LocalTime(&tSec, tTime);

    WORD32 dwCurHour   = tTime.tm_hour;
    WORD32 dwCurMin    = tTime.tm_min;
    WORD32 dwCurSec    = tTime.tm_sec;
    WORD32 dwurMillSec = (WORD32)((lwMicroSec / 1000) % 1000);

    WORD32 dwCurMill = ((((dwCurHour * 60) + dwCurMin) * 60) + dwCurSec) * 1000 + dwurMillSec;
    WORD32 dwTimeOut = (((((ucHour % 24) * 60) + (ucMinute % 60)) * 60) + (ucSecond % 60)) * 1000 + (wMillSec % 1000);

    WORD64 lwTick = (dwTimeOut >= dwCurMill) ? 
                        (dwTimeOut - dwCurMill) : (86400000 + dwTimeOut - dwCurMill);

    m_pNext       = NULL;
    m_pPrev       = NULL;
    m_lwTimeoutUs = lwMicroSec + (lwTick * 1000);
    m_pObj        = pObj;
    m_pFunc       = pFunc;

    m_tCBParam.dwID       = dwID;
    m_tCBParam.dwExtendID = dwExtendID;
    m_tCBParam.dwTransID  = dwTransID;
    m_tCBParam.dwResvID   = dwResvID;
    m_tCBParam.pContext   = pContext;
    m_tCBParam.pUserData  = pUserData;
}


CTimerNode::~CTimerNode()
{
    m_pNext       = NULL;
    m_pPrev       = NULL;
    m_lwTimeoutUs = 0;
    m_pObj        = NULL;
    m_pFunc       = NULL;
    memset(&m_tCBParam, 0x00, sizeof(m_tCBParam));
}


