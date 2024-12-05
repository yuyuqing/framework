

#ifndef _BASE_DATA_CONTAINER_H_
#define _BASE_DATA_CONTAINER_H_


#include "base_allocator.h"


typedef struct tagT_DataHeadPad
{
    WORD32    dwSize;        /* 内存块大小; 单位 : 64字节 */
    WORD32    dwPoolID;      /* 所属内存池;  */
    WORD32    dwBlockID;     /* 所属内存池; */
    WORD32    dwPoint;       /* 内存申请点; */
    WORD64    lwMagicValue;  /* 保留 */
    WORD64    lwMemAddr;     /* 保留 */
}T_DataHeadPad;


/* ALIGN_FLAG : 按CACHE_SIZE对齐标志 */
template <typename T, BOOL ALIGN_FLAG>
class CLinkTpl
{
public :
    CLinkTpl() {}
    virtual ~CLinkTpl() = 0;
};


template <typename T>
class CLinkTpl<T, TRUE>
{
public :
    typedef struct tagT_DataHeader
    {
        tagT_DataHeader  *m_pNext;
        tagT_DataHeader  *m_pPrev;
        WORD32            m_dwRefCount;  /* 0xFFFFFFFF : 空闲未分配; 引用计数 */
        WORD32            m_dwIndex;
        T_DataHeadPad     m_tPad;
        WORD64            m_lwMemPool;
        BYTE              m_aucData[sizeof(T)];

        operator T & ()
        {
            return *((T *)(m_aucData));
        }

        operator T * ()
        {
            return (T *)(m_aucData);
        }
    }T_DataHeader;
    static_assert(offsetof(T_DataHeader, m_aucData) == CACHE_SIZE, "unexpected layout");
};


template <typename T>
class CLinkTpl<T, FALSE>
{
public :
    typedef struct tagT_DataHeader
    {
        tagT_DataHeader  *m_pNext;
        tagT_DataHeader  *m_pPrev;
        WORD32            m_dwRefCount;  /* 0xFFFFFFFF : 空闲未分配; 引用计数 */
        WORD32            m_dwIndex;
        WORD64            m_lwMemPool;
        BYTE              m_aucData[sizeof(T)];

        operator T & ()
        {
            return *((T *)(m_aucData));
        }

        operator T * ()
        {
            return (T *)(m_aucData);
        }
    }T_DataHeader;
    static_assert(offsetof(T_DataHeader, m_aucData) == 32, "unexpected layout");
};


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG = FALSE>
class CBaseDataContainer : public CLinkTpl<T, ALIGN_FLAG>, public CAllocInterface
{
public :
    typedef typename CLinkTpl<T, ALIGN_FLAG>::T_DataHeader  T_DataHeader;

    static const WORD32 s_dwNodeSize   = ROUND_UP(sizeof(T_DataHeader), CACHE_SIZE);
    static const WORD32 s_dwDataOffset = offsetof(T_DataHeader, m_aucData);
    static const WORD32 s_dwTotalSize  = (s_dwNodeSize * NODE_NUM) + CACHE_SIZE;

public :
    CBaseDataContainer ();
    virtual ~CBaseDataContainer();

    virtual WORD32 Initialize();

    BYTE * Malloc(WORD32 dwSize = 0);
    BYTE * Malloc(WORD32 dwSize, WORD32 dwIndex);
    BYTE * Malloc(WORD32 dwSize, WORD32 dwPoolID, WORD32 dwPoint);

    T * Malloc(WORD32 *pdwIndex);

    WORD32 Free(VOID *pAddr);
    WORD32 Free(WORD32 dwIndex);

    T_DataHeader & operator[] (WORD32 dwIndex);
    T * operator() (WORD32 dwIndex);

    BOOL IsFree(WORD32 dwIndex);
    BOOL IsValid(VOID *pAddr);

protected :
    WORD64         m_lwBegin;
    WORD64         m_lwEnd;

    T_DataHeader  *m_pFreeHeader;
    BYTE           m_aucData[s_dwTotalSize];
};


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::CBaseDataContainer ()
{
    WORD64 lwAlign = CACHE_SIZE;
    WORD64 lwBegin = (WORD64)(&(m_aucData[0]));

    m_lwBegin     = ROUND_UP(lwBegin, lwAlign);
    m_lwEnd       = m_lwBegin + (s_dwNodeSize * NODE_NUM);
    m_pFreeHeader = NULL;
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::~CBaseDataContainer ()
{
    m_pFreeHeader = NULL;
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
WORD32 CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::Initialize()
{
    m_pFreeHeader = (T_DataHeader *)(m_lwBegin);

    WORD32        dwIndex   = NODE_NUM;
    WORD64        lwMemPool = (WORD64)this;
    WORD64        lwAddr    = m_lwEnd;
    T_DataHeader *pNextNode = NULL;
    T_DataHeader *pCurNode  = NULL;

    while (lwAddr > m_lwBegin)
    {
        dwIndex--;
        lwAddr  -= s_dwNodeSize;
        pCurNode = (T_DataHeader *)(lwAddr);

        pCurNode->m_pNext      = pNextNode;
        pCurNode->m_pPrev      = NULL;
        pCurNode->m_dwRefCount = INVALID_DWORD;
        pCurNode->m_dwIndex    = dwIndex;
        pCurNode->m_lwMemPool  = lwMemPool;

        if (NULL != pNextNode)
        {
            pNextNode->m_pPrev = pCurNode;
        }

        pNextNode = pCurNode;
    }

    return SUCCESS;
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline BYTE * CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::Malloc(WORD32 dwSize)
{
    T_DataHeader *pCurHead = m_pFreeHeader;

    if (pCurHead)
    {
        m_pFreeHeader          = pCurHead->m_pNext;
        pCurHead->m_pNext      = NULL;
        pCurHead->m_dwRefCount = 0;
        m_pFreeHeader->m_pPrev = NULL;

        return pCurHead->m_aucData;
    }
    else
    {
        return NULL;
    }
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline BYTE * CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::Malloc(
    WORD32 dwSize,
    WORD32 dwIndex)
{
    if (unlikely(dwIndex >= NODE_NUM))
    {
        return NULL;
    }

    T_DataHeader *pData = NULL;
    T_DataHeader *pPrev = NULL;
    T_DataHeader *pNext = NULL;

    pData = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * dwIndex));
    if (INVALID_DWORD != pData->m_dwRefCount)
    {
        return NULL;
    }

    pNext = pData->m_pNext;
    pPrev = pData->m_pPrev;

    if (NULL == pPrev)
    {
        /* 已经处于链表头 */
        m_pFreeHeader = pNext;
    }
    else
    {
        pPrev->m_pNext = pNext;
    }

    if (NULL == pNext)
    {
        /* 已经处于链表尾 */
    }
    else
    {
        pNext->m_pPrev = pPrev;
    }

    pData->m_pNext      = NULL;
    pData->m_pPrev      = NULL;
    pData->m_dwRefCount = 0;

    return pData->m_aucData;
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline BYTE * CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::Malloc(
    WORD32 dwSize,
    WORD32 dwPoolID,
    WORD32 dwPoint)
{
    return CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::Malloc(dwSize);
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::Malloc(WORD32 *pdwIndex)
{
    T_DataHeader *pCurHead = m_pFreeHeader;

    if (pCurHead)
    {
        m_pFreeHeader          = pCurHead->m_pNext;
        pCurHead->m_pNext      = NULL;
        pCurHead->m_dwRefCount = 0;
        m_pFreeHeader->m_pPrev = NULL;

        *pdwIndex = pCurHead->m_dwIndex;

        return (T *)(pCurHead->m_aucData);
    }
    else
    {
        return NULL;
    }
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline WORD32 CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::Free(VOID *pAddr)
{
    if (!IsValid(pAddr))
    {
        return FAIL;
    }

    T_DataHeader *pCur = (T_DataHeader *)((WORD64)(pAddr) - s_dwDataOffset);
    if ((pCur->m_dwIndex >= NODE_NUM) || (INVALID_DWORD == pCur->m_dwRefCount))
    {
        return FAIL;
    }

    T_DataHeader *pNode = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * (pCur->m_dwIndex)));
    if (unlikely(pCur != pNode))
    {
        return FAIL;
    }

    m_pFreeHeader->m_pPrev = pNode;
    pNode->m_pNext         = m_pFreeHeader;
    pNode->m_dwRefCount    = INVALID_DWORD;
    m_pFreeHeader          = pNode;

    return SUCCESS;
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline WORD32 CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::Free(WORD32 dwIndex)
{
    if (unlikely(dwIndex >= NODE_NUM))
    {
        return FAIL;
    }

    T_DataHeader *pNode = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * dwIndex));
    if (INVALID_DWORD == pNode->m_dwRefCount)
    {
        return FAIL;
    }

    m_pFreeHeader->m_pPrev = pNode;
    pNode->m_pNext         = m_pFreeHeader;
    pNode->m_dwRefCount    = INVALID_DWORD;
    m_pFreeHeader          = pNode;

    return SUCCESS;
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline typename CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::T_DataHeader & 
CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::operator[] (WORD32 dwIndex)
{
    if (unlikely(dwIndex >= NODE_NUM))
    {
        return *(T_DataHeader *)NULL;
    }

    T_DataHeader *pData = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * dwIndex));

    return *pData;
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::operator() (WORD32 dwIndex)
{
    if (unlikely(dwIndex >= NODE_NUM))
    {
        return NULL;
    }

    T_DataHeader *pData = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * dwIndex));
    if (INVALID_DWORD == pData->m_dwRefCount)
    {
        return NULL;
    }

    return (T *)(pData->m_aucData);
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline BOOL CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::IsFree(WORD32 dwIndex)
{
    if (unlikely(dwIndex >= NODE_NUM))
    {
        return FALSE;
    }

    T_DataHeader *pData = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * dwIndex));

    return (INVALID_DWORD == pData->m_dwRefCount);
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline BOOL CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::IsValid(VOID *pAddr)
{
    if (unlikely(NULL == pAddr))
    {
        return FALSE;
    }

    WORD64 lwAddr = (WORD64)pAddr;

    if (unlikely((lwAddr < (m_lwBegin + s_dwDataOffset)) || (lwAddr >= m_lwEnd)))
    {
        return FALSE;
    }

    return (0 == ((lwAddr - m_lwBegin - s_dwDataOffset) % s_dwNodeSize));
}


template <typename T>
class CSelfDataContainer : public CLinkTpl<T, TRUE>, public CAllocInterface 
{
public :
    typedef typename CLinkTpl<T, TRUE>::T_DataHeader  T_DataHeader;

    static const WORD32 s_dwNodeSize   = ROUND_UP(sizeof(T_DataHeader), CACHE_SIZE);
    static const WORD32 s_dwDataOffset = offsetof(T_DataHeader, m_aucData);
    static const WORD32 s_dwPadOffset  = CACHE_SIZE - sizeof(T_DataHeadPad);

public :
    CSelfDataContainer ();
    virtual ~CSelfDataContainer();

    WORD32 Initialize(CAllocInterface *pMemPool, WORD32 dwNodeNum);

    BYTE * Malloc(WORD32 dwSize = 0);
    BYTE * Malloc(WORD32 dwSize, WORD32 dwIndex);
    WORD32 Free(VOID *pAddr);

    T_DataHeader & operator[] (WORD32 dwIndex);
    T * operator() (WORD32 dwIndex);

    BOOL IsFree(WORD32 dwIndex);
    BOOL IsValid(VOID *pAddr);

protected :
    CAllocInterface  *m_pMemPool;

    /* 内存基址信息(初始化后保持不变) */
    WORD32            m_dwNodeNum;
    WORD32            m_dwTotalSize;
    WORD64            m_lwBegin;
    WORD64            m_lwEnd;

    T_DataHeader     *m_pFreeHeader;
};


template <typename T>
CSelfDataContainer<T>::CSelfDataContainer ()
{
    m_pMemPool    = NULL;
    m_dwNodeNum   = 0;
    m_dwTotalSize = 0;
    m_lwBegin     = 0;
    m_lwEnd       = 0;
    m_pFreeHeader = NULL;
}


template <typename T>
CSelfDataContainer<T>::~CSelfDataContainer()
{
    if (0 != m_lwBegin)
    {
        m_pMemPool->Free((VOID *)m_lwBegin);
    }

    m_pMemPool    = NULL;
    m_dwNodeNum   = 0;
    m_dwTotalSize = 0;
    m_lwBegin     = 0;
    m_lwEnd       = 0;
    m_pFreeHeader = NULL;
}


template <typename T>
WORD32 CSelfDataContainer<T>::Initialize(CAllocInterface *pMemPool, WORD32 dwNodeNum)
{
    m_pMemPool    = pMemPool;
    m_dwNodeNum   = dwNodeNum + 1;
    m_dwTotalSize = m_dwNodeNum * s_dwNodeSize;

    BYTE *pBuf = m_pMemPool->Malloc(m_dwTotalSize);
    if (NULL == pBuf)
    {
        return FAIL;
    }

    WORD32        dwIndex   = m_dwNodeNum;
    WORD64        lwMemPool = (WORD64)this;
    WORD64        lwAddr    = (WORD64)pBuf;
    WORD64        lwSize    = m_dwTotalSize;
    T_DataHeader *pNextNode = NULL;
    T_DataHeader *pCurNode  = NULL;

    m_lwBegin     = lwAddr;
    m_lwEnd       = lwAddr + lwSize;
    m_pFreeHeader = (T_DataHeader *)(m_lwBegin);

    lwAddr = m_lwEnd;

    while (lwAddr > m_lwBegin)
    {
        dwIndex--;
        lwAddr  -= s_dwNodeSize;
        pCurNode = (T_DataHeader *)(lwAddr);

        pCurNode->m_pNext      = pNextNode;
        pCurNode->m_pPrev      = NULL;
        pCurNode->m_dwRefCount = INVALID_DWORD;
        pCurNode->m_dwIndex    = dwIndex;
        pCurNode->m_lwMemPool  = lwMemPool;

        if (NULL != pNextNode)
        {
            pNextNode->m_pPrev = pCurNode;
        }

        pNextNode = pCurNode;
    }

    return SUCCESS;
}


template <typename T>
inline BYTE * CSelfDataContainer<T>::Malloc(WORD32 dwSize)
{
    T_DataHeader *pCurHead = m_pFreeHeader;

    if (pCurHead)
    {
        m_pFreeHeader          = pCurHead->m_pNext;
        pCurHead->m_pNext      = NULL;
        pCurHead->m_dwRefCount = 0;
        m_pFreeHeader->m_pPrev = NULL;

        return pCurHead->m_aucData;
    }
    else
    {
        return NULL;
    }
}


template <typename T>
inline BYTE * CSelfDataContainer<T>::Malloc(WORD32 dwSize, WORD32 dwIndex)
{
    if (unlikely(dwIndex >= m_dwNodeNum))
    {
        return NULL;
    }

    T_DataHeader *pData = NULL;
    T_DataHeader *pPrev = NULL;
    T_DataHeader *pNext = NULL;

    pData = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * dwIndex));
    if (INVALID_DWORD != pData->m_dwRefCount)
    {
        return NULL;
    }

    pNext = pData->m_pNext;
    pPrev = pData->m_pPrev;

    if (NULL == pPrev)
    {
        /* 已经处于链表头 */
        m_pFreeHeader = pNext;
    }
    else
    {
        pPrev->m_pNext = pNext;
    }

    if (NULL == pNext)
    {
        /* 已经处于链表尾 */
    }
    else
    {
        pNext->m_pPrev = pPrev;
    }

    pData->m_pNext      = NULL;
    pData->m_pPrev      = NULL;
    pData->m_dwRefCount = 0;

    return pData->m_aucData;
}


template <typename T>
inline WORD32 CSelfDataContainer<T>::Free(VOID *pAddr)
{
    if (!IsValid(pAddr))
    {
        return FAIL;
    }

    T_DataHeader *pCur = (T_DataHeader *)((WORD64)(pAddr) - s_dwDataOffset);
    if ((pCur->m_dwIndex >= m_dwNodeNum) || (INVALID_DWORD == pCur->m_dwRefCount))
    {
        return FAIL;
    }

    T_DataHeader *pNode = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * (pCur->m_dwIndex)));
    if (unlikely(pCur != pNode))
    {
        return FAIL;
    }

    m_pFreeHeader->m_pPrev = pNode;
    pNode->m_pNext         = m_pFreeHeader;
    pNode->m_dwRefCount    = INVALID_DWORD;
    m_pFreeHeader          = pNode;

    return SUCCESS;
}


template <typename T>
inline typename CSelfDataContainer<T>::T_DataHeader & 
CSelfDataContainer<T>::operator[] (WORD32 dwIndex)
{
    if (unlikely(dwIndex >= m_dwNodeNum))
    {
        return *(T_DataHeader *)NULL;
    }

    T_DataHeader *pData = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * dwIndex));

    return *pData;
}


template <typename T>
inline T * CSelfDataContainer<T>::operator() (WORD32 dwIndex)
{
    if (unlikely(dwIndex >= m_dwNodeNum))
    {
        return NULL;
    }

    T_DataHeader *pData = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * dwIndex));
    if (INVALID_DWORD == pData->m_dwRefCount)
    {
        return NULL;
    }

    return (T *)(pData->m_aucData);
}


template <typename T>
inline BOOL CSelfDataContainer<T>::IsFree(WORD32 dwIndex)
{
    if (unlikely(dwIndex >= m_dwNodeNum))
    {
        return FALSE;
    }

    T_DataHeader *pData = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * dwIndex));

    return (INVALID_DWORD == pData->m_dwRefCount);
}


template <typename T>
inline BOOL CSelfDataContainer<T>::IsValid(VOID *pAddr)
{
    if (unlikely(NULL == pAddr))
    {
        return FALSE;
    }

    WORD64 lwAddr = (WORD64)pAddr;

    if (unlikely((lwAddr < (m_lwBegin + s_dwDataOffset)) || (lwAddr >= m_lwEnd)))
    {
        return FALSE;
    }

    return (0 == ((lwAddr - m_lwBegin - s_dwDataOffset) % s_dwNodeSize));
}


#endif


