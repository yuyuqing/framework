

#ifndef _BASE_LOCK_H_
#define _BASE_LOCK_H_


#include <stdint.h>
#include <unistd.h>

#ifdef ARCH_ARM64    
#else
#include <emmintrin.h>
#endif

#include <pthread.h>
#include <semaphore.h>
#include <atomic>


#include "pub_typedef.h"


template<typename T>
class CBaseLock
{
public :
    CBaseLock () {}    
    virtual ~CBaseLock() {};

    virtual WORD32 Init()
    {
        return SUCCESS;
    }

    virtual WORD32 Destroy()
    {
        return SUCCESS;
    }

    virtual WORD32 Lock() = 0;
    virtual WORD32 UnLock() = 0;

    operator T& ()
    {
        return m_tLock;
    }

    operator T* ()
    {
        return &m_tLock;
    }

protected :
    T  m_tLock;
};


template <typename Lock>
class CGuardLock 
{
public :
    CGuardLock(Lock &rLock)
        : m_rLock(rLock)
    {
        m_rLock.Lock();
    }

    ~CGuardLock()
    {
        m_rLock.UnLock();
    }

protected :
    Lock &m_rLock;
};


class CNullLock : public CBaseLock<WORD32>
{
public :
    CNullLock () {}
    virtual ~CNullLock() {}

    WORD32 Lock();
    WORD32 UnLock();
};


inline WORD32 CNullLock::Lock()
{
    return SUCCESS;
}


inline WORD32 CNullLock::UnLock()
{
    return SUCCESS;
}


class CMutex : public CBaseLock<pthread_mutex_t>
{
public :
    CMutex ();
    ~CMutex();
    
    WORD32 Init();    
    WORD32 Destroy();
    WORD32 Lock();
    WORD32 UnLock();
};


class CCondition
{
public :
    CCondition (CMutex &rMutex);    
    ~CCondition();

    WORD32 Init();
    WORD32 Destroy();
    WORD32 Wait();
    WORD32 Notify();
    WORD32 NotifyAll();

protected :
    CMutex         &m_rMutex;
    pthread_cond_t  m_tCond;
};


class CSemaphore
{
public :
    CSemaphore ();
    virtual ~CSemaphore();

    WORD32 Init();
    WORD32 Destroy();
    WORD32 Wait();
    WORD32 Post();

    /* 在当前系统时间基础上增加dwTick个毫秒等待 : 单位us */
    WORD32 TimeWait(WORD64 lwTick);  

protected :
    sem_t      m_tSem;
};


inline WORD32 CSemaphore::Post()
{
    return sem_post(&m_tSem);
}


class CUserSemaphore
{
public :
    CUserSemaphore();
    virtual ~CUserSemaphore();

    WORD32 Wait();
    WORD32 Post();

protected :
    std::atomic<WORD32>  m_dwCount;
};


inline WORD32 CUserSemaphore::Wait()
{
    while (0 == m_dwCount.load(std::memory_order_relaxed))
    {
    #ifdef ARCH_ARM64
        asm volatile("yield" ::: "memory");
    #else
        _mm_pause();
    #endif
    }

    m_dwCount.fetch_sub(1, std::memory_order_relaxed);

    return SUCCESS;
}


inline WORD32 CUserSemaphore::Post()
{
    m_dwCount.fetch_add(1, std::memory_order_relaxed);

    return SUCCESS;
}


#define TRY_LOCK_THRESHOLD    ((WORD32)(64))


class CSpinLock
{
public :
    CSpinLock ();
    virtual ~CSpinLock();

    WORD32 TryLock(WORD32 dwLoopThreshold = TRY_LOCK_THRESHOLD);

    VOID Lock();
    VOID UnLock();

protected :    
    volatile SWORD32  m_iLock;
};


inline WORD32 CSpinLock::TryLock(WORD32 dwLoopThreshold)
{
    SWORD32 iValue = 0;

    while (!__atomic_compare_exchange_n(&m_iLock, 
                                        &iValue, 
                                        1, 0,
                                        __ATOMIC_ACQUIRE, 
                                        __ATOMIC_RELAXED))
    {
        while (__atomic_load_n(&m_iLock, __ATOMIC_RELAXED))
        {
            dwLoopThreshold--;
            if (0 == dwLoopThreshold)
            {
                return FAIL;
            }

            #ifdef ARCH_ARM64    
            asm volatile("yield" ::: "memory");
            #else
            _mm_pause();
            #endif
        }

        iValue = 0;
    }

    return SUCCESS;
}


inline VOID CSpinLock::Lock()
{
    SWORD32 iValue = 0;

    while (!__atomic_compare_exchange_n(&m_iLock, 
                                        &iValue, 
                                        1, 0,
                                        __ATOMIC_ACQUIRE, 
                                        __ATOMIC_RELAXED))
    {
        while (__atomic_load_n(&m_iLock, __ATOMIC_RELAXED))
        {
            #ifdef ARCH_ARM64    
            asm volatile("yield" ::: "memory");
            #else
            _mm_pause();
            #endif
        }

        iValue = 0;
    }
}


inline VOID CSpinLock::UnLock()
{
    __atomic_store_n(&m_iLock, 0, __ATOMIC_RELEASE);
}


class CAtomicLock
{
public :
    CAtomicLock ();
    ~CAtomicLock();

    WORD32 TryLock(WORD32 dwLoopThreshold);

    VOID Lock();
    VOID UnLock();

protected :
    std::atomic<BOOL>  m_bLocked;
};


inline VOID CAtomicLock::Lock()
{
    BOOL bResult = FALSE;
    BOOL bExpect = FALSE;

    do
    {
        bExpect = FALSE;
        bResult = m_bLocked.compare_exchange_strong(bExpect,
                                                    TRUE,
                                                    std::memory_order_relaxed);
    }while(!bResult);
}


inline VOID CAtomicLock::UnLock()
{
    m_bLocked.store(FALSE, std::memory_order_relaxed);
}


class CTicketLock
{
public :
    typedef union tagT_TicketLock
    {
        WORD32  dwTicket;
        struct
        {
            WORD16  wCurrent;
            WORD16  wNext;
        } s;
    }T_TicketLock;

public :
    CTicketLock ();
    virtual ~CTicketLock();

    VOID Lock();
    VOID UnLock();

protected :
    volatile T_TicketLock  m_tLock;
};


inline VOID CTicketLock::Lock()
{
    /* 加锁 */
    WORD16 wCurrent = __atomic_fetch_add(&(m_tLock.s.wNext), 
                                         1, 
                                         __ATOMIC_RELAXED);
    while (wCurrent != __atomic_load_n(&(m_tLock.s.wCurrent), __ATOMIC_ACQUIRE))
    {
    #ifdef ARCH_ARM64    
        asm volatile("yield" ::: "memory");
    #else
        _mm_pause();
    #endif
    }
}


inline VOID CTicketLock::UnLock()
{
    /* 解锁 */
    WORD16 wCurrent = __atomic_load_n(&(m_tLock.s.wCurrent), __ATOMIC_RELAXED);
    __atomic_store_n(&(m_tLock.s.wCurrent), wCurrent + 1, __ATOMIC_RELEASE);
}


#endif


