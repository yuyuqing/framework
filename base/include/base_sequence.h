

#ifndef _BASE_SEQUENCE_H_
#define _BASE_SEQUENCE_H_


#include <assert.h>

#include "base_data_container.h"


template <class T, WORD32 NODE_NUM>
class CDoubleLink
{
public :
    typedef struct tagT_DataHeader
    {
        WORD32            m_dwIndex;
        BOOL              m_bFree;
        tagT_DataHeader  *m_pNext;
        tagT_DataHeader  *m_pPrev;
    }T_DataHeader;

    class CDataNode : public CBaseData
    {
    public :
        T_DataHeader  m_tHeader;
        T             m_tData;

        CDataNode ()
        {
            m_tHeader.m_dwIndex = INVALID_DWORD;
            m_tHeader.m_bFree   = TRUE;
            m_tHeader.m_pNext   = NULL;
            m_tHeader.m_pPrev   = NULL;
        }

        ~CDataNode ()
        {
            m_tHeader.m_dwIndex = INVALID_DWORD;
            m_tHeader.m_bFree   = TRUE;
            m_tHeader.m_pNext   = NULL;
            m_tHeader.m_pPrev   = NULL;
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

    static const WORD32 s_dwNodeSize = ROUND_UP(sizeof(CDataNode), CACHE_SIZE);
    static const WORD32 s_dwSize     = (s_dwNodeSize * NODE_NUM) + CACHE_SIZE;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"

    static const WORD32 s_dwHeadOffset = offsetof(CDataNode, m_tHeader);
    static const WORD32 s_dwDataOffset = offsetof(CDataNode, m_tData);

#pragma GCC diagnostic pop

public :
    CDoubleLink ();
    virtual ~CDoubleLink();

    WORD32 Initialize();

    T * Malloc(WORD32 &rdwIndex);

    VOID Free(T *ptr);
    VOID Free(WORD32 dwIndex);

    CDataNode & operator[] (WORD32 dwIndex);
    T * operator() (WORD32 dwIndex);

    BOOL IsFree(WORD32 dwIndex);
    BOOL IsValid(VOID *pAddr);

protected :
    VOID Free(CDataNode *pNode);

protected :
    WORD64         m_lwBegin;
    WORD64         m_lwEnd;

    T_DataHeader  *m_pFreeHeader;
    BYTE           m_aucData[s_dwSize];
};


template <class T, WORD32 NODE_NUM>
CDoubleLink<T, NODE_NUM>::CDoubleLink ()
{
    WORD64 lwAlign = CACHE_SIZE;
    WORD64 lwBegin = (WORD64)(&(m_aucData[0]));

    m_lwBegin     = ROUND_UP(lwBegin, lwAlign);
    m_lwEnd       = m_lwBegin + (s_dwNodeSize * NODE_NUM);
    m_pFreeHeader = NULL;
}


template <class T, WORD32 NODE_NUM>
CDoubleLink<T, NODE_NUM>::~CDoubleLink()
{
    m_pFreeHeader = NULL;
}


template <class T, WORD32 NODE_NUM>
WORD32 CDoubleLink<T, NODE_NUM>::Initialize()
{
    m_pFreeHeader = (T_DataHeader *)(m_lwBegin + s_dwHeadOffset);

    WORD32        dwIndex   = NODE_NUM;
    WORD64        lwAddr    = m_lwEnd;
    T_DataHeader *pNextNode = NULL;
    T_DataHeader *pCurNode  = NULL;

    while (lwAddr > m_lwBegin)
    {
        dwIndex--;
        lwAddr  -= s_dwNodeSize;
        pCurNode = (T_DataHeader *)(lwAddr + s_dwHeadOffset);

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


template <class T, WORD32 NODE_NUM>
inline T * CDoubleLink<T, NODE_NUM>::Malloc(WORD32 &rdwIndex)
{
    T_DataHeader *pCurHead = m_pFreeHeader;

    if (pCurHead)
    {
        m_pFreeHeader          = pCurHead->m_pNext;
        pCurHead->m_pNext      = NULL;
        pCurHead->m_bFree      = FALSE;
        m_pFreeHeader->m_pPrev = NULL;

        rdwIndex = pCurHead->m_dwIndex;

        return (T *)((WORD64)(pCurHead) - s_dwHeadOffset + s_dwDataOffset);
    }
    else
    {
        return NULL;
    }
}


template <class T, WORD32 NODE_NUM>
inline VOID CDoubleLink<T, NODE_NUM>::Free(T *ptr)
{
    if (!IsValid(ptr))
    {
        return ;
    }

    CDataNode *pCur = (CDataNode *)((WORD64)(ptr) - s_dwDataOffset);
    if ((pCur->m_tHeader.m_dwIndex >= NODE_NUM) || (pCur->m_tHeader.m_bFree))
    {
        return ;
    }

    CDataNode *pNode = (CDataNode *)(m_lwBegin + (s_dwNodeSize * (pCur->m_tHeader.m_dwIndex)));
    if (unlikely(pCur != pNode))
    {
        return ;
    }

    Free(pNode);
}


template <class T, WORD32 NODE_NUM>
inline VOID CDoubleLink<T, NODE_NUM>::Free(WORD32 dwIndex)
{
    CDataNode *pNode = &((*this)[dwIndex]);
    if (unlikely(NULL == pNode))
    {
        return ;
    }

    if (pNode->m_tHeader.m_bFree)
    {
        return ;
    }

    Free(pNode);
}


template <class T, WORD32 NODE_NUM>
inline typename CDoubleLink<T, NODE_NUM>::CDataNode & 
CDoubleLink<T, NODE_NUM>::operator[] (WORD32 dwIndex)
{
    if (unlikely(dwIndex >= NODE_NUM))
    {
        return *(CDataNode *)NULL;
    }

    return *((CDataNode *)(m_lwBegin + (s_dwNodeSize * dwIndex)));
}


template <class T, WORD32 NODE_NUM>
inline T * CDoubleLink<T, NODE_NUM>::operator() (WORD32 dwIndex)
{
    if (unlikely(dwIndex >= NODE_NUM))
    {
        return NULL;
    }

    CDataNode *pNode = ((CDataNode *)(m_lwBegin + (s_dwNodeSize * dwIndex)));
    if (pNode->m_tHeader.m_bFree)
    {
        return NULL;
    }

    return (T *)(*pNode);
}


template <class T, WORD32 NODE_NUM>
inline BOOL CDoubleLink<T, NODE_NUM>::IsFree(WORD32 dwIndex)
{
    if (unlikely(dwIndex >= NODE_NUM))
    {
        return FALSE;
    }

    CDataNode *pNode = ((CDataNode *)(m_lwBegin + (s_dwNodeSize * dwIndex)));

    return pNode->m_tHeader.m_bFree;
}


template <class T, WORD32 NODE_NUM>
inline BOOL CDoubleLink<T, NODE_NUM>::IsValid(VOID *pAddr)
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


template <class T, WORD32 NODE_NUM>
inline VOID CDoubleLink<T, NODE_NUM>::Free(CDataNode *pNode)
{
    if (unlikely(NULL == pNode))
    {
        return ;
    }

    m_pFreeHeader->m_pPrev   = &(pNode->m_tHeader);
    pNode->m_tHeader.m_bFree = TRUE;
    pNode->m_tHeader.m_pNext = m_pFreeHeader;
    m_pFreeHeader = &(pNode->m_tHeader);
}


template <class T, WORD32 NODE_NUM>
class CBaseList
{
public :
    typedef CDoubleLink<T, NODE_NUM>          CLinkList;
    typedef typename CLinkList::T_DataHeader  T_LinkHeader;

public :
    CBaseList ();
    virtual ~CBaseList();

    WORD32 Initialize();

    T * operator[] (WORD32 dwIndex);

    /* 添加到尾部 */
    T * Create();

    /* 添加到首部 */
    T * FrontCreate();

    WORD32 Remove(T *pInst);

    WORD32 GetCount();

protected :
    WORD32         m_dwCount;
    T_LinkHeader  *m_ptHeader;
    T_LinkHeader  *m_ptTailer;
    CLinkList      m_cList;
};


template <class T, WORD32 NODE_NUM>
CBaseList<T, NODE_NUM>::CBaseList ()
{
    m_dwCount  = 0;
    m_ptHeader = NULL;
    m_ptTailer = NULL;
}


template <class T, WORD32 NODE_NUM>
CBaseList<T, NODE_NUM>::~CBaseList()
{
    m_dwCount  = 0;
    m_ptHeader = NULL;
    m_ptTailer = NULL;
}


template <class T, WORD32 NODE_NUM>
WORD32 CBaseList<T, NODE_NUM>::Initialize()
{
    m_cList.Initialize();

    return SUCCESS;
}


template <class T, WORD32 NODE_NUM>
inline T * CBaseList<T, NODE_NUM>::operator[] (WORD32 dwIndex)
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

    return pData;
}


/* 添加到尾部 */
template <class T, WORD32 NODE_NUM>
inline T * CBaseList<T, NODE_NUM>::Create()
{
    WORD32 dwInstID = INVALID_DWORD;
    WORD64 lwAddr   = 0;

    T *pData = m_cList.Malloc(dwInstID);
    if (NULL == pData)
    {
        return NULL;
    }

    lwAddr = (WORD64)pData;

    T_LinkHeader *pCurHead = (T_LinkHeader *)(lwAddr - (CLinkList::s_dwDataOffset) + (CLinkList::s_dwHeadOffset));
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
template <class T, WORD32 NODE_NUM>
inline T * CBaseList<T, NODE_NUM>::FrontCreate()
{
    WORD32 dwInstID = INVALID_DWORD;
    WORD64 lwAddr   = 0;

    T *pData = m_cList.Malloc(dwInstID);
    if (NULL == pData)
    {
        return NULL;
    }

    lwAddr = (WORD64)pData;

    T_LinkHeader *pCurHead = (T_LinkHeader *)(lwAddr - (CLinkList::s_dwDataOffset) + (CLinkList::s_dwHeadOffset));
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


template <class T, WORD32 NODE_NUM>
inline WORD32 CBaseList<T, NODE_NUM>::Remove(T *pInst)
{
    WORD32        dwInstID = INVALID_DWORD;
    T            *pData    = NULL;
    T_LinkHeader *pCurHead = m_ptHeader;
    T_LinkHeader *pPrev    = NULL;
    T_LinkHeader *pNext    = NULL;

    while (pCurHead)
    {
        dwInstID = pCurHead->m_dwIndex;
        pData    = m_cList(dwInstID);
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

            m_cList.Free(dwInstID);

            return SUCCESS;
        }

        pCurHead = pCurHead->m_pNext;
    }

    return FAIL;
}


template <class T, WORD32 NODE_NUM>
inline WORD32 CBaseList<T, NODE_NUM>::GetCount()
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

    typedef CDoubleLink<CSequenceData, NODE_NUM>  CSequenceList;
    typedef typename CSequenceList::T_DataHeader  T_SequenceHeader;

public :
    CBaseSequence ();
    virtual ~CBaseSequence();

    WORD32 Initialize();

    T * Find(const K &rKey);
    T * operator[] (WORD32 dwIndex);

    /* 添加到尾部 */
    T * Create(const K &rKey);

    /* 添加到首部 */
    T * FrontCreate(const K &rKey);

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


/* 添加到尾部 */
template <typename K, class T, WORD32 NODE_NUM>
inline T * CBaseSequence<K, T, NODE_NUM>::Create(const K &rKey)
{
    WORD32 dwInstID = INVALID_DWORD;
    WORD64 lwAddr   = 0;

    CSequenceData *pData = m_cList.Malloc(dwInstID);
    if (NULL == pData)
    {
        return NULL;
    }

    lwAddr = (WORD64)pData;

    T_SequenceHeader *pCurHead = (T_SequenceHeader *)(lwAddr - (CSequenceList::s_dwDataOffset) + (CSequenceList::s_dwHeadOffset));
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


/* 添加到首部 */
template <typename K, class T, WORD32 NODE_NUM>
inline T * CBaseSequence<K, T, NODE_NUM>::FrontCreate(const K &rKey)
{
    WORD32 dwInstID = INVALID_DWORD;
    WORD64 lwAddr   = 0;

    CSequenceData *pData = m_cList.Malloc(dwInstID);
    if (NULL == pData)
    {
        return NULL;
    }

    lwAddr = (WORD64)pData;

    T_SequenceHeader *pCurHead = (T_SequenceHeader *)(lwAddr - (CSequenceList::s_dwDataOffset) + (CSequenceList::s_dwHeadOffset));
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


