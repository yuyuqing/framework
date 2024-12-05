

#ifndef _BASE_ALLOCATOR_H_
#define _BASE_ALLOCATOR_H_


#include "base_data.h"


class CAllocInterface : public CBaseData
{
public :
    virtual ~CAllocInterface() {}

    virtual BYTE * Malloc(WORD32 dwSize);
    virtual BYTE * Malloc(WORD32 dwSize, WORD32 dwIndex);
    virtual BYTE * Malloc(WORD32 dwSize, WORD32 dwPoolID, WORD32 dwPoint);
    virtual WORD32 Free(VOID *pAddr);
};


typedef struct tagT_SelfData
{
    VOID * operator new (size_t           size,
                         CAllocInterface *pAllocator)
    {
        WORD32 dwSize    = (WORD32)size;
        WORD64 lwMemPool = (WORD64)pAllocator;

        BYTE *pMem = pAllocator->Malloc(dwSize);
        if (NULL == pMem)
        {
            return NULL;
        }

        WORD64 *plwAllocAddr = (WORD64 *)(pMem - sizeof(WORD64));
        assert((*plwAllocAddr) == lwMemPool);

        return (VOID *)pMem;
    }

    VOID * operator new (size_t           size,
                         CAllocInterface *pAllocator,
                         WORD32           dwPoolID,
                         WORD32           dwPoint)
    {
        WORD32 dwSize    = (WORD32)size;
        WORD64 lwMemPool = (WORD64)pAllocator;

        BYTE *pMem = pAllocator->Malloc(dwSize, dwPoolID, dwPoint);
        if (NULL == pMem)
        {
            return NULL;
        }

        WORD64 *plwAllocAddr = (WORD64 *)(pMem - sizeof(WORD64));
        assert((*plwAllocAddr) == lwMemPool);

        return (VOID *)pMem;
    }

    VOID operator delete (VOID *ptr)
    {
        WORD64 *plwAllocAddr = (WORD64 *)(((BYTE *)ptr) - sizeof(WORD64));

        CAllocInterface *pAllocator = (CAllocInterface *)(*plwAllocAddr);

        pAllocator->Free(ptr);
    }

private :
    VOID * operator new (size_t size)
    {
        return NULL;
    }
}T_SelfData;


template <typename T>
class CSelfData
{
public :
    const static WORD32 s_dwDataSize = sizeof(T);

public :
    VOID * operator new (size_t           size,
                         CAllocInterface *pAllocator)
    {
        if (unlikely(size > s_dwDataSize))
        {
            return NULL;
        }

        WORD64  lwMemPool = (WORD64)pAllocator;
        BYTE   *pMem      = pAllocator->Malloc(s_dwDataSize);
        if (NULL == pMem)
        {
            return NULL;
        }

        WORD64 *plwAllocAddr = (WORD64 *)(pMem - sizeof(WORD64));
        assert((*plwAllocAddr) == lwMemPool);

        return (VOID *)pMem;
    }

    VOID * operator new (size_t           size,
                         CAllocInterface *pAllocator,
                         WORD32           dwIndex)
    {
        if (unlikely(size > s_dwDataSize))
        {
            return NULL;
        }

        WORD64  lwMemPool = (WORD64)pAllocator;
        BYTE    *pMem     = pAllocator->Malloc(s_dwDataSize, dwIndex);
        if (NULL == pMem)
        {
            return NULL;
        }

        WORD64 *plwAllocAddr = (WORD64 *)(pMem - sizeof(WORD64));
        assert((*plwAllocAddr) == lwMemPool);

        return (VOID *)pMem;
    }

    VOID * operator new (size_t           size,
                         CAllocInterface *pAllocator,
                         WORD32           dwPoolID,
                         WORD32           dwPoint)
    {
        if (unlikely(size > s_dwDataSize))
        {
            return NULL;
        }

        WORD64  lwMemPool = (WORD64)pAllocator;
        BYTE    *pMem     = pAllocator->Malloc(s_dwDataSize, dwPoolID, dwPoint);
        if (NULL == pMem)
        {
            return NULL;
        }

        WORD64 *plwAllocAddr = (WORD64 *)(pMem - sizeof(WORD64));
        assert((*plwAllocAddr) == lwMemPool);

        return (VOID *)pMem;
    }

    VOID operator delete (VOID *ptr)
    {
        WORD64 *plwAllocAddr = (WORD64 *)(((BYTE *)ptr) - sizeof(WORD64));

        CAllocInterface *pAllocator = (CAllocInterface *)(*plwAllocAddr);

        pAllocator->Free(ptr);
    }

    CSelfData ()
    {
        m_pSelf = static_cast<T *>(this);
    }

    virtual ~CSelfData()
    {
        m_pSelf = NULL;
    }

private :
    VOID * operator new (size_t size)
    {
        return NULL;
    }

protected :
    CSelfData (CSelfData &) = delete;
    CSelfData (const CSelfData &) = delete;
    CSelfData & operator= (CSelfData &) = delete;
    CSelfData & operator= (const CSelfData &) = delete;

protected :
    T    *m_pSelf;
};


#endif


