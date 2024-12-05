

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


/* ��ThreadID��С�������򴴽��߳�ʵ��(����˳����������˳��) */
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
                   "pCreateFunc : %p, pResetFunc : %p, "
                   "pDestroyFunc : %p, aucName : %s\n",
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


/* ��ThreadID��С�������򴴽��߳�ʵ��(����˳����������˳��) */
WORD32 CThreadCntrl::Initialize(CCentralMemPool *pMemInterface, WORD32 dwProcID)
{
    m_pMemInterface = pMemInterface;

    FetchJsonConfig();

    /* ��С�������� */
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


/* ����ά��(�ڸ������������ӡ��Ϣ) */
VOID CThreadCntrl::Printf()
{
    printf("m_dwThreadNum : %d\n", m_dwThreadNum);

    E_ThreadClass   eThrdClass  = E_THREAD_INVALID;
    CBaseThread    *pBaseThread = NULL;
    CPollingThread *pPollThread = NULL;
    CRTThread      *pRTThread   = NULL;
    CWorkThread    *pWorkThread = NULL;
    CMsgMemPool    *pMsgMemPool = NULL;
    T_MsgMemMeasure tMsgMemMeas;
    T_RingHeadTail  tProd;
    T_RingHeadTail  tCons;    
    WORD32          dwRingNum = 0;
    T_RingHeadTail  atProd[PACKET_RING_NUM];
    T_RingHeadTail  atCons[PACKET_RING_NUM];

    for (WORD32 dwIndex = 0; dwIndex < m_dwThreadNum; dwIndex++)
    {
        memset(&tMsgMemMeas, 0x00, sizeof(tMsgMemMeas));

        printf("===========================================================\n");

        printf("dwThreadID : %3d, dwLogicalID : %2d, dwPolicy : %d, "
               "dwPriority : %2d, dwMemSize : %9d, dwAppNum : %2d, "
               "Addr : %lu, Name : %s\n",
               m_atThreadInfo[dwIndex].dwThreadID,
               m_atThreadInfo[dwIndex].dwLogicalID,
               m_atThreadInfo[dwIndex].dwPolicy,
               m_atThreadInfo[dwIndex].dwPriority,
               m_atThreadInfo[dwIndex].dwMemSize,
               m_atThreadInfo[dwIndex].dwAppNum,
               (WORD64)(m_atThreadInfo[dwIndex].pWorker),
               m_atThreadInfo[dwIndex].aucName);

        pBaseThread = m_atThreadInfo[dwIndex].pWorker;
        eThrdClass  = pBaseThread->GetThreadClass();
        pMsgMemPool = pBaseThread->GetMsgMemPool();

        pMsgMemPool->GetMeasure(tMsgMemMeas);
        for (WORD32 dwIndex1 = 0; dwIndex1 < tMsgMemMeas.dwTypeNum; dwIndex1++)
        {
            printf("MsgMemPool[type : %d, Used : %15lu, Free : %15lu]\n",
                   dwIndex1,
                   tMsgMemMeas.alwUsedCount[dwIndex1],
                   tMsgMemMeas.alwFreeCount[dwIndex1]);
        }

        switch (eThrdClass)
        {
        case E_THREAD_POLLING :
            {
                pPollThread = (CPollingThread *)pBaseThread;

                CMessageRing   &rRingH = (pPollThread->m_MsgRingH);
                CMessageRing   &rRingL = (pPollThread->m_MsgRingL);
                CDataPlaneRing &rRingD = (pPollThread->m_MsgRingD);

                rRingH.SnapShot(tProd, tCons);
                
                printf("HRing      : Prod[%10u, %10u] Cons[%10u, %10u]\n",
                       tProd.dwHead, tProd.dwTail,
                       tCons.dwHead, tCons.dwTail);

                rRingL.SnapShot(tProd, tCons);

                printf("LRing      : Prod[%10u, %10u] Cons[%10u, %10u]\n",
                       tProd.dwHead, tProd.dwTail,
                       tCons.dwHead, tCons.dwTail);

                rRingD.SnapShot(dwRingNum, atProd, atCons);

                for (WORD32 dwIndex1 = 0; dwIndex1 < dwRingNum; dwIndex1++)
                {
                    printf("DRingID[%d] : Prod[%10u, %10u] Cons[%10u, %10u]\n",
                           dwIndex1,
                           atProd[dwIndex1].dwHead,
                           atProd[dwIndex1].dwTail,
                           atCons[dwIndex1].dwHead,
                           atCons[dwIndex1].dwTail);
                }
            }
            break ;

        case E_THREAD_RT :
            {
                pRTThread = (CRTThread *)pBaseThread;

                CMessageRing   &rRingH = (pRTThread->m_MsgRingH);
                CMessageRing   &rRingL = (pRTThread->m_MsgRingL);
                CDataPlaneRing &rRingD = (pRTThread->m_MsgRingD);

                rRingH.SnapShot(tProd, tCons);

                printf("HRing      : Prod[%10u, %10u] Cons[%10u, %10u]\n",
                       tProd.dwHead, tProd.dwTail,
                       tCons.dwHead, tCons.dwTail);

                rRingL.SnapShot(tProd, tCons);

                printf("LRing      : Prod[%10u, %10u] Cons[%10u, %10u]\n",
                       tProd.dwHead, tProd.dwTail,
                       tCons.dwHead, tCons.dwTail);

                rRingD.SnapShot(dwRingNum, atProd, atCons);

                for (WORD32 dwIndex1 = 0; dwIndex1 < dwRingNum; dwIndex1++)
                {
                    printf("DRingID[%d] : Prod[%10u, %10u] Cons[%10u, %10u]\n",
                           dwIndex1,
                           atProd[dwIndex1].dwHead,
                           atProd[dwIndex1].dwTail,
                           atCons[dwIndex1].dwHead,
                           atCons[dwIndex1].dwTail);
                }
            }
            break ;

        case E_THREAD_WORK :
            {
                pWorkThread = (CWorkThread *)pBaseThread;

                CMessageRing   &rRingH = (pWorkThread->m_MsgRingH);
                CMessageRing   &rRingL = (pWorkThread->m_MsgRingL);
                CDataPlaneRing &rRingD = (pWorkThread->m_MsgRingD);

                rRingH.SnapShot(tProd, tCons);

                printf("HRing      : Prod[%10u, %10u] Cons[%10u, %10u]\n",
                       tProd.dwHead, tProd.dwTail,
                       tCons.dwHead, tCons.dwTail);

                rRingL.SnapShot(tProd, tCons);

                printf("LRing      : Prod[%10u, %10u] Cons[%10u, %10u]\n",
                       tProd.dwHead, tProd.dwTail,
                       tCons.dwHead, tCons.dwTail);

                rRingD.SnapShot(dwRingNum, atProd, atCons);

                for (WORD32 dwIndex1 = 0; dwIndex1 < dwRingNum; dwIndex1++)
                {
                    printf("DRingID[%d] : Prod[%10u, %10u] Cons[%10u, %10u]\n",
                           dwIndex1,
                           atProd[dwIndex1].dwHead,
                           atProd[dwIndex1].dwTail,
                           atCons[dwIndex1].dwHead,
                           atCons[dwIndex1].dwTail);
                }
            }
            break ;

        default :
            break ;
        }

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
            /* ���ô��� */
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
            /* �̴߳���ʧ�� */
            assert(0);
        }

        /* �߳����� */
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
        /* ���ô��� */
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


