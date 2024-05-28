

#ifndef _BASE_DATA_CONTAINER_H_
#define _BASE_DATA_CONTAINER_H_


#include <string.h>
#include <stddef.h>


#include "pub_typedef.h"


/* 分配内存块的最小粒度 */
#define CACHE_SIZE            ((WORD32)(64))
#define PAGE_SIZE             ((WORD32)(4096))
#define DOUBLE_CACHE_SIZE     ((WORD32)(128))
#define TRIPLE_CACHE_SIZE     ((WORD32)(192))
#define QUATRA_CACHE_SIZE     ((WORD32)(256))
#define PENTA_CACHE_SIZE      ((WORD32)(320))
#define OCTUPLE_CACHE_SIZE    ((WORD32)(512))
#define QUARTER_PAGE_SIZE     ((WORD32)(1024))


template <typename T>
class CGuard 
{
public :
    CGuard(T **pInst) : m_pInst(pInst) { }
    
    virtual ~CGuard()
    {
        if (NULL != *m_pInst)
        {
            delete (*m_pInst);
        }
    }

protected :
    T **m_pInst;
};


template <typename T>
class CGuardVariable
{
public :
    CGuardVariable(T &rValue, T tNewValue)
        : m_rValue(rValue)
    {
        m_OldValue = rValue;
        m_rValue   = tNewValue;
    }

    virtual ~CGuardVariable()
    {
        m_rValue = m_OldValue;
    }

protected :
    T &m_rValue;
    T  m_OldValue;
};


#define GUARD(T, P)                CGuard<T> __Guard_##P##_(&P)
#define GUARD_VARIABLE(T, RV, NV)  CGuardVariable<T> __Guard_##RV##_(RV, NV)


class CBaseData
{
public :
    virtual ~CBaseData() {};
    
    VOID * operator new (size_t size, VOID *ptr)
    {
        return ptr;
    }

    VOID operator delete (VOID *ptr)
    {
        return ;
    }

    VOID operator delete (VOID *pMem, VOID *ptr)
    {
    }
};


template <class T>
class CDecorateData : public CBaseData
{
public :
    static T* Create(BYTE *pMem)
    {
        memset(pMem, 0x00, sizeof(CDecorateData<T>));        
        CDecorateData<T> *pValue = new (pMem) CDecorateData<T>;

        return (*pValue);
    }

    static T* Reset(BYTE *pMem)
    {
        CDecorateData<T> *pValue = (CDecorateData<T> *)(pMem);        
        delete pValue;
        
        memset(pMem, 0x00, sizeof(CDecorateData<T>));
        pValue = new (pMem) CDecorateData<T>;

        return (*pValue);
    }

    static VOID Destroy(BYTE *pMem)
    {
        CDecorateData<T> *pValue = (CDecorateData<T> *)(pMem);        
        delete pValue;
    }

public :
    CDecorateData () {}
    virtual ~CDecorateData() {}

    operator T& ()
    {
        return m_cValue;
    }

    operator T* ()
    {
        return &m_cValue;
    }

protected :
    T   m_cValue;
};


template <class T, WORD32 NODE_NUM>
class CBaseDataContainer
{
public :
    enum { HEAD_PAD_SIZE = 40 };

    typedef struct tagT_DataHeader
    {
        WORD32            m_dwIndex;
        BOOL              m_bFree;
        tagT_DataHeader  *m_pNext;
        BYTE              aucPad[HEAD_PAD_SIZE];
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
        }

        ~CDataNode()
        {
            m_tHeader.m_bFree = TRUE;
            m_tHeader.m_pNext = NULL;
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
    static_assert(s_dwDataOffset == CACHE_SIZE, "unexpected CDataNode layout");

#pragma GCC diagnostic pop


public :        
    CBaseDataContainer ();
    virtual ~CBaseDataContainer();

    WORD32 Initialize();

    T * Malloc(WORD32 *pdwIndex = NULL);

    VOID Free(T *ptr);
    VOID Free(WORD32 dwIndex);

    CDataNode & operator[] (WORD32 dwIndex);

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
CBaseDataContainer<T, NODE_NUM>::CBaseDataContainer ()
{
    WORD64 lwAlign = CACHE_SIZE;
    WORD64 lwBegin = (WORD64)(&(m_aucData[0]));

    m_lwBegin     = ROUND_UP(lwBegin, lwAlign);
    m_lwEnd       = m_lwBegin + (s_dwNodeSize * NODE_NUM);
    m_pFreeHeader = NULL;
}


template <class T, WORD32 NODE_NUM>
CBaseDataContainer<T, NODE_NUM>::~CBaseDataContainer ()
{
    m_pFreeHeader = NULL;
}


template <class T, WORD32 NODE_NUM>
WORD32 CBaseDataContainer<T, NODE_NUM>::Initialize()
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

        pNextNode = pCurNode;
    }

    return SUCCESS;
}


template <class T, WORD32 NODE_NUM>
inline T * CBaseDataContainer<T, NODE_NUM>::Malloc(WORD32 *pdwIndex)
{
    T_DataHeader *pCurHead = m_pFreeHeader;

    if (pCurHead)
    {
        m_pFreeHeader     = pCurHead->m_pNext;
        pCurHead->m_pNext = NULL;
        pCurHead->m_bFree = FALSE;

        if (NULL != pdwIndex)
        {
            *pdwIndex = pCurHead->m_dwIndex;
        }

        return (T *)((WORD64)(pCurHead) - s_dwHeadOffset + s_dwDataOffset);
    }
    else
    {
        return NULL;
    }
}


template <class T, WORD32 NODE_NUM>
inline VOID CBaseDataContainer<T, NODE_NUM>::Free(T *ptr)
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
inline VOID CBaseDataContainer<T, NODE_NUM>::Free(WORD32 dwIndex)
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
inline typename CBaseDataContainer<T, NODE_NUM>::CDataNode & 
CBaseDataContainer<T, NODE_NUM>::operator[] (WORD32 dwIndex)
{
    if (dwIndex >= NODE_NUM)
    {
        return *(CDataNode *)NULL;
    }

    return *((CDataNode *)(m_lwBegin + (s_dwNodeSize * dwIndex)));
}


template <class T, WORD32 NODE_NUM>
inline BOOL CBaseDataContainer<T, NODE_NUM>::IsValid(VOID *pAddr)
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
inline VOID CBaseDataContainer<T, NODE_NUM>::Free(
    typename CBaseDataContainer<T, NODE_NUM>::CDataNode *pNode)
{
    if (unlikely(NULL == pNode))
    {
        return ;
    }

    pNode->m_tHeader.m_bFree = TRUE;
    pNode->m_tHeader.m_pNext = m_pFreeHeader;
    m_pFreeHeader = &(pNode->m_tHeader);
}


#endif


