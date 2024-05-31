

#ifndef _BASE_APP_CNTRL_H_
#define _BASE_APP_CNTRL_H_


#include "base_singleton_tpl.h"
#include "base_mem_interface.h"
#include "base_app_interface.h"


class CAppCntrl : public CSingleton<CAppCntrl>
{
public :
    enum { APP_THREAD_TABLE = 256 };

    /* T必须是CAppInterface的派生类 */
    template <class T>
    static WORD32 DefineApp(const CHAR *pName)
    {
        CAppCntrl *pInstance = CAppCntrl::GetInstance();

        T_AppDefInfo *ptInfo = pInstance->Define(pName);
        if (NULL == ptInfo)
        {
            return FAIL;
        }

        ptInfo->pCreateFunc  = (PCreateApp)(&CDecorateData<T>::Create);
        ptInfo->pResetFunc   = (PResetApp)(&CDecorateData<T>::Reset);
        ptInfo->pDestroyFunc = (PDestroyApp)(&CDecorateData<T>::Destroy);
        ptInfo->dwMemSize    = sizeof(CDecorateData<T>);

        return SUCCESS;
    }

public :
    CAppCntrl ();
    virtual ~CAppCntrl();

    WORD32 Initialize(CMemInterface *pMemInterface);

    T_AppInfo * Create(const CHAR   *pName, 
                       WORD32        dwThreadID,
                       T_AppJsonCfg *ptConfig);

    const T_AppInfo * Find(WORD32 dwAppID);

    T_AppInfo * operator[] (WORD32 dwIndex);

    WORD32 RegisterTable(WORD32     dwAppID, 
                         WORD32     dwThreadID,
                         E_AppClass eClass,
                         WORD32     dwAssocNum,
                         WORD32    *pdwAssocID);

    WORD32 GetAppNum();

    WORD32 GetThreadIDByAppID(WORD32 dwAppID);

    WORD32 GetThreadIDByAppAssoc(E_AppClass eClass, WORD32 dwAssocID = INVALID_DWORD);

    WORD32 GetAppIDByAppClass(E_AppClass eClass, WORD32 dwAssocID = INVALID_DWORD);

    CAppInterface * GetApp(E_AppClass eClass, WORD32 dwAssocID = INVALID_DWORD);

    VOID Dump();

protected :
    T_AppDefInfo * Define(const CHAR *pName);

    T_AppDefInfo * FindDef(const CHAR *pName);

protected :
    WORD32          m_dwDefNum;
    T_AppDefInfo    m_atDefInfo[MAX_APP_NUM];

    WORD32          m_dwAppNum;
    T_AppInfo       m_atAppInfo[MAX_APP_NUM];

    WORD32          m_adwAppThrdTbl[APP_THREAD_TABLE];
    WORD32          m_adwAppClass[E_APP_NUM][MAX_ASSOCIATE_NUM_PER_APP];

    CMemInterface  *m_pMemInterface;
};


inline WORD32 CAppCntrl::GetAppNum()
{
    return m_dwAppNum;
}


inline WORD32 CAppCntrl::GetThreadIDByAppID(WORD32 dwAppID)
{
    if (unlikely(dwAppID >= APP_THREAD_TABLE))
    {
        return INVALID_DWORD;
    }

    return m_adwAppThrdTbl[dwAppID];
}


inline WORD32 CAppCntrl::GetThreadIDByAppAssoc(E_AppClass eClass, WORD32 dwAssocID)
{
    if (INVALID_DWORD == dwAssocID)
    {
        dwAssocID = 0;
    }

    if (unlikely((eClass >= E_APP_NUM) 
              || (dwAssocID >= MAX_ASSOCIATE_NUM_PER_APP)))
    {
        return INVALID_DWORD;
    }

    WORD32 dwAppID = m_adwAppClass[eClass][dwAssocID];

    if (unlikely(dwAppID >= APP_THREAD_TABLE))
    {
        return INVALID_DWORD;
    }

    return m_adwAppThrdTbl[dwAppID];
}


inline WORD32 CAppCntrl::GetAppIDByAppClass(E_AppClass eClass, WORD32 dwAssocID)
{
    if (INVALID_DWORD == dwAssocID)
    {
        dwAssocID = 0;
    }

    if (unlikely((eClass >= E_APP_NUM) 
              || (dwAssocID >= MAX_ASSOCIATE_NUM_PER_APP)))
    {
        return INVALID_DWORD;
    }

    return m_adwAppClass[eClass][dwAssocID];
}


inline CAppInterface * CAppCntrl::GetApp(E_AppClass eClass, WORD32 dwAssocID)
{
    if (INVALID_DWORD == dwAssocID)
    {
        dwAssocID = 0;
    }

    if (unlikely((eClass >= E_APP_NUM) 
              || (dwAssocID >= MAX_ASSOCIATE_NUM_PER_APP)))
    {
        return NULL;
    }

    WORD32 dwAppID = m_adwAppClass[eClass][dwAssocID];

    const T_AppInfo *ptAppInfo = Find(dwAppID);
    if (NULL == ptAppInfo)
    {
        return NULL;
    }

    return ptAppInfo->pAppState->GetAppInst();
}


#define DEFINE_APP(T)    \
    WORD32 __attribute__((used)) __dwApp_##T##_ = CAppCntrl::DefineApp<T>(#T)


#endif


