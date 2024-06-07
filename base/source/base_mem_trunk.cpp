

#include "base_sort.h"
#include "base_log.h"
#include "base_mem_mgr.h"


CMemPools * CMemPools::s_pInstance = NULL;


CBlockMemObject::CBlockMemObject (CCentralMemPool &rCentralMemPool)
    : m_rCentralMemPool(rCentralMemPool)
{
    m_dwPoolID    = INVALID_DWORD;
    m_dwBlockID   = INVALID_DWORD;
    m_dwPowerNum  = 0;
    m_dwBufSize   = 0;
    m_pTrunkPool  = NULL;
}


CBlockMemObject::~CBlockMemObject()
{
    if (NULL != m_pTrunkPool)
    {
        delete m_pTrunkPool;
        m_rCentralMemPool.Free((BYTE *)m_pTrunkPool);
    }

    m_dwPoolID    = INVALID_DWORD;
    m_dwBlockID   = INVALID_DWORD;
    m_dwPowerNum  = 0;
    m_dwBufSize   = 0;
    m_pTrunkPool  = NULL;
}


WORD32 CBlockMemObject::Initialize(WORD32 dwPoolID, 
                                   WORD32 dwBlockID, 
                                   WORD32 dwPowerNum, 
                                   WORD32 dwTrunkSize)
{
    if ((0 == dwPowerNum) || (0 == dwTrunkSize))
    {
        assert(0);
    }

    m_dwPoolID   = dwPoolID;
    m_dwBlockID  = dwBlockID;
    m_dwPowerNum = dwPowerNum;
    m_dwBufSize  = ROUND_UP(dwTrunkSize, CACHE_SIZE) - sizeof(T_MemBufHeader);
    m_pTrunkPool = CreateTrunPool();
    if (NULL == m_pTrunkPool)
    {
        assert(0);
    }

    return SUCCESS;
}


VOID CBlockMemObject::Dump()
{
    WORD64 lwSize     = 0;
    WORD64 lwUsedSize = 0;
    WORD64 lwFreeSize = 0;

    m_pTrunkPool->GetUsage(lwSize, lwUsedSize, lwFreeSize);

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "%4d  %5d  %5d      %8d     "
               "%8d      %8d    %p  %lu  %10lu    "
               "%10lu    %15lu  %15lu\n",
               m_dwPoolID,
               m_dwBlockID,
               m_dwPowerNum,
               m_dwBufSize,
               m_pTrunkPool->m_dwPowerNum,
               m_pTrunkPool->m_dwBufSize,
               m_pTrunkPool->m_pOriAddr,
               m_pTrunkPool->m_lwAddr,
               m_pTrunkPool->m_lwSize,
               lwSize,
               lwUsedSize,
               lwFreeSize);
}


CObjMemPoolInterface * CBlockMemObject::CreateTrunPool()
{
    switch (m_dwPowerNum)
    {
    case  0 :
    case  1 :
    case  2 :
    case  3 :
    case  4 :
    case  5 :
    case  6 :
    case  7 :
    case  8 :
    case  9 :
    case 10 :
        {
            BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<10>));
            if (NULL == pMem)
            {
                assert(0);
            }

            CObjectMemPool<10> *pTrunkPool = new (pMem) CObjectMemPool<10>(m_rCentralMemPool);
            pTrunkPool->Initialize(m_dwPoolID, m_dwBlockID, m_dwPowerNum, m_dwBufSize);

            return (CObjMemPoolInterface *)pTrunkPool;
        }
        break ;

    case 11 :
    case 12 :
    case 13 :
    case 14 :
    case 15 :
        {
            BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<15>));
            if (NULL == pMem)
            {
                assert(0);
                return NULL;
            }

            CObjectMemPool<15> *pTrunkPool = new (pMem) CObjectMemPool<15>(m_rCentralMemPool);
            pTrunkPool->Initialize(m_dwPoolID, m_dwBlockID, m_dwPowerNum, m_dwBufSize);

            return (CObjMemPoolInterface *)pTrunkPool;
        }
        break ;

    case 16 :
    case 17 :
    case 18 :
        {
            BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<18>));
            if (NULL == pMem)
            {
                assert(0);
                return NULL;
            }

            CObjectMemPool<18> *pTrunkPool = new (pMem) CObjectMemPool<18>(m_rCentralMemPool);
            pTrunkPool->Initialize(m_dwPoolID, m_dwBlockID, m_dwPowerNum, m_dwBufSize);

            return (CObjMemPoolInterface *)pTrunkPool;
        }
        break ;

    case 19 :
    case 20 :
        {
            BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<20>));
            if (NULL == pMem)
            {
                assert(0);
                return NULL;
            }

            CObjectMemPool<20> *pTrunkPool = new (pMem) CObjectMemPool<20>(m_rCentralMemPool);
            pTrunkPool->Initialize(m_dwPoolID, m_dwBlockID, m_dwPowerNum, m_dwBufSize);

            return (CObjMemPoolInterface *)pTrunkPool;
        }
        break ;

    default :
        break ;
    }

    return NULL;
}


CBlockMemPool::CBlockMemPool (CCentralMemPool &rCentralMemPool)
    : m_rCentralMemPool(rCentralMemPool)
{
    m_dwPoolID   = INVALID_DWORD;
    m_dwBlockNum = 0;

    memset(m_atBlock, 0x00, sizeof(m_atBlock));
}


CBlockMemPool::~CBlockMemPool()
{
    CBlockMemObject *pBlock = NULL;

    for (WORD32 dwIndex = 0; dwIndex < MAX_MEM_BLOCK_NUM; dwIndex++)
    {
        pBlock = m_atBlock[dwIndex].pMemObj;
        if (NULL != pBlock)
        {
            delete pBlock;
            m_rCentralMemPool.Free((BYTE *)pBlock);
        }
    }

    m_dwPoolID   = INVALID_DWORD;
    m_dwBlockNum = 0;

    memset(m_atBlock, 0x00, sizeof(m_atBlock));
}


WORD32 CBlockMemPool::Initialize(T_MemJsonPool &rCfg)
{
    m_dwPoolID   = rCfg.dwPoolID;
    m_dwBlockNum = MIN(rCfg.dwBlockNum, MAX_MEM_BLOCK_NUM);

    for (WORD32 dwIndex = 0; dwIndex < m_dwBlockNum; dwIndex++)
    {
        m_atBlock[dwIndex].dwPowerNum  = rCfg.atBlock[dwIndex].dwPowerNum;
        m_atBlock[dwIndex].dwBufNum    = 1 << m_atBlock[dwIndex].dwPowerNum;
        m_atBlock[dwIndex].dwTrunkSize = rCfg.atBlock[dwIndex].dwTrunkSize;
        m_atBlock[dwIndex].dwBufSize   = ROUND_UP((rCfg.atBlock[dwIndex].dwTrunkSize + sizeof(T_MemBufHeader)), CACHE_SIZE)
                                            - sizeof(T_MemBufHeader);
    }

    /* 从小到大排序 */
    HeapSort<T_BlockObject, WORD32> (m_atBlock, m_dwBlockNum,
                                     (&T_BlockObject::dwBufSize),
                                     (PComparePFunc<T_BlockObject, WORD32>)(& GreaterV<T_BlockObject, WORD32>));

    for (WORD32 dwIndex = 0; dwIndex < m_dwBlockNum; dwIndex++)
    {
        m_atBlock[dwIndex].pMemObj = CreateBlock(m_dwPoolID, dwIndex, m_atBlock[dwIndex]);
    }

    return SUCCESS;
}


VOID CBlockMemPool::Dump()
{
    TRACE_STACK("CBlockMemPool::Dump()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "PoolID : %d, BlockNum : %d\n",
               m_dwPoolID,
               m_dwBlockNum);

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "PoolID  BlockID  PowerNum  BufSize  "
               "PowerSize     TrunkSize    pOriAddr        "
               "lwAddr              lwSize        lwSize           "
               "lwUsedSize       lwFreeSize\n");

    for (WORD32 dwIndex = 0; dwIndex < m_dwBlockNum; dwIndex++)
    {
        m_atBlock[dwIndex].pMemObj->Dump();
    }    
}


CBlockMemObject * CBlockMemPool::CreateBlock(WORD32 dwPoolID, WORD32 dwBlockID, T_BlockObject &rtBlock)
{
    BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CBlockMemObject));
    if (NULL == pMem)
    {
        assert(0);
        return NULL;
    }

    CBlockMemObject *pBlock = new (pMem) CBlockMemObject(m_rCentralMemPool);

    pBlock->Initialize(dwPoolID, dwBlockID, rtBlock.dwPowerNum, rtBlock.dwTrunkSize);

    return pBlock;
}


CMemPools * CMemPools::GetInstance(CCentralMemPool *pCentralMemPool)
{
    if (NULL != s_pInstance)
    {
        return s_pInstance;
    }

    if (NULL == pCentralMemPool)
    {
        return NULL;
    }

    BYTE *pMem = pCentralMemPool->Malloc(sizeof(CMemPools));
    if (NULL == pMem)
    {
        assert(0);
    }

    s_pInstance = new (pMem) CMemPools(*pCentralMemPool);

    WORD32 dwResult = s_pInstance->Initialize();
    if (SUCCESS != dwResult)
    {
        delete s_pInstance;
        s_pInstance = NULL;
        
        pCentralMemPool->Free(pMem);
        assert(0);
    }

    return s_pInstance;
}


VOID CMemPools::Destroy()
{
    CCentralMemPool *pCentralMemPool = NULL;

    if (NULL != s_pInstance)
    {
        pCentralMemPool = &(s_pInstance->m_rCentralMemPool);
        delete s_pInstance;
        pCentralMemPool->Free((BYTE *)s_pInstance);
    }

    s_pInstance = NULL;
}


VOID CMemPools::GetMeasure(T_MemMeasure &rtMeasure)
{
    for (WORD32 dwIndex = 0; dwIndex < E_MP_MALLOC_POINT_NUM; dwIndex++)
    {
        rtMeasure.alwMallocStat[dwIndex] = m_tMeasure.alwMallocStat[dwIndex].load(std::memory_order_relaxed);
        rtMeasure.alwFreeStat[dwIndex]   = m_tMeasure.alwFreeStat[dwIndex].load(std::memory_order_relaxed);
    }
}


VOID CMemPools::Dump()
{
    TRACE_STACK("CMemPools::Dump()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "PoolNum : %d\n",
               m_dwPoolNum);

    for (WORD32 dwIndex = 0; dwIndex < m_dwPoolNum; dwIndex++)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                   "dwPoolID : %d\n",
                   m_atPool[dwIndex].dwPoolID);

        m_atPool[dwIndex].pBlockPool->Dump();
    }    
}


CMemPools::CMemPools (CCentralMemPool &rCentralMemPool)
    : m_rCentralMemPool(rCentralMemPool)
{
    m_dwPoolNum = 0;
    memset(m_atPool, 0x00, sizeof(m_atPool));

    for (WORD32 dwIndex = 0; dwIndex < E_MP_MALLOC_POINT_NUM; dwIndex++)
    {
        m_tMeasure.alwMallocStat[dwIndex] = 0;
        m_tMeasure.alwFreeStat[dwIndex]   = 0;
    }

    T_MemMetaHead *pMetaHead = CMemMgr::GetInstance()->GetMetaHead();
    pMetaHead->lwMemPools    = (WORD64)this;
}


CMemPools::~CMemPools()
{
    CBlockMemPool *pPool = NULL;

    for (WORD32 dwIndex = 0; dwIndex < MAX_MEM_POOL_NUM; dwIndex++)
    {
        pPool = m_atPool[dwIndex].pBlockPool;        
        if (NULL != pPool)
        {
            delete pPool;
            m_rCentralMemPool.Free((BYTE *)pPool);
        }
    }

    m_dwPoolNum = 0;
    memset(m_atPool, 0x00, sizeof(m_atPool));

    for (WORD32 dwIndex = 0; dwIndex < E_MP_MALLOC_POINT_NUM; dwIndex++)
    {
        m_tMeasure.alwMallocStat[dwIndex] = 0;
        m_tMeasure.alwFreeStat[dwIndex]   = 0;
    }
}


WORD32 CMemPools::Initialize()
{
    T_MemJsonCfg &rtMemCfg = CBaseConfigFile::GetInstance()->GetMemJsonCfg();

    m_dwPoolNum = MIN(MAX_MEM_POOL_NUM, rtMemCfg.ucPoolNum);

    for (WORD32 dwIndex = 0; dwIndex < m_dwPoolNum; dwIndex++)
    {
        WORD32 dwPoolID = rtMemCfg.atPool[dwIndex].dwPoolID;
        if (dwPoolID >= MAX_MEM_POOL_NUM)
        {
            assert(0);
        }

        if (m_atPool[dwPoolID].bFlag)
        {
            assert(0);
        }

        m_atPool[dwPoolID].dwPoolID   = dwPoolID;
        m_atPool[dwPoolID].bFlag      = TRUE;
        m_atPool[dwPoolID].pBlockPool = CreatePool(rtMemCfg.atPool[dwIndex]);
    }

    return SUCCESS;
}


CBlockMemPool * CMemPools::CreatePool(T_MemJsonPool &rCfg)
{
    BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CBlockMemPool));
    if (NULL == pMem)
    {
        assert(0);
    }

    CBlockMemPool *pPool = new (pMem) CBlockMemPool(m_rCentralMemPool);
    pPool->Initialize(rCfg);

    return pPool;
}


