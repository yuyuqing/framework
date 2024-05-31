

#ifndef _BASE_MEM_INTERFACE_H_
#define _BASE_MEM_INTERFACE_H_


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>


#include "base_data_container.h"
#include "base_util.h"
#include "base_lock.h"
#include "base_ring_array.h"
#include "base_variable.h"


/* 1G大页内存 */
#define HUGEPAGE_FILE_SIZE    ((WORD64)(1 * 1024 * 1024 * 1024))

/* 大页内存页面数量 */
#define PAGE_NUM              ((WORD32)(4))

#define MEMPOOL_THRESHOLD     ((WORD16)(4))


typedef enum tagE_MemType
{
    E_MEM_SHARE_TYPE    = 0,
    E_MEM_HUGEPAGE_TYPE = 1,
    E_MEM_HEAP_TYPE     = 2,
}E_MemType;


/* sizeof(T_MemBufHeader)确保为64字节(1个CACHE_LINE大小) */
typedef struct tagT_MemBufHeader
{
    WORD32             dwSize;            /* 内存块大小, 单位 : CACHE_SIZE(64) */
    WORD32             dwRefCount;        /* 0xFFFFFFFF : 空闲未分配; 引用计数 */
    WORD64             lwOffset;          /* 从内存起始位置的偏移 */
    tagT_MemBufHeader *pNext;

    WORD32             dwPoolID;          /* 内存所属pool */
    WORD32             dwBlockID;         /* 内存所属block */

    WORD32             dwAllocThreadID;   /* 分配内存时的所属线程ID */
    WORD32             dwFreeThreadID;    /* 回收内存时的所属线程ID */
    WORD32             dwAllocPoint;      /* 分配内存时的位置信息, 唯一编号 */
    WORD32             dwHoldPeriod;      /* 分配后持有的时长(单位:Cycle) */
    WORD64             lwAllocTimeStamp;  /* 分配内存时的时间戳信息 */
    WORD64             lwStepTrace;       /* 跟踪内存处理路径(bitmap) */
}T_MemBufHeader;
static_assert(sizeof(T_MemBufHeader) == CACHE_SIZE, "unexpected T_MemBufHeader layout");


typedef struct tagT_ObjMemBuf
{
    T_MemBufHeader  tHeader;           /* 64 Byte */
    BYTE            aucData[];
}T_ObjMemBuf;


typedef struct tagT_MemBaseInfo
{
    VOID     *pAddr;   /* 起始地址 */
    WORD64    lwAddr;  /* 起始地址 */
    WORD64    lwSize;  /* 内存块大小 */
    SWORD32   iFD;
}T_MemBaseInfo;


typedef enum tagE_MemErrno
{
    E_MEM_ERR_INVALID = 0,
    E_MEM_ERR_COVER,           /* 内存被踩 */
    E_MEM_ERR_ILLEGAL_ADDR,    /* 释放非法地址 */
    E_MEM_ERR_SHIFT_ADDR,      /* 释放地址存在偏移 */
    E_MEM_ERR_REPEAT,          /* 重复释放 */
}E_MemErrno;


inline WORD32 AllocReplenish(T_MemBufHeader *ptMemHeader, 
                             WORD32          dwPoint)
{
    WORD32 dwSize   = ptMemHeader->dwSize * CACHE_SIZE + sizeof(T_MemBufHeader);
    WORD32 dwRemain = (WORD32)(ptMemHeader->lwOffset % dwSize);

    ptMemHeader->dwAllocThreadID  = m_dwSelfThreadID;
    ptMemHeader->dwFreeThreadID   = INVALID_DWORD;
    ptMemHeader->dwHoldPeriod     = 0;
    ptMemHeader->lwAllocTimeStamp = GetCycle();
    ptMemHeader->lwStepTrace      = 0;

    /* 检查内存是否出现踩踏 */
    if (unlikely(dwRemain != 0))
    {
        return (WORD32)(E_MEM_ERR_COVER);
    }
    
    return SUCCESS;
}


inline WORD32 FreeReplenish(T_MemBufHeader *ptMemHeader, 
                            WORD64          lwOriAddr,
                            WORD64          lwEndAddr,
                            WORD32          dwTrunkSize)
{
    WORD32 dwSize   = ptMemHeader->dwSize * CACHE_SIZE + sizeof(T_MemBufHeader);
    WORD32 dwRemain = (WORD32)(ptMemHeader->lwOffset % dwSize);
    WORD64 lwAddr   = (WORD64)ptMemHeader;

    if ((lwAddr < lwOriAddr) || (lwAddr >= lwEndAddr))
    {
        /* 待释放内存地址不在合法地址范围内 */
        return (WORD32)(E_MEM_ERR_ILLEGAL_ADDR);
    }

    if (0 != ((lwAddr - lwOriAddr) % dwTrunkSize))
    {
        /* 待释放内存地址虽然在合法地址范围内, 但地址发生偏移 */
        return (WORD32)(E_MEM_ERR_SHIFT_ADDR);
    }

    if ((dwSize != dwTrunkSize) || (dwRemain != 0))
    {
        /* 待释放地址合法, 但可能被踩踏 */
        return (WORD32)(E_MEM_ERR_COVER);
    }

    if (INVALID_DWORD == ptMemHeader->dwRefCount)
    {
        /* 待释放地址合法, 但可能发生重复释放或被踩 */
        return (WORD32)(E_MEM_ERR_REPEAT);
    }

    WORD64 lwFreeTimeStamp = GetCycle();

    ptMemHeader->dwFreeThreadID = m_dwSelfThreadID;
    ptMemHeader->dwHoldPeriod   = (WORD32)(lwFreeTimeStamp - ptMemHeader->lwAllocTimeStamp);

    return SUCCESS;
}


class CMemInterface : public CBaseData
{
public :
    CMemInterface ();
    virtual ~CMemInterface();

    virtual WORD32 Initialize(VOID *pOriAddr, WORD64 lwSize);

    virtual BYTE * Malloc(WORD32 dwSize,
                          WORD16 wThreshold = MEMPOOL_THRESHOLD,
                          WORD32 dwPoint = INVALID_DWORD) = 0;
    virtual WORD32 Free(BYTE *pAddr) = 0;
    
    virtual BOOL IsEmpty() = 0;
    virtual BOOL IsValid(BYTE *pAddr) = 0;

    WORD64 GetBaseAddr();

    /* 获取使用量信息 */
    VOID GetUsage(WORD64 &lwSize, WORD64 &lwUsedSize, WORD64 &lwFreeSize);

    virtual VOID Dump();

protected :
    /* 内存基址信息(初始化后保持不变) */
    VOID                *m_pOriAddr;      
    WORD64               m_lwAddr;
    WORD64               m_lwSize;

    WORD64               m_lwBegin;
    WORD64               m_lwEnd;

    std::atomic<WORD64>  m_lwUsedSize;
    std::atomic<WORD64>  m_lwFreeSize;

    T_MemBufHeader      *m_pFreeHeader;   /* 空闲链表 */
};


inline WORD64 CMemInterface::GetBaseAddr()
{
    return (WORD64)m_pOriAddr;
}


/* 获取使用量信息 */
inline VOID CMemInterface::GetUsage(WORD64 &lwSize, WORD64 &lwUsedSize, WORD64 &lwFreeSize)
{
    lwSize     = m_lwSize;
    lwUsedSize = m_lwUsedSize.load(std::memory_order_relaxed);;
    lwFreeSize = m_lwFreeSize.load(std::memory_order_relaxed);;
}


class CCentralMemPool : public CMemInterface
{    
public :
    CCentralMemPool ();
    virtual ~CCentralMemPool();

    virtual BYTE * Malloc(WORD32 dwSize,
                          WORD16 wThreshold = MEMPOOL_THRESHOLD,
                          WORD32 dwPoint = INVALID_DWORD);

    virtual WORD32 Free(BYTE *pAddr);

    virtual BOOL IsEmpty();
    virtual BOOL IsValid(BYTE *pAddr);

protected :    
    /* 内存分配/回收加锁 */
    CTicketLock    m_cLock;
};


inline BOOL CCentralMemPool::IsEmpty()
{
    BOOL bEmpty = FALSE;

    m_cLock.Lock();
    bEmpty = (NULL == m_pFreeHeader);
    m_cLock.UnLock();

    return bEmpty;
}


inline BOOL CCentralMemPool::IsValid(BYTE *pAddr)
{
    if (unlikely(NULL == pAddr))
    {
        return FALSE;
    }

    WORD64 lwAddr = (WORD64)pAddr;

    if (unlikely((lwAddr < m_lwBegin)
              || (lwAddr >= m_lwEnd)))
    {
        return FALSE;
    }

    return (0 == ((lwAddr - m_lwAddr - sizeof(T_MemBufHeader)) % CACHE_SIZE));
}


class CBlockMemObject;


/* 分配定长内存块的内存池 */
class CObjMemPoolInterface : public CMemInterface
{
public :
    friend class CBlockMemObject;

protected :
    using CMemInterface::Initialize;

public :
    CObjMemPoolInterface (CCentralMemPool &rCentralMemPool);
    virtual ~CObjMemPoolInterface();

    /* dwPowerNum : 内存块数量(2^N); dwBufSize : 每个内存块大小 */
    virtual WORD32 Initialize(WORD32 dwPowerNum, WORD32 dwBufSize);

    WORD32 GetBufSize();

    virtual VOID Dump();

protected :
    CCentralMemPool  &m_rCentralMemPool;
    WORD32            m_dwPowerNum;
    WORD32            m_dwBufNum;
    WORD32            m_dwBufSize;
};


inline WORD32 CObjMemPoolInterface::GetBufSize()
{
    return m_dwBufSize;
}


template <WORD32 POWER_NUM>
class CObjectMemPool : public CObjMemPoolInterface
{
public :
    static const WORD32 s_dwRingPower = (POWER_NUM + 1);
    static const WORD32 s_dwOffset    = offsetof(T_ObjMemBuf, aucData);
    static_assert(s_dwOffset == CACHE_SIZE, "unexpected T_ObjMemBuf layout");

    typedef CSimpleRing<s_dwRingPower>  CMemRing;

public :
    CObjectMemPool (CCentralMemPool &rCentralMemPool);
    virtual ~CObjectMemPool();

    /* Trunk内存池类型时调用 */
    WORD32 Initialize(WORD32 dwPoolID, WORD32 dwBlockID, WORD32 dwPowerNum, WORD32 dwBufSize);

    virtual WORD32 Initialize(WORD32 dwPowerNum, WORD32 dwBufSize);

    virtual BYTE * Malloc(WORD32 dwSize, WORD16 wThreshold = MEMPOOL_THRESHOLD, WORD32 dwPoint = INVALID_DWORD);
    virtual WORD32 Free(BYTE *pAddr);

    virtual BOOL IsEmpty();
    virtual BOOL IsValid(BYTE *pAddr);

    virtual WORD32 GetBufSize();

protected :
    CMemRing  m_cRing;
};


template <WORD32 POWER_NUM>
CObjectMemPool<POWER_NUM>::CObjectMemPool (CCentralMemPool &rCentralMemPool)
    : CObjMemPoolInterface(rCentralMemPool)
{
}


template <WORD32 POWER_NUM>
CObjectMemPool<POWER_NUM>::~CObjectMemPool()
{
}


/* Trunk内存池类型时调用 */
template <WORD32 POWER_NUM>
WORD32 CObjectMemPool<POWER_NUM>::Initialize(WORD32 dwPoolID, 
                                             WORD32 dwBlockID, 
                                             WORD32 dwPowerNum, 
                                             WORD32 dwBufSize)
{
    if (dwPowerNum > POWER_NUM)
    {
        assert(0);
    }

    m_cRing.Initialize();

    CObjMemPoolInterface::Initialize(dwPowerNum, dwBufSize);

    T_MemBufHeader *pObj = NULL;

    for (WORD32 dwIndex = 0; dwIndex < m_dwBufNum; dwIndex++)
    {
        pObj = (T_MemBufHeader *)(m_lwBegin + (m_dwBufSize * dwIndex));

        pObj->dwSize     = (m_dwBufSize - s_dwOffset) / CACHE_SIZE;
        pObj->dwRefCount = INVALID_DWORD;
        pObj->lwOffset   = m_dwBufSize * dwIndex;
        pObj->pNext      = NULL;
        pObj->dwPoolID   = dwPoolID;
        pObj->dwBlockID  = dwBlockID;

        m_cRing.Enqueue((VOID *)pObj);
    }

    return SUCCESS;
}


template <WORD32 POWER_NUM>
WORD32 CObjectMemPool<POWER_NUM>::Initialize(WORD32 dwPowerNum, WORD32 dwBufSize)
{
    if (dwPowerNum != POWER_NUM)
    {
        assert(0);
    }

    return CObjectMemPool<POWER_NUM>::Initialize(INVALID_DWORD,
                                                 INVALID_DWORD,
                                                 dwPowerNum,
                                                 dwBufSize);
}


template <WORD32 POWER_NUM>
inline BYTE * CObjectMemPool<POWER_NUM>::Malloc(WORD32 dwSize, 
                                                WORD16 wThreshold, 
                                                WORD32 dwPoint)
{
    if (unlikely(dwSize > (m_dwBufSize - s_dwOffset)))
    {
        return NULL;
    }

    BYTE           *pValue = NULL;
    WORD32          dwNum  = 0;
    T_MemBufHeader *pObj   = NULL;

    dwNum = m_cRing.Dequeue((VOID **)(&pObj), wThreshold);
    if (0 == dwNum)
    {
        return NULL;
    }

    m_lwUsedSize++;
    pObj->dwRefCount   = 0;
    pObj->dwAllocPoint = dwPoint;

#ifdef MEM_CHECK
    if (INVALID_DWORD != dwPoint)
    {
        WORD32 dwResult = AllocReplenish(pObj, dwPoint);
        if (SUCCESS != dwResult)
        {
            assert(0);
        }
    }
#endif

    pValue = (BYTE *)((WORD64)(pObj) + s_dwOffset);
    __builtin_prefetch(((CHAR *)pValue), 1, 2);

    return pValue;
}


template <WORD32 POWER_NUM>
inline WORD32 CObjectMemPool<POWER_NUM>::Free(BYTE *pAddr)
{
    T_MemBufHeader *pObj = (T_MemBufHeader *)((WORD64)(pAddr) - s_dwOffset);
    __builtin_prefetch((CHAR *)pObj, 1, 1);

#ifdef MEM_CHECK
    WORD32 dwResult = FreeReplenish(pObj, m_lwBegin, m_lwEnd, m_dwBufSize);
    if (SUCCESS != dwResult)
    {
        assert(0);
    }
#else
    if (!IsValid(pAddr))
    {
        assert(0);
    }
#endif

    if ( (pObj->dwSize != ((m_dwBufSize - s_dwOffset) / CACHE_SIZE))
      || (INVALID_DWORD == pObj->dwRefCount))
    {
        /* 内存被踩 or 重复释放 */
        assert(0);
    }

    if (0 != pObj->dwRefCount)
    {
        pObj->dwRefCount--;
        return FAIL;
    }

    pObj->dwRefCount = INVALID_DWORD;

    m_cRing.Enqueue((VOID *)pObj);
    m_lwFreeSize++;

    return SUCCESS;
}


template <WORD32 POWER_NUM>
inline BOOL CObjectMemPool<POWER_NUM>::IsEmpty()
{
    return (0 == m_cRing.Count());
}


template <WORD32 POWER_NUM>
inline BOOL CObjectMemPool<POWER_NUM>::IsValid(BYTE *pAddr)
{
    if (unlikely(NULL == pAddr))
    {
        return FALSE;
    }

    WORD64 lwAddr = (WORD64)pAddr;

    if ( (lwAddr < (m_lwBegin + s_dwOffset)) 
      || (lwAddr >= m_lwEnd))
    {
        return FALSE;
    }

    return (0 == ((lwAddr - m_lwBegin - s_dwOffset) % m_dwBufSize));
}


template <WORD32 POWER_NUM>
inline WORD32 CObjectMemPool<POWER_NUM>::GetBufSize()
{
    return m_dwBufSize;
}


#endif


