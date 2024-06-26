

#ifndef _BASE_RING_ARRAY_H_
#define _BASE_RING_ARRAY_H_


#include <assert.h>

#include "base_ring.h"


typedef enum tagE_ArrayErrno
{
    E_ARRAY_ERR_INVALID = 0,
    E_ARRAY_ERR_CROSS_BORDER,    /* 数组越界 */
    E_ARRAY_ERR_REPEAT,          /* 重复释放 */
    E_ARRAY_ERR_ILLEGAL_ADDR,    /* 释放非法地址 */
    E_ARRAY_ERR_COVER,           /* 内存被踩 */
    E_ARRAY_ERR_ENQUEUE_FAIL,    /* CRingArray入队列失败 */
    E_ARRAY_ERR_NOT_FIND,        /* 查找不到实例 */
}E_ArrayErrno;


template <class T, WORD32 INST_NUM>
class CBaseArray
{
public :
    class CArrayData : public CBaseData
    {
    public :
        T    m_tData;

        CArrayData () {}
        ~CArrayData() {}

        operator T& ()
        {
            return m_tData;
        }

        operator T* ()
        {
            return &m_tData;
        }
    };

    typedef struct tagT_ArrayData
    {
        BOOL    bFree;
        WORD32  dwIndex;
        BYTE    aucData[sizeof(CArrayData)];
    }T_ArrayData;

    static const WORD32 s_dwOffset   = offsetof(CArrayData, m_tData) + offsetof(T_ArrayData, aucData);
    static const WORD32 s_dwNodeSize = ROUND_UP(sizeof(T_ArrayData), CACHE_SIZE);
    static const WORD32 s_dwSize     = (s_dwNodeSize * INST_NUM) + CACHE_SIZE;

public :
    CBaseArray ();
    virtual ~CBaseArray();

    WORD32 Initialize();

    T_ArrayData * operator[] (WORD32 dwIndex);

    T * Find(WORD32 dwIndex);

    BYTE * Malloc(WORD32 dwIndex);

    T * Create(WORD32 dwIndex);

    WORD32 Delete(WORD32 dwIndex);

    WORD32 Delete(T *pData);

protected :
    BOOL IsValid(T *pData);

    VOID Clear();

protected :
    WORD64         m_lwBegin;
    WORD64         m_lwEnd;

    WORD32         m_dwNum;
    T_ArrayData   *m_apData[INST_NUM];

    BYTE           m_aucData[s_dwSize];
};


template <class T, WORD32 INST_NUM>
CBaseArray<T, INST_NUM>::CBaseArray ()
{
    WORD64 lwAlign = CACHE_SIZE;
    WORD64 lwBegin = (WORD64)(&(m_aucData[0]));

    m_lwBegin = ROUND_UP(lwBegin, lwAlign);
    m_lwEnd   = m_lwBegin + (s_dwNodeSize * INST_NUM);
    m_dwNum   = 0;

    memset(m_apData, 0x00, (INST_NUM * sizeof(T_ArrayData *)));
}


template <class T, WORD32 INST_NUM>
CBaseArray<T, INST_NUM>::~CBaseArray()
{
    Clear();

    m_dwNum = 0;
}


template <class T, WORD32 INST_NUM>
WORD32 CBaseArray<T, INST_NUM>::Initialize()
{
    WORD64 lwAddr = m_lwBegin;

    for (WORD32 dwIndex = 0; dwIndex < INST_NUM; dwIndex++)
    {
        m_apData[dwIndex]          = (T_ArrayData *)(lwAddr);
        m_apData[dwIndex]->bFree   = TRUE;
        m_apData[dwIndex]->dwIndex = dwIndex;

        lwAddr += s_dwNodeSize;
    }

    return SUCCESS;
}


template <class T, WORD32 INST_NUM>
inline typename CBaseArray<T, INST_NUM>::T_ArrayData * 
CBaseArray<T, INST_NUM>::operator[] (WORD32 dwIndex)
{
    if (unlikely(dwIndex >= INST_NUM))
    {
        return NULL;
    }

    return m_apData[dwIndex];
}


template <class T, WORD32 INST_NUM>
inline T * CBaseArray<T, INST_NUM>::Find(WORD32 dwIndex)
{
    if (unlikely(dwIndex >= INST_NUM))
    {
        return NULL;
    }

    if (TRUE == m_apData[dwIndex]->bFree)
    {
        return NULL;
    }

    CArrayData *pNode = (CArrayData *)(m_apData[dwIndex]->aucData);

    return (*pNode);
}


template <class T, WORD32 INST_NUM>
inline BYTE * CBaseArray<T, INST_NUM>::Malloc(WORD32 dwIndex)
{
    if (unlikely(dwIndex >= INST_NUM))
    {
        return NULL;
    }

    /* 重复(对应ID已经分配) */
    if (TRUE != m_apData[dwIndex]->bFree)
    {
        return NULL;
    }

    m_apData[dwIndex]->bFree = FALSE;

    m_dwNum++;

    return (m_apData[dwIndex]->aucData);
}


template <class T, WORD32 INST_NUM>
inline T * CBaseArray<T, INST_NUM>::Create(WORD32 dwIndex)
{
    if (unlikely(dwIndex >= INST_NUM))
    {
        return NULL;
    }

    /* 重复(对应ID已经分配) */
    if (TRUE != m_apData[dwIndex]->bFree)
    {
        return NULL;
    }

    CArrayData *pData = new (m_apData[dwIndex]->aucData) CArrayData();
    m_apData[dwIndex]->bFree = FALSE;

    m_dwNum++;

    return (*pData);
}


template <class T, WORD32 INST_NUM>
inline WORD32 CBaseArray<T, INST_NUM>::Delete(WORD32 dwIndex)
{
    if (unlikely(dwIndex >= INST_NUM))
    {
        return (WORD32)(E_ARRAY_ERR_CROSS_BORDER);
    }

    if (TRUE == m_apData[dwIndex]->bFree)
    {
        return (WORD32)(E_ARRAY_ERR_REPEAT);
    }

    if (dwIndex != m_apData[dwIndex]->dwIndex)
    {
        return (WORD32)(E_ARRAY_ERR_COVER);
    }

    CArrayData *pData = (CArrayData *)(m_apData[dwIndex]->aucData);
    delete pData;
    m_apData[dwIndex]->bFree = TRUE;

    m_dwNum--;

    return SUCCESS;
}


template <class T, WORD32 INST_NUM>
inline WORD32 CBaseArray<T, INST_NUM>::Delete(T *pData)
{
    if (unlikely(NULL == pData))
    {
        return (WORD32)(E_ARRAY_ERR_CROSS_BORDER);
    }

    if (!IsValid(pData))
    {
        return (WORD32)(E_ARRAY_ERR_ILLEGAL_ADDR);
    }

    T_ArrayData *pNode = (T_ArrayData *)(((WORD64)(pData)) - s_dwOffset);
    if (pNode->bFree)
    {
        return (WORD32)(E_ARRAY_ERR_REPEAT);
    }

    if (pNode->dwIndex >= INST_NUM)
    {
        return (WORD32)(E_ARRAY_ERR_COVER);
    }

    CArrayData *pInst = (CArrayData *)(pNode->aucData);
    delete pInst;
    pNode->bFree = TRUE;

    m_dwNum--;

    return SUCCESS;
}


template <class T, WORD32 INST_NUM>
inline BOOL CBaseArray<T, INST_NUM>::IsValid(T *pData)
{
    WORD64 lwAddr = (WORD64)pData;

    if (unlikely((lwAddr < (m_lwBegin + s_dwOffset)) || (lwAddr >= m_lwEnd)))
    {
        return FALSE;
    }

    return (0 == ((lwAddr - m_lwBegin - s_dwOffset) % s_dwNodeSize));
}


template <class T, WORD32 INST_NUM>
VOID CBaseArray<T, INST_NUM>::Clear()
{
    CArrayData *pData = NULL;

    for (WORD32 dwIndex = 0; dwIndex < INST_NUM; dwIndex++)
    {
        if (m_apData[dwIndex]->bFree)
        {
            continue ;
        }

        pData = (CArrayData *)(m_apData[dwIndex]->aucData);
        delete pData;
        m_apData[dwIndex]->bFree = TRUE;
    }

    m_dwNum = 0;
}


template <class T, WORD32 INST_NUM, WORD32 POWER_NUM>
class CRingArray
{
public :
    typedef typename CBaseArray<T, INST_NUM>::T_ArrayData  T_RingObject;

public :
    CRingArray ();
    virtual ~CRingArray();

    WORD32 Initialize();

    T * Create();

    WORD32 Delete(T *pData);

protected :
    CSimpleRing<POWER_NUM>   m_cRing;
    CBaseArray<T, INST_NUM>  m_cArray;
};


template <class T, WORD32 INST_NUM, WORD32 POWER_NUM>
CRingArray<T, INST_NUM, POWER_NUM>::CRingArray ()
{
}


template <class T, WORD32 INST_NUM, WORD32 POWER_NUM>
CRingArray<T, INST_NUM, POWER_NUM>::~CRingArray()
{
}


template <class T, WORD32 INST_NUM, WORD32 POWER_NUM>
WORD32 CRingArray<T, INST_NUM, POWER_NUM>::Initialize()
{
    m_cRing.Initialize();
    m_cArray.Initialize();

    T_RingObject *pObj = NULL;

    for (WORD32 dwIndex = 0; dwIndex < INST_NUM; dwIndex++)
    {
        pObj = m_cArray[dwIndex];
        m_cRing.Enqueue((VOID *)pObj);
    }

    return SUCCESS;
}


template <class T, WORD32 INST_NUM, WORD32 POWER_NUM>
inline T * CRingArray<T, INST_NUM, POWER_NUM>::Create()
{
    T            *pData = NULL; 
    WORD32        dwNum = 0;
    T_RingObject *pObj  = NULL;

    dwNum = m_cRing.Dequeue((VOID **)(&pObj));
    if (0 == dwNum)
    {
        return NULL;
    }

    pData = m_cArray.Create(pObj->dwIndex);
    if (NULL == pData)
    {
        /* 异常 */
        assert(0);
        return NULL;
    }

    return pData;
}


template <class T, WORD32 INST_NUM, WORD32 POWER_NUM>
inline WORD32 CRingArray<T, INST_NUM, POWER_NUM>::Delete(T *pData)
{
    WORD32 dwResult = m_cArray.Delete(pData);
    if (SUCCESS != dwResult)
    {
        return dwResult;
    }

    T_RingObject *pObj = (T_RingObject *)((WORD64)(pData) - (m_cArray.s_dwOffset));

    WORD32 dwNum = m_cRing.Enqueue(pObj);
    if (0 == dwNum)
    {
        /* 异常 */
        assert(0);
        return (WORD32)(E_ARRAY_ERR_ENQUEUE_FAIL);
    }

    return SUCCESS;
}


#endif


