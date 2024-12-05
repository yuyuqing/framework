

#ifndef _BASE_MEM_POOL_MSG_H_
#define _BASE_MEM_POOL_MSG_H_


#include "base_mem_interface.h"
#include "base_measure.h"


typedef enum tagE_MemSizeType
{
    E_MemSizeType_256  =   192,
    E_MemSizeType_512  =   448,
    E_MemSizeType_01K  =   960,
    E_MemSizeType_04K  =  4032,
    E_MemSizeType_16K  = 16320,
    E_MemSizeType_64K  = 65472,
}E_MemSizeType;


typedef enum tagE_MemBufPowerNum
{
    E_MemBufPowerNum_256 = 12,  /* 2^12 = 4096   1M */
    E_MemBufPowerNum_512 = 10,  /* 2^10 = 1024 512K */
    E_MemBufPowerNum_01K =  9,  /* 2^9  =  512 512K */
    E_MemBufPowerNum_04K =  8,  /* 2^8  =  256   1M */
    E_MemBufPowerNum_16K =  6,  /* 2^6  =   64   1M */
    E_MemBufPowerNum_64K =  5,  /* 2^5  =   32   2M */  
}E_MemBufPowerNum;


typedef struct tagT_GroupMemPool
{
    E_MemSizeType          eType;
    E_MemBufPowerNum       eNum;
    BYTE                  *pBuf;
    CObjMemPoolInterface  *pMemPool;
}T_GroupMemPool;


class CMsgMemPool : public CBaseData
{
public :
    enum { MEM_POOL_NUM = 6 };

    const static WORD32 s_adwBufSize[MEM_POOL_NUM];

public :
    CMsgMemPool (CCentralMemPool &rCentralMemPool);
    virtual ~CMsgMemPool();
    
    virtual WORD32 Initialize();
    
    virtual BYTE * Malloc(WORD32 dwSize);

    virtual WORD32 Free(BYTE *pAddr);
    
    virtual BOOL IsValid(BYTE *pAddr);

    VOID GetMeasure(T_MsgMemMeasure &rtMeasure);

    VOID Dump();

protected :
    WORD32 Calc(WORD32 dwSize);

protected :
    CCentralMemPool  &m_rCentralMemPool;
    T_GroupMemPool    m_atPool[MEM_POOL_NUM];
};


inline BYTE * CMsgMemPool::Malloc(WORD32 dwSize)
{
    WORD32 dwPos = Calc(dwSize);
    if (unlikely(dwPos >= MEM_POOL_NUM))
    {
        return NULL;
    }

    BYTE *pValue = NULL;

    for (; dwPos < MEM_POOL_NUM; dwPos++)
    {
        pValue = m_atPool[dwPos].pMemPool->Malloc(dwSize);
        if (NULL != pValue)
        {
            break;
        }
    }

    return pValue;
}


inline WORD32 CMsgMemPool::Free(BYTE *pAddr)
{
    WORD32 dwPos  = 0;
    BOOL   bValid = FALSE;

    for (; dwPos < MEM_POOL_NUM; dwPos++)
    {
        bValid = m_atPool[dwPos].pMemPool->IsValid(pAddr);
        if (bValid)
        {
            break ;
        }
    }

    if (unlikely(dwPos >= MEM_POOL_NUM))
    {
        return FAIL;
    }

    return m_atPool[dwPos].pMemPool->Free(pAddr);
}


inline BOOL CMsgMemPool::IsValid(BYTE *pAddr)
{
    BOOL bValid = FALSE;

    for (WORD32 dwIndex = 0; dwIndex < MEM_POOL_NUM; dwIndex++)
    {
        bValid = m_atPool[dwIndex].pMemPool->IsValid(pAddr);
        if (bValid)
        {
            break ;
        }
    }

    return bValid;
}


inline WORD32 CMsgMemPool::Calc(WORD32 dwSize)
{
    WORD32 dwPos = 0;
    for (; dwPos < MEM_POOL_NUM; dwPos++)
    {
        if (dwSize <= s_adwBufSize[dwPos])
        {
            break ;
        }
    }

    return dwPos;
}


#endif


