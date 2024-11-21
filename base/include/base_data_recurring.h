

#ifndef _BASE_DATA_RECURRING_H_
#define _BASE_DATA_RECURRING_H_


#include "base_call_back.h"
#include "base_mem_interface.h"


#define SELF_DATA_DEFAULT_NUM    ((WORD32)(2048))


template <typename T>
class CSelfDataContainer : public CBaseData
{
public :
    static CSelfDataContainer<T> * GetInstance(
        CCentralMemPool *pCentranMemPool = NULL,
        WORD32           dwDataNum = SELF_DATA_DEFAULT_NUM)
    {
        if (NULL != s_pInstance)
        {
            return s_pInstance;
        }

        if (NULL == pCentranMemPool)
        {
            return NULL;
        }

        BYTE *pMem = pCentranMemPool->Malloc(sizeof(CSelfDataContainer<T>));
        if (NULL == pMem)
        {
            return NULL;
        }

        s_pInstance = new (pMem) CSelfDataContainer<T> (*pCentranMemPool);

        s_pInstance->Initialize(dwDataNum);

        return s_pInstance;
    }

    static VOID Destroy()
    {
        CCentralMemPool *pCentralMemPool = NULL;

        if (NULL != s_pInstance)
        {
            pCentralMemPool = &(s_pInstance->m_rCentralMemPool);
            delete s_pInstance;
            pCentralMemPool->Free((BYTE *)s_pInstance);
        }

        s_pInstance = NULL;
    }

    typedef struct tagT_DataHeader
    {
        WORD32            m_dwIndex;
        BOOL              m_bFree;
        tagT_DataHeader  *m_pNext;
        tagT_DataHeader  *m_pPrev;
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
    static_assert(offsetof(T_DataHeader, m_aucData) == 24, "unexpected layout");

    static const WORD32 s_dwNodeSize = ROUND_UP(sizeof(T_DataHeader), CACHE_SIZE);
    static const WORD32 s_dwDataOffset = offsetof(T_DataHeader, m_aucData);

protected :
    CSelfDataContainer (CCentralMemPool &rCentralMemPool);
    virtual ~CSelfDataContainer();

public :
    WORD32 Initialize(WORD32 dwDataNum);

    BYTE * Malloc(WORD32 dwSize);
    BYTE * Malloc(WORD32 dwSize, WORD32 dwIndex);

    WORD32 Free(BYTE *pMem);

    T * operator[] (WORD32 dwIndex);

    WORD32 GetIndex(T *pData);

protected :
    CSelfDataContainer (CSelfDataContainer &) = delete;
    CSelfDataContainer (const CSelfDataContainer &) = delete;
    CSelfDataContainer & operator= (CSelfDataContainer &) = delete;
    CSelfDataContainer & operator= (const CSelfDataContainer &) = delete;

protected :
    CCentralMemPool               &m_rCentralMemPool;

    /* 内存基址信息(初始化后保持不变) */
    WORD32                         m_dwDataNum;
    WORD32                         m_dwTotalSize;
    WORD64                         m_lwBegin;
    WORD64                         m_lwEnd;

    T_DataHeader                  *m_pFreeHeader;

protected :
    static CSelfDataContainer<T>  *s_pInstance;
};


template<typename T>
CSelfDataContainer<T>  * CSelfDataContainer<T>::s_pInstance = NULL;


template <typename T>
CSelfDataContainer<T>::CSelfDataContainer (CCentralMemPool &rCentralMemPool)
    : m_rCentralMemPool(rCentralMemPool)
{
    m_dwDataNum   = 0;
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
        m_rCentralMemPool.Free((BYTE *)m_lwBegin);
    }

    m_dwDataNum   = 0;
    m_dwTotalSize = 0;
    m_lwBegin     = 0;
    m_lwEnd       = 0;
    m_pFreeHeader = NULL;
}


template <typename T>
WORD32 CSelfDataContainer<T>::Initialize(WORD32 dwDataNum)
{
    m_dwDataNum   = dwDataNum + 1;
    m_dwTotalSize = m_dwDataNum * s_dwNodeSize;

    BYTE *pBuf = m_rCentralMemPool.Malloc(m_dwTotalSize);
    if (NULL == pBuf)
    {
        return FAIL;
    }

    WORD32        dwIndex   = m_dwDataNum;
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

        pCurNode->m_dwIndex = dwIndex;
        pCurNode->m_bFree   = TRUE;
        pCurNode->m_pNext   = pNextNode;
        pCurNode->m_pPrev   = NULL;

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
        pCurHead->m_bFree      = FALSE;
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
    if (unlikely(dwIndex >= m_dwDataNum))
    {
        return NULL;
    }

    T_DataHeader *pData = NULL;
    T_DataHeader *pPrev = NULL;
    T_DataHeader *pNext = NULL;

    pData = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * dwIndex));
    if (FALSE == pData->m_bFree)
    {
        return NULL;
    }

    pPrev = pData->m_pPrev;
    pNext = pData->m_pNext;

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

    pData->m_pPrev = NULL;
    pData->m_pNext = NULL;
    pData->m_bFree = FALSE;

    return pData->m_aucData;
}


template <typename T>
inline WORD32 CSelfDataContainer<T>::Free(BYTE *pMem)
{
    WORD64 lwAddr = (WORD64)pMem;

    if (unlikely(NULL == pMem))
    {
        return FAIL;
    }

    if (unlikely((lwAddr < (m_lwBegin + s_dwDataOffset)) || (lwAddr >= m_lwEnd)))
    {
        return FAIL;
    }

    if (0 != ((lwAddr - m_lwBegin - s_dwDataOffset) % s_dwNodeSize))
    {
        return FAIL;
    }

    T_DataHeader *pCur = (T_DataHeader *)(lwAddr - s_dwDataOffset);
    if ((pCur->m_dwIndex >= m_dwDataNum) || (pCur->m_bFree))
    {
        return FAIL;
    }

    T_DataHeader *pNode = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * (pCur->m_dwIndex)));
    if (unlikely(pCur != pNode))
    {
        return FAIL;
    }

    m_pFreeHeader->m_pPrev = pNode;
    pNode->m_bFree         = TRUE;
    pNode->m_pNext         = m_pFreeHeader;
    m_pFreeHeader          = pNode;

    return SUCCESS;
}


template <typename T>
inline T * CSelfDataContainer<T>::operator[] (WORD32 dwIndex)
{
    if (unlikely(dwIndex >= m_dwDataNum))
    {
        return NULL;
    }

    T_DataHeader *pData = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * dwIndex));
    if (pData->m_bFree)
    {
        return NULL;
    }

    return (T *)(pData->m_aucData);
}


template <typename T>
inline WORD32 CSelfDataContainer<T>::GetIndex(T *pData)
{
    WORD64 lwAddr = (WORD64)pData;

    if (unlikely(NULL == pData))
    {
        return INVALID_DWORD;
    }

    if (unlikely((lwAddr < (m_lwBegin + s_dwDataOffset)) || (lwAddr >= m_lwEnd)))
    {
        return INVALID_DWORD;
    }

    if (0 != ((lwAddr - m_lwBegin - s_dwDataOffset) % s_dwNodeSize))
    {
        return INVALID_DWORD;
    }

    T_DataHeader *pCur = (T_DataHeader *)(lwAddr - s_dwDataOffset);

    return pCur->m_dwIndex;
}


template <typename T>
class CBaseSelfData : public CCBObject
{
public :
    const static WORD32 s_dwDataSize  = sizeof(T);
    static WORD32 s_dwDataCount;
    static WORD64 s_lwDataID;

public :
    VOID * operator new (size_t size)
    {
        if (unlikely(size > s_dwDataSize))
        {
            return NULL;
        }

        CSelfDataContainer<T> *pContainer = CSelfDataContainer<T>::GetInstance();
        if (unlikely(NULL == pContainer))
        {
            return NULL;
        }

        BYTE *pMem = pContainer->Malloc(s_dwDataSize);
        if (NULL == pMem)
        {
            return NULL;
        }

        s_dwDataCount++;
        s_lwDataID++;

        return (VOID *)pMem;
    }

    VOID * operator new (size_t size, WORD32 dwIndex)
    {
        if (unlikely(size > s_dwDataSize))
        {
            return NULL;
        }

        CSelfDataContainer<T> *pContainer = CSelfDataContainer<T>::GetInstance();
        if (unlikely(NULL == pContainer))
        {
            return NULL;
        }

        BYTE *pMem = pContainer->Malloc(s_dwDataSize, dwIndex);
        if (NULL == pMem)
        {
            return NULL;
        }

        s_dwDataCount++;
        s_lwDataID++;

        return (VOID *)pMem;
    }

    VOID operator delete (VOID *ptr)
    {
        CSelfDataContainer<T> *pContainer = CSelfDataContainer<T>::GetInstance();
        if (unlikely(NULL == pContainer))
        {
            return ;
        }

        WORD32 dwResult = pContainer->Free((BYTE *)ptr);
        if (SUCCESS == dwResult)
        {
            s_dwDataCount--;
        }
    }

    CBaseSelfData ()
    {
        m_pSelf     = static_cast<T *>(this);
        m_lwDataID  = s_lwDataID;
        m_dwDataIdx = CSelfDataContainer<T>::GetInstance()->GetIndex(m_pSelf);
    }

    virtual ~CBaseSelfData()
    {
        m_pSelf     = NULL;
        m_lwDataID  = 0;
        m_dwDataIdx = 0;
    }

protected :
    CBaseSelfData (CBaseSelfData &) = delete;
    CBaseSelfData (const CBaseSelfData &) = delete;
    CBaseSelfData & operator= (CBaseSelfData &) = delete;
    CBaseSelfData & operator= (const CBaseSelfData &) = delete;

protected :
    T        *m_pSelf;
    WORD64    m_lwDataID;
    WORD32    m_dwDataIdx;
};


template<typename T>
WORD32 CBaseSelfData<T>::s_dwDataCount = 0;

template<typename T>
WORD64 CBaseSelfData<T>::s_lwDataID = INVALID_LWORD;


#endif


