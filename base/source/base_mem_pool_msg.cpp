

#include "base_mem_pool_msg.h"
#include "base_log.h"


const WORD32 CMsgMemPool::s_adwBufSize[] = {
    E_MemSizeType_256, E_MemSizeType_512, E_MemSizeType_01K,
    E_MemSizeType_04K, E_MemSizeType_16K, E_MemSizeType_64K};


CMsgMemPool::CMsgMemPool (CCentralMemPool &rCentralMemPool)
    : m_rCentralMemPool(rCentralMemPool)
{
    memset(m_atPool, 0x00, sizeof(m_atPool));
}


CMsgMemPool::~CMsgMemPool()
{
    for (WORD32 dwIndex = 0; dwIndex < MEM_POOL_NUM; dwIndex++)
    {
        if (NULL != m_atPool[dwIndex].pMemPool)
        {
            delete m_atPool[dwIndex].pMemPool;
            m_rCentralMemPool.Free(m_atPool[dwIndex].pBuf);
        }
    }

    memset(m_atPool, 0x00, sizeof(m_atPool));
}


WORD32 CMsgMemPool::Initialize()
{
    m_atPool[0].eType     = E_MemSizeType_256;
    m_atPool[0].eNum      = E_MemBufPowerNum_256;
    m_atPool[0].pBuf      = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<E_MemBufPowerNum_256>));
    m_atPool[0].pMemPool  = new (m_atPool[0].pBuf) CObjectMemPool<E_MemBufPowerNum_256> (m_rCentralMemPool);
    m_atPool[0].pMemPool->Initialize(E_MemSizeType_256, 0, 0);

    m_atPool[1].eType     = E_MemSizeType_512;
    m_atPool[1].eNum      = E_MemBufPowerNum_512;
    m_atPool[1].pBuf      = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<E_MemBufPowerNum_512>));
    m_atPool[1].pMemPool  = new (m_atPool[1].pBuf) CObjectMemPool<E_MemBufPowerNum_512> (m_rCentralMemPool);
    m_atPool[1].pMemPool->Initialize(E_MemSizeType_512, 0, 0);

    m_atPool[2].eType     = E_MemSizeType_01K;
    m_atPool[2].eNum      = E_MemBufPowerNum_01K;
    m_atPool[2].pBuf      = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<E_MemBufPowerNum_01K>));
    m_atPool[2].pMemPool  = new (m_atPool[2].pBuf) CObjectMemPool<E_MemBufPowerNum_01K> (m_rCentralMemPool);
    m_atPool[2].pMemPool->Initialize(E_MemSizeType_01K, 0, 0);

    m_atPool[3].eType     = E_MemSizeType_04K;
    m_atPool[3].eNum      = E_MemBufPowerNum_04K;
    m_atPool[3].pBuf      = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<E_MemBufPowerNum_04K>));
    m_atPool[3].pMemPool  = new (m_atPool[3].pBuf) CObjectMemPool<E_MemBufPowerNum_04K> (m_rCentralMemPool);
    m_atPool[3].pMemPool->Initialize(E_MemSizeType_04K, 0, 0);

    m_atPool[4].eType     = E_MemSizeType_16K;
    m_atPool[4].eNum      = E_MemBufPowerNum_16K;
    m_atPool[4].pBuf      = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<E_MemBufPowerNum_16K>));
    m_atPool[4].pMemPool  = new (m_atPool[4].pBuf) CObjectMemPool<E_MemBufPowerNum_16K> (m_rCentralMemPool);
    m_atPool[4].pMemPool->Initialize(E_MemSizeType_16K, 0, 0);

    m_atPool[5].eType    = E_MemSizeType_64K;
    m_atPool[5].eNum     = E_MemBufPowerNum_64K;
    m_atPool[5].pBuf     = m_rCentralMemPool.Malloc(sizeof(CObjectMemPool<E_MemBufPowerNum_64K>));
    m_atPool[5].pMemPool = new (m_atPool[5].pBuf) CObjectMemPool<E_MemBufPowerNum_64K> (m_rCentralMemPool);
    m_atPool[5].pMemPool->Initialize(E_MemSizeType_64K, 0, 0);

    return SUCCESS;
}


VOID CMsgMemPool::GetMeasure(T_MsgMemMeasure &rtMeasure)
{
    rtMeasure.dwTypeNum = MIN(MEM_POOL_NUM, MSG_MEM_TYPE_NUM);

    for (WORD32 dwIndex = 0; dwIndex < rtMeasure.dwTypeNum; dwIndex++)
    {
        m_atPool[dwIndex].pMemPool->GetUsage(rtMeasure.alwTotalSize[dwIndex],
                                             rtMeasure.alwUsedCount[dwIndex],
                                             rtMeasure.alwFreeCount[dwIndex]);
    }
}


VOID CMsgMemPool::Dump()
{
    TRACE_STACK("CMsgMemPool::Dump()");

    WORD64 lwSize     = 0;
    WORD64 lwUsedSize = 0;
    WORD64 lwFreeSize = 0;

    for (WORD32 dwIndex = 0; dwIndex < MEM_POOL_NUM; dwIndex++)
    {
        m_atPool[dwIndex].pMemPool->GetUsage(lwSize, lwUsedSize, lwFreeSize);

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                   "eType = %d, eNum = %d, lwSize = %d, "
                   "lwUsedSize = %ld, lwFreeSize = %ld\n",
                   m_atPool[dwIndex].eType,
                   m_atPool[dwIndex].eNum,
                   lwSize, lwUsedSize, lwFreeSize);
    }
}


