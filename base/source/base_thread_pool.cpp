

#include "base_sort.h"
#include "base_thread_pool.h"
#include "base_thread_multi.h"
#include "base_init_component.h"


DEFINE_THREAD(CPollingThread);
DEFINE_THREAD(CRTThread);
DEFINE_THREAD(CWorkThread);
DEFINE_THREAD(CMultiThread);


WORD32 ExitThreadCtrl(VOID *pArg)
{    
    CThreadCntrl::Destroy();

    return SUCCESS;
}


/* 按ThreadID从小到大排序创建线程实例(启动顺序依赖创建顺序) */
WORD32 InitThreadCtrl(WORD32 dwProcID, VOID *pArg)
{
    TRACE_STACK("InitThreadCtrl()");

    T_InitFunc *ptInitFunc = (T_InitFunc *)pArg;
    ptInitFunc->pExitFunc  = &ExitThreadCtrl;

    CCentralMemPool *pCentralMemPool = ptInitFunc->pMemInterface;

    BYTE *pMem = pCentralMemPool->Malloc(sizeof(CThreadCntrl));
    if (NULL == pMem)
    {
        assert(0);
    }

    CThreadCntrl *pThreadCntrl = CThreadCntrl::GetInstance(pMem);

    pThreadCntrl->Initialize(ptInitFunc->pMemInterface, dwProcID);
    pThreadCntrl->StartThreads();

    pThreadCntrl->Dump();

    return SUCCESS;
}
INIT_EXPORT(InitThreadCtrl, 9);


VOID CFactoryThread::Dump()
{
    TRACE_STACK("CFactoryThread::Dump()");

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


CThreadCntrl::CThreadCntrl ()
{
    g_pThreadPool   = this;
    m_pMemInterface = NULL;
    m_dwThreadNum   = 0;

    memset((BYTE *)(&(m_atThreadInfo[0])), 0x00, sizeof(m_atThreadInfo));

    T_MemMetaHead *pMetaHead     = CMemMgr::GetInstance()->GetMetaHead();
    pMetaHead->lwThreadCntrlAddr = (WORD64)this;
}


CThreadCntrl::~CThreadCntrl()
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwThreadNum; dwIndex++)
    {
        if (NULL != m_atThreadInfo[dwIndex].pWorker)
        {
            (*(m_atThreadInfo[dwIndex].pDestroyFunc)) (m_atThreadInfo[dwIndex].pMem);
            m_pMemInterface->Free(m_atThreadInfo[dwIndex].pMem);

            m_atThreadInfo[dwIndex].pMem    = NULL;
            m_atThreadInfo[dwIndex].pWorker = NULL;
        }
    }

    g_pThreadPool   = NULL;
    m_pMemInterface = NULL;
    m_dwThreadNum   = 0;
    memset((BYTE *)(&(m_atThreadInfo[0])), 0x00, sizeof(m_atThreadInfo));
}


/* 按ThreadID从小到大排序创建线程实例(启动顺序依赖创建顺序) */
WORD32 CThreadCntrl::Initialize(CCentralMemPool *pMemInterface, WORD32 dwProcID)
{
    m_pMemInterface = pMemInterface;

    FetchJsonConfig();

    /* 从小到大排序 */
    HeapSort<T_ThreadInfo, WORD32> (m_atThreadInfo, m_dwThreadNum,
                                    (&T_ThreadInfo::dwThreadID),
                                    (PComparePFunc<T_ThreadInfo, WORD32>)(& GreaterV<T_ThreadInfo, WORD32>));

    for (WORD32 dwIndex = 0; dwIndex < m_dwThreadNum; dwIndex++)
    {
        m_atThreadInfo[dwIndex].pMem = m_pMemInterface->Malloc(m_atThreadInfo[dwIndex].dwMemSize);

        T_ThreadParam tParam;
        tParam.dwProcID      = dwProcID;
        tParam.dwThreadID    = m_atThreadInfo[dwIndex].dwThreadID;
        tParam.dwLogicalID   = m_atThreadInfo[dwIndex].dwLogicalID;
        tParam.dwPolicy      = m_atThreadInfo[dwIndex].dwPolicy;
        tParam.dwPriority    = m_atThreadInfo[dwIndex].dwPriority;
        tParam.dwStackSize   = m_atThreadInfo[dwIndex].dwStackSize;
        tParam.dwCBNum       = m_atThreadInfo[dwIndex].dwCBNum;
        tParam.dwPacketCBNum = m_atThreadInfo[dwIndex].dwPacketCBNum;
        tParam.dwMultiCBNum  = m_atThreadInfo[dwIndex].dwMultiCBNum;
        tParam.dwTimerThresh = m_atThreadInfo[dwIndex].dwTimerThresh;
        tParam.bAloneLog     = m_atThreadInfo[dwIndex].bAloneLog;
        tParam.pMemPool      = m_pMemInterface;

        m_atThreadInfo[dwIndex].pWorker = (CBaseThread *)((*(m_atThreadInfo[dwIndex].pCreateFunc)) (m_atThreadInfo[dwIndex].pMem, &tParam));

        m_atThreadInfo[dwIndex].pWorker->Initialize();

        LoadApp(m_atThreadInfo[dwIndex]);
    }

    return SUCCESS;
}


WORD32 CThreadCntrl::StartThreads()
{
    TRACE_STACK("CThreadCntrl::StartThreads()");

    for (WORD32 dwIndex = 0; dwIndex < m_dwThreadNum; dwIndex++)
    {
        CBaseThread *pWorker = m_atThreadInfo[dwIndex].pWorker;
        if (NULL == pWorker)
        {
            continue ;
        }

        pWorker->GenerateThrdName(m_atThreadInfo[dwIndex].aucName);
        pWorker->Start();
    }

    return SUCCESS;
}


WORD32 CThreadCntrl::JoinThreads()
{
    TRACE_STACK("CThreadCntrl::JoinThreads()");

    for (WORD32 dwIndex = 0; dwIndex < m_dwThreadNum; dwIndex++)
    {
        CBaseThread *pWorker = m_atThreadInfo[dwIndex].pWorker;
        if (NULL == pWorker)
        {
            continue ;
        }

        pWorker->Join();
    }
    
    return SUCCESS;
}


VOID CThreadCntrl::Dump()
{
    TRACE_STACK("CThreadCntrl::Dump()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "m_dwThreadNum : %d\n",
               m_dwThreadNum);

    for (WORD32 dwIndex = 0; dwIndex < m_dwThreadNum; dwIndex++)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                   "dwThreadID : %3d, dwLogicalID : %2d, dwPolicy : %d, "
                   "dwPriority : %2d, dwMemSize : %9d, dwAppNum : %2d\n",
                   m_atThreadInfo[dwIndex].dwThreadID,
                   m_atThreadInfo[dwIndex].dwLogicalID,
                   m_atThreadInfo[dwIndex].dwPolicy,
                   m_atThreadInfo[dwIndex].dwPriority,
                   m_atThreadInfo[dwIndex].dwMemSize,
                   m_atThreadInfo[dwIndex].dwAppNum);

        m_atThreadInfo[dwIndex].pWorker->Dump();
    }
}


/* 用于维测(在辅进程中输出打印信息) */
VOID CThreadCntrl::Printf()
{
    printf("m_dwThreadNum : %d\n", m_dwThreadNum);

    for (WORD32 dwIndex = 0; dwIndex < m_dwThreadNum; dwIndex++)
    {
        printf("===========================================================\n");

        printf("dwThreadID : %3d, dwLogicalID : %2d, dwPolicy : %d, "
               "dwPriority : %2d, dwMemSize : %9d, dwAppNum : %2d\n",
               m_atThreadInfo[dwIndex].dwThreadID,
               m_atThreadInfo[dwIndex].dwLogicalID,
               m_atThreadInfo[dwIndex].dwPolicy,
               m_atThreadInfo[dwIndex].dwPriority,
               m_atThreadInfo[dwIndex].dwMemSize,
               m_atThreadInfo[dwIndex].dwAppNum);

        m_atThreadInfo[dwIndex].pWorker->Printf();

        printf("===========================================================\n");
    }
}


WORD32 CThreadCntrl::FetchJsonConfig()
{
    T_ThreadPoolJsonCfg &rJsonCfg = CBaseConfigFile::GetInstance()->GetWorkerJsonCfg();
    T_ThreadJsonCfg     *ptWorker = NULL;

    T_ProductDefInfo *ptDefInfo    = NULL;
    T_ThreadInfo     *ptThreadInfo = NULL;

    CFactoryThread *pFactory = CFactoryThread::GetInstance();

    for (WORD32 dwIndex = 0; dwIndex < rJsonCfg.dwWorkerNum; dwIndex++)
    {
        ptWorker = &(rJsonCfg.atWorker[dwIndex]);

        ptThreadInfo = FindThreadInfo(ptWorker->dwThreadID);
        if (NULL != ptThreadInfo)
        {
            continue ;
        }

        ptDefInfo = pFactory->FindDef(ptWorker->aucType);
        if (NULL == ptDefInfo)
        {
            /* 配置错误 */
            assert(0);
        }

        ptThreadInfo = CreateInfo(ptWorker->dwThreadID,
                                  ptWorker->dwLogicalID,
                                  ptWorker->dwPolicy,
                                  ptWorker->dwPriority,
                                  ptWorker->dwStackSize,
                                  ptWorker->dwCBNum,
                                  ptWorker->dwPacketCBNum,
                                  ptWorker->dwMultiCBNum,
                                  ptWorker->dwTimerThresh,
                                  ptWorker->bAloneLog,
                                  ptDefInfo);
        if (NULL == ptThreadInfo)
        {
            /* 线程创建失败 */
            assert(0);
        }

        /* 线程名称 */
        memcpy(ptThreadInfo->aucName, ptWorker->aucName, WORKER_NAME_LEN);

        ptThreadInfo->dwAppNum = ptWorker->dwAppNum;

        for (WORD32 dwIndex1 = 0; dwIndex1 < ptThreadInfo->dwAppNum; dwIndex1++)
        {
            ptThreadInfo->atApp[dwIndex1].dwAppID      = ptWorker->atApp[dwIndex1].dwAppID;
            ptThreadInfo->atApp[dwIndex1].dwEventBegin = ptWorker->atApp[dwIndex1].dwEventBegin;
            ptThreadInfo->atApp[dwIndex1].bAssocFlag   = ptWorker->atApp[dwIndex1].bAssocFlag;
            ptThreadInfo->atApp[dwIndex1].dwAssocNum   = ptWorker->atApp[dwIndex1].dwAssocNum;

            memcpy((ptThreadInfo->atApp[dwIndex1].aucName),
                   (ptWorker->atApp[dwIndex1].aucName),
                   APP_NAME_LEN);
            memcpy((ptThreadInfo->atApp[dwIndex1].adwAssocID),
                   (ptWorker->atApp[dwIndex1].adwAssocID),
                   MAX_ASSOCIATE_NUM_PER_APP * sizeof(WORD32));
        }
    }

    return SUCCESS;
}


T_ThreadInfo * CThreadCntrl::CreateInfo(WORD32            dwThreadID,
                                        WORD32            dwLogicalID,
                                        WORD32            dwPolicy,
                                        WORD32            dwPriority,
                                        WORD32            dwStakcSize,
                                        WORD32            dwCBNum,
                                        WORD32            dwPacketCBNum,
                                        WORD32            dwMultiCBNum,
                                        WORD32            dwTimerThresh,
                                        BOOL              bAloneLog,
                                        T_ProductDefInfo *ptDefInfo)
{
    if ((NULL == ptDefInfo) || (m_dwThreadNum >= MAX_WORKER_NUM))
    {
        return NULL;
    }

    T_ThreadInfo *ptThreadInfo = FindThreadInfo(dwThreadID);
    if (NULL != ptThreadInfo)
    {
        /* 配置错误 */
        assert(0);
    }

    ptThreadInfo = &(m_atThreadInfo[m_dwThreadNum]);

    ptThreadInfo->dwThreadID    = dwThreadID;
    ptThreadInfo->dwLogicalID   = dwLogicalID;
    ptThreadInfo->dwPolicy      = dwPolicy;
    ptThreadInfo->dwPriority    = dwPriority;
    ptThreadInfo->dwStackSize   = dwStakcSize;
    ptThreadInfo->dwCBNum       = dwCBNum;
    ptThreadInfo->dwPacketCBNum = dwPacketCBNum;
    ptThreadInfo->dwMultiCBNum  = dwMultiCBNum;
    ptThreadInfo->dwTimerThresh = dwTimerThresh;
    ptThreadInfo->bAloneLog     = bAloneLog;
    ptThreadInfo->pCreateFunc   = ptDefInfo->pCreateFunc;
    ptThreadInfo->pDestroyFunc  = ptDefInfo->pDestroyFunc;
    ptThreadInfo->pWorker       = NULL;
    ptThreadInfo->dwMemSize     = ptDefInfo->dwMemSize;
    ptThreadInfo->dwAppNum      = 0;
    ptThreadInfo->pMem          = NULL;

    m_dwThreadNum++;

    return ptThreadInfo;
}


WORD32 CThreadCntrl::LoadApp(T_ThreadInfo &rtThread)
{
    TRACE_STACK("CThreadCntrl::LoadApp()");

    CBaseThread   *pWorker   = rtThread.pWorker;
    T_AppJsonCfg  *ptAppCfg  = NULL;
    T_AppInfo     *ptAppInfo = NULL;
    CAppInterface *pAppInst  = NULL;

    for (WORD32 dwIndex = 0; dwIndex < rtThread.dwAppNum; dwIndex++)
    {
        ptAppCfg  = &(rtThread.atApp[dwIndex]);
        ptAppInfo = g_pAppCntrl->Create(ptAppCfg->aucName,
                                        rtThread.dwThreadID,
                                        ptAppCfg);
        if (NULL == ptAppInfo)
        {
            continue ;
        }

        pAppInst = ptAppInfo->pAppState->GetAppInst();

        pAppInst->PreInit(pWorker, ptAppInfo);
        pWorker->LoadApp(ptAppInfo);
    }

    return SUCCESS;
}


