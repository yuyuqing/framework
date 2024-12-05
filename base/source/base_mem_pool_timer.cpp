

#include "base_mem_pool_timer.h"


CTimerMemPool::CTimerMemPool (CCentralMemPool &rCentralMemPool)
    : CObjectMemPool<POWER_OF_TIMER_MEMPOOL>(rCentralMemPool)
{
}


CTimerMemPool::~CTimerMemPool()
{
}


WORD32 CTimerMemPool::Initialize()
{
    return CObjectMemPool<POWER_OF_TIMER_MEMPOOL>::Initialize(TIMER_MSG_SIZE, 0, 0);
}


