

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


/* ALIGN_FLAG : 按CACHE_SIZE对齐标志 */
template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
class CLinkTpl
{
public :
    CLinkTpl() {}
    virtual ~CLinkTpl() = 0;
};


template <class T, WORD32 NODE_NUM>
class CLinkTpl<T, NODE_NUM, TRUE>
{
public :
    enum { HEAD_PAD_SIZE = 48 };

    typedef struct tagT_DataHeader
    {
        WORD32            m_dwIndex;
        BOOL              m_bFree;
        tagT_DataHeader  *m_pNext;
        BYTE              m_aucPad[HEAD_PAD_SIZE];
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


template <class T, WORD32 NODE_NUM>
class CLinkTpl<T, NODE_NUM, FALSE>
{
public :
    enum { HEAD_PAD_SIZE = 48 };

    typedef struct tagT_DataHeader
    {
        WORD32            m_dwIndex;
        BOOL              m_bFree;
        tagT_DataHeader  *m_pNext;
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
    static_assert(offsetof(T_DataHeader, m_aucData) == 16, "unexpected layout");
};


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG = FALSE>
class CBaseDataContainer : public CLinkTpl<T, NODE_NUM, ALIGN_FLAG>
{
public :
    typedef typename CLinkTpl<T, NODE_NUM, ALIGN_FLAG>::T_DataHeader  T_DataHeader;

    static const WORD32 s_dwNodeSize = ROUND_UP(sizeof(T_DataHeader), CACHE_SIZE);
    static const WORD32 s_dwSize     = (s_dwNodeSize * NODE_NUM) + CACHE_SIZE;

    static const WORD32 s_dwDataOffset = offsetof(T_DataHeader, m_aucData);

public :        
    CBaseDataContainer ();
    virtual ~CBaseDataContainer();

    WORD32 Initialize();

    T * Malloc(WORD32 *pdwIndex = NULL);
    T * Malloc(WORD32 &rdwIndex);

    VOID Free(T *ptr);
    VOID Free(WORD32 dwIndex);

    T_DataHeader & operator[] (WORD32 dwIndex);
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
CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::CBaseDataContainer ()
{
    WORD64 lwAlign = CACHE_SIZE;
    WORD64 lwBegin = (WORD64)(&(m_aucData[0]));

    m_lwBegin     = ROUND_UP(lwBegin, lwAlign);
    m_lwEnd       = m_lwBegin + (s_dwNodeSize * NODE_NUM);
    m_pFreeHeader = NULL;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::~CBaseDataContainer ()
{
    m_pFreeHeader = NULL;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
WORD32 CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::Initialize()
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

        pNextNode = pCurNode;
    }

    return SUCCESS;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::Malloc(WORD32 *pdwIndex)
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

        return (T *)(pCurHead->m_aucData);
    }
    else
    {
        return NULL;
    }
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::Malloc(WORD32 &rdwIndex)
{
    T_DataHeader *pCurHead = m_pFreeHeader;

    if (pCurHead)
    {
        m_pFreeHeader     = pCurHead->m_pNext;
        pCurHead->m_pNext = NULL;
        pCurHead->m_bFree = FALSE;

        rdwIndex = pCurHead->m_dwIndex;

        return (T *)(pCurHead->m_aucData);
    }
    else
    {
        return NULL;
    }
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline VOID CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::Free(T *ptr)
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

    T_DataHeader *pNode = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * (pCur->m_dwIndex)));
    if (unlikely(pCur != pNode))
    {
        return ;
    }

    Free(pNode);
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline VOID CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::Free(WORD32 dwIndex)
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


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline T * CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::operator() (WORD32 dwIndex)
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
inline BOOL CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::IsFree(WORD32 dwIndex)
{
    if (unlikely(dwIndex >= NODE_NUM))
    {
        return FALSE;
    }

    T_DataHeader *pData = (T_DataHeader *)(m_lwBegin + (s_dwNodeSize * dwIndex));

    return pData->m_bFree;
}


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
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


template <class T, WORD32 NODE_NUM, BOOL ALIGN_FLAG>
inline VOID CBaseDataContainer<T, NODE_NUM, ALIGN_FLAG>::Free(T_DataHeader *pData)
{
    if (unlikely(NULL == pData))
    {
        return ;
    }

    pData->m_bFree = TRUE;
    pData->m_pNext = m_pFreeHeader;
    m_pFreeHeader  = pData;
}


#endif


