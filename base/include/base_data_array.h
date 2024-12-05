

#ifndef _BASE_DATA_ARRAY_H_
#define _BASE_DATA_ARRAY_H_


#include "base_data.h"


template <typename T, WORD32 INST_NUM>
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
    static const WORD32 s_dwOffset   = offsetof(CArrayData, m_tData) + offsetof(T_ArrayData, aucData);
#pragma GCC diagnostic pop

    static const WORD32 s_dwNodeSize = ROUND_UP(sizeof(T_ArrayData), CACHE_SIZE);
    static const WORD32 s_dwSize     = (s_dwNodeSize * INST_NUM) + CACHE_SIZE;

public :
    CBaseArray ();
    virtual ~CBaseArray();

    WORD32 Initialize();

    T * operator[] (WORD32 dwIndex);
    T * operator() (WORD32 dwIndex);

    T * Create(WORD32 dwIndex);

    WORD32 Delete(T *pData);
    WORD32 Delete(WORD32 dwIndex);

protected :
    BOOL IsValid(T *pData);

    VOID Clear();

protected :
    WORD64         m_lwBegin;
    WORD64         m_lwEnd;

    BYTE           m_aucData[s_dwSize];
};


template <typename T, WORD32 INST_NUM>
CBaseArray<T, INST_NUM>::CBaseArray ()
{
    WORD64 lwAlign = CACHE_SIZE;
    WORD64 lwBegin = (WORD64)(&(m_aucData[0]));

    m_lwBegin = ROUND_UP(lwBegin, lwAlign);
    m_lwEnd   = m_lwBegin + (s_dwNodeSize * INST_NUM);
}


template <typename T, WORD32 INST_NUM>
CBaseArray<T, INST_NUM>::~CBaseArray()
{
    Clear();
}


template <typename T, WORD32 INST_NUM>
WORD32 CBaseArray<T, INST_NUM>::Initialize()
{
    WORD64       lwAddr = m_lwBegin;
    T_ArrayData *ptData = NULL;

    for (WORD32 dwIndex = 0; dwIndex < INST_NUM; dwIndex++)
    {
        ptData          = (T_ArrayData *)(lwAddr);
        ptData->bFree   = TRUE;
        ptData->dwIndex = dwIndex;

        lwAddr += s_dwNodeSize;
    }

    return SUCCESS;
}


template <typename T, WORD32 INST_NUM>
inline T * CBaseArray<T, INST_NUM>::operator[] (WORD32 dwIndex)
{
    if (unlikely(dwIndex >= INST_NUM))
    {
        return NULL;
    }

    WORD64 lwAddr = m_lwBegin + (dwIndex * s_dwNodeSize) + s_dwOffset;

    return (T *)lwAddr;
}


template <typename T, WORD32 INST_NUM>
inline T * CBaseArray<T, INST_NUM>::operator() (WORD32 dwIndex)
{
    WORD64 lwAddr = m_lwBegin + (dwIndex * s_dwNodeSize);

    if (unlikely(dwIndex >= INST_NUM))
    {
        return NULL;
    }

    T_ArrayData *ptData = (T_ArrayData *)lwAddr;
    if (TRUE == ptData->bFree)
    {
        return NULL;
    }

    return (T *)(lwAddr + s_dwOffset);
}


template <typename T, WORD32 INST_NUM>
inline T * CBaseArray<T, INST_NUM>::Create(WORD32 dwIndex)
{
    if (unlikely(dwIndex >= INST_NUM))
    {
        return NULL;
    }

    /* 重复(对应ID已经分配) */
    T_ArrayData *ptData = (T_ArrayData *)(m_lwBegin + (dwIndex * s_dwNodeSize));
    if (TRUE != ptData->bFree)
    {
        return NULL;
    }

    CArrayData *pInst = new (ptData->aucData) CArrayData();
    ptData->bFree = FALSE;

    return *pInst;
}


template <typename T, WORD32 INST_NUM>
inline WORD32 CBaseArray<T, INST_NUM>::Delete(T *pData)
{
    if (unlikely(NULL == pData))
    {
        return FAIL;
    }

    T_ArrayData *ptData     = NULL;
    CArrayData  *pInst      = NULL;
    WORD64       lwDistance = 0;
    WORD32       dwIndex    = 0;
    WORD64       lwAddr     = (WORD64)pData;

    if (unlikely((lwAddr < (m_lwBegin + s_dwOffset)) || (lwAddr >= m_lwEnd)))
    {
        assert(0);
    }

    lwDistance = lwAddr - m_lwBegin - s_dwOffset;
    if (0 != (lwDistance % s_dwNodeSize))
    {
        assert(0);
    }

    ptData = (T_ArrayData *)(lwAddr- s_dwOffset);
    if (TRUE == ptData->bFree)
    {
        return FAIL;
    }

    dwIndex = (WORD32)(lwDistance / s_dwNodeSize);
    if (dwIndex != ptData->dwIndex)
    {
        assert(0);
    }

    pInst = (CArrayData *)(ptData->aucData);
    delete pInst;
    ptData->bFree = TRUE;

    return SUCCESS;
}


template <typename T, WORD32 INST_NUM>
inline WORD32 CBaseArray<T, INST_NUM>::Delete(WORD32 dwIndex)
{
    if (unlikely(dwIndex >= INST_NUM))
    {
        return FAIL;
    }

    T_ArrayData *ptData = (T_ArrayData *)(m_lwBegin + (dwIndex * s_dwNodeSize));
    if (TRUE == ptData->bFree)
    {
        return FAIL;
    }

    CArrayData *pInst = (CArrayData *)(ptData->aucData);
    delete pInst;
    ptData->bFree = TRUE;

    return SUCCESS;
}


template <typename T, WORD32 INST_NUM>
VOID CBaseArray<T, INST_NUM>::Clear()
{
    WORD64       lwAddr = m_lwBegin;
    T_ArrayData *ptData = NULL;
    CArrayData  *pInst  = NULL;

    for (WORD32 dwIndex = 0; dwIndex < INST_NUM; dwIndex++)
    {
        ptData = (T_ArrayData *)(lwAddr);
        pInst  = (CArrayData *)(ptData->aucData);

        if (TRUE != ptData->bFree)
        {
            delete pInst;
            ptData->bFree = TRUE;
        }

        lwAddr += s_dwNodeSize;
    }
}


#endif


