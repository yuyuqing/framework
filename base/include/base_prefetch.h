

#ifndef _BASE_PREFETCH_H_
#define _BASE_PREFETCH_H_


#include <algorithm>
#include <atomic>

#include "base_data_container.h"


template<WORD32 SIZE, WORD32 RW, WORD32 LOC>
struct T_Prefetcher;


template<WORD32 RW, WORD32 LOC>
struct T_Prefetcher<0, RW, LOC>
{
    T_Prefetcher(CHAR *pData)
    {
    }
};


template<WORD32 SIZE, WORD32 RW, WORD32 LOC>
struct T_Prefetcher
{
    T_Prefetcher(CHAR *pData)
    {
        __builtin_prefetch(pData, RW, LOC);
        std::atomic_signal_fence(std::memory_order_seq_cst);
        T_Prefetcher<(SIZE - CACHE_SIZE), RW, LOC>(pData + CACHE_SIZE);
    }
};


template<typename T, WORD32 LOC>
VOID PrefetchR(T *pData)
{
    T_Prefetcher<ROUND_UP(MIN(sizeof(T), PAGE_SIZE), CACHE_SIZE), 0, LOC>((CHAR *)pData);
}


template<typename T, WORD32 SIZE, WORD32 LOC>
VOID PrefetchR(T *pData)
{
    T_Prefetcher<ROUND_UP(MIN(sizeof(T), SIZE), CACHE_SIZE), 0, LOC>((CHAR *)pData);
}


template<WORD32 SIZE, WORD32 LOC>
VOID PrefetchR(VOID *pData)
{
    T_Prefetcher<ROUND_UP(MIN(SIZE, PAGE_SIZE), CACHE_SIZE), 0, LOC>((CHAR *)pData);
}


template<typename T, WORD32 LOC>
VOID PrefetchW(T *pData)
{
    T_Prefetcher<ROUND_UP(MIN(sizeof(T), PAGE_SIZE), CACHE_SIZE), 1, LOC>((CHAR *)pData);
}


template<typename T, WORD32 SIZE, WORD32 LOC>
VOID PrefetchW(T *pData)
{
    T_Prefetcher<ROUND_UP(MIN(sizeof(T), SIZE), CACHE_SIZE), 1, LOC>((CHAR *)pData);
}


template<WORD32 SIZE, WORD32 LOC>
VOID PrefetchW(VOID *pData)
{
    T_Prefetcher<ROUND_UP(MIN(SIZE, PAGE_SIZE), CACHE_SIZE), 1, LOC>((CHAR *)pData);
}


#endif


