

#ifndef _BASE_LIST_H_
#define _BASE_LIST_H_


#include <assert.h>

#include "base_data_container.h"


/* ALIGN_FLAG : 按CACHE_SIZE对齐标志 */
template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
class CDoubleLinkTpl
{
public :
    CDoubleLinkTpl() {}
    virtual ~CDoubleLinkTpl() = 0;
};


template <class T, WORD32 NODE_NUM>
class CDoubleLinkTpl<T, NODE_NUM, TRUE>
{
public :
    enum { HEAD_PAD_SIZE = 40 };

    typedef struct tagT_DataHeader
    {
        WORD32            m_dwIndex;
        BOOL              m_bFree;
        tagT_DataHeader  *m_pNext;
        tagT_DataHeader  *m_pPrev;
        BYTE              m_aucPad[HEAD_PAD_SIZE];
        BYTE              m_aucData[sizeof(T)];
    }T_DataHeader;
    static_assert(offsetof(T_DataHeader, m_aucData) == CACHE_SIZE, "unexpected layout");
};


template <class T, WORD32 NODE_NUM>
class CDoubleLinkTpl<T, NODE_NUM, FALSE>
{
public :
    enum { HEAD_PAD_SIZE = 40 };

    typedef struct tagT_DataHeader
    {
        WORD32            m_dwIndex;
        BOOL              m_bFree;
        tagT_DataHeader  *m_pNext;
        tagT_DataHeader  *m_pPrev;
        BYTE              m_aucData[sizeof(T)];
    }T_DataHeader;
    static_assert(offsetof(T_DataHeader, m_aucData) == 24, "unexpected layout");
};


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
class CDoubleLink : public CDoubleLinkTpl<T, NODE_NUM, ALIGN_FLAG>
{
public :
    typedef typename CDoubleLinkTpl<T, NODE_NUM, ALIGN_FLAG>::T_DataHeader  T_DataHeader;

    static const WORD32 s_dwNodeSize = ROUND_UP(sizeof(T_DataHeader), CACHE_SIZE);
    static const WORD32 s_dwSize     = (s_dwNodeSize * NODE_NUM) + CACHE_SIZE;

    static const WORD32 s_dwDataOffset = offsetof(T_DataHeader, m_aucData);

public :
    CDoubleLink ();
    virtual ~CDoubleLink();

    virtual WORD32 Initialize();

    T * Malloc(WORD32 &rdwIndex);

    VOID Free(T *ptr);
    VOID Free(WORD32 dwIndex);

    T * operator() (WORD32 dwIndex);

    BOOL IsFree(WORD32 dwIndex);
    BOOL IsValid(VOID *pAddr);

protected :
    VOID Free(T_DataHeader *pData);

protected :
    WORD64         m_lwBegin;
    WORD64         m_lwEnd;

    T_DataHeader  *m_pFreeHeader;
    BYTE           m_aucData[s_dwSize];
};


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
CDoubleLink<T, NODE_NUM, ALIGN_FLAG>::CDoubleLink ()
{
    WORD64 lwAlign = CACHE_SIZE;
    WORD64 lwBegin = (WORD64)(&(m_aucData[0]));

    m_lwBegin     = ROUND_UP(lwBegin, lwAlign);
    m_lwEnd       = m_lwBegin + (s_dwNodeSize * NODE_NUM);
    m_pFreeHeader = NULL;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
CDoubleLink<T, NODE_NUM, ALIGN_FLAG>::~CDoubleLink()
{
    m_pFreeHeader = NULL;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
WORD32 CDoubleLink<T, NODE_NUM, ALIGN_FLAG>::Initialize()
{
    m_pFreeHeader = (T_DataHeader *)(m_lwBegin);

    WORD32        dwIndex   = NODE_NUM;
    WORD64        lwAddr    = m_lwEnd;
    T_DataHeader *pNextNode = NULL;
    T_DataHeader *pCurNode  = NULL;

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


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CDoubleLink<T, NODE_NUM, ALIGN_FLAG>::Malloc(WORD32 &rdwIndex)
{
    T_DataHeader *pCurHead = m_pFreeHeader;

    if (pCurHead)
    {
        m_pFreeHeader          = pCurHead->m_pNext;
        pCurHead->m_pNext      = NULL;
        pCurHead->m_bFree      = FALSE;
        m_pFreeHeader->m_pPrev = NULL;

        rdwIndex = pCurHead->m_dwIndex;

        return (T *)(pCurHead->m_aucData);
    }
    else
    {
        return NULL;
    }
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline VOID CDoubleLink<T, NODE_NUM, ALIGN_FLAG>::Free(T *ptr)
{
    if (!IsValid(ptr))
    {
        return ;
    }

    T_DataHeader *pCur = (T_DataHeader *)((WORD64)(ptr) - s_dwDataOffset);
    if ((pCur->m_dwIndex >= NODE_NUM) || (pCur->m_bFree))
    {
        return ;
    }

    T_DataHeader *pData = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * (pCur->m_dwIndex)));
    if (unlikely(pCur != pData))
    {
        return ;
    }

    Free(pData);
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline VOID CDoubleLink<T, NODE_NUM, ALIGN_FLAG>::Free(WORD32 dwIndex)
{
    if (unlikely(dwIndex >= NODE_NUM))
    {
        return ;
    }

    T_DataHeader *pData = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * dwIndex));
    if (pData->m_bFree)
    {
        return ;
    }

    Free(pData);
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CDoubleLink<T, NODE_NUM, ALIGN_FLAG>::operator() (WORD32 dwIndex)
{
    if (unlikely(dwIndex >= NODE_NUM))
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


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline BOOL CDoubleLink<T, NODE_NUM, ALIGN_FLAG>::IsFree(WORD32 dwIndex)
{
    if (unlikely(dwIndex >= NODE_NUM))
    {
        return FALSE;
    }

    T_DataHeader *pData = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * dwIndex));

    return pData->m_bFree;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline BOOL CDoubleLink<T, NODE_NUM, ALIGN_FLAG>::IsValid(VOID *pAddr)
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


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline VOID CDoubleLink<T, NODE_NUM, ALIGN_FLAG>::Free(T_DataHeader *pData)
{
    if (unlikely(NULL == pData))
    {
        return ;
    }

    m_pFreeHeader->m_pPrev = pData;
    pData->m_bFree         = TRUE;
    pData->m_pNext         = m_pFreeHeader;
    m_pFreeHeader          = pData;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG = FALSE>
class CBaseList : public CDoubleLink<T, NODE_NUM, ALIGN_FLAG>
{
public :
    typedef typename CDoubleLink<T, NODE_NUM, ALIGN_FLAG>::T_DataHeader  T_LinkHeader;

public :
    CBaseList ();
    virtual ~CBaseList();

    WORD32 Initialize();

    T * operator[] (WORD32 dwIndex);

    /* 创建节点并添加到尾部 */
    T * CreateTail();

    /* 创建节点并添加到首部 */
    T * CreateHead();

    /* 将节点添加到尾部 */
    WORD32 InsertTail(T *pInst);

    /* 将节点添加到首部 */
    WORD32 InsertHead(T *pInst);

    /* 从链表中移除节点并释放节点 */
    WORD32 Remove(T *pInst);

    WORD32 GetCount();

protected :
    WORD32         m_dwCount;
    T_LinkHeader  *m_ptHeader;
    T_LinkHeader  *m_ptTailer;
};


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
CBaseList<T, NODE_NUM, ALIGN_FLAG>::CBaseList ()
{
    m_dwCount  = 0;
    m_ptHeader = NULL;
    m_ptTailer = NULL;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
CBaseList<T, NODE_NUM, ALIGN_FLAG>::~CBaseList()
{
    m_dwCount  = 0;
    m_ptHeader = NULL;
    m_ptTailer = NULL;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
WORD32 CBaseList<T, NODE_NUM, ALIGN_FLAG>::Initialize()
{
    CDoubleLink<T, NODE_NUM, ALIGN_FLAG>::Initialize();

    return SUCCESS;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CBaseList<T, NODE_NUM, ALIGN_FLAG>::operator[] (WORD32 dwIndex)
{
    if (unlikely(dwIndex >= m_dwCount))
    {
        return NULL;
    }

    WORD32        dwTmpIdx = 0;
    T            *pData    = NULL;
    T_LinkHeader *pCurHead = m_ptHeader;

    while (pCurHead)
    {
        if (dwTmpIdx == dwIndex)
        {
            pData = (*this)(pCurHead->m_dwIndex);
            break ;
        }

        dwTmpIdx++;
        pCurHead = pCurHead->m_pNext;
    }

    if (unlikely(NULL == pData))
    {
        assert(0);
    }

    return pData;
}


/* 创建节点并添加到尾部 */
template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CBaseList<T, NODE_NUM, ALIGN_FLAG>::CreateTail()
{
    WORD32 dwInstID = INVALID_DWORD;
    WORD64 lwAddr   = 0;

    T *pData = this->Malloc(dwInstID);
    if (NULL == pData)
    {
        return NULL;
    }

    lwAddr = (WORD64)pData;

    T_LinkHeader *pCurHead = (T_LinkHeader *)(lwAddr - this->s_dwDataOffset);
    if (NULL == m_ptTailer)
    {
        m_ptHeader = pCurHead;
        m_ptTailer = pCurHead;
    }
    else
    {
        pCurHead->m_pPrev   = m_ptTailer;
        m_ptTailer->m_pNext = pCurHead;
        m_ptTailer          = pCurHead;
    }

    m_dwCount++;

    return pData;
}


/* 添加到首部 */
template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CBaseList<T, NODE_NUM, ALIGN_FLAG>::CreateHead()
{
    WORD32 dwInstID = INVALID_DWORD;
    WORD64 lwAddr   = 0;

    T *pData = this->Malloc(dwInstID);
    if (NULL == pData)
    {
        return NULL;
    }

    lwAddr = (WORD64)pData;

    T_LinkHeader *pCurHead = (T_LinkHeader *)(lwAddr - this->s_dwDataOffset);
    if (NULL == m_ptHeader)
    {
        m_ptHeader = pCurHead;
        m_ptTailer = pCurHead;
    }
    else
    {
        pCurHead->m_pNext   = m_ptHeader;
        m_ptHeader->m_pPrev = pCurHead;
        m_ptHeader          = pCurHead;
    }

    m_dwCount++;

    return pData;
}


/* 将节点添加到尾部 */
template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline WORD32 CBaseList<T, NODE_NUM, ALIGN_FLAG>::InsertTail(T *pInst)
{
    if (FALSE == this->IsValid(pInst))
    {
        return FAIL;
    }

    WORD64 lwAddr = (WORD64)pInst;

    T_LinkHeader *pCurHead = (T_LinkHeader *)(lwAddr - this->s_dwDataOffset);
    if (NULL == m_ptTailer)
    {
        m_ptHeader = pCurHead;
        m_ptTailer = pCurHead;
    }
    else
    {
        pCurHead->m_pPrev   = m_ptTailer;
        m_ptTailer->m_pNext = pCurHead;
        m_ptTailer          = pCurHead;
    }

    m_dwCount++;

    return SUCCESS;
}


/* 将节点添加到首部 */
template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline WORD32 CBaseList<T, NODE_NUM, ALIGN_FLAG>::InsertHead(T *pInst)
{
    if (FALSE == this->IsValid(pInst))
    {
        return FAIL;
    }

    WORD64 lwAddr = (WORD64)pInst;

    T_LinkHeader *pCurHead = (T_LinkHeader *)(lwAddr - this->s_dwDataOffset);
    if (NULL == m_ptHeader)
    {
        m_ptHeader = pCurHead;
        m_ptTailer = pCurHead;
    }
    else
    {
        pCurHead->m_pNext   = m_ptHeader;
        m_ptHeader->m_pPrev = pCurHead;
        m_ptHeader          = pCurHead;
    }

    m_dwCount++;

    return SUCCESS;
}


/* 从链表中移除节点并释放节点 */
template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline WORD32 CBaseList<T, NODE_NUM, ALIGN_FLAG>::Remove(T *pInst)
{
    WORD32        dwInstID = INVALID_DWORD;
    T            *pData    = NULL;
    T_LinkHeader *pCurHead = m_ptHeader;
    T_LinkHeader *pPrev    = NULL;
    T_LinkHeader *pNext    = NULL;

    while (pCurHead)
    {
        dwInstID = pCurHead->m_dwIndex;
        pData    = (*this)(dwInstID);
        if (unlikely(NULL == pData))
        {
            assert(0);
        }

        if (pInst == pData)
        {
            pPrev = pCurHead->m_pPrev;
            pNext = pCurHead->m_pNext;

            if (NULL == pPrev)
            {
                /* 删除头节点 */
                m_ptHeader = pNext;
            }
            else
            {
                pPrev->m_pNext = pNext;
            }

            if (NULL == pNext)
            {
                /* 删除尾节点 */
                m_ptTailer = pPrev;
            }
            else
            {
                pNext->m_pPrev = pPrev;
            }

            pCurHead->m_pPrev = NULL;
            pCurHead->m_pNext = NULL;

            m_dwCount--;

            this->Free(dwInstID);

            return SUCCESS;
        }

        pCurHead = pCurHead->m_pNext;
    }

    return FAIL;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline WORD32 CBaseList<T, NODE_NUM, ALIGN_FLAG>::GetCount()
{
    return m_dwCount;
}


template <typename K, class T, WORD32 NODE_NUM>
class CBaseSequence
{
public :
    class CSequenceData : public CBaseData
    {
    public :
        K  m_tKey;
        T  m_tData;

        CSequenceData(const K &rKey)
            : m_tKey(rKey)
        {
        }

        ~CSequenceData() {}

        BOOL operator== (const K &rKey)
        {
            return (rKey == m_tKey);
        }

        operator T& ()
        {
            return m_tData;
        }

        operator T* ()
        {
            return &m_tData;
        }
    };

    typedef CDoubleLink<CSequenceData, NODE_NUM, FALSE>  CSequenceList;
    typedef typename CSequenceList::T_DataHeader         T_SequenceHeader;

    static const WORD32 s_dwValueOffset = offsetof(CSequenceData, m_tData);

public :
    CBaseSequence ();
    virtual ~CBaseSequence();

    WORD32 Initialize();

    T * Find(const K &rKey);
    T * operator[] (WORD32 dwIndex);

    /* 创建节点并添加到尾部 */
    T * CreateTail(const K &rKey);

    /* 创建节点并添加到首部 */
    T * CreateHead(const K &rKey);

    /* 仅创建节点 */
    T * Create(const K &rKey);

    /* 将节点添加到尾部 */
    WORD32 InsertTail(T *pInst);

    /* 将节点添加到首部 */
    WORD32 InsertHead(T *pInst);

    /* 从链表中移除节点并释放节点 */
    WORD32 Remove(const K &rKey);

    WORD32 GetCount();

protected :
    WORD32             m_dwCount;
    T_SequenceHeader  *m_ptHeader;
    T_SequenceHeader  *m_ptTailer;
    CSequenceList      m_cList;
};


template <typename K, class T, WORD32 NODE_NUM>
CBaseSequence<K, T, NODE_NUM>::CBaseSequence ()
{
    m_dwCount  = 0;
    m_ptHeader = NULL;
    m_ptTailer = NULL;
}


template <typename K, class T, WORD32 NODE_NUM>
CBaseSequence<K, T, NODE_NUM>::~CBaseSequence()
{
    m_dwCount  = 0;
    m_ptHeader = NULL;
    m_ptTailer = NULL;
}


template <typename K, class T, WORD32 NODE_NUM>
WORD32 CBaseSequence<K, T, NODE_NUM>::Initialize()
{
    m_cList.Initialize();

    return SUCCESS;
}


template <typename K, class T, WORD32 NODE_NUM>
inline T * CBaseSequence<K, T, NODE_NUM>::Find(const K &rKey)
{
    CSequenceData    *pData    = NULL;
    T_SequenceHeader *pCurHead = m_ptHeader;

    while (pCurHead)
    {
        pData = m_cList(pCurHead->m_dwIndex);
        if (unlikely(NULL == pData))
        {
            assert(0);
        }

        if (rKey == pData->m_tKey)
        {
            return (*pData);
        }

        pCurHead = pCurHead->m_pNext;
    }

    return NULL;
}


template <typename K, class T, WORD32 NODE_NUM>
inline T * CBaseSequence<K, T, NODE_NUM>::operator[] (WORD32 dwIndex)
{
    if (unlikely(dwIndex >= m_dwCount))
    {
        return NULL;
    }

    WORD32            dwTmpIdx = 0;
    CSequenceData    *pData    = NULL;
    T_SequenceHeader *pCurHead = m_ptHeader;

    while (pCurHead)
    {
        if (dwTmpIdx == dwIndex)
        {
            pData = m_cList(pCurHead->m_dwIndex);
            break ;
        }

        dwTmpIdx++;
        pCurHead = pCurHead->m_pNext;
    }

    if (unlikely(NULL == pData))
    {
        assert(0);
    }

    return *pData;
}


/* 创建节点并添加到尾部 */
template <typename K, class T, WORD32 NODE_NUM>
inline T * CBaseSequence<K, T, NODE_NUM>::CreateTail(const K &rKey)
{
    WORD32 dwInstID = INVALID_DWORD;
    WORD64 lwAddr   = 0;

    CSequenceData *pData = m_cList.Malloc(dwInstID);
    if (NULL == pData)
    {
        return NULL;
    }

    lwAddr = (WORD64)pData;

    T_SequenceHeader *pCurHead = (T_SequenceHeader *)(lwAddr - (CSequenceList::s_dwDataOffset));
    if (NULL == m_ptTailer)
    {
        m_ptHeader = pCurHead;
        m_ptTailer = pCurHead;
    }
    else
    {
        pCurHead->m_pPrev   = m_ptTailer;
        m_ptTailer->m_pNext = pCurHead;
        m_ptTailer          = pCurHead;
    }

    new (pData) CSequenceData(rKey);

    m_dwCount++;

    return *pData;
}


/* 创建节点并添加到首部 */
template <typename K, class T, WORD32 NODE_NUM>
inline T * CBaseSequence<K, T, NODE_NUM>::CreateHead(const K &rKey)
{
    WORD32 dwInstID = INVALID_DWORD;
    WORD64 lwAddr   = 0;

    CSequenceData *pData = m_cList.Malloc(dwInstID);
    if (NULL == pData)
    {
        return NULL;
    }

    lwAddr = (WORD64)pData;

    T_SequenceHeader *pCurHead = (T_SequenceHeader *)(lwAddr - (CSequenceList::s_dwDataOffset));
    if (NULL == m_ptHeader)
    {
        m_ptHeader = pCurHead;
        m_ptTailer = pCurHead;
    }
    else
    {
        pCurHead->m_pNext   = m_ptHeader;
        m_ptHeader->m_pPrev = pCurHead;
        m_ptHeader          = pCurHead;
    }

    new (pData) CSequenceData(rKey);

    m_dwCount++;

    return *pData;
}


/* 仅创建节点 */
template <typename K, class T, WORD32 NODE_NUM>
inline T * CBaseSequence<K, T, NODE_NUM>::Create(const K &rKey)
{
    WORD32 dwInstID = INVALID_DWORD;

    CSequenceData *pData = m_cList.Malloc(dwInstID);
    if (NULL == pData)
    {
        return NULL;
    }

    new (pData) CSequenceData(rKey);

    return *pData;
}


/* 将节点添加到尾部 */
template <typename K, class T, WORD32 NODE_NUM>
inline WORD32 CBaseSequence<K, T, NODE_NUM>::InsertTail(T *pInst)
{
    WORD64 lwAddr = (WORD64)(pInst) - s_dwValueOffset;

    if (FALSE == m_cList.IsValid((VOID *)lwAddr))
    {
        return FAIL;
    }

    T_SequenceHeader *pCurHead = (T_SequenceHeader *)(lwAddr - (CSequenceList::s_dwDataOffset));
    if (NULL == m_ptTailer)
    {
        m_ptHeader = pCurHead;
        m_ptTailer = pCurHead;
    }
    else
    {
        pCurHead->m_pPrev   = m_ptTailer;
        m_ptTailer->m_pNext = pCurHead;
        m_ptTailer          = pCurHead;
    }

    m_dwCount++;

    return SUCCESS;
}


/* 将节点添加到首部 */
template <typename K, class T, WORD32 NODE_NUM>
inline WORD32 CBaseSequence<K, T, NODE_NUM>::InsertHead(T *pInst)
{
    WORD64 lwAddr = (WORD64)(pInst) - s_dwValueOffset;

    if (FALSE == m_cList.IsValid((VOID *)lwAddr))
    {
        return FAIL;
    }

    T_SequenceHeader *pCurHead = (T_SequenceHeader *)(lwAddr - (CSequenceList::s_dwDataOffset));
    if (NULL == m_ptHeader)
    {
        m_ptHeader = pCurHead;
        m_ptTailer = pCurHead;
    }
    else
    {
        pCurHead->m_pNext   = m_ptHeader;
        m_ptHeader->m_pPrev = pCurHead;
        m_ptHeader          = pCurHead;
    }

    m_dwCount++;

    return SUCCESS;
}


/* 从链表中移除节点并释放节点 */
template <typename K, class T, WORD32 NODE_NUM>
inline WORD32 CBaseSequence<K, T, NODE_NUM>::Remove(const K &rKey)
{
    WORD32            dwInstID = INVALID_DWORD;
    CSequenceData    *pData    = NULL;
    T_SequenceHeader *pCurHead = m_ptHeader;
    T_SequenceHeader *pPrev    = NULL;
    T_SequenceHeader *pNext    = NULL;

    while (pCurHead)
    {
        dwInstID = pCurHead->m_dwIndex;
        pData    = m_cList(dwInstID);
        if (unlikely(NULL == pData))
        {
            assert(0);
        }

        if (rKey == pData->m_tKey)
        {
            pPrev = pCurHead->m_pPrev;
            pNext = pCurHead->m_pNext;

            if (NULL == pPrev)
            {
                /* 删除头节点 */
                m_ptHeader = pNext;
            }
            else
            {
                pPrev->m_pNext = pNext;
            }

            if (NULL == pNext)
            {
                /* 删除尾节点 */
                m_ptTailer = pPrev;
            }
            else
            {
                pNext->m_pPrev = pPrev;
            }

            pCurHead->m_pPrev = NULL;
            pCurHead->m_pNext = NULL;

            delete pData;
            m_dwCount--;

            m_cList.Free(dwInstID);

            return SUCCESS;
        }

        pCurHead = pCurHead->m_pNext;
    }

    return FAIL;
}


template <typename K, class T, WORD32 NODE_NUM>
inline WORD32 CBaseSequence<K, T, NODE_NUM>::GetCount()
{
    return m_dwCount;
}


#endif


