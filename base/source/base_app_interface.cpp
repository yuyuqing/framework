

#include "base_app_cntrl.h"


CAppInterface::CAppInterface(E_AppClass eClass)
{
    m_pOwner          = NULL;
    m_eClass          = eClass;
    m_dwAppID         = INVALID_DWORD;
    m_dwThreadID      = INVALID_DWORD;
    m_dwStartEventID  = 0;
    m_dwDataRingID    = INVALID_DWORD;
    m_dwCyclePer100NS = CYCLE_NUM_PER_1US / 10;
    m_bAssocFlag      = FALSE;
    m_dwAssocNum      = 0;
    memset(m_aucName,    0x00, sizeof(m_aucName));
    memset(m_adwAssocID, 0x00, sizeof(m_adwAssocID));
}


CAppInterface::~CAppInterface()
{
    if (INVALID_DWORD != m_dwDataRingID)
    {
        RemovePacketFunc();
    }

    m_pOwner          = NULL;
    m_eClass          = E_APP_INVALID;
    m_dwAppID         = INVALID_DWORD;
    m_dwThreadID      = INVALID_DWORD;
    m_dwStartEventID  = 0;
    m_dwDataRingID    = INVALID_DWORD;
    m_dwCyclePer100NS = CYCLE_NUM_PER_1US / 10;
    m_bAssocFlag      = FALSE;
    m_dwAssocNum      = 0;
    memset(m_aucName,    0x00, sizeof(m_aucName));
    memset(m_adwAssocID, 0x00, sizeof(m_adwAssocID));
}


WORD32 CAppInterface::PreInit(CBaseThread *pOwner, const T_AppInfo *ptAppInfo)
{
    m_pOwner         = pOwner;
    m_dwAppID        = ptAppInfo->dwAppID;
    m_dwThreadID     = ptAppInfo->dwThreadID;
    m_dwStartEventID = ptAppInfo->dwStartEventID;
    m_bAssocFlag     = ptAppInfo->bAssocFlag;
    m_dwAssocNum     = ptAppInfo->dwAssocNum;

    memcpy(m_aucName,    ptAppInfo->aucName, APP_NAME_LEN);
    memcpy(m_adwAssocID, ptAppInfo->adwAssocID, sizeof(m_adwAssocID));

    g_pAppCntrl->RegisterTable(m_dwAppID,
                               m_dwThreadID,
                               m_eClass,
                               m_dwAssocNum,
                               m_adwAssocID);

    /* 初始化CAppInterface基类维测信息 */
    InitMeasure();

    /* 初始化CAppInterface派生类信息 */
    this->InitApp();

    return SUCCESS;
}


/* 在启动线程前执行, 派生类可重载此方法, 初始化CAppInterface派生类信息 */
WORD32 CAppInterface::InitApp()
{
    return SUCCESS;
}


/* 在启动线程后执行(通知App上电启动) */
WORD32 CAppInterface::Start()
{
    return SUCCESS;
}


/* 注册用户面报文回调函数 */
WORD32 CAppInterface::RegisterPacketFunc(PMsgCallBack pFunc)
{
    CDataPlaneRing          *pRing   = NULL;
    CDataPlaneRing::CSTRing *pSTRing = NULL;

    pRing = m_pOwner->GetMsgRingD();
    if (NULL == pRing)
    {
        return FAIL;
    }

    pSTRing = pRing->CreateSTRing((VOID *)this, pFunc, m_dwDataRingID);
    if (NULL == pSTRing)
    {
        return FAIL;
    }

    return SUCCESS;
}


/* 去注册用户面报文回调函数(必须按照注册的逆序去注册) */
WORD32 CAppInterface::RemovePacketFunc()
{
    CDataPlaneRing *pRing = m_pOwner->GetMsgRingD();
    if (NULL == pRing)
    {
        return FAIL;
    }

    return pRing->DelSTRing(m_dwDataRingID);
}


/* 向App发送单条报文 */
WORD32 CAppInterface::SendPacket(VOID *pBuf)
{
    CDataPlaneRing *pRing = m_pOwner->GetMsgRingD();
    if (unlikely(NULL == pRing))
    {
        return 0;
    }

    return pRing->Enqueue(m_dwDataRingID, pBuf);
}


/* 向App发送多条报文 */
WORD32 CAppInterface::SendPacket(WORD32 dwNum, VOID **pBuf)
{
    CDataPlaneRing *pRing = m_pOwner->GetMsgRingD();
    if (unlikely(NULL == pRing))
    {
        return 0;
    }

    return pRing->Enqueue(m_dwDataRingID, dwNum, pBuf);
}


/* 定时器注册接口 : 超时后该定时器自动删除, 无需再调用KillTimer删除
   dwTick     : 超时时长, 单位ms
   pObj       : 回调对象
   pFunc      : 回调函数
   dwID       : 回调出参
   dwExtendID : 回调出参
   lwTransID  : 回调出参
   pContext   : 回调出参
   pUserData  : 回调出参
 */
WORD32 CAppInterface::RegisterTimer(WORD32     dwTick,
                                    CCBObject *pObj,
                                    PCBFUNC    pFunc,
                                    WORD32     dwID,
                                    WORD32     dwExtendID,
                                    WORD32     dwTransID,
                                    WORD32     dwResvID,
                                    VOID      *pContext,
                                    VOID      *pUserData)
{
    CTimerRepo *pTimerRepo = m_pOwner->GetTimerRepo();
    if (unlikely(NULL == pTimerRepo))
    {
        return INVALID_DWORD;
    }

    WORD32 dwTimerID = pTimerRepo->RegisterTimer(dwTick,
                                                 pObj, pFunc,
                                                 dwID,
                                                 dwExtendID,
                                                 dwTransID,
                                                 dwResvID,
                                                 pContext,
                                                 pUserData);

    return dwTimerID;
}


/* 绝对定时器 */
WORD32 CAppInterface::RegisterTimer(BYTE       ucHour,
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
    CTimerRepo *pTimerRepo = m_pOwner->GetTimerRepo();
    if (unlikely(NULL == pTimerRepo))
    {
        return INVALID_DWORD;
    }

    WORD32 dwTimerID = pTimerRepo->RegisterTimer(ucHour, ucMinute, ucSecond, wMillSec,
                                                 pObj, pFunc,
                                                 dwID,
                                                 dwExtendID,
                                                 dwTransID,
                                                 dwResvID,
                                                 pContext,
                                                 pUserData);

    return dwTimerID;
}


WORD32 CAppInterface::KillTimer(WORD32 dwTimerID)
{
    CTimerRepo *pTimerRepo = m_pOwner->GetTimerRepo();
    if (unlikely(NULL == pTimerRepo))
    {
        return FAIL;
    }

    WORD32 dwResult = pTimerRepo->KillTimer(dwTimerID);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    return SUCCESS;
}


VOID CAppInterface::GetMeasure(T_AppMeasure &rtMeasure)
{
    rtMeasure.dwAppID         = m_tMeasure.dwAppID;
    rtMeasure.dwAppClass      = m_tMeasure.dwAppClass;

    rtMeasure.lwProcNum       = m_tMeasure.lwProcNum.load(std::memory_order_relaxed);
    rtMeasure.dwMsgID         = m_tMeasure.dwMsgID.load(std::memory_order_relaxed);
    rtMeasure.dwMaxTimeUsed   = m_tMeasure.dwMaxTimeUsed.load(std::memory_order_relaxed);
    rtMeasure.lwTimeUsedTotal = m_tMeasure.lwTimeUsedTotal.load(std::memory_order_relaxed);

    for (WORD32 dwIndex = 0; dwIndex < BIT_NUM_OF_WORD32; dwIndex++)
    {
        rtMeasure.alwStat[dwIndex] = m_tMeasure.alwStat[dwIndex].load(std::memory_order_relaxed);
    }

    rtMeasure.tMsgQStat.lwMsgCount      = m_tMeasure.tMsgQStat.lwMsgCount.load(std::memory_order_relaxed);
    rtMeasure.tMsgQStat.dwQMaxMsgID     = m_tMeasure.tMsgQStat.dwQMaxMsgID.load(std::memory_order_relaxed);
    rtMeasure.tMsgQStat.dwMsgQMaxUsed   = m_tMeasure.tMsgQStat.dwMsgQMaxUsed.load(std::memory_order_relaxed);
    rtMeasure.tMsgQStat.lwMsgQTotalTime = m_tMeasure.tMsgQStat.lwMsgQTotalTime.load(std::memory_order_relaxed);

    for (WORD32 dwIndex = 0; dwIndex < BIT_NUM_OF_WORD32; dwIndex++)
    {
        rtMeasure.tMsgQStat.alwStat[dwIndex] = m_tMeasure.tMsgQStat.alwStat[dwIndex].load(std::memory_order_relaxed);
    }
}


VOID CAppInterface::Dump()
{
    TRACE_STACK("CAppInterface::Dump()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "AppID : %3d, ThreadID : %3d, StartEventID : %5d, AssocFlag : %d, "
               "AssocNum : %d, m_aucName : %s\n",
               m_dwAppID,
               m_dwThreadID,
               m_dwStartEventID,
               m_bAssocFlag,
               m_dwAssocNum,
               m_aucName);

    m_pOwner->Dump();
}


VOID CAppInterface::Printf()
{
    WORD64 lwProcNum = m_tMeasure.lwProcNum.load(std::memory_order_relaxed);

    printf("AppID : %3d, ThreadID : %3d, lwProcNum : %lu, Name : %s\n",
           m_dwAppID,
           m_dwThreadID,
           lwProcNum,
           m_aucName);
}


WORD32 CAppInterface::InitMeasure()
{
    m_tMeasure.dwAppID         = m_dwAppID;
    m_tMeasure.dwAppClass      = (WORD32)m_eClass;

    m_tMeasure.lwProcNum       = 0;
    m_tMeasure.dwMsgID         = 0;
    m_tMeasure.dwMaxTimeUsed   = 0;
    m_tMeasure.lwTimeUsedTotal = 0;

    for (WORD32 dwIndex = 0; dwIndex < BIT_NUM_OF_WORD32; dwIndex++)
    {
        m_tMeasure.alwStat[dwIndex] = 0;
    }

    m_tMeasure.tMsgQStat.lwMsgCount      = 0;
    m_tMeasure.tMsgQStat.dwQMaxMsgID     = 0;
    m_tMeasure.tMsgQStat.dwMsgQMaxUsed   = 0;
    m_tMeasure.tMsgQStat.lwMsgQTotalTime = 0;

    for (WORD32 dwIndex = 0; dwIndex < BIT_NUM_OF_WORD32; dwIndex++)
    {
        m_tMeasure.tMsgQStat.alwStat[dwIndex] = 0;
    }

    return SUCCESS;
}


