

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


#include "base_util.h"
#include "base_lock.h"
#include "base_data_container.h"
#include "base_data_array.h"
#include "base_ring.h"
#include "base_variable.h"


/* 1G大页内存 */
#define HUGEPAGE_FILE_SIZE    ((WORD64)(1 * 1024 * 1024 * 1024))

/* 大页内存页面数量 */
#define PAGE_NUM              ((WORD32)(8))

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
    tagT_MemBufHeader *pNext;
    WORD64             lwOffset;          /* 从内存起始位置的偏移 */
    WORD32             dwRefCount;        /* 0xFFFFFFFF : 空闲未分配; 引用计数 */
    WORD32             dwIndex;           /* 前24字节保持与T_DataHeader一致 */

    WORD32             dwSize;            /* 内存块大小, 单位 : CACHE_SIZE(64) */
    WORD32             dwPoolID;          /* 内存所属pool */
    WORD32             dwBlockID;         /* 内存所属block */
    WORD32             dwPoint;           /* 分配内存时的位置信息, 唯一编号 */
    WORD64             lwMagicValue;      /* 保留 */
    WORD64             lwMemAddr;         /* 保留 */
    WORD64             lwMemPool;         /* 所属内存池实例地址 */
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


class CMemInterface : public CAllocInterface
{
public :
    CMemInterface ();
    virtual ~CMemInterface();

    virtual WORD32 Initialize(VOID *pOriAddr, WORD64 lwSize);

    virtual BOOL IsEmpty() = 0;
    virtual BOOL IsValid(VOID *pAddr) = 0;

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

    virtual BYTE * Malloc(WORD32 dwSize);
    virtual WORD32 Free(VOID *pAddr);

    virtual BOOL IsEmpty();
    virtual BOOL IsValid(VOID *pAddr);

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


inline BOOL CCentralMemPool::IsValid(VOID *pAddr)
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
protected :
    using CMemInterface::Initialize;

public :
    friend class CBlockMemObject;

public :
    CObjMemPoolInterface (CCentralMemPool &rCentralMemPool);
    virtual ~CObjMemPoolInterface();

    virtual WORD32 Initialize(WORD32 dwBufSize, WORD32 dwPoolID, WORD32 dwBlockID) = 0;

    WORD32 GetBufSize();

    virtual VOID Dump();

protected :
    /* dwPowerNum : 内存块数量(2^N); dwBufSize : 每个内存块大小 */
    WORD32 Initialize(WORD32 dwPowerNum, WORD32 dwBufSize);

protected :
    CCentralMemPool  &m_rCentralMemPool;
    WORD32            m_dwPowerNum;
    WORD32            m_dwBufNum;
    WORD32            m_dwBufSize;
    WORD16            m_wThreshold;
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

    typedef CBaseRingTpl<s_dwRingPower>  CMemRing;

public :
    CObjectMemPool (CCentralMemPool &rCentralMemPool);
    virtual ~CObjectMemPool();

    WORD32 Initialize(WORD32 dwBufSize, WORD32 dwPoolID, WORD32 dwBlockID);

    virtual BYTE * Malloc(WORD32 dwSize);
    virtual BYTE * Malloc(WORD32 dwSize, WORD32 dwPoolID, WORD32 dwPoint);
    virtual WORD32 Free(VOID *pAddr);

    virtual BOOL IsEmpty();
    virtual BOOL IsValid(VOID *pAddr);

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
WORD32 CObjectMemPool<POWER_NUM>::Initialize(WORD32 dwBufSize,
                                             WORD32 dwPoolID,
                                             WORD32 dwBlockID)
{
    m_cRing.Initialize();

    CObjMemPoolInterface::Initialize(POWER_NUM, dwBufSize);

    WORD64          lwOffset  = 0;
    WORD64          lwMemPool = (WORD64)this;
    T_MemBufHeader *pObj      = NULL;

    for (WORD32 dwIndex = 0; dwIndex < m_dwBufNum; dwIndex++)
    {
        pObj = (T_MemBufHeader *)(m_lwBegin + lwOffset);

        pObj->pNext      = NULL;
        pObj->lwOffset   = lwOffset;
        pObj->dwRefCount = INVALID_DWORD;
        pObj->dwIndex    = dwIndex;
        pObj->dwSize     = (m_dwBufSize - s_dwOffset) / CACHE_SIZE;
        pObj->dwPoolID   = dwPoolID;
        pObj->dwBlockID  = dwBlockID;
        pObj->lwMemPool  = lwMemPool;

        lwOffset += m_dwBufSize;

        m_cRing.Enqueue((VOID *)pObj);
    }

    return SUCCESS;
}


template <WORD32 POWER_NUM>
inline BYTE * CObjectMemPool<POWER_NUM>::Malloc(WORD32 dwSize)
{
    if (unlikely(dwSize > (m_dwBufSize - s_dwOffset)))
    {
        return NULL;
    }

    BYTE           *pValue = NULL;
    WORD32          dwNum  = 0;
    T_MemBufHeader *pObj   = NULL;

    dwNum = m_cRing.Dequeue((VOID **)(&pObj), m_wThreshold);
    if (0 == dwNum)
    {
        return NULL;
    }

    m_lwUsedSize++;
    pObj->dwRefCount = 0;

    pValue = (BYTE *)((WORD64)(pObj) + s_dwOffset);
    __builtin_prefetch(((CHAR *)pValue), 1, 2);

    return pValue;
}


template <WORD32 POWER_NUM>
inline BYTE * CObjectMemPool<POWER_NUM>::Malloc(WORD32 dwSize,
                                                WORD32 dwPoolID,
                                                WORD32 dwPoint)
{
    if (unlikely(dwSize > (m_dwBufSize - s_dwOffset)))
    {
        return NULL;
    }

    BYTE           *pValue = NULL;
    WORD32          dwNum  = 0;
    T_MemBufHeader *pObj   = NULL;

    dwNum = m_cRing.Dequeue((VOID **)(&pObj), m_wThreshold);
    if (0 == dwNum)
    {
        return NULL;
    }

    m_lwUsedSize++;
    pObj->dwRefCount = 0;
    pObj->dwPoint    = dwPoint;

    pValue = (BYTE *)((WORD64)(pObj) + s_dwOffset);
    __builtin_prefetch(((CHAR *)pValue), 1, 2);

    return pValue;
}


template <WORD32 POWER_NUM>
inline WORD32 CObjectMemPool<POWER_NUM>::Free(VOID *pAddr)
{
    T_MemBufHeader *pObj = (T_MemBufHeader *)((WORD64)(pAddr) - s_dwOffset);
    __builtin_prefetch((CHAR *)pObj, 1, 1);

    if (!IsValid(pAddr))
    {
        assert(0);
    }

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
inline BOOL CObjectMemPool<POWER_NUM>::IsValid(VOID *pAddr)
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


