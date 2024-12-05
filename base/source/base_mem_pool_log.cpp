

#include "base_mem_pool_log.h"


CLogMemPool::CLogMemPool (CCentralMemPool &rCentralMemPool)
    : CObjectMemPool<POWER_OF_LOG_MEMPOOL>(rCentralMemPool)
{
}


CLogMemPool::~CLogMemPool()
{
}


WORD32 CLogMemPool::Initialize()
{
    return CObjectMemPool<POWER_OF_LOG_MEMPOOL>::Initialize(LOG_MEM_BUF_SIZE, 0, 0);
}


