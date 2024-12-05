

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

    /* ��ʼ��CAppInterface����ά����Ϣ */
    InitMeasure();

    /* ��ʼ��CAppInterface��������Ϣ */
    this->InitApp();

    return SUCCESS;
}


/* �������߳�ǰִ��, ����������ش˷���, ��ʼ��CAppInterface��������Ϣ */
WORD32 CAppInterface::InitApp()
{
    return SUCCESS;
}


/* �������̺߳�ִ��(֪ͨApp�ϵ�����) */
WORD32 CAppInterface::Start()
{
    return SUCCESS;
}


/* ע���û��汨�Ļص����� */
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


/* ȥע���û��汨�Ļص�����(���밴��ע�������ȥע��) */
WORD32 CAppInterface::RemovePacketFunc()
{
    CDataPlaneRing *pRing = m_pOwner->GetMsgRingD();
    if (NULL == pRing)
    {
        return FAIL;
    }

    return pRing->DelSTRing(m_dwDataRingID);
}


/* ��App���͵������� */
WORD32 CAppInterface::SendPacket(VOID *pBuf)
{
    CDataPlaneRing *pRing = m_pOwner->GetMsgRingD();
    if (unlikely(NULL == pRing))
    {
        return 0;
    }

    return pRing->Enqueue(m_dwDataRingID, pBuf);
}


/* ��App���Ͷ������� */
WORD32 CAppInterface::SendPacket(WORD32 dwNum, VOID **pBuf)
{
    CDataPlaneRing *pRing = m_pOwner->GetMsgRingD();
    if (unlikely(NULL == pRing))
    {
        return 0;
    }

    return pRing->Enqueue(m_dwDataRingID, dwNum, pBuf);
}


/* ��ʱ��ע��ӿ� : ��ʱ��ö�ʱ���Զ�ɾ��, �����ٵ���KillTimerɾ��
   dwTick     : ��ʱʱ��, ��λms
   pObj       : �ص�����
   pFunc      : �ص�����
   dwID       : �ص�����
   dwExtendID : �ص�����
   lwTransID  : �ص�����
   pContext   : �ص�����
   pUserData  : �ص�����
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


/* ���Զ�ʱ�� */
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


