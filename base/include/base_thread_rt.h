

#ifndef _BASE_THREAD_RT_H_
#define _BASE_THREAD_RT_H_


#include "base_thread_polling.h"


class CRTThread : public CPollingThread
{
public :
    CRTThread (const T_ThreadParam &rtParam);
    virtual ~CRTThread();

    /* �����߳�ʵ����ִ�г�ʼ��(�����߳�ջ�ռ�ִ��) */
    virtual WORD32 Initialize();

    virtual WORD32 Notify();

    VOID Dump();

protected :
    virtual VOID DoRun();

protected :
    CSemaphore      m_cSemaphore;

private :
    CUserSemaphore  m_cUserSemaphore;
};


inline WORD32 CRTThread::Notify()
{
    return m_cUserSemaphore.Post();
}


#endif


