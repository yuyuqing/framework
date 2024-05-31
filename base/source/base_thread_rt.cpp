

#include "base_thread_rt.h"


CRTThread::CRTThread (const T_ThreadParam &rtParam)
    : CPollingThread(rtParam)
{
    m_eType   = RT_THREAD;
    m_eClass  = E_THREAD_RT;
}


CRTThread::~CRTThread()
{
}


/* �����߳�ʵ����ִ�г�ʼ��(�����߳�ջ�ռ�ִ��) */
WORD32 CRTThread::Initialize()
{
    TRACE_STACK("CRTThread::Initialize()");

    WORD32 dwResult = CPollingThread::Initialize();
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    return SUCCESS;
}


VOID CRTThread::Dump()
{
    TRACE_STACK("CRTThread::Dump()");

    CPollingThread::Dump();
}


VOID CRTThread::DoRun()
{
    TRACE_STACK("CRTThread::DoRun()");

    WORD32 dwCountH         = 0;
    WORD32 dwCountL         = 0;
    WORD32 dwCountD         = 0;
    WORD32 dwProcHNum       = 0;
    WORD32 dwProcLNum       = 0;
    WORD32 dwProcDNum       = 0;
    WORD32 dwRemainLNum     = 0;
    WORD32 dwRemainDNum     = 0;
    WORD64 lwTimeStampStart = 0;
    WORD64 lwTimeStampProcH = 0;
    WORD64 lwTimeStampProcL = 0;
    WORD64 lwTimeStampEnd   = 0;

    while (TRUE)
    {
        do
        {
            dwCountH = m_MsgRingH.Count();
            dwCountL = m_MsgRingL.Count();
            dwCountD = m_MsgRingD.Count();

            if ((dwCountH + dwCountL + dwCountD) > 0)
            {
                break ;
            }

            m_cUserSemaphore.Wait();
        } while(TRUE);

        lwTimeStampStart  = GetCycle();
        m_dwCyclePer100NS = g_pGlobalClock->GetCyclePer100NS();

        /* ���ȴ��������и����ȼ������е���Ϣ */
        dwProcHNum = m_MsgRingH.Dequeue(this, &CPollingThread::ProcMessageHP);

        lwTimeStampProcH = GetCycle();

        if (likely(0 == dwCountL))
        {
            dwRemainLNum = 0;
            dwProcLNum   = 0;
        }
        else
        {
            /* ÿ��ѭ���̶��ӵ����ȼ�������ȡָ����������Ϣ */
            dwRemainLNum = m_MsgRingL.Dequeue(m_dwCBNum, 
                                              this, 
                                              &CPollingThread::ProcMessageLP, 
                                              dwProcLNum);
        }

        lwTimeStampProcL = GetCycle();

        /* �����ݶ��н����û��汨�� */
        dwRemainDNum = m_MsgRingD.Dequeue(m_dwPacketCBNum, dwProcDNum);

        lwTimeStampEnd = GetCycle();

        Statistic(dwProcHNum, dwProcLNum, dwProcDNum, 
                  dwRemainLNum, dwRemainDNum, 
                  lwTimeStampStart, 
                  lwTimeStampProcH, lwTimeStampProcL, lwTimeStampEnd, 
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


