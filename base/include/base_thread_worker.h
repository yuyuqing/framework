

#ifndef _BASE_THREAD_WORKER_H_
#define _BASE_THREAD_WORKER_H_


#include "base_thread_rt.h"


class CWorkThread : public CRTThread
{
public :
    friend class CThreadCntrl;

public :
    CWorkThread (const T_ThreadParam &rtParam);
    virtual ~CWorkThread ();

    /* �����߳�ʵ����ִ�г�ʼ��(�����߳�ջ�ռ�ִ��) */
    virtual WORD32 Initialize();

    virtual WORD32 Notify();

    virtual CTimerRepo * GetTimerRepo();

protected : 
    virtual VOID DoRun();

    /* ���㳬ʱ */
    WORD32 Execute(WORD64 &rlwTimeOutNum, WORD32 dwThreshCount = INVALID_DWORD);

protected :
    CTimerRepo    m_cTimerRepo;
};


inline WORD32 CWorkThread::Notify()
{
    return SUCCESS;
}


inline CTimerRepo * CWorkThread::GetTimerRepo()
{
    return &m_cTimerRepo;
}


/* ���㳬ʱ */
inline WORD32 CWorkThread::Execute(WORD64 &rlwTimeOutNum, WORD32 dwThreshCount)
{
    /* û��App������ʱ��, �����κδ��� */
    if (0 == m_cTimerRepo.GetCount())
    {
        return SUCCESS;
    }

    /* ȡ��ǰϵͳʱ�� */
    WORD64 lwMicroSec = 0;
    WORD64 lwCycle    = 0;

    g_pGlobalClock->GetTime3(lwMicroSec, lwCycle);

    m_cTimerRepo.Decrease(lwMicroSec, dwThreshCount, rlwTimeOutNum);

    return SUCCESS;
}


#endif


