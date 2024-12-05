

#ifndef _BASE_FACTORY_TPL_H_
#define _BASE_FACTORY_TPL_H_


#include "base_data.h"


#define PRODUCT_NAME_LEN                 ((WORD32)(32))
#define PRODUCT_NUM                      ((WORD32)(64))


/* V : 具体的产品类, 必须是CCBObject派生类 */
template <typename V>
class CProductTpl : public CBaseData
{
public :
    CProductTpl () {}
    virtual ~CProductTpl() {}

    template <typename P>
    CProductTpl (const P *pParam)
        : m_cValue(*pParam)
    {
    }

    operator V& ()
    {
        return m_cValue;
    }

    operator V* ()
    {
        return &m_cValue;
    }

protected :
    V   m_cValue;
};


using PCreateProduct  = VOID * (*) (BYTE *pMem, ...);
using PResetProduct   = VOID * (*) (BYTE *pMem, ...);
using PDestroyProduct = VOID (*) (BYTE *pMem);


typedef struct tagT_ProductDefInfo
{
    CHAR             aucName[PRODUCT_NAME_LEN];
    WORD32           dwMemSize;
    PCreateProduct   pCreateFunc;
    PResetProduct    pResetFunc;
    PDestroyProduct  pDestroyFunc;
}T_ProductDefInfo;


/* T : 具体的工厂类(CFactoryApp/CFactoryThread) */
template <typename T>
class CFactoryTpl
{
public :
    static T * GetInstance()
    {
        if (NULL != s_pInstance)
        {
            return s_pInstance;
        }

        s_pInstance = new T();

        return s_pInstance;
    }

    static VOID Destroy()
    {
        if (NULL != s_pInstance)
        {
            delete s_pInstance;
            s_pInstance = NULL;
        }
    }

    template <typename V>
    static V * CreateProduct(BYTE *pMem)
    {
        memset(pMem, 0x00, sizeof(CProductTpl<V>));
        CProductTpl<V> *pValue = new (pMem) CProductTpl<V>();

        return (*pValue);
    }

    template <typename V, typename P>
    static V * CreateProduct(BYTE *pMem, const P *pParam)
    {
        memset(pMem, 0x00, sizeof(CProductTpl<V>));
        CProductTpl<V> *pValue = new (pMem) CProductTpl<V>(pParam);

        return (*pValue);
    }

    template <typename V>
    static V * ResetProduct(BYTE *pMem)
    {
        CProductTpl<V> *pValue = (CProductTpl<V> *)(pMem);
        delete pValue;

        memset(pMem, 0x00, sizeof(CProductTpl<V>));
        pValue = new (pMem) CProductTpl<V>();

        return (*pValue);
    }

    template <typename V, typename P>
    static V * ResetProduct(BYTE *pMem, const P *pParam)
    {
        CProductTpl<V> *pValue = (CProductTpl<V> *)(pMem);
        delete pValue;

        memset(pMem, 0x00, sizeof(CProductTpl<V>));
        pValue = new (pMem) CProductTpl<V>(pParam);

        return (*pValue);
    }

    template <typename V>
    static VOID DestroyProduct(BYTE *pMem)
    {
        CProductTpl<V> *pValue = (CProductTpl<V> *)(pMem);
        delete pValue;
    }

    /* V : 具体的产品类(CAppXXX/CThreadXXX) */
    template <typename V>
    static WORD32 DefineProduct(const CHAR *pName)
    {
        T *pFactory = CFactoryTpl<T>::GetInstance();

        T_ProductDefInfo *ptInfo = pFactory->Define(pName);
        if (NULL == ptInfo)
        {
            return FAIL;
        }

        ptInfo->dwMemSize    = sizeof(CProductTpl<V>);
        ptInfo->pCreateFunc  = (PCreateProduct)(&CFactoryTpl<T>::CreateProduct<V>);
        ptInfo->pResetFunc   = (PResetProduct)(&CFactoryTpl<T>::ResetProduct<V>);
        ptInfo->pDestroyFunc = (PDestroyProduct)(&CFactoryTpl<T>::DestroyProduct<V>);

        return SUCCESS;
    }

    /* V : 具体的产品类(CAppXXX/CThreadXXX) */
    template <typename V, typename P>
    static WORD32 DefineProduct(const CHAR *pName)
    {
        T *pFactory = CFactoryTpl<T>::GetInstance();

        T_ProductDefInfo *ptInfo = pFactory->Define(pName);
        if (NULL == ptInfo)
        {
            return FAIL;
        }

        ptInfo->dwMemSize    = sizeof(CProductTpl<V>);
        ptInfo->pCreateFunc  = (PCreateProduct)(&CFactoryTpl<T>::CreateProduct<V, P>);
        ptInfo->pResetFunc   = (PResetProduct)(&CFactoryTpl<T>::ResetProduct<V, P>);
        ptInfo->pDestroyFunc = (PDestroyProduct)(&CFactoryTpl<T>::DestroyProduct<V>);

        return SUCCESS;
    }

    T_ProductDefInfo * FindDef(const CHAR *pName);

protected :
    T_ProductDefInfo * Define(const CHAR *pName);

    CFactoryTpl () 
    {
        m_dwDefNum = 0;
        memset(m_atDefInfo, 0x00, sizeof(m_atDefInfo));
    }

    virtual ~CFactoryTpl() {}

    CFactoryTpl (CFactoryTpl &) = delete;
    CFactoryTpl (const CFactoryTpl &) = delete;
    CFactoryTpl& operator= (const CFactoryTpl &) = delete;

protected :
    WORD32              m_dwDefNum;
    T_ProductDefInfo    m_atDefInfo[PRODUCT_NUM];

protected :
    static T *s_pInstance;
};


template <typename T>
T * CFactoryTpl<T>::s_pInstance = NULL;


template <typename T>
T_ProductDefInfo * CFactoryTpl<T>::FindDef(const CHAR *pName)
{
    if (NULL == pName)
    {
        return NULL;
    }

    WORD32 dwLen = strlen(pName);

    T_ProductDefInfo *ptInfo = NULL;

    for (WORD32 dwIndex = 0; dwIndex < m_dwDefNum; dwIndex++)
    {
        ptInfo = &(m_atDefInfo[dwIndex]);

        if (dwLen != strlen(ptInfo->aucName))
        {
            continue ;
        }

        if (0 == memcmp(pName, ptInfo->aucName, dwLen))
        {
            return ptInfo;
        }
    }

    return NULL;
}


template <typename T>
T_ProductDefInfo * CFactoryTpl<T>::Define(const CHAR *pName)
{
    if ((m_dwDefNum >= PRODUCT_NUM) || (NULL == pName))
    {
        return NULL;
    }

    WORD32 dwNameLen = strlen(pName);
    if (dwNameLen >= PRODUCT_NAME_LEN)
    {
        return NULL;
    }

    T_ProductDefInfo *ptInfo = &(m_atDefInfo[m_dwDefNum]);

    memcpy(ptInfo->aucName, pName, dwNameLen);

    m_dwDefNum++;

    return ptInfo;
}


#endif


