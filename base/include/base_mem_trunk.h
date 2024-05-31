

#ifndef _BASE_MEM_TRUNK_H_
#define _BASE_MEM_TRUNK_H_


#include "base_mem_interface.h"
#include "base_measure.h"
#include "base_config_file.h"


#define MAX_MEM_BLOCK_NUM                      ((WORD32)(MAX_BLOCK_NUM))
#define MAX_MEM_POOL_NUM                       ((WORD32)(MAX_POOL_NUM))


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
    return m_pTrunkPool->Malloc(dwSize, MEMPOOL_THRESHOLD, dwPoint);
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

    WORD32 Initialize(T_MemJsonPool &rCfg);

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
    T_BlockObject     m_atBlock[MAX_MEM_BLOCK_NUM];
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


typedef struct tagT_PoolObject
{
    WORD32          dwPoolID;
    BOOL            bFlag;
    CBlockMemPool  *pBlockPool;
}T_PoolObject;


class CMemPools : public CBaseData
{
public :
    static CMemPools * GetInstance(CCentralMemPool *pCentralMemPool = NULL);
    static VOID Destroy();

public :
    BYTE * Malloc(WORD32 dwPoolID, WORD32 dwBlockID, WORD32 dwSize, WORD32 dwPoint);
    BYTE * Malloc(WORD32 dwPoolID, WORD32 dwSize, WORD32 dwPoint);

    WORD32 Free(BYTE *pAddr);

    /* 引用计数++操作 */
    VOID IncRefCount(BYTE *pAddr);

    VOID GetMeasure(T_MemMeasure &rtMeasure);

    VOID Dump();

    CCentralMemPool & GetCentralMemPool();

protected :
    CMemPools (CCentralMemPool &rCentralMemPool);
    virtual ~CMemPools();

    WORD32 Initialize();

    CBlockMemPool * CreatePool(T_MemJsonPool &rCfg);

    CBlockMemPool * FindPool(WORD32 dwPoolID);

protected :
    CMemPools (CMemPools &) = delete;
    CMemPools (const CMemPools &) = delete;
    CMemPools& operator= (const CMemPools &) = delete;

protected :
    CCentralMemPool     &m_rCentralMemPool;

    WORD32               m_dwPoolNum;
    T_PoolObject         m_atPool[MAX_MEM_POOL_NUM];

    /* 内存申请/释放维测信息 */
    T_AtomicMemMeasure   m_tMeasure;

private :
    static CMemPools    *s_pInstance;
};


inline CCentralMemPool & CMemPools::GetCentralMemPool()
{
    return m_rCentralMemPool;
}


inline BYTE * CMemPools::Malloc(WORD32 dwPoolID,
                                WORD32 dwBlockID,
                                WORD32 dwSize,
                                WORD32 dwPoint)
{
    CBlockMemPool *pPool = FindPool(dwPoolID);
    if (unlikely(NULL == pPool))
    {
        return NULL;
    }

    CBlockMemObject *pBlock = pPool->FindBlock(dwBlockID);
    if (unlikely(NULL == pBlock))
    {
        return NULL;
    }

    BYTE *pMem = pBlock->Malloc(dwSize, dwPoint);
    if (NULL == pMem)
    {
        return NULL;
    }

    m_tMeasure.alwMallocStat[dwPoint]++;

    return pMem;
}


inline BYTE * CMemPools::Malloc(WORD32 dwPoolID, WORD32 dwSize, WORD32 dwPoint)
{
    CBlockMemPool *pPool = FindPool(dwPoolID);
    if (unlikely(NULL == pPool))
    {
        return NULL;
    }

    WORD32 dwBlockID = pPool->GetBlockID(dwSize);

    BYTE *pMem = pPool->Malloc(dwBlockID, dwSize, dwPoint);
    if (NULL == pMem)
    {
        return NULL;
    }

    m_tMeasure.alwMallocStat[dwPoint]++;

    return pMem;
}


inline WORD32 CMemPools::Free(BYTE *pAddr)
{
    if (unlikely(NULL == pAddr))
    {
        return FAIL;
    }

    T_MemBufHeader *ptBuf = (T_MemBufHeader *)(((WORD64)pAddr) - sizeof(T_MemBufHeader));

    CBlockMemPool *pPool = FindPool(ptBuf->dwPoolID);
    if (unlikely(NULL == pPool))
    {
        assert(0);
    }

    CBlockMemObject *pBlock = pPool->FindBlock(ptBuf->dwBlockID);
    if (unlikely(NULL == pBlock))
    {
        assert(0);
    }

    WORD32 dwResult = pBlock->Free(pAddr);
    if (SUCCESS == dwResult)
    {
        m_tMeasure.alwFreeStat[ptBuf->dwAllocPoint]++;
    }

    return dwResult;
}


/* 引用计数++操作 */
inline VOID CMemPools::IncRefCount(BYTE *pAddr)
{
    if (unlikely(NULL == pAddr))
    {
        return ;
    }

    T_MemBufHeader *ptBuf = (T_MemBufHeader *)(((WORD64)pAddr) - sizeof(T_MemBufHeader));

    CBlockMemPool *pPool = FindPool(ptBuf->dwPoolID);
    if (unlikely(NULL == pPool))
    {
        assert(0);
    }

    CBlockMemObject *pBlock = pPool->FindBlock(ptBuf->dwBlockID);
    if (unlikely(NULL == pBlock))
    {
        assert(0);
    }

    if (FALSE == pBlock->IsValid(pAddr))
    {
        assert(0);
    }

    ptBuf->dwRefCount++;
}


inline CBlockMemPool * CMemPools::FindPool(WORD32 dwPoolID)
{
    if (unlikely(dwPoolID >= MAX_MEM_POOL_NUM))
    {
        return NULL;
    }

    if (unlikely((FALSE == m_atPool[dwPoolID].bFlag)
              || (NULL  == m_atPool[dwPoolID].pBlockPool)))
    {
        return NULL;
    }

    return m_atPool[dwPoolID].pBlockPool;
}


#endif


