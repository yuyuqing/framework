

#ifndef _BASE_MEM_POOL_H_
#define _BASE_MEM_POOL_H_


#include "base_lock.h"
#include "base_variable.h"
#include "base_ring_array.h"
#include "base_mem_interface.h"
#include "base_measure.h"


typedef enum tagE_MemSizeType
{
    E_MemSizeType_256  =   192,
    E_MemSizeType_512  =   448,
    E_MemSizeType_01K  =   960,
    E_MemSizeType_04K  =  4032,
    E_MemSizeType_16K  = 16320,
    E_MemSizeType_64K  = 65472,
}E_MemSizeType;


typedef enum tagE_MemBufPowerNum
{
    E_MemBufPowerNum_256 = 12,  /* 2^12 = 4096   1M */
    E_MemBufPowerNum_512 = 10,  /* 2^10 = 1024 512K */
    E_MemBufPowerNum_01K =  9,  /* 2^9  =  512 512K */
    E_MemBufPowerNum_04K =  8,  /* 2^8  =  256   1M */
    E_MemBufPowerNum_16K =  6,  /* 2^6  =   64   1M */
    E_MemBufPowerNum_64K =  5,  /* 2^5  =   32   2M */  
}E_MemBufPowerNum;


enum { MEM_POOL_NUM = 6 };


typedef struct tagT_GroupMemPool
{
    E_MemSizeType          eType;
    E_MemBufPowerNum       eNum;
    BYTE                  *pBuf;
    CObjMemPoolInterface  *pMemPool;
}T_GroupMemPool;


class CMsgMemPool : public CBaseData
{
public :
    const static WORD32 s_adwBufSize[MEM_POOL_NUM];

public :
    CMsgMemPool (CCentralMemPool &rCentralMemPool);
    virtual ~CMsgMemPool();
    
    virtual WORD32 Initialize();
    
    virtual BYTE * Malloc(WORD32 dwSize);

    virtual WORD32 Free(BYTE *pAddr);
    
    virtual BOOL IsValid(BYTE *pAddr);

    VOID GetMeasure(T_MsgMemMeasure &rtMeasure);

    VOID Dump();

protected :
    WORD32 Calc(WORD32 dwSize);

protected :
    CCentralMemPool  &m_rCentralMemPool;
    T_GroupMemPool    m_atPool[MEM_POOL_NUM];
};


inline BYTE * CMsgMemPool::Malloc(WORD32 dwSize)
{
    WORD32 dwPos = Calc(dwSize);
    if (unlikely(dwPos >= MEM_POOL_NUM))
    {
        return NULL;
    }

    BYTE *pValue = NULL;

    for (; dwPos < MEM_POOL_NUM; dwPos++)
    {
        pValue = m_atPool[dwPos].pMemPool->Malloc(dwSize);
        if (NULL != pValue)
        {
            break;
        }
    }

    return pValue;
}


inline WORD32 CMsgMemPool::Free(BYTE *pAddr)
{
    WORD32 dwPos  = 0;    
    BOOL   bValid = FALSE;

    for (; dwPos < MEM_POOL_NUM; dwPos++)
    {
        bValid = m_atPool[dwPos].pMemPool->IsValid(pAddr);
        if (bValid)
        {
            break ;
        }
    }

    if (unlikely(dwPos >= MEM_POOL_NUM))
    {
        return FAIL;
    }

    return m_atPool[dwPos].pMemPool->Free(pAddr);
}


inline BOOL CMsgMemPool::IsValid(BYTE *pAddr)
{
    BOOL bValid = FALSE;

    for (WORD32 dwIndex = 0; dwIndex < MEM_POOL_NUM; dwIndex++)
    {
        bValid = m_atPool[dwIndex].pMemPool->IsValid(pAddr);
        if (bValid)
        {
            break ;
        }
    }

    return bValid;
}


inline WORD32 CMsgMemPool::Calc(WORD32 dwSize)
{
    WORD32 dwPos = 0;
    for (; dwPos < MEM_POOL_NUM; dwPos++)
    {
        if (dwSize <= s_adwBufSize[dwPos])
        {
            break ;
        }
    }

    return dwPos;
}


#define POWER_OF_MEMPOOL              ((WORD32)(10))
#define LOG_MEM_BUF_SIZE              ((WORD32)(1024 - sizeof(T_MemBufHeader)))
#define TIMER_MSG_SIZE                ((WORD32)(256  - sizeof(T_MemBufHeader)))


typedef struct tagT_LogBufHeader
{
    WORD64   lwAddr;
    WORD64   lwFile;
    BYTE     ucLogType;     /* 0:普通接口日志, 1:Fast接口日志 */
    BYTE     ucResved1;
    BYTE     ucResved2;
    BYTE     ucResved3;
    WORD16   wModuleID;
    WORD16   wMsgLen;
}T_LogBufHeader;


typedef struct tagT_TimerMsgHeader
{
    WORD64  lwAddr;
    WORD64  lwStartCycle;
    WORD64  lwEndCycle;
    WORD32  dwMsgID;
    WORD16  wResved;
    WORD16  wMsgLen;
}T_TimerMsgHeader;


class CLogMemPool : public CObjectMemPool<POWER_OF_MEMPOOL>
{
public :
    static const WORD32 s_dwLogSize = LOG_MEM_BUF_SIZE - sizeof(T_LogBufHeader);

    typedef struct tagT_LogBuf
    {
        T_LogBufHeader  tLogHeader;
        BYTE            aucData[s_dwLogSize];
    }T_LogBuf;

public :
    CLogMemPool (CCentralMemPool &rCentralMemPool);
    virtual ~CLogMemPool();

    WORD32 Initialize();

    BYTE * Malloc(WORD32 dwSize = s_dwLogSize, WORD16 wThreshold = INVALID_WORD);
    WORD32 Free(BYTE *pAddr);
};


inline BYTE * CLogMemPool::Malloc(WORD32 dwSize, WORD16 wThreshold)
{    
    BYTE *pValue = CObjectMemPool<POWER_OF_MEMPOOL>::Malloc(dwSize, wThreshold, INVALID_DWORD);

    return (pValue) ? (pValue + sizeof(T_LogBufHeader)) : NULL;
}


inline WORD32 CLogMemPool::Free(BYTE *pAddr)
{
    if (unlikely(NULL == pAddr))
    {
        return FAIL;
    }

    BYTE *pValue = (BYTE *)((WORD64)pAddr - sizeof(T_LogBufHeader));

    return CObjectMemPool<POWER_OF_MEMPOOL>::Free(pValue);    
}


class CTimerMemPool : public CObjectMemPool<POWER_OF_MEMPOOL>
{
public :
    static const WORD32 s_dwTimerSize = TIMER_MSG_SIZE - sizeof(T_TimerMsgHeader);

public :
    CTimerMemPool (CCentralMemPool &rCentralMemPool);
    virtual ~CTimerMemPool();

    WORD32 Initialize();

    BYTE * Malloc(WORD32 dwSize = s_dwTimerSize, WORD16 wThreshold = INVALID_WORD);

    WORD32 Free(BYTE *pAddr);
};


/* 备注 : 分配时未偏移头, 释放时也不作偏移处理 */
inline BYTE * CTimerMemPool::Malloc(WORD32 dwSize, WORD16 wThreshold)
{
    BYTE *pValue = CObjectMemPool<POWER_OF_MEMPOOL>::Malloc(dwSize, wThreshold, INVALID_DWORD);

    return pValue;
}


/* 备注 : 分配时未偏移头, 释放时也不作偏移处理 */
inline WORD32 CTimerMemPool::Free(BYTE *pAddr)
{
    if (unlikely(NULL == pAddr))
    {
        return FAIL;
    }

    return CObjectMemPool<POWER_OF_MEMPOOL>::Free(pAddr);    
}


/* T的类型必须是CObjectMemPool<>的派生类, 例如: CLogMemPool/CTimerMemPool 
 * 用于多线程场景下的高并发处理(例如: 多个线程写日志, 多个线程启停定时器等)
 */
template<class T>
class CMultiMemPool : public CBaseData
{
public :
    enum { SINGLE_POOL_NUM = 128 };

    typedef CBaseArray<T, SINGLE_POOL_NUM>    CSTMemPoolArray;

public :
    CMultiMemPool (CCentralMemPool &rCentralMemPool);
    virtual ~CMultiMemPool();

    virtual WORD32 Initialize();

    /* 在RegistZone时调用(即, 在业务线程创建后, 在业务线程的栈空间调用) */
    T * CreateSTPool();

    BYTE * Malloc(WORD32 dwSize, WORD16 wThreshold);

    WORD32 Free(BYTE *pAddr);

protected :
    CCentralMemPool   &m_rCentralMemPool;

    CAtomicLock        m_cLock;
    volatile WORD32    m_dwRingNum;
    T                 *m_apPool[SINGLE_POOL_NUM];
};


template<class T>
CMultiMemPool<T>::CMultiMemPool (CCentralMemPool &rCentralMemPool)
    : m_rCentralMemPool(rCentralMemPool)
{
    m_dwRingNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < SINGLE_POOL_NUM; dwIndex++)
    {
        m_apPool[dwIndex] = NULL;
    }
}


template<class T>
CMultiMemPool<T>::~CMultiMemPool()
{
    m_dwRingNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < SINGLE_POOL_NUM; dwIndex++)
    {
        if (NULL != m_apPool[dwIndex])
        {
            delete m_apPool[dwIndex];
            m_rCentralMemPool.Free((BYTE *)m_apPool[dwIndex]);
        }

        m_apPool[dwIndex] = NULL;
    }
}


template<class T>
WORD32 CMultiMemPool<T>::Initialize()
{
    return SUCCESS;
}


/* 在RegistZone时调用(即, 在业务线程创建后, 在业务线程的栈空间调用) */
template<class T>
T * CMultiMemPool<T>::CreateSTPool()
{
    WORD32 dwQueueID = m_dwSelfRingID % SINGLE_POOL_NUM;
    if (m_dwRingNum >= SINGLE_POOL_NUM)
    {
        assert(0);
    }

    BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(T));
    if (NULL == pMem)
    {
        assert(0);
    }

    T *pPool = new (pMem) T(m_rCentralMemPool);
    pPool->Initialize();

    m_cLock.Lock();

    if (NULL != m_apPool[dwQueueID])
    {
        /* 避免重复创建 */
        assert(0);
    }

    m_apPool[dwQueueID] = pPool;

    m_dwRingNum++;

    m_cLock.UnLock();

    return pPool;
}


template<class T>
inline BYTE * CMultiMemPool<T>::Malloc(WORD32 dwSize, WORD16 wThreshold)
{
    WORD32 dwQueueID = m_dwSelfRingID % SINGLE_POOL_NUM;
    if (unlikely(NULL == m_apPool[dwQueueID]))
    {
        return NULL;
    }

    return m_apPool[dwQueueID]->Malloc(dwSize, wThreshold);
}


template<class T>
inline WORD32 CMultiMemPool<T>::Free(BYTE *pAddr)
{
    WORD32 dwQueueID = m_dwSelfRingID % SINGLE_POOL_NUM;
    if (unlikely(NULL == m_apPool[dwQueueID]))
    {
        return FAIL;
    }

    return m_apPool[dwQueueID]->Free(pAddr);
}


#endif


