

#include <assert.h>

#include "base_app_cntrl.h"


CAppCntrl::CAppCntrl ()
{
    m_dwDefNum = 0;
    m_dwAppNum = 0;

    memset(m_atDefInfo,     0x00, sizeof(m_atDefInfo));
    memset(m_atAppInfo,     0x00, sizeof(m_atAppInfo));
    memset(m_adwAppThrdTbl, 0xFF, sizeof(m_adwAppThrdTbl));
    memset(m_adwAppClass,   0xFF, sizeof(m_adwAppClass));

    m_pMemInterface = NULL;
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

    m_dwDefNum = 0;
    m_dwAppNum = 0;

    memset(m_atDefInfo,     0x00, sizeof(m_atDefInfo));
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

    T_AppDefInfo *ptInfo = FindDef(pName);
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

    pInst = (*(ptAppInfo->pCreateFunc)) (ptAppInfo->pMem);
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


/* ���dwAssocNumΪ0, �����������AppΪ����; ����, ����������AppΪ����
 * ���ڶ������͵�App, ��¼ÿ��Appʵ��������С����Ϣ
 */
WORD32 CAppCntrl::RegisterTable(WORD32     dwAppID, 
                                WORD32     dwThreadID,
                                E_AppClass eClass,
                                WORD32     dwAssocNum,
                                WORD32    *pdwAssocID)
{
    if ((dwAppID >= APP_THREAD_TABLE) || (eClass >= E_APP_NUM))
    {
        /* �����쳣 */
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

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE, 
               "m_dwDefNum : %2d, m_dwAppNum : %2d\n",
               m_dwDefNum,
               m_dwAppNum);

    for (WORD32 dwIndex = 0; dwIndex < m_dwDefNum; dwIndex++)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE, 
                   "pCreateFunc : %p, pResetFunc : %p, pDestroyFunc : %p, aucName : %s\n",
                   m_atDefInfo[dwIndex].pCreateFunc,
                   m_atDefInfo[dwIndex].pResetFunc,
                   m_atDefInfo[dwIndex].pDestroyFunc,
                   m_atDefInfo[dwIndex].aucName);
    }

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


T_AppDefInfo * CAppCntrl::Define(const CHAR *pName)
{
    if ((m_dwDefNum >= MAX_APP_NUM) || (NULL == pName))
    {
        return NULL;
    }

    WORD32 dwNameLen = MIN(strlen(pName), (APP_NAME_LEN - 1));

    T_AppDefInfo *ptInfo = &(m_atDefInfo[m_dwDefNum]);

    memcpy(ptInfo->aucName, pName, dwNameLen);

    m_dwDefNum++;

    return ptInfo;
}


T_AppDefInfo * CAppCntrl::FindDef(const CHAR *pName)
{
    if (NULL == pName)
    {
        return NULL;
    }

    WORD32 dwLen = strlen(pName);

    T_AppDefInfo *ptInfo = NULL;

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


