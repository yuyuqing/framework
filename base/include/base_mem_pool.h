

#ifndef _BASE_MEM_POOL_H_
#define _BASE_MEM_POOL_H_


#include "base_mem_interface.h"
#include "base_config_file.h"


#define SINGLE_POOL_NUM              ((WORD32)(128))


/* T的类型必须是CObjectMemPool<>的派生类, 例如: CLogMemPool/CTimerMemPool 
 * 用于多线程场景下的高并发处理(例如: 多个线程写日志, 多个线程启停定时器等)
 */
template<typename T, WORD32 POOL_NUM = SINGLE_POOL_NUM>
class CMultiMemPool : public CBaseData
{
public :
    CMultiMemPool (CCentralMemPool &rCentralMemPool);
    virtual ~CMultiMemPool();

    virtual WORD32 Initialize();

    /* 在RegistZone时调用(即, 在业务线程创建后, 在业务线程的栈空间调用) */
    T * CreateSTPool(WORD32 dwRingID);

    BYTE * Malloc(WORD32 dwRingID, WORD32 dwSize);
    WORD32 Free(WORD32 dwRingID, BYTE *pAddr);

protected :
    CCentralMemPool   &m_rCentralMemPool;

    CAtomicLock        m_cLock;
    volatile WORD32    m_dwRingNum;
    T                 *m_apPool[POOL_NUM];
};


template<typename T, WORD32 POOL_NUM>
CMultiMemPool<T, POOL_NUM>::CMultiMemPool (CCentralMemPool &rCentralMemPool)
    : m_rCentralMemPool(rCentralMemPool)
{
    m_dwRingNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < POOL_NUM; dwIndex++)
    {
        m_apPool[dwIndex] = NULL;
    }
}


template<typename T, WORD32 POOL_NUM>
CMultiMemPool<T, POOL_NUM>::~CMultiMemPool()
{
    m_dwRingNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < POOL_NUM; dwIndex++)
    {
        if (NULL != m_apPool[dwIndex])
        {
            delete m_apPool[dwIndex];
            m_rCentralMemPool.Free((VOID *)(m_apPool[dwIndex]));
        }

        m_apPool[dwIndex] = NULL;
    }
}


template<typename T, WORD32 POOL_NUM>
WORD32 CMultiMemPool<T, POOL_NUM>::Initialize()
{
    return SUCCESS;
}


/* 在RegistZone时调用(即, 在业务线程创建后, 在业务线程的栈空间调用) */
template<typename T, WORD32 POOL_NUM>
T * CMultiMemPool<T, POOL_NUM>::CreateSTPool(WORD32 dwRingID)
{
    if (unlikely(dwRingID >= POOL_NUM))
    {
        assert(0);
    }

    BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(T));
    if (NULL == pMem)
    {
        assert(0);
    }

    m_cLock.Lock();

    if ((NULL != m_apPool[dwRingID]) || (m_dwRingNum >= POOL_NUM))
    {
        /* 重复创建也会assert */
        assert(0);
    }

    T *pPool = new (pMem) T(m_rCentralMemPool);
    pPool->Initialize();

    m_apPool[dwRingID] = pPool;

    m_dwRingNum++;

    m_cLock.UnLock();

    return pPool;
}


template<typename T, WORD32 POOL_NUM>
inline BYTE * CMultiMemPool<T, POOL_NUM>::Malloc(
    WORD32 dwRingID,
    WORD32 dwSize)
{
    if (unlikely((dwRingID >= POOL_NUM) || (NULL == m_apPool[dwRingID])))
    {
        return NULL;
    }

    return m_apPool[dwRingID]->Malloc(dwSize);
}


template<typename T, WORD32 POOL_NUM>
inline WORD32 CMultiMemPool<T, POOL_NUM>::Free(WORD32 dwRingID, BYTE *pAddr)
{
    if (unlikely((dwRingID >= POOL_NUM) || (NULL == m_apPool[dwRingID])))
    {
        return NULL;
    }

    return m_apPool[dwRingID]->Free(pAddr);
}


class CBlockMemObject : public CBaseData
{
public :
    CBlockMemObject (CCentralMemPool &rCentralMemPool);
    virtual ~CBlockMemObject();

    WORD32 Initialize(WORD32 dwPoolID, WORD32 dwBlockID, WORD32 dwPowerNum, WORD32 dwTrunkSize);

    BYTE * Malloc(WORD32 dwSize, WORD32 dwPoint);

    WORD32 Free(BYTE *pAddr);

    BOOL IsValid(BYTE *pAddr);

    WORD32 GetBufSize();

    VOID Dump();

protected :
    CObjMemPoolInterface * CreateTrunPool();

protected :
    WORD32                 m_dwPoolID;
    WORD32                 m_dwBlockID;

    WORD32                 m_dwPowerNum;
    WORD32                 m_dwBufSize;

    CCentralMemPool       &m_rCentralMemPool;
    CObjMemPoolInterface  *m_pTrunkPool;
};


inline BYTE * CBlockMemObject::Malloc(WORD32 dwSize, WORD32 dwPoint)
{
    return m_pTrunkPool->Malloc(dwSize, m_dwPoolID, dwPoint);
}


inline WORD32 CBlockMemObject::Free(BYTE *pAddr)
{
    return m_pTrunkPool->Free(pAddr);
}


inline BOOL CBlockMemObject::IsValid(BYTE *pAddr)
{
    return m_pTrunkPool->IsValid(pAddr);
}


inline WORD32 CBlockMemObject::GetBufSize()
{
    return m_dwBufSize;
}


typedef struct tagT_BlockObject
{
    WORD32            dwPowerNum;    /* 控制Ring的大小 */
    WORD32            dwBufNum;      /* 2^PowerNum */
    WORD32            dwTrunkSize;   /* 配置文件中指定的Trunk大小 */
    WORD32            dwBufSize;     /* CACHE_SIZE对齐后的内存块大小 */
    CBlockMemObject  *pMemObj;
}T_BlockObject;


class CBlockMemPool : public CBaseData
{
public :
    CBlockMemPool (CCentralMemPool &rCentralMemPool);
    virtual ~CBlockMemPool();

    WORD32 Initialize(T_BlockParam &rtParam);

    BYTE * Malloc(WORD32 dwBlockID, WORD32 dwSize, WORD32 dwPoint);

    CBlockMemObject * FindBlock(WORD32 dwBlockID);

    /* 根据内存块大小查找匹配的BlockID */
    WORD32 GetBlockID(WORD32 dwSize);

    VOID Dump();

protected :    
    CBlockMemObject * CreateBlock(WORD32 dwPoolID, WORD32 dwBlockID, T_BlockObject &rtBlock);

protected :
    CCentralMemPool  &m_rCentralMemPool;

    WORD32            m_dwPoolID;
    WORD32            m_dwBlockNum;
    T_BlockObject     m_atBlock[MAX_BLOCK_NUM];
};


inline BYTE * CBlockMemPool::Malloc(WORD32 dwBlockID,
                                    WORD32 dwSize,
                                    WORD32 dwPoint)
{
    if (unlikely(dwBlockID >= m_dwBlockNum))
    {
        return NULL;
    }

    return m_atBlock[dwBlockID].pMemObj->Malloc(dwSize, dwPoint);
}


inline CBlockMemObject * CBlockMemPool::FindBlock(WORD32 dwBlockID)
{
    if (unlikely(dwBlockID >= m_dwBlockNum))
    {
        return NULL;
    }

    return m_atBlock[dwBlockID].pMemObj;
}


/* 根据内存块大小查找匹配的BlockID */
inline WORD32 CBlockMemPool::GetBlockID(WORD32 dwSize)
{
    WORD32 dwBlockID = 0;

    dwSize += sizeof(T_MemBufHeader);

    for (; dwBlockID < m_dwBlockNum; dwBlockID++)
    {
        if (dwSize <= m_atBlock[dwBlockID].dwBufSize)
        {
            break ;
        }
    }

    return dwBlockID;
}


#endif


