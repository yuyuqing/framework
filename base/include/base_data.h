

#ifndef _BASE_DATA_H_
#define _BASE_DATA_H_


#include <string.h>
#include <stddef.h>
#include <assert.h>

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
#define HALF_PAGE_SIZE        ((WORD32)(2048))
#define QUATRA_PAGE_SIZE      ((WORD32)(16384))
#define OCTA_PAGE_SIZE        ((WORD32)(32768))


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


#endif


