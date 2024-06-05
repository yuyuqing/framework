

#include <assert.h>

#include "base_app_cntrl.h"
#include "base_mem_mgr.h"


CAppCntrl * CAppCntrl::s_pInstance = NULL;


VOID CFactoryApp::Dump()
{
    TRACE_STACK("CFactoryApp::Dump()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE, 
               "m_dwDefNum : %2d\n",
               m_dwDefNum);

    for (WORD32 dwIndex = 0; dwIndex < m_dwDefNum; dwIndex++)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE, 
                   "pCreateFunc : %p, pResetFunc : %p, pDestroyFunc : %p, aucName : %s\n",
                   m_atDefInfo[dwIndex].pCreateFunc,
                   m_atDefInfo[dwIndex].pResetFunc,
                   m_atDefInfo[dwIndex].pDestroyFunc,
                   m_atDefInfo[dwIndex].aucName);
    }
}


CAppCntrl * CAppCntrl::GetInstance(BYTE *pMem)
{
    if (likely(NULL != s_pInstance))
    {
        return s_pInstance;
    }

    if (NULL == pMem)
    {
        return NULL;
    }

    s_pInstance = new (pMem) CAppCntrl();

    return s_pInstance;
}


VOID CAppCntrl::Destroy()
{
    if (NULL != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = NULL;
    }
}


CAppCntrl::CAppCntrl ()
{
    m_dwAppNum = 0;

    memset(m_atAppInfo,     0x00, sizeof(m_atAppInfo));
    memset(m_adwAppThrdTbl, 0xFF, sizeof(m_adwAppThrdTbl));
    memset(m_adwAppClass,   0xFF, sizeof(m_adwAppClass));

    m_pMemInterface = NULL;

    T_MemMetaHead *pMetaHead  = CMemMgr::GetInstance()->GetMetaHead();
    pMetaHead->lwAppCntrlAddr = (WORD64)this;
}


CAppCntrl::~CAppCntrl()
{
    TRACE_STACK("CAppCntrl::~CAppCntrl()");

    for (WORD32 dwIndex = 0; dwIndex < m_dwAppNum; dwIndex++)
    {
        delete (m_atAppInfo[dwIndex].pAppState);

        (*(m_atAppInfo[dwIndex].pDestroyFunc))(m_atAppInfo[dwIndex].pMem);
        m_pMemInterface->Free(m_atAppInfo[dwIndex].pMem);
    }

    m_dwAppNum = 0;

    memset(m_atAppInfo,     0x00, sizeof(m_atAppInfo));
    memset(m_adwAppThrdTbl, 0xFF, sizeof(m_adwAppThrdTbl));
    memset(m_adwAppClass,   0xFF, sizeof(m_adwAppClass));

    m_pMemInterface = NULL;
}


WORD32 CAppCntrl::Initialize(CMemInterface *pMemInterface)
{
    m_pMemInterface = pMemInterface;

    return SUCCESS;
}


T_AppInfo * CAppCntrl::Create(const CHAR   *pName, 
                              WORD32        dwThreadID,
                              T_AppJsonCfg *ptConfig)
{
    TRACE_STACK("CAppCntrl::Create()");

    if ((m_dwAppNum >= MAX_APP_NUM) || (NULL == pName) || (NULL == ptConfig))
    {
        return NULL;
    }

    CFactoryApp *pFactory = CFactoryApp::GetInstance();

    T_ProductDefInfo *ptInfo = pFactory->FindDef(pName);
    if (NULL == ptInfo)
    {
        return NULL;
    }

    T_AppInfo *ptAppInfo = (T_AppInfo *)(Find(ptConfig->dwAppID));
    if (NULL != ptAppInfo)
    {
        return NULL;
    }

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "AppName : %s, ThreadID : %d, AppID : %d, StartEventID : %d, "
               "AssocFlag : %d, AssocNum : %d, [%d, %d, %d, %d]\n",
               pName,
               dwThreadID,
               ptConfig->dwAppID,
               ptConfig->dwEventBegin,
               ptConfig->bAssocFlag,
               ptConfig->dwAssocNum,
               ptConfig->adwAssocID[0], ptConfig->adwAssocID[1],
               ptConfig->adwAssocID[2], ptConfig->adwAssocID[3]);

    WORD32 dwNameLen = strlen(pName);
    if (dwNameLen >= APP_NAME_LEN)
    {
        return NULL;
    }

    CAppInterface *pInst = NULL;

    ptAppInfo = &(m_atAppInfo[m_dwAppNum]);
    memcpy(ptAppInfo->aucName, pName, dwNameLen);

    ptAppInfo->dwAppID        = ptConfig->dwAppID;
    ptAppInfo->dwThreadID     = dwThreadID;
    ptAppInfo->dwStartEventID = ptConfig->dwEventBegin;    
    ptAppInfo->bAssocFlag     = ptConfig->bAssocFlag;
    ptAppInfo->dwAssocNum     = ptConfig->dwAssocNum;
    ptAppInfo->dwMemSize      = ptInfo->dwMemSize;
    ptAppInfo->pAppState      = NULL;
    ptAppInfo->pCreateFunc    = ptInfo->pCreateFunc;
    ptAppInfo->pResetFunc     = ptInfo->pResetFunc;
    ptAppInfo->pDestroyFunc   = ptInfo->pDestroyFunc;

    memcpy((ptAppInfo->adwAssocID),
           (ptConfig->adwAssocID),
           MAX_ASSOCIATE_NUM_PER_APP * sizeof(WORD32));

    ptAppInfo->pMem = m_pMemInterface->Malloc(ptAppInfo->dwMemSize);

    pInst = (CAppInterface *)((*(ptAppInfo->pCreateFunc)) (ptAppInfo->pMem));
    pInst->Initialize();

    ptAppInfo->pAppState = new (ptAppInfo->aucAppState) CAppState(pInst);

    m_dwAppNum++;

    return ptAppInfo;
}


const T_AppInfo * CAppCntrl::Find(WORD32 dwAppID)
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwAppNum; dwIndex++)
    {
        if (m_atAppInfo[dwIndex].dwAppID == dwAppID)
        {
            return &(m_atAppInfo[dwIndex]);
        }
    }

    return NULL;
}


T_AppInfo * CAppCntrl::operator[] (WORD32 dwIndex)
{
    if (unlikely(dwIndex >= m_dwAppNum))
    {
        return NULL;
    }

    return &(m_atAppInfo[dwIndex]);
}


/* 如果dwAssocNum为0, 则表明该类型App为单例; 否则, 表明该类型App为多例
 * 对于多例类型的App, 记录每个App实例关联的小区信息
 */
WORD32 CAppCntrl::RegisterTable(WORD32     dwAppID, 
                                WORD32     dwThreadID,
                                E_AppClass eClass,
                                WORD32     dwAssocNum,
                                WORD32    *pdwAssocID)
{
    if ((dwAppID >= APP_THREAD_TABLE) || (eClass >= E_APP_NUM))
    {
        /* 配置异常 */
        assert(0);
    }

    m_adwAppThrdTbl[dwAppID] = dwThreadID;

    WORD32 dwAssocID = 0;

    for (WORD32 dwIndex = 0; dwIndex < MAX_ASSOCIATE_NUM_PER_APP; dwIndex++)
    {
        if (0 == dwAssocNum)
        {
            m_adwAppClass[eClass][0] = dwAppID;
            break ;
        }
        else
        {
            if (dwIndex >= dwAssocNum)
            {
                break ;
            }

            dwAssocID = pdwAssocID[dwIndex];
            if (dwAssocID >= MAX_ASSOCIATE_NUM_PER_APP)
            {
                assert(0);
            }

            m_adwAppClass[eClass][dwAssocID] = dwAppID;
        }
    }

    return SUCCESS;
}


VOID CAppCntrl::Dump()
{
    TRACE_STACK("CAppCntrl::Dump()");

    CFactoryApp::GetInstance()->Dump();

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE, 
               "m_dwAppNum : %2d\n",
               m_dwAppNum);

    for (WORD32 dwIndex = 0; dwIndex < m_dwAppNum; dwIndex++)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE, 
                   "dwAppID : %3d, dwThreadID : %3d, dwStartEventID : %5d, "
                   "dwMemSize : %9d, aucName : %s\n",
                   m_atAppInfo[dwIndex].dwAppID,
                   m_atAppInfo[dwIndex].dwThreadID,
                   m_atAppInfo[dwIndex].dwStartEventID,
                   m_atAppInfo[dwIndex].dwMemSize,
                   m_atAppInfo[dwIndex].aucName);

        m_atAppInfo[dwIndex].pAppState->Dump();
    }
}


VOID CAppCntrl::Printf()
{
    printf("m_dwAppNum : %d\n", m_dwAppNum);

    for (WORD32 dwIndex = 0; dwIndex < m_dwAppNum; dwIndex++)
    {
        CAppInterface *pAppInst = m_atAppInfo[dwIndex].pAppState->GetAppInst();

        printf("-----------------------------------------------------------\n");

        pAppInst->Printf();

        printf("-----------------------------------------------------------\n");
    }
}


