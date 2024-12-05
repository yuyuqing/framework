

#include "base_sort.h"
#include "base_mem_pool.h"
#include "base_log.h"


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
        {
            BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<8>));
            if (NULL == pMem)
            {
                assert(0);
            }

            CObjectMemPool<8> *pTrunkPool = new (pMem) CObjectMemPool<8>(m_rCentralMemPool);
            pTrunkPool->Initialize(m_dwBufSize, m_dwPoolID, m_dwBlockID);

            return (CObjMemPoolInterface *)pTrunkPool;
        }
        break ;

    case  9 :
        {
            BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<9>));
            if (NULL == pMem)
            {
                assert(0);
            }

            CObjectMemPool<9> *pTrunkPool = new (pMem) CObjectMemPool<9>(m_rCentralMemPool);
            pTrunkPool->Initialize(m_dwBufSize, m_dwPoolID, m_dwBlockID);

            return (CObjMemPoolInterface *)pTrunkPool;
        }
        break ;

    case 10 :
        {
            BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<10>));
            if (NULL == pMem)
            {
                assert(0);
            }

            CObjectMemPool<10> *pTrunkPool = new (pMem) CObjectMemPool<10>(m_rCentralMemPool);
            pTrunkPool->Initialize(m_dwBufSize, m_dwPoolID, m_dwBlockID);

            return (CObjMemPoolInterface *)pTrunkPool;
        }
        break ;

    case 11 :
        {
            BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<11>));
            if (NULL == pMem)
            {
                assert(0);
            }

            CObjectMemPool<11> *pTrunkPool = new (pMem) CObjectMemPool<11>(m_rCentralMemPool);
            pTrunkPool->Initialize(m_dwBufSize, m_dwPoolID, m_dwBlockID);

            return (CObjMemPoolInterface *)pTrunkPool;
        }
        break ;

    case 12 :
        {
            BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<12>));
            if (NULL == pMem)
            {
                assert(0);
            }

            CObjectMemPool<12> *pTrunkPool = new (pMem) CObjectMemPool<12>(m_rCentralMemPool);
            pTrunkPool->Initialize(m_dwBufSize, m_dwPoolID, m_dwBlockID);

            return (CObjMemPoolInterface *)pTrunkPool;
        }
        break ;

    case 13 :
        {
            BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<13>));
            if (NULL == pMem)
            {
                assert(0);
            }

            CObjectMemPool<13> *pTrunkPool = new (pMem) CObjectMemPool<13>(m_rCentralMemPool);
            pTrunkPool->Initialize(m_dwBufSize, m_dwPoolID, m_dwBlockID);

            return (CObjMemPoolInterface *)pTrunkPool;
        }
        break ;

    case 14 :
        {
            BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<14>));
            if (NULL == pMem)
            {
                assert(0);
            }

            CObjectMemPool<14> *pTrunkPool = new (pMem) CObjectMemPool<14>(m_rCentralMemPool);
            pTrunkPool->Initialize(m_dwBufSize, m_dwPoolID, m_dwBlockID);

            return (CObjMemPoolInterface *)pTrunkPool;
        }
        break ;

    case 15 :
        {
            BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<15>));
            if (NULL == pMem)
            {
                assert(0);
            }

            CObjectMemPool<15> *pTrunkPool = new (pMem) CObjectMemPool<15>(m_rCentralMemPool);
            pTrunkPool->Initialize(m_dwBufSize, m_dwPoolID, m_dwBlockID);

            return (CObjMemPoolInterface *)pTrunkPool;
        }
        break ;

    case 16 :
        {
            BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<16>));
            if (NULL == pMem)
            {
                assert(0);
            }

            CObjectMemPool<16> *pTrunkPool = new (pMem) CObjectMemPool<16>(m_rCentralMemPool);
            pTrunkPool->Initialize(m_dwBufSize, m_dwPoolID, m_dwBlockID);

            return (CObjMemPoolInterface *)pTrunkPool;
        }
        break ;

    case 17 :
        {
            BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<17>));
            if (NULL == pMem)
            {
                assert(0);
            }

            CObjectMemPool<17> *pTrunkPool = new (pMem) CObjectMemPool<17>(m_rCentralMemPool);
            pTrunkPool->Initialize(m_dwBufSize, m_dwPoolID, m_dwBlockID);

            return (CObjMemPoolInterface *)pTrunkPool;
        }
        break ;

    case 18 :
        {
            BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<18>));
            if (NULL == pMem)
            {
                assert(0);
            }

            CObjectMemPool<18> *pTrunkPool = new (pMem) CObjectMemPool<18>(m_rCentralMemPool);
            pTrunkPool->Initialize(m_dwBufSize, m_dwPoolID, m_dwBlockID);

            return (CObjMemPoolInterface *)pTrunkPool;
        }
        break ;

    case 19 :
        {
            BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<19>));
            if (NULL == pMem)
            {
                assert(0);
            }

            CObjectMemPool<19> *pTrunkPool = new (pMem) CObjectMemPool<19>(m_rCentralMemPool);
            pTrunkPool->Initialize(m_dwBufSize, m_dwPoolID, m_dwBlockID);

            return (CObjMemPoolInterface *)pTrunkPool;
        }
        break ;

    case 20 :
        {
            BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<20>));
            if (NULL == pMem)
            {
                assert(0);
            }

            CObjectMemPool<20> *pTrunkPool = new (pMem) CObjectMemPool<20>(m_rCentralMemPool);
            pTrunkPool->Initialize(m_dwBufSize, m_dwPoolID, m_dwBlockID);

            return (CObjMemPoolInterface *)pTrunkPool;
        }
        break ;

    default :
        {
            assert(0);
        }
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

    for (WORD32 dwIndex = 0; dwIndex < MAX_BLOCK_NUM; dwIndex++)
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


WORD32 CBlockMemPool::Initialize(T_BlockParam &rtParam)
{
    m_dwPoolID   = rtParam.dwPoolID;
    m_dwBlockNum = MIN(rtParam.dwBlockNum, MAX_BLOCK_NUM);

    for (WORD32 dwIndex = 0; dwIndex < m_dwBlockNum; dwIndex++)
    {
        m_atBlock[dwIndex].dwPowerNum  = rtParam.atBlock[dwIndex].dwPowerNum;
        m_atBlock[dwIndex].dwBufNum    = 1 << m_atBlock[dwIndex].dwPowerNum;
        m_atBlock[dwIndex].dwTrunkSize = rtParam.atBlock[dwIndex].dwTrunkSize;
        m_atBlock[dwIndex].dwBufSize   = ROUND_UP((rtParam.atBlock[dwIndex].dwTrunkSize + sizeof(T_MemBufHeader)), CACHE_SIZE)
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
    }

    CBlockMemObject *pBlock = new (pMem) CBlockMemObject(m_rCentralMemPool);

    pBlock->Initialize(dwPoolID, dwBlockID, rtBlock.dwPowerNum, rtBlock.dwTrunkSize);

    return pBlock;
}


