

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

    /* 创建线程实例后执行初始化(在主线程栈空间执行) */
    virtual WORD32 Initialize();

    virtual WORD32 Notify();

    virtual CTimerRepo * GetTimerRepo();

protected : 
    virtual VOID DoRun();

    /* 计算超时 */
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


/* 计算超时 */
inline WORD32 CWorkThread::Execute(WORD64 &rlwTimeOutNum, WORD32 dwThreshCount)
{
    /* 没有App启动定时器, 无需任何处理 */
    if (0 == m_cTimerRepo.GetCount())
    {
        return SUCCESS;
    }

    /* 取当前系统时间 */
    WORD64 lwMicroSec = 0;
    WORD64 lwCycle    = 0;

    g_pGlobalClock->GetTime3(lwMicroSec, lwCycle);

    m_cTimerRepo.Decrease(lwMicroSec, dwThreshCount, rlwTimeOutNum);

    return SUCCESS;
}


#endif


