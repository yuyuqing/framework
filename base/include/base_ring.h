

#ifndef _BASE_RING_H_
#define _BASE_RING_H_

#ifdef ARCH_ARM64    
#else
#include <emmintrin.h>
#endif

#include <pthread.h>

#include "base_data_container.h"


inline WORD32 AtomicCompareSet(volatile WORD32 *pdwDst, 
                               WORD32 dwExp, 
                               WORD32 dwSrc)
{
#ifdef ARCH_ARM64    
    return __sync_bool_compare_and_swap(pdwDst, dwExp, dwSrc);
#else
    BYTE ucRes;

    asm volatile(
        "lock ; "
        "cmpxchgl %[dwSrc], %[pdwDst];"
        "sete %[ucRes];"
        : [ucRes] "=a" (ucRes),     /* output */
          [pdwDst] "=m" (*pdwDst)
        : [dwSrc] "r" (dwSrc),      /* input */
          "a" (dwExp),
          "m" (*pdwDst)
        : "memory");                /* no-clobber list */
    
    return ucRes;
#endif
}


/**< Ring size mask */
#define RTE_RING_SZ_MASK  (0x7fffffffU) 


typedef enum tagE_RingQueueBehavior 
{
	RING_QUEUE_FIXED = 0, /* Enq/Deq a fixed number of items from a ring */
	RING_QUEUE_VARIABLE   /* Enq/Deq as many items as possible from ring */
}E_RingQueueBehavior;


typedef struct tagT_RingHeadTail
{
    volatile WORD32 dwHead;
    volatile WORD32 dwTail;
}T_RingHeadTail;


typedef struct tagT_RingMeasure
{
    WORD32  dwProdHead;
    WORD32  dwProdTail;
    WORD32  dwConsHead;
    WORD32  dwConsTail;
    WORD32  dwOldValue;
    WORD32  dwNewValue;
}T_RingMeasure;


/* true if x is a power of 2 */
#define POWEROF2(x) ((((x)-1) & (x)) == 0)


#define RING_POWER_NUM   ((WORD32)(16))


class CBaseRingInterface : public CBaseData
{
public :
    CBaseRingInterface() {}
    virtual ~CBaseRingInterface() {};

    virtual WORD32 Count() = 0;
    virtual WORD32 FreeCount() = 0;

    /* 快照, 用于维测 */
    virtual VOID SnapShot(T_RingHeadTail &rtProd, T_RingHeadTail &rtCons) = 0;
};


template <WORD32 POWER_NUM>
class CSimpleRing : public CBaseRingInterface
{
public :
    static const WORD32 s_dwSize = (1 << POWER_NUM);
    static const WORD32 s_dwMask = (s_dwSize - 1);
    
public :
    CSimpleRing ();
    virtual ~CSimpleRing();

    WORD32 Initialize();

    virtual WORD32 Count();
    virtual WORD32 FreeCount();

    WORD32 Enqueue(VOID *pObj);

    WORD32 Dequeue(VOID **pObj);

    WORD32 Dequeue(VOID **pObj, WORD16 wThreshold);

    /* 快照, 用于维测 */
    virtual VOID SnapShot(T_RingHeadTail &rtProd, T_RingHeadTail &rtCons);

protected :
    WORD32 MoveProdHead(WORD32  dwNum,
                        WORD32 &rdwOldHead,
                        WORD32 &rdwNewHead,
                        WORD32 &rdwFreeEntries);

    WORD32 MoveConsHead(WORD32  dwNum,  
                        WORD32 &rdwOldHead,
                        WORD32 &rdwNewHead,
                        WORD32 &rdwEntries);

    VOID UpdateTail(T_RingHeadTail &rtTail, 
                    WORD32          dwOldValue, 
                    WORD32          dwNewValue, 
                    WORD32          dwEnqueue);

protected :
    T_RingHeadTail   m_tProd;
    T_RingHeadTail   m_tCons;
    VOID            *m_pQueue[s_dwSize];
};


template <WORD32 POWER_NUM>
CSimpleRing<POWER_NUM>::CSimpleRing ()
{
    m_tProd.dwHead = 0;
    m_tProd.dwTail = 0;
    m_tCons.dwHead = 0;
    m_tCons.dwTail = 0;
}


template <WORD32 POWER_NUM>
CSimpleRing<POWER_NUM>::~CSimpleRing()
{
    m_tProd.dwHead = 0;
    m_tProd.dwTail = 0;
    m_tCons.dwHead = 0;
    m_tCons.dwTail = 0;
    memset(m_pQueue, 0x00, (s_dwSize * sizeof(VOID *)));
}


template <WORD32 POWER_NUM>
WORD32 CSimpleRing<POWER_NUM>::Initialize()
{
    memset(m_pQueue, 0x00, (s_dwSize * sizeof(VOID *)));

    return SUCCESS;
}


template <WORD32 POWER_NUM>
inline WORD32 CSimpleRing<POWER_NUM>::Count()
{
    WORD32 dwProdTail = m_tProd.dwTail;
    WORD32 dwConsTail = m_tCons.dwTail;
    WORD32 dwCount    = (dwProdTail - dwConsTail) & (s_dwMask);
    
    return (dwCount > s_dwMask) ? s_dwMask : dwCount;
}


template <WORD32 POWER_NUM>
inline WORD32 CSimpleRing<POWER_NUM>::FreeCount()
{
    return (s_dwMask - Count());
}


template <WORD32 POWER_NUM>
inline WORD32 CSimpleRing<POWER_NUM>::Enqueue(VOID *pObj)
{
    WORD32 dwNum         = 0;
    WORD32 dwProdHead    = 0;
    WORD32 dwProdNext    = 0;
    WORD32 dwFreeEntries = 0;

    do
    {
        dwNum = MoveProdHead(1,
                             dwProdHead,
                             dwProdNext,
                             dwFreeEntries);
    } while(0 == dwNum);

    WORD32 dwIndex = (dwProdHead & s_dwMask);

    m_pQueue[dwIndex] = pObj;

    UpdateTail(m_tProd, dwProdHead, dwProdNext, 1);
        
    return dwNum;
}


template <WORD32 POWER_NUM>
inline WORD32 CSimpleRing<POWER_NUM>::Dequeue(VOID **pObj)
{
    WORD32 dwNum      = 0;
    WORD32 dwConsHead = 0;
    WORD32 dwConsNext = 0;
    WORD32 dwEntries  = 0;

    dwNum = MoveConsHead(1,  
                         dwConsHead,
                         dwConsNext,
                         dwEntries);
    if (0 == dwNum)
    {
        return dwNum;
    }

    WORD32 dwIndex = (dwConsHead & s_dwMask);

    *pObj = m_pQueue[dwIndex];

    __builtin_prefetch(((CHAR *)pObj), 1, 2);
    
    UpdateTail(m_tCons, dwConsHead, dwConsNext, 0);
    
    return dwNum;
}


template <WORD32 POWER_NUM>
inline WORD32 CSimpleRing<POWER_NUM>::Dequeue(VOID **pObj, WORD16 wThreshold)
{
    WORD32 dwNum      = 0;
    WORD32 dwConsHead = 0;
    WORD32 dwConsNext = 0;
    WORD32 dwEntries  = 0;

    do 
    {
        dwNum = MoveConsHead(1,  
                             dwConsHead,
                             dwConsNext,
                             dwEntries);
        if (0 == wThreshold)
        {
            break ;
        }

        wThreshold--;
    } while(0 == dwNum);

    if (0 == dwNum)
    {
        return dwNum;
    }

    WORD32 dwIndex = (dwConsHead & s_dwMask);

    *pObj = m_pQueue[dwIndex];

    __builtin_prefetch(((CHAR *)pObj), 1, 2);
    
    UpdateTail(m_tCons, dwConsHead, dwConsNext, 0);
    
    return dwNum;
}


template <WORD32 POWER_NUM>
inline VOID CSimpleRing<POWER_NUM>::SnapShot(
    T_RingHeadTail &rtProd,
    T_RingHeadTail &rtCons)
{
    rtProd.dwHead = __atomic_load_n(&(m_tProd.dwHead), __ATOMIC_RELAXED);
    rtProd.dwTail = __atomic_load_n(&(m_tProd.dwTail), __ATOMIC_RELAXED);
    rtCons.dwHead = __atomic_load_n(&(m_tCons.dwHead), __ATOMIC_RELAXED);
    rtCons.dwTail = __atomic_load_n(&(m_tCons.dwTail), __ATOMIC_RELAXED);
}


template <WORD32 POWER_NUM>
inline WORD32 CSimpleRing<POWER_NUM>::MoveProdHead(WORD32  dwNum,
                                                   WORD32 &rdwOldHead,
                                                   WORD32 &rdwNewHead,
                                                   WORD32 &rdwFreeEntries)
{
    WORD32 dwCapacity = s_dwMask;
    WORD32 dwMax      = dwNum;
    WORD32 dwSucces   = 0;
    WORD32 dwConsTail = 0;

    rdwOldHead = __atomic_load_n(&(m_tProd.dwHead), __ATOMIC_RELAXED);

    do
    {
        dwNum = dwMax;
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
        dwConsTail = __atomic_load_n(&(m_tCons.dwTail), __ATOMIC_ACQUIRE);

        rdwFreeEntries = (dwCapacity + dwConsTail - rdwOldHead);

        if (unlikely(dwNum > rdwFreeEntries))
        {
            dwNum = rdwFreeEntries;
        }

        if (0 == dwNum)
        {
            return 0;
        }

        rdwNewHead = rdwOldHead + dwNum;

        /* 若dwHead取值与rdwOldHead相等, 则将rdwNewHead的值写入dwHead */
        /* 若dwHead取值与rdwOldHead不等, 则将dwHead的值写入rdwOldHead */
        dwSucces = __atomic_compare_exchange_n(&(m_tProd.dwHead),
                                               &rdwOldHead,
                                               rdwNewHead,
                                               0,
                                               __ATOMIC_RELAXED,
                                               __ATOMIC_RELAXED);
    }while (unlikely(dwSucces == 0));

    return dwNum;
}


template <WORD32 POWER_NUM>
inline WORD32 CSimpleRing<POWER_NUM>::MoveConsHead(WORD32  dwNum,
                                                   WORD32 &rdwOldHead,
                                                   WORD32 &rdwNewHead,
                                                   WORD32 &rdwEntries)
{
    WORD32 dwMax      = dwNum;
    WORD32 dwSucces   = 0;
    WORD32 dwProdTail = 0;

    rdwOldHead = __atomic_load_n(&(m_tCons.dwHead), __ATOMIC_RELAXED);

    do
    {
        dwNum = dwMax;
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
        dwProdTail = __atomic_load_n(&(m_tProd.dwTail), __ATOMIC_ACQUIRE);

        rdwEntries = (dwProdTail - rdwOldHead);

        if (dwNum > rdwEntries)
        {
            dwNum = rdwEntries;
        }

        if (unlikely(dwNum == 0))
        {
            return 0;
        }

        rdwNewHead = rdwOldHead + dwNum;

        /* 若dwHead取值与rdwOldHead相等, 则将rdwNewHead的值写入dwHead */
        /* 若dwHead取值与rdwOldHead不等, 则将dwHead的值写入rdwOldHead */
        dwSucces = __atomic_compare_exchange_n(&(m_tCons.dwHead),
                                               &rdwOldHead,
                                               rdwNewHead,
                                               0,
                                               __ATOMIC_RELAXED,
                                               __ATOMIC_RELAXED);
    }while (unlikely(dwSucces == 0));

    return dwNum;
}


template <WORD32 POWER_NUM>
inline VOID CSimpleRing<POWER_NUM>::UpdateTail(T_RingHeadTail &rtTail, 
                                               WORD32          dwOldValue, 
                                               WORD32          dwNewValue, 
                                               WORD32          dwEnqueue)
{
    while (__atomic_load_n(&(rtTail.dwTail), __ATOMIC_RELAXED) != dwOldValue)
    {
#ifdef ARCH_ARM64    
        asm volatile("yield" ::: "memory");
#else
        _mm_pause();
#endif
    }

    __atomic_store_n(&(rtTail.dwTail), dwNewValue, __ATOMIC_RELEASE);
}


template <WORD32 POWER_NUM = RING_POWER_NUM>
class CBaseRingTpl : public CBaseRingInterface
{
public :
    static const WORD32 s_dwSize = (1 << POWER_NUM);
    static const WORD32 s_dwMask = (s_dwSize - 1);
    
public :
    CBaseRingTpl ();
    virtual ~CBaseRingTpl();

    VOID Reset();

    virtual WORD32 Initialize();

    WORD32 Free();

    VOID ** GetQueue();

    virtual WORD32 Count();
    virtual WORD32 FreeCount();

    BOOL isFull();
    BOOL isEmpty();

    WORD32 GetSize();
    WORD32 GetCapacity();

    virtual WORD32 Enqueue(VOID *pObj);

    virtual WORD32 Enqueue(VOID *pObj, WORD16 wThreshold);

    WORD32 Dequeue(VOID **pObj);

    /* 写入dwNum个Objs, 并返回空闲空间大小 */
    WORD32 EnqueueBurst(WORD32   dwNum, 
                        VOID   **pObjs, 
                        WORD32  &rdwFreeSize);

    /* 读出dwNum个Objs, 并返回剩余待读取数量 */
    WORD32 DequeueBurst(WORD32   dwNum,
                        VOID   **pObjs,
                        WORD32  &rdwAvailable);

    /* 快照, 用于维测 */
    virtual VOID SnapShot(T_RingHeadTail &rtProd, T_RingHeadTail &rtCons);

protected :
    WORD32 DoEnqueue(WORD32   dwNum, 
                     VOID   **pObjs, 
                     WORD32  &rdwFreeSize);

    WORD32 DoDequeue(WORD32   dwNum,
                     VOID   **pObjs,
                     WORD32  &rdwAvailable);

    WORD32 MoveProdHead(WORD32  dwNum,
                        WORD32 &rdwOldHead,
                        WORD32 &rdwNewHead,
                        WORD32 &rdwFreeEntries);

    WORD32 MoveConsHead(WORD32  dwNum,  
                        WORD32 &rdwOldHead,
                        WORD32 &rdwNewNext,
                        WORD32 &rdwEntries);

    VOID UpdateTail(T_RingHeadTail &rtTail, 
                    WORD32          dwOldValue, 
                    WORD32          dwNewValue, 
                    WORD32          dwEnqueue);

protected :
    T_RingHeadTail   m_tProd;       /* 生产者游标 */
    T_RingHeadTail   m_tCons;       /* 消费者游标 */

    WORD32           m_dwProdID;
    WORD32           m_dwConsID;
    
    WORD32           m_dwCapacity;

    /* 队列指针 */
    VOID            *m_pQueue[s_dwSize];
};


template <WORD32 POWER_NUM>
CBaseRingTpl<POWER_NUM>::CBaseRingTpl ()
{
    m_dwCapacity = s_dwMask;
    Reset();
}


template <WORD32 POWER_NUM>
CBaseRingTpl<POWER_NUM>::~CBaseRingTpl()
{
    m_dwCapacity = s_dwMask;
    Free();
}


template <WORD32 POWER_NUM>
inline VOID CBaseRingTpl<POWER_NUM>::Reset()
{
    m_tProd.dwHead = 0;
    m_tProd.dwTail = 0;
    m_tCons.dwHead = 0;
    m_tCons.dwTail = 0;
}


template <WORD32 POWER_NUM>
WORD32 CBaseRingTpl<POWER_NUM>::Initialize()
{
    return Free();
}


template <WORD32 POWER_NUM>
inline WORD32 CBaseRingTpl<POWER_NUM>::Free()
{
    Reset();    
    memset(m_pQueue, 0x00, (s_dwSize * sizeof(VOID *)));

    return SUCCESS;
}


template <WORD32 POWER_NUM>
inline VOID ** CBaseRingTpl<POWER_NUM>::GetQueue()
{
    return m_pQueue;
}


template <WORD32 POWER_NUM>
inline WORD32 CBaseRingTpl<POWER_NUM>::Count()
{
    /* 内存屏障 */
#ifdef ARCH_ARM64    
    asm volatile("dmb " "ishld" : : : "memory");  /* 针对Arm64指令 */
#else        
    asm volatile ("" : : : "memory");             /* 针对X86指令 */
#endif

    WORD32 dwProdTail = __atomic_load_n(&(m_tProd.dwTail), __ATOMIC_RELAXED);
    WORD32 dwConsTail = __atomic_load_n(&(m_tCons.dwTail), __ATOMIC_RELAXED);
    WORD32 dwCount    = (dwProdTail - dwConsTail) & (s_dwMask);

    return (dwCount > m_dwCapacity) ? m_dwCapacity : dwCount;
}


template <WORD32 POWER_NUM>
inline WORD32 CBaseRingTpl<POWER_NUM>::FreeCount()
{
    return (m_dwCapacity - Count());
}


template <WORD32 POWER_NUM>
inline BOOL CBaseRingTpl<POWER_NUM>::isFull()
{
    return (0 == FreeCount());
}


template <WORD32 POWER_NUM>
inline BOOL CBaseRingTpl<POWER_NUM>::isEmpty()
{
    return (0 == Count());
}


template <WORD32 POWER_NUM>
inline WORD32 CBaseRingTpl<POWER_NUM>::GetSize()
{
    return s_dwSize;
}


template <WORD32 POWER_NUM>
inline WORD32 CBaseRingTpl<POWER_NUM>::GetCapacity()
{
    return m_dwCapacity;
}


template <WORD32 POWER_NUM>
inline WORD32 CBaseRingTpl<POWER_NUM>::Enqueue(VOID *pObj)
{
    WORD32 dwFreeSize = 0;
    return DoEnqueue(1, &pObj, dwFreeSize);
}


template <WORD32 POWER_NUM>
inline WORD32 CBaseRingTpl<POWER_NUM>::Enqueue(VOID *pObj, WORD16 wThreshold)
{
    WORD32 dwNum      = 0;
    WORD32 dwLoop     = 0;
    WORD32 dwFreeSize = 0;

    do
    {
        dwNum = DoEnqueue(1, &pObj, dwFreeSize);
        dwLoop++;
    }while ((0 == dwNum) && (dwLoop < wThreshold));

    return dwNum;
}


template <WORD32 POWER_NUM>
inline WORD32 CBaseRingTpl<POWER_NUM>::Dequeue(VOID **pObj)
{
    WORD32 dwAvailable = 0;
    return DoDequeue(1, pObj, dwAvailable);
}


/* 写入dwNum个Objs, 并返回空闲空间大小rdwFreeSize */
template <WORD32 POWER_NUM>
inline WORD32 CBaseRingTpl<POWER_NUM>::EnqueueBurst(
    WORD32   dwNum, 
    VOID   **pObjs, 
    WORD32  &rdwFreeSize)
{
    return DoEnqueue(dwNum, pObjs, rdwFreeSize);
}


/* 读出dwNum个Objs, 并返回剩余待读取数量 */
template <WORD32 POWER_NUM>
inline WORD32 CBaseRingTpl<POWER_NUM>::DequeueBurst(
    WORD32   dwNum,
    VOID   **pObjs,
    WORD32  &rdwAvailable)
{
    return DoDequeue(dwNum, pObjs, rdwAvailable);
}


template <WORD32 POWER_NUM>
inline VOID CBaseRingTpl<POWER_NUM>::SnapShot(
    T_RingHeadTail &rtProd,
    T_RingHeadTail &rtCons)
{
    rtProd.dwHead = __atomic_load_n(&(m_tProd.dwHead), __ATOMIC_RELAXED);
    rtProd.dwTail = __atomic_load_n(&(m_tProd.dwTail), __ATOMIC_RELAXED);
    rtCons.dwHead = __atomic_load_n(&(m_tCons.dwHead), __ATOMIC_RELAXED);
    rtCons.dwTail = __atomic_load_n(&(m_tCons.dwTail), __ATOMIC_RELAXED);
}


template <WORD32 POWER_NUM>
inline WORD32 CBaseRingTpl<POWER_NUM>::DoEnqueue(
    WORD32   dwNum, 
    VOID   **pObjs, 
    WORD32  &rdwFreeSize)
{
    WORD32 dwProdHead    = 0;
    WORD32 dwProdNext    = 0;
    WORD32 dwFreeEntries = 0;

    dwNum = MoveProdHead(dwNum,
                         dwProdHead,
                         dwProdNext,
                         dwFreeEntries);
    if (0 == dwNum)
    {
        rdwFreeSize = dwFreeEntries;
        return dwNum;
    }

    WORD32 dwI     = 0;
    WORD32 dwSize  = s_dwSize;
    WORD32 dwIndex = (dwProdHead & s_dwMask);
    VOID **pQueue  = m_pQueue;

    if (likely(dwIndex + dwNum < dwSize))
    {
        for (dwI = 0; dwI < (dwNum & ((~(unsigned)0x3))); dwI += 4, dwIndex += 4)
        {
            pQueue[dwIndex]   = pObjs[dwI];
            pQueue[dwIndex+1] = pObjs[dwI+1];
            pQueue[dwIndex+2] = pObjs[dwI+2];
            pQueue[dwIndex+3] = pObjs[dwI+3];
        }

        switch (dwNum & 0x03)
        {
        case 3:
            pQueue[dwIndex++] = pObjs[dwI++];
        case 2:
            pQueue[dwIndex++] = pObjs[dwI++];
        case 1:
            pQueue[dwIndex++] = pObjs[dwI++];
        }
    }
    else
    {
        for (dwI = 0; dwIndex < dwSize; dwI++, dwIndex++)
        {
            pQueue[dwIndex] = pObjs[dwI];
        }

        for (dwIndex = 0; dwI < dwNum; dwI++, dwIndex++)
        {
            pQueue[dwIndex] = pObjs[dwI];
        }
    }

    UpdateTail(m_tProd, dwProdHead, dwProdNext, 1);

    rdwFreeSize = dwFreeEntries - dwNum;

    return dwNum;
}


template <WORD32 POWER_NUM>
inline WORD32 CBaseRingTpl<POWER_NUM>::DoDequeue(
    WORD32   dwNum,
    VOID   **pObjs,
    WORD32  &rdwAvailable)
{
    WORD32 dwConsHead = 0;
    WORD32 dwConsNext = 0;
    WORD32 dwEntries  = 0;

    dwNum = MoveConsHead(dwNum,  
                         dwConsHead,
                         dwConsNext,
                         dwEntries);
    if (0 == dwNum)
    {
        rdwAvailable = dwEntries;
        return dwNum;
    }

    WORD32 dwI     = 0;
    WORD32 dwSize  = s_dwSize;
    WORD32 dwIndex = (dwConsHead & s_dwMask);
    VOID **pQueue  = m_pQueue;

    if (likely(dwIndex + dwNum < dwSize))
    {
        for (dwI = 0; dwI < (dwNum & ((~(unsigned)0x3))); dwI += 4, dwIndex += 4)
        {
            pObjs[dwI]   = pQueue[dwIndex];
            pObjs[dwI+1] = pQueue[dwIndex+1];
            pObjs[dwI+2] = pQueue[dwIndex+2];
            pObjs[dwI+3] = pQueue[dwIndex+3];
        }

        switch (dwNum & 0x03)
        {
        case 3:
            pObjs[dwI++] = pQueue[dwIndex++];
        case 2:
            pObjs[dwI++] = pQueue[dwIndex++];
        case 1:
            pObjs[dwI++] = pQueue[dwIndex++];
        }
    }
    else
    {
        for (dwI = 0; dwIndex < dwSize; dwI++, dwIndex++)
        {
            pObjs[dwI] = pQueue[dwIndex];
        }

        for (dwIndex = 0; dwI < dwNum; dwI++, dwIndex++)
        {
            pObjs[dwI] = pQueue[dwIndex];
        }
    }

    UpdateTail(m_tCons, dwConsHead, dwConsNext, 0);

    rdwAvailable = dwEntries - dwNum;

    return dwNum;
}


template <WORD32 POWER_NUM>
inline WORD32 CBaseRingTpl<POWER_NUM>::MoveProdHead(
    WORD32  dwNum,
    WORD32 &rdwOldHead,
    WORD32 &rdwNewHead,
    WORD32 &rdwFreeEntries)
{
    WORD32 dwCapacity = m_dwCapacity;
    WORD32 dwMax      = dwNum;
    WORD32 dwSucces   = 0;
    WORD32 dwConsTail = 0;

    rdwOldHead = __atomic_load_n(&(m_tProd.dwHead), __ATOMIC_RELAXED);

    do
    {
        dwNum = dwMax;

        __atomic_thread_fence(__ATOMIC_ACQUIRE);

        dwConsTail = __atomic_load_n(&(m_tCons.dwTail), __ATOMIC_ACQUIRE);

        rdwFreeEntries = (dwCapacity + dwConsTail - rdwOldHead);

        if (unlikely(dwNum > rdwFreeEntries))
        {
            dwNum = rdwFreeEntries;
        }

        if (0 == dwNum)
        {
            return 0;
        }

        rdwNewHead = rdwOldHead + dwNum;

        /* 若dwHead取值与rdwOldHead相等, 则将rdwNewHead的值写入dwHead */
        /* 若dwHead取值与rdwOldHead不等, 则将dwHead的值写入rdwOldHead */
        dwSucces = __atomic_compare_exchange_n(&(m_tProd.dwHead),
                                               &rdwOldHead,
                                               rdwNewHead,
                                               0,
                                               __ATOMIC_RELAXED,
                                               __ATOMIC_RELAXED);
    }while (unlikely(dwSucces == 0));

    return dwNum;
}


template <WORD32 POWER_NUM>
inline WORD32 CBaseRingTpl<POWER_NUM>::MoveConsHead(
    WORD32  dwNum,  
    WORD32 &rdwOldHead,
    WORD32 &rdwNewHead,
    WORD32 &rdwEntries)
{
    WORD32 dwMax      = dwNum;
    WORD32 dwSucces   = 0;
    WORD32 dwProdTail = 0;

    rdwOldHead = __atomic_load_n(&(m_tCons.dwHead), __ATOMIC_RELAXED);

    do
    {
        dwNum = dwMax;

        __atomic_thread_fence(__ATOMIC_ACQUIRE);

        dwProdTail = __atomic_load_n(&(m_tProd.dwTail), __ATOMIC_ACQUIRE);

        rdwEntries = (dwProdTail - rdwOldHead);

        if (dwNum > rdwEntries)
        {
            dwNum = rdwEntries;
        }

        if (unlikely(dwNum == 0))
        {
            return 0;
        }

        rdwNewHead = rdwOldHead + dwNum;

        /* 若dwHead取值与rdwOldHead相等, 则将rdwNewHead的值写入dwHead */
        /* 若dwHead取值与rdwOldHead不等, 则将dwHead的值写入rdwOldHead */
        dwSucces = __atomic_compare_exchange_n(&(m_tCons.dwHead),
                                               &rdwOldHead,
                                               rdwNewHead,
                                               0,
                                               __ATOMIC_RELAXED,
                                               __ATOMIC_RELAXED);
    }while (unlikely(dwSucces == 0));

    return dwNum;
}


template <WORD32 POWER_NUM>
inline VOID CBaseRingTpl<POWER_NUM>::UpdateTail(
    T_RingHeadTail &rtTail, 
    WORD32          dwOldValue, 
    WORD32          dwNewValue, 
    WORD32          dwEnqueue)
{
    while (__atomic_load_n(&(rtTail.dwTail), __ATOMIC_RELAXED) != dwOldValue)
    {
#ifdef ARCH_ARM64    
        asm volatile("yield" ::: "memory");
#else
        _mm_pause();
#endif
    }

    __atomic_store_n(&(rtTail.dwTail), dwNewValue, __ATOMIC_RELEASE);
}


#endif


