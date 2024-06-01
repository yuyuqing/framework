

#include "base_thread_multi.h"


WORD32 CMultiThread::ProcMultiMessage(VOID *pObj, VOID *pMsg)
{
    CMultiThread   *pThread = (CMultiThread *)pObj;
    T_BaseMessage  *ptMsg   = (T_BaseMessage *)pMsg;
    WORD32          dwMsgID = ptMsg->dwMsgID;

    ptMsg->lwEndCycle = GetCycle();

    pThread->StatisticMsgQ(pThread->m_tMeasure.tMultiMsgQStat, 
                           dwMsgID, 
                           ptMsg->lwStartCycle, 
                           ptMsg->lwEndCycle);

    pThread->Dispatch(dwMsgID, 
                      ptMsg, 
                      sizeof(T_BaseMessage) + ptMsg->wMsgLen);

    pThread->m_pMsgMemPool->Free((BYTE *)pMsg);

    return SUCCESS;
}


CMultiThread::CMultiThread (const T_ThreadParam &rtParam)
    : CWorkThread(rtParam)
{
    m_eType   = NRT_THREAD;
    m_eClass  = E_THREAD_MULTI;
}


CMultiThread::~CMultiThread ()
{
}


/* �����߳�ʵ����ִ�г�ʼ��(�����߳�ջ�ռ�ִ��) */
WORD32 CMultiThread::Initialize()
{
    TRACE_STACK("CMultiThread::Initialize()");

    WORD32 dwResult = CWorkThread::Initialize();
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    m_cMultiRing.Initialize();

    return SUCCESS;
}


VOID CMultiThread::DoRun()
{
    TRACE_STACK("CMultiThread::DoRun()");

    WORD64 lwTick           = THREAD_TICK_GRANULARITY;
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
    WORD64 lwTimeOutNum     = 0;

    while (TRUE)
    {
        while (m_MsgRingH.isEmpty()
            && m_MsgRingL.isEmpty()
            && m_MsgRingD.isEmpty()
            && m_cMultiRing.isEmpty())
        {
            /* ���㳬ʱ����(���������г�ʱ��ʱ��) */
            Execute(lwTimeOutNum);
            m_cSemaphore.TimeWait(lwTick);
        }

        lwTimeStampStart  = GetCycle();
        m_dwCyclePer100NS = g_pGlobalClock->GetCyclePer100NS();

        /* ���ȴ��������и����ȼ������е���Ϣ */
        dwProcHNum = m_MsgRingH.Dequeue(this, &CPollingThread::ProcMessageHP);

        lwTimeStampProcH = GetCycle();

        /* ÿ��ѭ���̶��ӵ����ȼ�������ȡָ����������Ϣ */
        dwRemainLNum = m_MsgRingL.Dequeue(m_dwCBNum, 
                                          this, 
                                          &CPollingThread::ProcMessageLP, 
                                          dwProcLNum);

        lwTimeStampProcL = GetCycle();

        /* �����ݶ��н����û��汨�� */
        dwRemainDNum = m_MsgRingD.Dequeue(m_dwPacketCBNum, dwProcDNum);

        /* ��MultiRing����ҵ����Ϣ */
        dwRemainDNum = m_cMultiRing.Dequeue(m_dwMultiCBNum, 
                                            this, 
                                            &CMultiThread::ProcMultiMessage,
                                            dwProcDNum);

        lwTimeStampProcD = GetCycle();

        /* ִ��Ԫ��ʱ������(��ദ��ָ�����޸�����ʱ��ʱ��) */
        Execute(lwTimeOutNum, m_dwTimerThresh);

        lwTimeStampEnd = GetCycle();

        Statistic(dwProcHNum, dwProcLNum, dwProcDNum, 
                  dwRemainLNum, dwRemainDNum, 
                  lwTimeStampStart, 
                  lwTimeStampProcH, lwTimeStampProcL, lwTimeStampProcD, 
                  lwTimeStampEnd);

        /* �߳��˳� */
        if (unlikely(isCancel()))
        {
            LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_DEBUG, TRUE, 
                       "Thread Cancel\n");
            break ;
        }
    }
}


