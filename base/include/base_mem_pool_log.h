

#ifndef _BASE_MEM_POOL_LOG_H_
#define _BASE_MEM_POOL_LOG_H_


#include "base_mem_interface.h"


#define POWER_OF_LOG_MEMPOOL          ((WORD32)(10))
#define LOG_MEM_BUF_SIZE              ((WORD32)(1024 - sizeof(T_MemBufHeader)))


typedef struct tagT_LogBufHeader
{
    WORD64   lwAddr;
    WORD64   lwFile;
    BYTE     ucLogType;     /* 0:普通接口日志, 1:Fast接口日志 */
    BYTE     ucResved1;
    BYTE     ucResved2;
    BYTE     ucResved3;
    WORD16   wModuleID;
    WORD16   wMsgLen;
}T_LogBufHeader;


class CLogMemPool : public CObjectMemPool<POWER_OF_LOG_MEMPOOL>
{
protected :
    using CObjectMemPool<POWER_OF_LOG_MEMPOOL>::Initialize;

public :
    static const WORD32 s_dwLogSize = LOG_MEM_BUF_SIZE - sizeof(T_LogBufHeader);

    typedef struct tagT_LogBuf
    {
        T_LogBufHeader  tLogHeader;
        BYTE            aucData[s_dwLogSize];
    }T_LogBuf;

public :
    CLogMemPool (CCentralMemPool &rCentralMemPool);
    virtual ~CLogMemPool();

    WORD32 Initialize();

    virtual BYTE * Malloc(WORD32 dwSize);
    virtual WORD32 Free(VOID *pAddr);
};


inline BYTE * CLogMemPool::Malloc(WORD32 dwSize)
{
    BYTE *pValue = CObjectMemPool<POWER_OF_LOG_MEMPOOL>::Malloc(dwSize);

    return (pValue) ? (pValue + sizeof(T_LogBufHeader)) : NULL;
}


inline WORD32 CLogMemPool::Free(VOID *pAddr)
{
    if (unlikely(NULL == pAddr))
    {
        return FAIL;
    }

    VOID *pValue = (VOID *)((WORD64)pAddr - sizeof(T_LogBufHeader));

    return CObjectMemPool<POWER_OF_LOG_MEMPOOL>::Free(pValue);    
}


#endif


