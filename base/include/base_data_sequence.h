

#ifndef _BASE_DATA_SEQUENCE_H_
#define _BASE_DATA_SEQUENCE_H_


#include "base_data_container.h"


template <typename K, typename T, WORD32 NODE_NUM>
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

    typedef CBaseDataContainer<CSequenceData, NODE_NUM, FALSE>  CSequenceContainer;
    typedef typename CSequenceContainer::T_DataHeader           T_SequenceHeader;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"

    static const WORD32 s_dwValueOffset = offsetof(CSequenceData, m_tData);

#pragma GCC diagnostic pop

public :
    CBaseSequence ();
    virtual ~CBaseSequence();

    WORD32 Initialize();

    T * Find(const K &rKey);

    /* �����ڵ㲢��ӵ�β�� */
    T * CreateTail(const K &rKey);

    /* �����ڵ㲢��ӵ��ײ� */
    T * CreateHead(const K &rKey);

    /* �������ڵ� */
    T * Create(const K &rKey);

    /* ���ͷŽڵ� */
    WORD32 Delete(T *pInst);

    /* ���ڵ���ӵ�β�� */
    WORD32 InsertTail(T *pInst);

    /* ���ڵ���ӵ��ײ� */
    WORD32 InsertHead(T *pInst);

    /* ���������Ƴ��ڵ㲢�ͷŽڵ� */
    WORD32 Remove(const K &rKey);

    /* ���������Ƴ��ڵ㲢�ͷŽڵ� */
    WORD32 Remove(T *pInst);

    T * GetHead();
    T * GetTail();
    T * Next(T *pData);
    T * Prev(T *pData);

    WORD32 GetCount();

protected :
    WORD32               m_dwCount;
    T_SequenceHeader    *m_ptHeader;
    T_SequenceHeader    *m_ptTailer;
    CSequenceContainer   m_cList;
};


template <typename K, typename T, WORD32 NODE_NUM>
CBaseSequence<K, T, NODE_NUM>::CBaseSequence ()
{
    m_dwCount  = 0;
    m_ptHeader = NULL;
    m_ptTailer = NULL;
}


template <typename K, typename T, WORD32 NODE_NUM>
CBaseSequence<K, T, NODE_NUM>::~CBaseSequence()
{
    m_dwCount  = 0;
    m_ptHeader = NULL;
    m_ptTailer = NULL;
}


template <typename K, typename T, WORD32 NODE_NUM>
WORD32 CBaseSequence<K, T, NODE_NUM>::Initialize()
{
    m_cList.Initialize();

    return SUCCESS;
}


template <typename K, typename T, WORD32 NODE_NUM>
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


/* �����ڵ㲢��ӵ�β�� */
template <typename K, typename T, WORD32 NODE_NUM>
inline T * CBaseSequence<K, T, NODE_NUM>::CreateTail(const K &rKey)
{
    CSequenceData *pData = (CSequenceData *)(m_cList.Malloc());
    if (NULL == pData)
    {
        return NULL;
    }

    WORD64 lwAddr = (WORD64)pData;

    T_SequenceHeader *pCurHead = (T_SequenceHeader *)(lwAddr - (CSequenceContainer::s_dwDataOffset));
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


/* �����ڵ㲢��ӵ��ײ� */
template <typename K, typename T, WORD32 NODE_NUM>
inline T * CBaseSequence<K, T, NODE_NUM>::CreateHead(const K &rKey)
{
    CSequenceData *pData = (CSequenceData *)(m_cList.Malloc());
    if (NULL == pData)
    {
        return NULL;
    }

    WORD64 lwAddr = (WORD64)pData;

    T_SequenceHeader *pCurHead = (T_SequenceHeader *)(lwAddr - (CSequenceContainer::s_dwDataOffset));
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


/* �������ڵ� */
template <typename K, typename T, WORD32 NODE_NUM>
inline T * CBaseSequence<K, T, NODE_NUM>::Create(const K &rKey)
{
    CSequenceData *pData = (CSequenceData *)(m_cList.Malloc());
    if (NULL == pData)
    {
        return NULL;
    }

    new (pData) CSequenceData(rKey);

    return *pData;
}


/* ���ͷŽڵ� */
template <typename K, typename T, WORD32 NODE_NUM>
inline WORD32 CBaseSequence<K, T, NODE_NUM>::Delete(T *pInst)
{
    WORD64 lwAddr = (WORD64)(pInst) - s_dwValueOffset;

    if (FALSE == m_cList.IsValid((VOID *)lwAddr))
    {
        return FAIL;
    }

    CSequenceData    *pData    = (CSequenceData *)lwAddr;
    T_SequenceHeader *pCurHead = (T_SequenceHeader *)(lwAddr - (CSequenceContainer::s_dwDataOffset));

    delete pData;
    m_cList.Free(pCurHead->m_dwIndex);

    return SUCCESS;
}


/* ���ڵ���ӵ�β�� */
template <typename K, typename T, WORD32 NODE_NUM>
inline WORD32 CBaseSequence<K, T, NODE_NUM>::InsertTail(T *pInst)
{
    WORD64 lwAddr = (WORD64)(pInst) - s_dwValueOffset;

    if (FALSE == m_cList.IsValid((VOID *)lwAddr))
    {
        return FAIL;
    }

    T_SequenceHeader *pCurHead = (T_SequenceHeader *)(lwAddr - (CSequenceContainer::s_dwDataOffset));
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


/* ���ڵ���ӵ��ײ� */
template <typename K, typename T, WORD32 NODE_NUM>
inline WORD32 CBaseSequence<K, T, NODE_NUM>::InsertHead(T *pInst)
{
    WORD64 lwAddr = (WORD64)(pInst) - s_dwValueOffset;

    if (FALSE == m_cList.IsValid((VOID *)lwAddr))
    {
        return FAIL;
    }

    T_SequenceHeader *pCurHead = (T_SequenceHeader *)(lwAddr - (CSequenceContainer::s_dwDataOffset));
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


/* ���������Ƴ��ڵ㲢�ͷŽڵ� */
template <typename K, typename T, WORD32 NODE_NUM>
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
                /* ɾ��ͷ�ڵ� */
                m_ptHeader = pNext;
            }
            else
            {
                pPrev->m_pNext = pNext;
            }

            if (NULL == pNext)
            {
                /* ɾ��β�ڵ� */
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


/* ���������Ƴ��ڵ㲢�ͷŽڵ� */
template <typename K, typename T, WORD32 NODE_NUM>
inline WORD32 CBaseSequence<K, T, NODE_NUM>::Remove(T *pInst)
{
    WORD64            lwAddr   = (WORD64)(pInst) - s_dwValueOffset;
    WORD32            dwInstID = INVALID_DWORD;
    CSequenceData    *pData    = NULL;
    T_SequenceHeader *pPrev    = NULL;
    T_SequenceHeader *pNext    = NULL;

    if (FALSE == m_cList.IsValid((VOID *)lwAddr))
    {
        return FAIL;
    }

    T_SequenceHeader *pCurNode = (T_SequenceHeader *)(lwAddr - m_cList.s_dwDataOffset);
    if (INVALID_DWORD == pCurNode->m_dwRefCount)
    {
        return FAIL;
    }

    dwInstID = pCurNode->m_dwIndex;
    pData    = *pCurNode;
    pPrev    = pCurNode->m_pPrev;
    pNext    = pCurNode->m_pNext;

    if (NULL == pPrev)
    {
        /* ɾ��ͷ�ڵ� */
        m_ptHeader = pNext;
    }
    else
    {
        pPrev->m_pNext = pNext;
    }

    if (NULL == pNext)
    {
        /* ɾ��β�ڵ� */
        m_ptTailer = pPrev;
    }
    else
    {
        pNext->m_pPrev = pPrev;
    }

    pCurNode->m_pPrev = NULL;
    pCurNode->m_pNext = NULL;

    delete pData;
    m_dwCount--;

    m_cList.Free(dwInstID);

    return SUCCESS;
}


template <typename K, typename T, WORD32 NODE_NUM>
inline T * CBaseSequence<K, T, NODE_NUM>::GetHead()
{
    if (NULL == m_ptHeader)
    {
        return NULL;
    }

    CSequenceData *pData = (*m_ptHeader);

    return *pData;
}


template <typename K, typename T, WORD32 NODE_NUM>
inline T * CBaseSequence<K, T, NODE_NUM>::GetTail()
{
    if (NULL == m_ptTailer)
    {
        return NULL;
    }

    CSequenceData *pData = (*m_ptTailer);

    return *pData;
}


template <typename K, typename T, WORD32 NODE_NUM>
inline T * CBaseSequence<K, T, NODE_NUM>::Next(T *pData)
{
    WORD64 lwAddr = (WORD64)(pData) - s_dwValueOffset;

    if (FALSE == m_cList.IsValid((VOID *)lwAddr))
    {
        return NULL;
    }

    T_SequenceHeader *pCurNode = (T_SequenceHeader *)(lwAddr - m_cList.s_dwDataOffset);
    if (INVALID_DWORD == pCurNode->m_dwRefCount)
    {
        return NULL;
    }

    T_SequenceHeader *pNextNode = pCurNode->m_pNext;

    if (NULL == pNextNode)
    {
        return NULL;
    }

    CSequenceData *pInnerData = (*pNextNode);

    return *pInnerData;
}


template <typename K, typename T, WORD32 NODE_NUM>
inline T * CBaseSequence<K, T, NODE_NUM>::Prev(T *pData)
{
    WORD64 lwAddr = (WORD64)(pData) - s_dwValueOffset;

    if (FALSE == m_cList.IsValid((VOID *)lwAddr))
    {
        return NULL;
    }

    T_SequenceHeader *pCurNode = (T_SequenceHeader *)(lwAddr - m_cList.s_dwDataOffset);
    if (INVALID_DWORD == pCurNode->m_dwRefCount)
    {
        return NULL;
    }

    T_SequenceHeader *pPrevNode = pCurNode->m_pPrev;

    if (NULL == pPrevNode)
    {
        return NULL;
    }

    CSequenceData *pInnerData = (*pPrevNode);

    return *pInnerData;
}


template <typename K, typename T, WORD32 NODE_NUM>
inline WORD32 CBaseSequence<K, T, NODE_NUM>::GetCount()
{
    return m_dwCount;
}


#endif


