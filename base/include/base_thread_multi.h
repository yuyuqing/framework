

#ifndef _BASE_THREAD_MULTI_H_
#define _BASE_THREAD_MULTI_H_


#include "base_thread_worker.h"


/* ���ж���ST-Ring���еĹ����߳� */
class CMultiThread : public CWorkThread
{
public :
    static WORD32 ProcMultiMessage(VOID *pObj, VOID *pMsg);

public :
    CMultiThread (const T_ThreadParam &rtParam);
    virtual ~CMultiThread ();

    /* �����߳�ʵ����ִ�г�ʼ��(�����߳�ջ�ռ�ִ��) */
    virtual WORD32 Initialize();

    virtual WORD32 Notify();

    virtual CMultiMessageRing * GetMultiRing();

protected :
    virtual VOID DoRun();

protected :
    CMultiMessageRing  m_cMultiRing;
};


inline WORD32 CMultiThread::Notify()
{
    return m_cSemaphore.Post();
}


inline CMultiMessageRing * CMultiThread::GetMultiRing()
{
    return &m_cMultiRing;
}


#endif


