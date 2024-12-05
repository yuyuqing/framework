

#ifndef _BASE_DATA_LIST_H_
#define _BASE_DATA_LIST_H_


#include "base_data_container.h"


/* ������ֵ��ΪSUCCESSʱ, ��ڵ㲻�����������, ֱ���˳�;
 * ����Χֵ����SUCCESSʱ, ����rbFlagȡֵ�ж��Ƿ������������
 * a. ��rbFlagΪTRUE , �����ڵ���˳�
 * b. ��rbFlagΪFALSE, ���������Ѱ�Һ��ʵĲ���λ��
 */
template <typename T>
using PInsertAFunc = WORD32 (*)(T &rLeft, T &rRight, BOOL &rbFlag);


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG = FALSE>
class CBaseList : public CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>
{
public :
    typedef typename CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::T_DataHeader  T_LinkHeader;

public :
    CBaseList ();
    virtual ~CBaseList();

    virtual WORD32 Initialize();

    /* �����ڵ㲢��ӵ�β�� */
    T * CreateTail();

    /* �����ڵ㲢��ӵ��ײ� */
    T * CreateHead();

    /* ���ڵ���ӵ�β�� */
    WORD32 InsertTail(T *pInst);

    /* ���ڵ���ӵ��ײ� */
    WORD32 InsertHead(T *pInst);

    /* ���ڵ㰴�����������뵽������ */
    WORD32 Insert(T *pInst, PInsertAFunc<T> pFunc);

    /* ���������Ƴ��ڵ㲢�ͷŽڵ� */
    WORD32 Remove(T *pInst);

    /* ���������Ƴ��ڵ㲢�ͷŽڵ�(����Key�ҵ����Ƴ��ڵ�) */
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


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
CBaseList<T, NODE_NUM, ALIGN_FLAG>::CBaseList ()
{
    m_dwCount  = 0;
    m_ptHeader = NULL;
    m_ptTailer = NULL;
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
CBaseList<T, NODE_NUM, ALIGN_FLAG>::~CBaseList()
{
    m_dwCount  = 0;
    m_ptHeader = NULL;
    m_ptTailer = NULL;
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
WORD32 CBaseList<T, NODE_NUM, ALIGN_FLAG>::Initialize()
{
    CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::Initialize();

    return SUCCESS;
}


/* �����ڵ㲢��ӵ�β�� */
template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CBaseList<T, NODE_NUM, ALIGN_FLAG>::CreateTail()
{
    BYTE *pData = this->Malloc();
    if (NULL == pData)
    {
        return NULL;
    }

    WORD64 lwAddr = (WORD64)pData;

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

    return (T *)pData;
}


/* ��ӵ��ײ� */
template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CBaseList<T, NODE_NUM, ALIGN_FLAG>::CreateHead()
{
    BYTE *pData = this->Malloc();
    if (NULL == pData)
    {
        return NULL;
    }

    WORD64 lwAddr = (WORD64)pData;

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

    return (T *)pData;
}


/* ���ڵ���ӵ�β�� */
template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
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


/* ���ڵ���ӵ��ײ� */
template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
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


/* ���ڵ㰴�����������뵽������ */
template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline WORD32 CBaseList<T, NODE_NUM, ALIGN_FLAG>::Insert(T *pInst, PInsertAFunc<T> pFunc)
{
    if (FALSE == this->IsValid(pInst))
    {
        return FAIL;
    }

    WORD32        dwResult = INVALID_DWORD;
    BOOL          bFlag    = FALSE;
    WORD64        lwAddr   = (WORD64)pInst;
    T_LinkHeader *pCurHead = (T_LinkHeader *)(lwAddr - this->s_dwDataOffset);
    T_LinkHeader *pNode    = m_ptHeader;
    T_LinkHeader *pNext    = NULL;
    T_LinkHeader *pPrev    = NULL;
    T            *pData    = NULL;

    while (pNode)
    {
        bFlag = FALSE;
        pData = *pNode;
        pPrev = pNode->m_pPrev;
        pNext = pNode->m_pNext;

        dwResult = (*pFunc) (*pInst, *pData, bFlag);
        if (SUCCESS != dwResult)
        {
            /* ����ʧ�� */
            return FAIL;
        }

        /* �����������, ����뵽pNode֮ǰ */
        if (TRUE == bFlag)
        {
            pCurHead->m_pPrev = pPrev;
            pCurHead->m_pNext = pNode;
            pNode->m_pPrev    = pCurHead;
            if (NULL == pPrev)
            {
                m_ptHeader = pCurHead;
            }
            else
            {
                pPrev->m_pNext = pCurHead;
            }

            m_dwCount++;
            return SUCCESS;
        }

        pNode = pNext;
    }

    /* �������������ʱ, ���ڵ�����β */
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


/* ���������Ƴ��ڵ㲢�ͷŽڵ� */
template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline WORD32 CBaseList<T, NODE_NUM, ALIGN_FLAG>::Remove(T *pInst)
{
    if (FALSE == this->IsValid(pInst))
    {
        return FAIL;
    }

    WORD64 lwAddr = (WORD64)pInst;

    T_LinkHeader *pCurNode = (T_LinkHeader *)(lwAddr - this->s_dwDataOffset);
    if (INVALID_DWORD == pCurNode->m_dwRefCount)
    {
        return FAIL;
    }

    T_LinkHeader *pPrev = pCurNode->m_pPrev;
    T_LinkHeader *pNext = pCurNode->m_pNext;

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

    m_dwCount--;

    this->Free(pCurNode->m_dwIndex);

    return SUCCESS;
}


/* ���������Ƴ��ڵ㲢�ͷŽڵ�(����Key�ҵ����Ƴ��ڵ�) */
template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
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

            m_dwCount--;

            this->Free(dwInstID);

            return SUCCESS;
        }

        pCurHead = pCurHead->m_pNext;
    }

    return FAIL;
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CBaseList<T, NODE_NUM, ALIGN_FLAG>::GetHead()
{
    if (NULL == m_ptHeader)
    {
        return NULL;
    }

    return *m_ptHeader;
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CBaseList<T, NODE_NUM, ALIGN_FLAG>::GetTail()
{
    if (NULL == m_ptTailer)
    {
        return NULL;
    }

    return *m_ptTailer;
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CBaseList<T, NODE_NUM, ALIGN_FLAG>::Next(T *pData)
{
    if (FALSE == this->IsValid(pData))
    {
        return NULL;
    }

    WORD64 lwAddr = (WORD64)pData;

    T_LinkHeader *pCurNode = (T_LinkHeader *)(lwAddr - this->s_dwDataOffset);
    if (INVALID_DWORD == pCurNode->m_dwRefCount)
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


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CBaseList<T, NODE_NUM, ALIGN_FLAG>::Prev(T *pData)
{
    if (FALSE == this->IsValid(pData))
    {
        return NULL;
    }

    WORD64 lwAddr = (WORD64)pData;

    T_LinkHeader *pCurNode = (T_LinkHeader *)(lwAddr - this->s_dwDataOffset);
    if (INVALID_DWORD == pCurNode->m_dwRefCount)
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


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline WORD32 CBaseList<T, NODE_NUM, ALIGN_FLAG>::GetCount()
{
    return m_dwCount;
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
class CShareList : public CBaseData
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

    /* �����ڵ㲢��ӵ�β�� */
    T * CreateTail();

    /* �����ڵ㲢��ӵ��ײ� */
    T * CreateHead();

    /* ���ڵ���ӵ�β�� */
    WORD32 InsertTail(T *pInst);

    /* ���ڵ���ӵ��ײ� */
    WORD32 InsertHead(T *pInst);

    /* ���ڵ㰴�����������뵽������ */
    WORD32 Insert(T *pInst, PInsertAFunc<T> pFunc);

    /* ���������Ƴ��ڵ㲢�ͷŽڵ� */
    WORD32 Remove(T *pInst);

    /* ���������Ƴ��ڵ㲢�ͷŽڵ�(����Key�ҵ����Ƴ��ڵ�) */
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


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
CShareList<T, NODE_NUM, ALIGN_FLAG>::~CShareList()
{
    m_dwCount  = 0;
    m_ptHeader = NULL;
    m_ptTailer = NULL;
}


/* �����ڵ㲢��ӵ�β�� */
template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CShareList<T, NODE_NUM, ALIGN_FLAG>::CreateTail()
{
    BYTE *pData = m_rContainer.Malloc();
    if (NULL == pData)
    {
        return NULL;
    }

    WORD64 lwAddr = (WORD64)pData;

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

    return (T *)pData;
}


/* �����ڵ㲢��ӵ��ײ� */
template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CShareList<T, NODE_NUM, ALIGN_FLAG>::CreateHead()
{
    BYTE *pData = m_rContainer.Malloc();
    if (NULL == pData)
    {
        return NULL;
    }

    WORD64 lwAddr = (WORD64)pData;

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

    return (T *)pData;
}


/* ���ڵ���ӵ�β�� */
template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
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


/* ���ڵ���ӵ��ײ� */
template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
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


/* ���ڵ㰴�����������뵽������ */
template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline WORD32 CShareList<T, NODE_NUM, ALIGN_FLAG>::Insert(T *pInst, PInsertAFunc<T> pFunc)
{
    if (FALSE == m_rContainer.IsValid(pInst))
    {
        return FAIL;
    }

    WORD32        dwResult = INVALID_DWORD;
    BOOL          bFlag    = FALSE;
    WORD64        lwAddr   = (WORD64)pInst;
    T_LinkHeader *pCurHead = (T_LinkHeader *)(lwAddr - m_rContainer.s_dwDataOffset);
    T_LinkHeader *pNode    = m_ptHeader;
    T_LinkHeader *pNext    = NULL;
    T_LinkHeader *pPrev    = NULL;
    T            *pData    = NULL;

    while (pNode)
    {
        bFlag = FALSE;
        pData = *pNode;
        pPrev = pNode->m_pPrev;
        pNext = pNode->m_pNext;

        dwResult = (*pFunc) (*pInst, *pData, bFlag);
        if (SUCCESS != dwResult)
        {
            /* ����ʧ�� */
            return FAIL;
        }

        /* �����������, ����뵽pNode֮ǰ */
        if (TRUE == bFlag)
        {
            pCurHead->m_pPrev = pPrev;
            pCurHead->m_pNext = pNode;
            pNode->m_pPrev    = pCurHead;
            if (NULL == pPrev)
            {
                m_ptHeader = pCurHead;
            }
            else
            {
                pPrev->m_pNext = pCurHead;
            }

            m_dwCount++;
            return SUCCESS;
        }

        pNode = pNext;
    }

    /* �������������ʱ, ���ڵ�����β */
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


/* ���������Ƴ��ڵ㲢�ͷŽڵ�(ȷ��T���ڱ�����������) */
template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
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

            m_dwCount--;

            m_rContainer.Free(dwInstID);

            return SUCCESS;
        }

        pCurHead = pCurHead->m_pNext;
    }

    return FAIL;
}


/* ���������Ƴ��ڵ㲢�ͷŽڵ�(����Key�ҵ����Ƴ��ڵ�) */
template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
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

            m_dwCount--;

            m_rContainer.Free(dwInstID);

            return SUCCESS;
        }

        pCurHead = pCurHead->m_pNext;
    }

    return FAIL;
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CShareList<T, NODE_NUM, ALIGN_FLAG>::GetHead()
{
    if (NULL == m_ptHeader)
    {
        return NULL;
    }

    return *m_ptHeader;
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CShareList<T, NODE_NUM, ALIGN_FLAG>::GetTail()
{
    if (NULL == m_ptTailer)
    {
        return NULL;
    }

    return *m_ptTailer;
}


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CShareList<T, NODE_NUM, ALIGN_FLAG>::Next(T *pData)
{
    if (FALSE == m_rContainer.IsValid(pData))
    {
        return NULL;
    }

    WORD64 lwAddr = (WORD64)pData;

    T_LinkHeader *pCurNode = (T_LinkHeader *)(lwAddr - m_rContainer.s_dwDataOffset);
    if (INVALID_DWORD == pCurNode->m_dwRefCount)
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


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CShareList<T, NODE_NUM, ALIGN_FLAG>::Prev(T *pData)
{
    if (FALSE == m_rContainer.IsValid(pData))
    {
        return NULL;
    }

    WORD64 lwAddr = (WORD64)pData;

    T_LinkHeader *pCurNode = (T_LinkHeader *)(lwAddr - m_rContainer.s_dwDataOffset);
    if (INVALID_DWORD == pCurNode->m_dwRefCount)
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


template <typename T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline WORD32 CShareList<T, NODE_NUM, ALIGN_FLAG>::GetCount()
{
    return m_dwCount;
}


template <typename T, WORD32 NODE_NUM, WORD32 LIST_NUM, BOOL ALIGN_FLAG>
class CLinkedList
{
public :
    using CDataList      = CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>;
    using CNestList      = CShareList<T, NODE_NUM, ALIGN_FLAG>;
    using CNestContainer = CBaseDataContainer<CNestList, (LIST_NUM + 1), FALSE>;

public :
    CLinkedList ();
    virtual ~CLinkedList();

    virtual WORD32 Initialize();

    CNestList * operator[] (WORD32 dwIndex);

    T * Malloc();
    VOID Free(T *ptr);

protected :
    CDataList       m_cDataList;
    CNestContainer  m_cListContainer;
    CNestList      *m_apList[LIST_NUM];
};


template <typename T, WORD32 NODE_NUM, WORD32 LIST_NUM, BOOL ALIGN_FLAG>
CLinkedList<T, NODE_NUM, LIST_NUM, ALIGN_FLAG>::CLinkedList()
{
}


template <typename T, WORD32 NODE_NUM, WORD32 LIST_NUM, BOOL ALIGN_FLAG>
CLinkedList<T, NODE_NUM, LIST_NUM, ALIGN_FLAG>::~CLinkedList()
{
}


template <typename T, WORD32 NODE_NUM, WORD32 LIST_NUM, BOOL ALIGN_FLAG>
WORD32 CLinkedList<T, NODE_NUM, LIST_NUM, ALIGN_FLAG>::Initialize()
{
    m_cDataList.Initialize();
    m_cListContainer.Initialize();

    CNestList *pList = NULL;

    for (WORD32 dwIndex = 0; dwIndex < LIST_NUM; dwIndex++)
    {
        pList = (CNestList *)(m_cListContainer.Malloc());
        new (pList) CNestList(m_cDataList);

        m_apList[dwIndex] = pList;
    }

    return SUCCESS;
}


template <typename T, WORD32 NODE_NUM, WORD32 LIST_NUM, BOOL ALIGN_FLAG>
inline typename CLinkedList<T, NODE_NUM, LIST_NUM, ALIGN_FLAG>::CNestList * 
CLinkedList<T, NODE_NUM, LIST_NUM, ALIGN_FLAG>::operator[] (WORD32 dwIndex)
{
    if (unlikely(dwIndex >= LIST_NUM))
    {
        return NULL;
    }

    return m_apList[dwIndex];
}


template <typename T, WORD32 NODE_NUM, WORD32 LIST_NUM, BOOL ALIGN_FLAG>
inline T * CLinkedList<T, NODE_NUM, LIST_NUM, ALIGN_FLAG>::Malloc()
{
    return (T *)(m_cDataList.Malloc());
}


template <typename T, WORD32 NODE_NUM, WORD32 LIST_NUM, BOOL ALIGN_FLAG>
inline VOID CLinkedList<T, NODE_NUM, LIST_NUM, ALIGN_FLAG>::Free(T *ptr)
{
    m_cDataList.Free(ptr);
}


#endif


