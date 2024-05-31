

#include "base_thread_worker.h"


CWorkThread::CWorkThread (const T_ThreadParam &rtParam)
    : CRTThread(rtParam)
{
    m_eType  = NRT_THREAD;
    m_eClass = E_THREAD_WORK;
}


CWorkThread::~CWorkThread ()
{
}


/* �����߳�ʵ����ִ�г�ʼ��(�����߳�ջ�ռ�ִ��) */
WORD32 CWorkThread::Initialize()
{
    TRACE_STACK("CWorkThread::Initialize()");

    WORD32 dwResult = CRTThread::Initialize();
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    m_cTimerRepo.Initialize();

    return SUCCESS;
}


VOID CWorkThread::DoRun()
{
    TRACE_STACK("CWorkThread::DoRun()");

    WORD64 lwTick           = THREAD_TICK_GRANULARITY;
    WORD32 dwResult         = 0;
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

        lwTimeStampProcD = GetCycle();

        /* ��ѯ���� */
        Polling();

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


