

#ifndef _BASE_THREAD_MULTI_H_
#define _BASE_THREAD_MULTI_H_


#include "base_thread_worker.h"


/* 持有多条ST-Ring队列的工作线程 */
class CMultiThread : public CWorkThread
{
public :
    static WORD32 ProcMultiMessage(VOID *pObj, VOID *pMsg);

public :
    CMultiThread (const T_ThreadParam &rtParam);
    virtual ~CMultiThread ();

    /* 创建线程实例后执行初始化(在主线程栈空间执行) */
    virtual WORD32 Initialize();

    virtual CMultiMessageRing * GetMultiRing();

protected :
    virtual VOID DoRun();

protected :
    CMultiMessageRing  m_cMultiRing;
};


inline CMultiMessageRing * CMultiThread::GetMultiRing()
{
    return &m_cMultiRing;
}


#endif


