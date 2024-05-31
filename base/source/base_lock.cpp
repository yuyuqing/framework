

#include <iostream>

#include "base_lock.h"
#include "base_time.h"
#include "base_variable.h"


using namespace std;


CMutex::CMutex ()
{
    Init();
}


CMutex::~CMutex()
{
    Destroy();
}


WORD32 CMutex::Init()
{
    return pthread_mutex_init(&m_tLock, NULL);
}


WORD32 CMutex::Destroy()
{
    return pthread_mutex_destroy(&m_tLock);
}


WORD32 CMutex::Lock()
{
    return pthread_mutex_lock(&m_tLock);
}


WORD32 CMutex::UnLock()
{
    return pthread_mutex_unlock(&m_tLock);
}


CCondition::CCondition (CMutex &rMutex)
    :m_rMutex(rMutex)
{
    Init();
}


CCondition::~CCondition()
{
    Destroy();
}


WORD32 CCondition::Init() 
{
    return pthread_cond_init(&m_tCond, NULL);
}


WORD32 CCondition::Destroy() 
{
    return pthread_cond_destroy(&m_tCond);
}


WORD32 CCondition::Wait()
{
    return pthread_cond_wait(&m_tCond, 
                             (pthread_mutex_t *)m_rMutex);
}


WORD32 CCondition::Notify()
{
    return pthread_cond_signal(&m_tCond);
}


WORD32 CCondition::NotifyAll()
{
    return pthread_cond_broadcast(&m_tCond);
}


CSemaphore::CSemaphore ()
{
    Init();
}


CSemaphore::~CSemaphore()
{
    Destroy();
}


WORD32 CSemaphore::Init()
{
    return sem_init(&m_tSem, FALSE, 0);
}


WORD32 CSemaphore::Destroy()
{
    return sem_destroy(&m_tSem);
}


WORD32 CSemaphore::Wait()
{
    SWORD32 iCount = 0;

    do 
    {
        iCount = sem_wait(&m_tSem);
    }while ((-1 == iCount) && (EINTR == errno));

    return (WORD32)iCount;
}


/* 在当前系统时间基础上增加dwTick个毫秒等待 : 单位us */
WORD32 CSemaphore::TimeWait(WORD64 lwTick)
{
    struct timespec tTime;

    SWORD32 iCount     = 0;
    WORD64  lwMicroSec = 0;
    WORD64  lwCycle    = 0;

    g_pGlobalClock->GetTime3(lwMicroSec , lwCycle);

    lwMicroSec += lwTick;

    tTime.tv_sec  = lwMicroSec / 1000000;
    tTime.tv_nsec = (lwMicroSec % 1000000) * 1000;

    do 
    {
        iCount = sem_timedwait(&m_tSem, &tTime);
        if (ETIMEDOUT == errno)
        {
            iCount = ETIMEDOUT;
        }
    }while ((-1 == iCount) && (EINTR == errno));
    
    return (WORD32)iCount;
}


CUserSemaphore::CUserSemaphore()
{
    m_bStatus = FALSE;
}


CUserSemaphore::~CUserSemaphore()
{
    m_bStatus = FALSE;
}


CSpinLock::CSpinLock ()
{
    m_iLock = 0;
}


CSpinLock::~CSpinLock()
{
    m_iLock = 0;
}


CAtomicLock::CAtomicLock ()
{
    m_bLocked = FALSE;
}


CAtomicLock::~CAtomicLock()
{
    m_bLocked = FALSE;
}


WORD32 CAtomicLock::TryLock(WORD32 dwLoopThreshold)
{
    BOOL    bExpect     = FALSE;
    BOOL    bLocked     = FALSE;
    WORD32  dwThreshold = dwLoopThreshold;

    do
    {
        bLocked = m_bLocked.load(std::memory_order_relaxed);

        dwThreshold--;
    }while((bLocked) && (dwThreshold > 0));

    if (bLocked)
    {
        return FAIL;
    }

    while(!(m_bLocked.compare_exchange_strong(bExpect,
                                              TRUE,
                                              std::memory_order_relaxed)))
    {
        bExpect = FALSE;

        dwLoopThreshold--;
        if (0 == dwLoopThreshold)
        {
            return FAIL;
        }
    }

    return SUCCESS;
}


CTicketLock::CTicketLock ()
{
    __atomic_store_n(&(m_tLock.dwTicket), 0, __ATOMIC_RELAXED);
}


CTicketLock::~CTicketLock()
{
    __atomic_store_n(&(m_tLock.dwTicket), 0, __ATOMIC_RELAXED);
}


