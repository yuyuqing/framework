

#ifndef _BASE_LIST_H_
#define _BASE_LIST_H_


#include <assert.h>

#include "base_data_container.h"


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG = FALSE>
class CBaseList : public CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>
{
public :
    typedef typename CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::T_DataHeader  T_LinkHeader;

public :
    CBaseList ();
    virtual ~CBaseList();

    virtual WORD32 Initialize();

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

    /* 从链表中移除节点并释放节点(根据Key找到待移除节点) */
    template <typename K>
    WORD32 Remove(const K &rKey);

    T * GetHead();
    T * GetTail();
    T * Next(T *pData);
    T * Prev(T *pData);

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
    CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::Initialize();

    return SUCCESS;
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
        pData    = (*pCurHead);

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


/* 从链表中移除节点并释放节点(根据Key找到待移除节点) */
template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
template <typename K>
inline WORD32 CBaseList<T, NODE_NUM, ALIGN_FLAG>::Remove(const K &rKey)
{
    WORD32        dwInstID = INVALID_DWORD;
    T            *pData    = NULL;
    T_LinkHeader *pCurHead = m_ptHeader;
    T_LinkHeader *pPrev    = NULL;
    T_LinkHeader *pNext    = NULL;

    while (pCurHead)
    {
        dwInstID = pCurHead->m_dwIndex;
        pData    = (*pCurHead);

        if ((*pData) == rKey)
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
inline T * CBaseList<T, NODE_NUM, ALIGN_FLAG>::GetHead()
{
    if (NULL == m_ptHeader)
    {
        return NULL;
    }

    return *m_ptHeader;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CBaseList<T, NODE_NUM, ALIGN_FLAG>::GetTail()
{
    if (NULL == m_ptTailer)
    {
        return NULL;
    }

    return *m_ptTailer;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CBaseList<T, NODE_NUM, ALIGN_FLAG>::Next(T *pData)
{
    if (FALSE == this->IsValid(pData))
    {
        return NULL;
    }

    WORD64 lwAddr = (WORD64)pData;

    T_LinkHeader *pCurNode = (T_LinkHeader *)(lwAddr - this->s_dwDataOffset);
    if (pCurNode->m_bFree)
    {
        return NULL;
    }

    T_LinkHeader *pNextNode = pCurNode->m_pNext;

    if (NULL == pNextNode)
    {
        return NULL;
    }

    return *pNextNode;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CBaseList<T, NODE_NUM, ALIGN_FLAG>::Prev(T *pData)
{
    if (FALSE == this->IsValid(pData))
    {
        return NULL;
    }

    WORD64 lwAddr = (WORD64)pData;

    T_LinkHeader *pCurNode = (T_LinkHeader *)(lwAddr - this->s_dwDataOffset);
    if (pCurNode->m_bFree)
    {
        return NULL;
    }

    T_LinkHeader *pPrevNode = pCurNode->m_pPrev;

    if (NULL == pPrevNode)
    {
        return NULL;
    }

    return *pPrevNode;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline WORD32 CBaseList<T, NODE_NUM, ALIGN_FLAG>::GetCount()
{
    return m_dwCount;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
class CShareList
{
public :
    using CContainer = CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>;
    typedef typename CContainer::T_DataHeader   T_LinkHeader;

public :
    CShareList (CContainer &rContainer)
        : m_rContainer(rContainer)
    {
        m_dwCount  = 0;
        m_ptHeader = NULL;
        m_ptTailer = NULL;
    }

    virtual ~CShareList();

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

    /* 从链表中移除节点并释放节点(根据Key找到待移除节点) */
    template <typename K>
    WORD32 Remove(const K &rKey);

    T * GetHead();
    T * GetTail();
    T * Next(T *pData);
    T * Prev(T *pData);

    WORD32 GetCount();

protected :
    CContainer    &m_rContainer;
    WORD32         m_dwCount;
    T_LinkHeader  *m_ptHeader;
    T_LinkHeader  *m_ptTailer;
};


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
CShareList<T, NODE_NUM, ALIGN_FLAG>::~CShareList()
{
    m_dwCount  = 0;
    m_ptHeader = NULL;
    m_ptTailer = NULL;
}


/* 创建节点并添加到尾部 */
template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CShareList<T, NODE_NUM, ALIGN_FLAG>::CreateTail()
{
    WORD32 dwInstID = INVALID_DWORD;
    WORD64 lwAddr   = 0;

    T *pData = m_rContainer.Malloc(dwInstID);
    if (NULL == pData)
    {
        return NULL;
    }

    lwAddr = (WORD64)pData;

    T_LinkHeader *pCurHead = (T_LinkHeader *)(lwAddr - m_rContainer.s_dwDataOffset);
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


/* 创建节点并添加到首部 */
template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CShareList<T, NODE_NUM, ALIGN_FLAG>::CreateHead()
{
    WORD32 dwInstID = INVALID_DWORD;
    WORD64 lwAddr   = 0;

    T *pData = m_rContainer.Malloc(dwInstID);
    if (NULL == pData)
    {
        return NULL;
    }

    lwAddr = (WORD64)pData;

    T_LinkHeader *pCurHead = (T_LinkHeader *)(lwAddr - m_rContainer.s_dwDataOffset);
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
inline WORD32 CShareList<T, NODE_NUM, ALIGN_FLAG>::InsertTail(T *pInst)
{
    if (FALSE == m_rContainer.IsValid(pInst))
    {
        return FAIL;
    }

    WORD64 lwAddr = (WORD64)pInst;

    T_LinkHeader *pCurHead = (T_LinkHeader *)(lwAddr - m_rContainer.s_dwDataOffset);
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
inline WORD32 CShareList<T, NODE_NUM, ALIGN_FLAG>::InsertHead(T *pInst)
{
    if (FALSE == m_rContainer.IsValid(pInst))
    {
        return FAIL;
    }

    WORD64 lwAddr = (WORD64)pInst;

    T_LinkHeader *pCurHead = (T_LinkHeader *)(lwAddr - m_rContainer.s_dwDataOffset);
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
inline WORD32 CShareList<T, NODE_NUM, ALIGN_FLAG>::Remove(T *pInst)
{
    WORD32        dwInstID = INVALID_DWORD;
    T            *pData    = NULL;
    T_LinkHeader *pCurHead = m_ptHeader;
    T_LinkHeader *pPrev    = NULL;
    T_LinkHeader *pNext    = NULL;

    while (pCurHead)
    {
        dwInstID = pCurHead->m_dwIndex;
        pData    = (*pCurHead);

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

            m_rContainer.Free(dwInstID);

            return SUCCESS;
        }

        pCurHead = pCurHead->m_pNext;
    }

    return FAIL;
}


/* 从链表中移除节点并释放节点(根据Key找到待移除节点) */
template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
template <typename K>
inline WORD32 CShareList<T, NODE_NUM, ALIGN_FLAG>::Remove(const K &rKey)
{
    WORD32        dwInstID = INVALID_DWORD;
    T            *pData    = NULL;
    T_LinkHeader *pCurHead = m_ptHeader;
    T_LinkHeader *pPrev    = NULL;
    T_LinkHeader *pNext    = NULL;

    while (pCurHead)
    {
        dwInstID = pCurHead->m_dwIndex;
        pData    = (*pCurHead);

        if ((*pData) == rKey)
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

            m_rContainer.Free(dwInstID);

            return SUCCESS;
        }

        pCurHead = pCurHead->m_pNext;
    }

    return FAIL;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CShareList<T, NODE_NUM, ALIGN_FLAG>::GetHead()
{
    if (NULL == m_ptHeader)
    {
        return NULL;
    }

    return *m_ptHeader;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CShareList<T, NODE_NUM, ALIGN_FLAG>::GetTail()
{
    if (NULL == m_ptTailer)
    {
        return NULL;
    }

    return *m_ptTailer;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CShareList<T, NODE_NUM, ALIGN_FLAG>::Next(T *pData)
{
    if (FALSE == m_rContainer.IsValid(pData))
    {
        return NULL;
    }

    WORD64 lwAddr = (WORD64)pData;

    T_LinkHeader *pCurNode = (T_LinkHeader *)(lwAddr - m_rContainer.s_dwDataOffset);
    if (pCurNode->m_bFree)
    {
        return NULL;
    }

    T_LinkHeader *pNextNode = pCurNode->m_pNext;

    if (NULL == pNextNode)
    {
        return NULL;
    }

    return *pNextNode;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CShareList<T, NODE_NUM, ALIGN_FLAG>::Prev(T *pData)
{
    if (FALSE == m_rContainer.IsValid(pData))
    {
        return NULL;
    }

    WORD64 lwAddr = (WORD64)pData;

    T_LinkHeader *pCurNode = (T_LinkHeader *)(lwAddr - m_rContainer.s_dwDataOffset);
    if (pCurNode->m_bFree)
    {
        return NULL;
    }

    T_LinkHeader *pPrevNode = pCurNode->m_pPrev;

    if (NULL == pPrevNode)
    {
        return NULL;
    }

    return *pPrevNode;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline WORD32 CShareList<T, NODE_NUM, ALIGN_FLAG>::GetCount()
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

    typedef CBaseDataContainer<CSequenceData, NODE_NUM, FALSE>  CSequenceList;
    typedef typename CSequenceList::T_DataHeader                T_SequenceHeader;

    static const WORD32 s_dwValueOffset = offsetof(CSequenceData, m_tData);

public :
    CBaseSequence ();
    virtual ~CBaseSequence();

    WORD32 Initialize();

    T * Find(const K &rKey);

    /* 创建节点并添加到尾部 */
    T * CreateTail(const K &rKey);

    /* 创建节点并添加到首部 */
    T * CreateHead(const K &rKey);

    /* 仅创建节点 */
    T * Create(const K &rKey);

    /* 仅释放节点 */
    WORD32 Delete(T *pInst);

    /* 将节点添加到尾部 */
    WORD32 InsertTail(T *pInst);

    /* 将节点添加到首部 */
    WORD32 InsertHead(T *pInst);

    /* 从链表中移除节点并释放节点 */
    WORD32 Remove(const K &rKey);

    T * GetHead();
    T * GetTail();
    T * Next(T *pData);
    T * Prev(T *pData);

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
        pData = (*pCurHead);

        if (pData->m_tKey == rKey)
        {
            return (*pData);
        }

        pCurHead = pCurHead->m_pNext;
    }

    return NULL;
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


/* 仅释放节点 */
template <typename K, class T, WORD32 NODE_NUM>
inline WORD32 CBaseSequence<K, T, NODE_NUM>::Delete(T *pInst)
{
    CSequenceData *pData = (CSequenceData *)(pInst) - s_dwValueOffset;

    m_cList.Free(pData);
    delete pData;

    return SUCCESS;
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
        pData    = (*pCurHead);

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
inline T * CBaseSequence<K, T, NODE_NUM>::GetHead()
{
    if (NULL == m_ptHeader)
    {
        return NULL;
    }

    return *m_ptHeader;
}


template <typename K, class T, WORD32 NODE_NUM>
inline T * CBaseSequence<K, T, NODE_NUM>::GetTail()
{
    if (NULL == m_ptTailer)
    {
        return NULL;
    }

    return *m_ptTailer;
}


template <typename K, class T, WORD32 NODE_NUM>
inline T * CBaseSequence<K, T, NODE_NUM>::Next(T *pData)
{
    if (FALSE == m_cList.IsValid(pData))
    {
        return NULL;
    }

    WORD64 lwAddr = (WORD64)pData;

    T_SequenceHeader *pCurNode = (T_SequenceHeader *)(lwAddr - m_cList.s_dwDataOffset);
    if (pCurNode->m_bFree)
    {
        return NULL;
    }

    T_SequenceHeader *pNextNode = pCurNode->m_pNext;

    if (NULL == pNextNode)
    {
        return NULL;
    }

    return *pNextNode;
}


template <typename K, class T, WORD32 NODE_NUM>
inline T * CBaseSequence<K, T, NODE_NUM>::Prev(T *pData)
{
    if (FALSE == m_cList.IsValid(pData))
    {
        return NULL;
    }

    WORD64 lwAddr = (WORD64)pData;

    T_SequenceHeader *pCurNode = (T_SequenceHeader *)(lwAddr - m_cList.s_dwDataOffset);
    if (pCurNode->m_bFree)
    {
        return NULL;
    }

    T_SequenceHeader *pPrevNode = pCurNode->m_pPrev;

    if (NULL == pPrevNode)
    {
        return NULL;
    }

    return *pPrevNode;
}


template <typename K, class T, WORD32 NODE_NUM>
inline WORD32 CBaseSequence<K, T, NODE_NUM>::GetCount()
{
    return m_dwCount;
}


#endif


