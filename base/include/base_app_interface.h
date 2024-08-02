

#ifndef _BASE_APP_INTERFACE_H_
#define _BASE_APP_INTERFACE_H_


#include "base_thread.h"


#define APP_CB_EVENT_NUM     ((WORD32)(EV_MSG_SEGMENT_LEN))

typedef CEventMap<APP_CB_EVENT_NUM>  CFrameWork;


class CAppInterface : public CFrameWork
{
public :
    friend class CAppState;
    friend class CAppIdleState;
    friend class CAppWorkState;
    friend class CAppExitState;
    
public :
    CAppInterface(E_AppClass eClass);
    virtual ~CAppInterface();

    /* �������߳�ǰִ�� */
    WORD32 PreInit(CBaseThread *pOwner, const T_AppInfo *ptAppInfo);

    /* �������߳�ǰִ��, ����������ش˷���, ��ʼ��CAppInterface��������Ϣ */
    virtual WORD32 InitApp();

    /* �������̺߳�ִ��(֪ͨApp�ϵ�����) */
    virtual WORD32 Start();

    virtual WORD32 Init() = 0;
    virtual WORD32 DeInit() = 0;
    virtual WORD32 Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen) = 0;

    virtual WORD32 Proc(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen);

    virtual WORD32 Polling();

    /* ע���û��汨�Ļص����� */
    WORD32 RegisterPacketFunc(PMsgCallBack pFunc);

    /* ȥע���û��汨�Ļص�����(���밴��ע�������ȥע��) */
    WORD32 RemovePacketFunc();

    /* ��App���͵������� */
    WORD32 SendPacket(VOID *pBuf);

    /* ��App���Ͷ������� */
    WORD32 SendPacket(WORD32 dwNum, VOID **pBuf);

    /* ��ʱ��ע��ӿ� : ��ʱ��ö�ʱ���Զ�ɾ��, �����ٵ���KillTimerɾ��
       dwTick     : ��ʱʱ��, ��λms
       pObj       : �ص�����
       pFunc      : �ص�����
       dwID       : �ص�����
       dwExtendID : �ص�����
       pContext   : �ص�����
     */
    WORD32 RegisterTimer(WORD32     dwTick, 
                         CCBObject *pObj, 
                         PCBFUNC    pFunc,
                         WORD32     dwID,
                         WORD32     dwExtendID = INVALID_DWORD,
                         WORD32     dwTransID  = INVALID_DWORD,
                         WORD32     dwResvID   = INVALID_DWORD,
                         VOID      *pContext   = NULL,
                         VOID      *pUserData  = NULL);

    /* ���Զ�ʱ�� */
    WORD32 RegisterTimer(BYTE       ucHour,
                         BYTE       ucMinute,
                         BYTE       ucSecond,
                         WORD16     wMillSec,
                         CCBObject *pObj, 
                         PCBFUNC    pFunc,
                         WORD32     dwID,
                         WORD32     dwExtendID = INVALID_DWORD,
                         WORD32     dwTransID  = INVALID_DWORD,
                         WORD32     dwResvID   = INVALID_DWORD,
                         VOID      *pContext   = NULL,
                         VOID      *pUserData  = NULL);

    WORD32 KillTimer(WORD32 dwTimerID);

    CBaseThread * GetOwner();

    E_AppClass GetAppClass();

    WORD32 GetAssocID(WORD32 dwPos);

    WORD32 GetAppID();

    /* ��ȡApp�����߳�ID */
    WORD32 GetThreadID();

    BOOL IsAssociate(E_AppClass eClass, WORD32 dwAssocID = INVALID_DWORD);

    VOID GetMeasure(T_AppMeasure &rtMeasure);
    
    virtual VOID Dump();

    VOID Printf();

protected :
    WORD32 InitMeasure();

public :
    VOID StatisticMsgQ(WORD32 dwMsgID, WORD64 lwStartCycle, WORD64 lwEndCycle);
    VOID Statistic(WORD32 dwMsgID, WORD64 lwTimeStampStart, WORD64 lwTimeStampEnd);

protected :
    CHAR                 m_aucName[APP_NAME_LEN];  
    CBaseThread         *m_pOwner;                 /* ��ǰ�����߳� */

    E_AppClass           m_eClass;                 /* App���� */
    WORD32               m_dwAppID;                /* �ڲ���ʶ��AppID */
    WORD32               m_dwThreadID;             /* �����߳�ID */
    WORD32               m_dwStartEventID;         /* ��ʼ��ϢID */
    WORD32               m_dwDataRingID;           /* ���ݱ��Ķ���RingID */

    WORD32               m_dwCyclePer100NS;        /* ���ڽ�Cycleתʱ�� */

    T_AtomicAppMeasure   m_tMeasure;

public :    
    BOOL                 m_bAssocFlag;             /* App�Ƿ���ڰ󶨱�־(��С���İ󶨹�ϵ) */
    WORD32               m_dwAssocNum;             /* App���������֮��İ󶨹�ϵ */
    WORD32               m_adwAssocID[MAX_ASSOCIATE_NUM_PER_APP];
};


inline WORD32 CAppState::Polling()
{
    if (unlikely(APP_STATE_WORK != m_eState))
    {
        return FAIL;
    }

    return m_pAppInst->Polling();
}


inline WORD32 CAppInterface::Proc(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen)
{
    return ProcessEvent(dwMsgID, pIn, wMsgLen);
}


inline WORD32 CAppInterface::Polling()
{
    return SUCCESS;
}


inline CBaseThread * CAppInterface::GetOwner()
{
    return m_pOwner;
}


inline E_AppClass CAppInterface::GetAppClass()
{
    return m_eClass;
}


inline WORD32 CAppInterface::GetAssocID(WORD32 dwPos)
{
    if (dwPos >= m_dwAssocNum)
    {
        return INVALID_DWORD;
    }
    
    return m_adwAssocID[dwPos];
}


inline WORD32 CAppInterface::GetAppID()
{
    return m_dwAppID;
}


/* ��ȡApp�����߳�ID */
inline WORD32 CAppInterface::GetThreadID()
{
    return m_dwThreadID;
}


inline BOOL CAppInterface::IsAssociate(E_AppClass eClass, WORD32 dwAssocID)
{
    if (eClass != m_eClass)
    {
        return FALSE;
    }

    if (!m_bAssocFlag)
    {
        return TRUE;
    }

    for (WORD32 dwIndex = 0; dwIndex < m_dwAssocNum; dwIndex++)
    {
        if (dwAssocID == m_adwAssocID[dwIndex])
        {
            return TRUE;
        }
    }

    return FALSE;
}


inline VOID CAppInterface::StatisticMsgQ(WORD32 dwMsgID,
                                         WORD64 lwStartCycle,
                                         WORD64 lwEndCycle)
{
    /* ÿ���յ���Ϣ, ֻ��Ҫ��ȡһ����Ƶ, ��������Ϣ�����ʱͳ��ʱֱ�Ӹ��� */
    m_dwCyclePer100NS = g_pGlobalClock->GetCyclePer100NS();

    m_tMeasure.tMsgQStat.lwMsgCount++;

    WORD64 lwTimeStat = lwEndCycle - lwStartCycle;
    WORD32 dwTimeUsed = TRANSFER_CYCLE_TO_100NS2(lwTimeStat, m_dwCyclePer100NS);
    WORD32 dwMaxUsed  = m_tMeasure.tMsgQStat.dwMsgQMaxUsed.load(std::memory_order_relaxed);

    if (dwTimeUsed > dwMaxUsed)
    {
        m_tMeasure.tMsgQStat.dwQMaxMsgID   = dwMsgID;
        m_tMeasure.tMsgQStat.dwMsgQMaxUsed = dwTimeUsed;
    }

    m_tMeasure.tMsgQStat.lwMsgQTotalTime += dwTimeUsed;
    m_tMeasure.tMsgQStat.alwStat[base_bsr_uint32(dwTimeUsed)]++;
}


inline VOID CAppInterface::Statistic(WORD32 dwMsgID,
                                     WORD64 lwTimeStampStart,
                                     WORD64 lwTimeStampEnd)
{
    WORD64 lwTimeStat = lwTimeStampEnd - lwTimeStampStart;
    WORD32 dwTimeUsed = TRANSFER_CYCLE_TO_100NS2(lwTimeStat, m_dwCyclePer100NS);

    m_tMeasure.lwProcNum++;

    if (dwTimeUsed > m_tMeasure.dwMaxTimeUsed)
    {
        m_tMeasure.dwMsgID       = dwMsgID;
        m_tMeasure.dwMaxTimeUsed = dwTimeUsed;
    }

    m_tMeasure.lwTimeUsedTotal += dwTimeUsed;
    m_tMeasure.alwStat[base_bsr_uint32(dwTimeUsed)]++;
}


#endif


