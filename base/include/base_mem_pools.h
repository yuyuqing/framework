

#ifndef _BASE_MEM_POOLS_H_
#define _BASE_MEM_POOLS_H_


#include "base_mem_pool.h"
#include "base_measure.h"
#include "base_config_file.h"


typedef struct tagT_PoolObject
{
    WORD32          dwPoolID;
    BOOL            bFlag;
    CBlockMemPool  *pBlockPool;
}T_PoolObject;


class CMemPools : public CAllocInterface
{
public :
    static CMemPools * GetInstance(CCentralMemPool *pCentralMemPool = NULL);
    static VOID Destroy();

public :
    virtual BYTE * Malloc(WORD32 dwSize, WORD32 dwPoolID, WORD32 dwPoint);
    virtual WORD32 Free(VOID *pAddr);

    /* 引用计数++操作 */
    VOID IncRefCount(BYTE *pAddr);

    VOID GetMeasure(T_MemMeasure &rtMeasure);

    VOID Dump();

protected :
    CMemPools (CCentralMemPool &rCentralMemPool);
    virtual ~CMemPools();

    WORD32 Initialize();

    CBlockMemPool * CreatePool(T_BlockParam &rtParam);

    CBlockMemPool * FindPool(WORD32 dwPoolID);

protected :
    CMemPools (CMemPools &) = delete;
    CMemPools (const CMemPools &) = delete;
    CMemPools& operator= (const CMemPools &) = delete;

protected :
    CCentralMemPool     &m_rCentralMemPool;

    WORD32               m_dwPoolNum;
    T_PoolObject         m_atPool[MAX_POOL_NUM];

    /* 内存申请/释放维测信息 */
    T_AtomicMemMeasure   m_tMeasure;

private :
    static CMemPools    *s_pInstance;
};


inline BYTE * CMemPools::Malloc(WORD32 dwSize, WORD32 dwPoolID, WORD32 dwPoint)
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


inline WORD32 CMemPools::Free(VOID *pAddr)
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

    WORD32 dwResult = pBlock->Free((BYTE *)pAddr);
    if (SUCCESS == dwResult)
    {
        m_tMeasure.alwFreeStat[ptBuf->dwPoint]++;
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

#ifdef MEM_CHECK
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
#endif

    ptBuf->dwRefCount++;
}


inline CBlockMemPool * CMemPools::FindPool(WORD32 dwPoolID)
{
    if (unlikely(dwPoolID >= MAX_POOL_NUM))
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


