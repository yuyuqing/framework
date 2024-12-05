

#ifndef _BASE_MEM_POOL_TIMER_H_
#define _BASE_MEM_POOL_TIMER_H_


#include "base_mem_interface.h"


#define POWER_OF_TIMER_MEMPOOL        ((WORD32)(10))
#define TIMER_MSG_SIZE                ((WORD32)(256  - sizeof(T_MemBufHeader)))


typedef struct tagT_TimerMsgHeader
{
    WORD64  lwAddr;
    WORD64  lwStartCycle;
    WORD64  lwEndCycle;
    WORD32  dwMsgID;
    WORD16  wResved;
    WORD16  wMsgLen;
}T_TimerMsgHeader;


class CTimerMemPool : public CObjectMemPool<POWER_OF_TIMER_MEMPOOL>
{
protected :
    using CObjectMemPool<POWER_OF_TIMER_MEMPOOL>::Initialize;

public :
    static const WORD32 s_dwTimerSize = TIMER_MSG_SIZE - sizeof(T_TimerMsgHeader);

public :
    CTimerMemPool (CCentralMemPool &rCentralMemPool);
    virtual ~CTimerMemPool();

    WORD32 Initialize();

    virtual BYTE * Malloc(WORD32 dwSize);
    virtual WORD32 Free(VOID *pAddr);
};


/* ��ע : ����ʱδƫ��ͷ, �ͷ�ʱҲ����ƫ�ƴ��� */
inline BYTE * CTimerMemPool::Malloc(WORD32 dwSize)
{
    BYTE *pValue = CObjectMemPool<POWER_OF_TIMER_MEMPOOL>::Malloc(dwSize);

    return pValue;
}


/* ��ע : ����ʱδƫ��ͷ, �ͷ�ʱҲ����ƫ�ƴ��� */
inline WORD32 CTimerMemPool::Free(VOID *pAddr)
{
    if (unlikely(NULL == pAddr))
    {
        return FAIL;
    }

    return CObjectMemPool<POWER_OF_TIMER_MEMPOOL>::Free(pAddr);    
}


#endif


