

#ifndef _BASE_APP_CNTRL_H_
#define _BASE_APP_CNTRL_H_


#include "base_singleton_tpl.h"
#include "base_mem_interface.h"
#include "base_app_interface.h"


class CFactoryApp : public CFactoryTpl<CFactoryApp>
{
public :
    CFactoryApp () {}
    virtual ~CFactoryApp() {}

    VOID Dump();
};


#define DEFINE_APP(V)    \
    WORD32 __attribute__((used)) __dwApp_##V##_ = CFactoryApp::DefineProduct<V>(#V)


class CAppCntrl : public CSingleton<CAppCntrl>, public CBaseData
{
public :
    enum { APP_THREAD_TABLE = 256 };

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

    VOID Printf();

protected :
    WORD32             m_dwAppNum;
    T_AppInfo          m_atAppInfo[MAX_APP_NUM];

    WORD32             m_adwAppThrdTbl[APP_THREAD_TABLE];
    WORD32             m_adwAppClass[E_APP_NUM][MAX_ASSOCIATE_NUM_PER_APP];

    CMemInterface     *m_pMemInterface;
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


#endif


