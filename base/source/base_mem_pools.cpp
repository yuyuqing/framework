

#include "base_mem_mgr.h"
#include "base_log.h"


CMemPools * CMemPools::s_pInstance = NULL;


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

    for (WORD32 dwIndex = 0; dwIndex < MAX_POOL_NUM; dwIndex++)
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

    m_dwPoolNum = MIN(MAX_POOL_NUM, rtMemCfg.ucPoolNum);

    for (WORD32 dwIndex = 0; dwIndex < m_dwPoolNum; dwIndex++)
    {
        WORD32 dwPoolID = rtMemCfg.atPool[dwIndex].dwPoolID;
        if (dwPoolID >= MAX_POOL_NUM)
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


CBlockMemPool * CMemPools::CreatePool(T_BlockParam &rtParam)
{
    BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CBlockMemPool));
    if (NULL == pMem)
    {
        assert(0);
    }

    CBlockMemPool *pPool = new (pMem) CBlockMemPool(m_rCentralMemPool);
    pPool->Initialize(rtParam);

    return pPool;
}


