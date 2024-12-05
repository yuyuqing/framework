

#include "base_shm_mgr.h"
#include "base_init_component.h"
#include "base_log.h"


CShmMgr * CShmMgr::s_pInstance = NULL;


WORD32 ExitShmMgr(VOID *pArg)
{
    CShmMgr::Destroy();

    g_pShmMgr = NULL;

    return SUCCESS;
}


WORD32 InitShmMgr(WORD32 dwProcID, VOID *pArg)
{
    TRACE_STACK("InitShmMgr()");

    T_InitFunc *ptInitFunc = (T_InitFunc *)pArg;
    ptInitFunc->pExitFunc  = &ExitShmMgr;

    T_ShmJsonCfg &rJsonCfg = CBaseConfigFile::GetInstance()->GetShmJsonCfg();
    if (FALSE == rJsonCfg.bCreateFlag)
    {
        return SUCCESS;
    }

    g_pShmMgr = CShmMgr::CreateShmMgr(dwProcID, rJsonCfg);
    if (NULL == g_pShmMgr)
    {
        assert(0);
    }

    return SUCCESS;
}
INIT_EXPORT(InitShmMgr, 1);


CShmMgr * CShmMgr::CreateShmMgr(WORD32 dwProcID, T_ShmJsonCfg &rtParam)
{
    if (NULL != s_pInstance)
    {
        return s_pInstance;
    }

    WORD32 dwRole = rtParam.dwRole;
    if (dwRole > E_SHM_OBSERVER)
    {
        assert(0);
    }

    T_ShmMetaHead *ptHead = Attach((E_ShmRole)(dwRole), rtParam);
    if (NULL == ptHead)
    {
        assert(0);
    }

    return s_pInstance;
}


CShmMgr * CShmMgr::GetInstance()
{
    return s_pInstance;
}


VOID CShmMgr::Destroy()
{
    T_ShmMetaHead *ptHead = NULL;
    E_ShmRole      eRole  = E_SHM_OBSERVER;
    SWORD32        iShmID = -1;

    if (NULL != s_pInstance)
    {
        ptHead = s_pInstance->m_pMetaHead;
        eRole  = s_pInstance->m_eRole;

        delete s_pInstance;

        if (NULL != ptHead)
        {
            iShmID = ptHead->iShmID;

            if (E_SHM_MASTER == eRole)
            {
                Detach((VOID *)ptHead, iShmID);
            }
            else
            {
                Detach((VOID *)ptHead, -1);
            }
        }

        s_pInstance = NULL;
    }
}


VOID CShmMgr::LockGlobal(T_ShmMetaHead &rtHead)
{
    SWORD32 iValue = 0;

    while (!__atomic_compare_exchange_n(&(rtHead.iGlobalLock), 
                                        &iValue, 
                                        1, 0,
                                        __ATOMIC_ACQUIRE, 
                                        __ATOMIC_RELAXED))
    {
        while (__atomic_load_n(&(rtHead.iGlobalLock), __ATOMIC_RELAXED))
        {
    #ifdef ARCH_ARM64    
            asm volatile("yield" ::: "memory");
    #else
            _mm_pause();
    #endif
        }

        iValue = 0;
    }
}


VOID CShmMgr::UnLockGlobal(T_ShmMetaHead &rtHead)
{
    __atomic_store_n(&(rtHead.iGlobalLock), 0, __ATOMIC_RELEASE);
}


T_ShmMetaHead * CShmMgr::Attach(E_ShmRole eRole, T_ShmJsonCfg &rtParam)
{
    key_t          tShmKey     = static_cast<key_t>(s_dwShmKey);
    SWORD32        iShmgetFlag = 0666;
    SWORD32        iShmID      = -1;
    WORD32         dwResult    = 0;
    WORD64         lwSize      = s_lwGranularity;
    VOID          *pAddr       = NULL;
    T_ShmMetaHead *ptShmHead   = NULL;
    VOID          *pShmAddr    = (E_SHM_MASTER == eRole) 
                               ? ((VOID *)(s_lwVirBassAddr)) : NULL;

    struct shmid_ds tShmDS;

    /* 创建共享内存 */
    iShmID = shmget(tShmKey, lwSize, iShmgetFlag);
    if ((iShmID < 0) && (ENOENT == errno))
    {
        iShmgetFlag = 0666 | IPC_CREAT | IPC_EXCL;
        //iShmgetFlag = 0666 | SHM_HUGETLB | IPC_CREAT | IPC_EXCL;

        iShmID = shmget(tShmKey, lwSize, iShmgetFlag);
        if (iShmID < 0)
        {
            return NULL;
        }
    }

    pAddr = shmat(iShmID, pShmAddr, 0);
    if (((VOID *)-1) == pAddr)
    {
        return NULL;
    }

    if (shmctl(iShmID, IPC_STAT, &tShmDS) < 0)
    {
        Detach(pAddr, iShmID);
        return NULL;
    }

    if (lwSize != tShmDS.shm_segsz)
    {
        Detach(pAddr, iShmID);
        return NULL;
    }

    dwResult = InitMetaHead(eRole, iShmID, (T_ShmMetaHead *)pAddr, rtParam);
    if (SUCCESS != dwResult)
    {
        Detach(pAddr, iShmID);
        return NULL;
    }

    ptShmHead = (T_ShmMetaHead *)pAddr;
    pShmAddr  = (VOID *)(ptShmHead->lwMetaAddr);

    assert(((WORD64)pShmAddr) == s_lwVirBassAddr);

    if (E_SHM_MASTER != eRole)
    {
        shmdt(pAddr);

        pAddr = shmat(iShmID, pShmAddr, 0);
        if (((VOID *)-1) == pAddr)
        {
            return NULL;
        }

        ptShmHead = (T_ShmMetaHead *)pAddr;

        if (E_SHM_SLAVE == eRole)
        {
            s_pInstance = InitSlave(ptShmHead, rtParam);
        }
        else
        {
            s_pInstance = InitObserver(ptShmHead, rtParam);
        }

        assert(NULL != s_pInstance);
    }

    assert(((WORD64)pAddr) == s_lwVirBassAddr);

    return ptShmHead;
}


WORD32 CShmMgr::Detach(VOID *ptAddr, SWORD32 iShmID)
{
    if (NULL != ptAddr)
    {
        shmdt(ptAddr);
    }

    if (iShmID >= 0)
    {
        shmctl(iShmID, IPC_RMID, 0);
    }

    return SUCCESS;
}


WORD32 CShmMgr::InitMetaHead(E_ShmRole      eRole,
                             SWORD32        iShmID,
                             T_ShmMetaHead *ptHead,
                             T_ShmJsonCfg  &rtParam)
{
    WORD32 dwWaitCount = 0;

    if (E_SHM_MASTER == eRole)
    {
        ptHead->lwMagic      = s_lwMagicValue;
        ptHead->dwVersion    = s_dwVersion;
        ptHead->dwHeadSize   = sizeof(T_ShmMetaHead);
        ptHead->dwShmKey     = s_dwShmKey;
        ptHead->iShmID       = iShmID;
        ptHead->lwMasterLock = 0;
        ptHead->iGlobalLock  = 0;
        ptHead->bInitFlag    = FALSE;
        ptHead->lwMetaAddr   = 0;
        ptHead->lwMetaSize   = s_lwGranularity;
        ptHead->iMLock       = 0;
        ptHead->dwChannelNum = rtParam.dwChannelNum;

        LockGlobal(*ptHead);

        s_pInstance = InitMaster(ptHead, rtParam);
        if (NULL == s_pInstance)
        {
            UnLockGlobal(*ptHead);
            return FAIL;
        }

        ptHead->lwMetaAddr = (WORD64)(ptHead);
        ptHead->bInitFlag  = TRUE;

        UnLockGlobal(*ptHead);
    }
    else
    {
        do
        {
            if ( (s_lwMagicValue        != ptHead->lwMagic)
              || (s_dwVersion           != ptHead->dwVersion)
              || (sizeof(T_ShmMetaHead) != ptHead->dwHeadSize)
              || (s_dwShmKey            != ptHead->dwShmKey))
            {
                sleep(1);

                dwWaitCount++;
                if (dwWaitCount >= SHM_SLAVE_INIT_WAIT)
                {
                    return FAIL;
                }
            }
            else
            {
                LockGlobal(*ptHead);

                if (TRUE == ptHead->bInitFlag)
                {
                    UnLockGlobal(*ptHead);
                    break ;
                }
                else
                {
                    UnLockGlobal(*ptHead);
                    usleep(100);
                }
            }
        } while(TRUE);
    }

    return SUCCESS;
}


CShmMgr * CShmMgr::InitMaster(T_ShmMetaHead *ptHead, T_ShmJsonCfg &rtParam)
{
    BYTE    *pMem    = (BYTE *)(&(ptHead->aucMaster[0]));
    CShmMgr *pShmMgr = new (pMem) CShmMgr();

    WORD32 dwResult = pShmMgr->Initiaize(ptHead, rtParam);
    if (SUCCESS != dwResult)
    {
        return NULL;
    }

    return pShmMgr;
}


CShmMgr * CShmMgr::InitSlave(T_ShmMetaHead *ptHead, T_ShmJsonCfg &rtParam)
{
    BYTE    *pMem    = (BYTE *)(&(ptHead->aucSlave[0]));
    CShmMgr *pShmMgr = new (pMem) CShmMgr();

    WORD32 dwResult = pShmMgr->Initiaize(ptHead, rtParam);
    if (SUCCESS != dwResult)
    {
        return NULL;
    }

    return pShmMgr;
}


CShmMgr * CShmMgr::InitObserver(T_ShmMetaHead *ptHead, T_ShmJsonCfg &rtParam)
{
    BYTE    *pMem    = (BYTE *)(&(ptHead->aucObserver[0]));
    CShmMgr *pShmMgr = new (pMem) CShmMgr();

    WORD32 dwResult = pShmMgr->Initiaize(ptHead, rtParam);
    if (SUCCESS != dwResult)
    {
        return NULL;
    }

    return pShmMgr;
}


CShmMgr::CShmMgr()
{
    m_pMetaHead    = NULL;
    m_eRole        = E_SHM_OBSERVER;
    m_dwChannelNum = 0;
    m_pMemPool     = NULL;
    m_pOamChannel  = NULL;
    m_pCtrlChannel = NULL;

    for (WORD32 dwIndex = 0; dwIndex < MAX_CHANNEL_NUM; dwIndex++)
    {
        m_apDataChannel[dwIndex] = NULL;
    }

    memset(&m_tOamChannel,  0x00, sizeof(m_tOamChannel));
    memset(&m_tCtrlChannel, 0x00, sizeof(m_tCtrlChannel));
    memset(m_atDataChannel, 0x00, sizeof(m_atDataChannel));
}


CShmMgr::~CShmMgr()
{
    if (NULL != m_pOamChannel)
    {
        delete m_pOamChannel;
    }

    if (NULL != m_pCtrlChannel)
    {
        delete m_pCtrlChannel;
    }

    for (WORD32 dwIndex = 0; dwIndex < MAX_CHANNEL_NUM; dwIndex++)
    {
        if (NULL != m_apDataChannel[dwIndex])
        {
            delete m_apDataChannel[dwIndex];
        }
    }

    if (NULL != m_pMemPool)
    {
        delete m_pMemPool;
    }

    m_pMetaHead    = NULL;
    m_eRole        = E_SHM_OBSERVER;
    m_dwChannelNum = 0;
    m_pMemPool     = NULL;
    m_pOamChannel  = NULL;
    m_pCtrlChannel = NULL;

    for (WORD32 dwIndex = 0; dwIndex < MAX_CHANNEL_NUM; dwIndex++)
    {
        m_apDataChannel[dwIndex] = NULL;
    }

    memset(&m_tOamChannel,  0x00, sizeof(m_tOamChannel));
    memset(&m_tCtrlChannel, 0x00, sizeof(m_tCtrlChannel));
    memset(m_atDataChannel, 0x00, sizeof(m_atDataChannel));
}


WORD32 CShmMgr::Initiaize(T_ShmMetaHead *ptHead, T_ShmJsonCfg &rtParam)
{
    m_pMetaHead     = ptHead;
    m_eRole         = (E_ShmRole)(rtParam.dwRole);
    m_dwChannelNum  = rtParam.dwChannelNum;

    if (E_SHM_MASTER == m_eRole)
    {
        BYTE   *pMem     = (BYTE *)(&(ptHead->aucMemPool[0]));
        VOID   *pOriAddr = (BYTE *)(&(ptHead->aucOriAddr[0]));
        WORD64  lwSize   = ptHead->lwMetaSize - offsetof(T_ShmMetaHead, aucOriAddr);

        m_pMemPool = new (pMem) CCentralMemPool();
        m_pMemPool->Initialize(pOriAddr, lwSize);
    }

    m_pOamChannel = InitOamChannel(rtParam);
    if (NULL == m_pOamChannel)
    {
        return FAIL;
    }

    m_pCtrlChannel = InitCtrlChannel(rtParam);
    if (NULL == m_pCtrlChannel)
    {
        return FAIL;
    }

    for (WORD32 dwIndex = 0; dwIndex < m_dwChannelNum; dwIndex++)
    {
        m_apDataChannel[dwIndex] = InitDataChannel(dwIndex, rtParam);
        if (NULL == m_apDataChannel[dwIndex])
        {
            return FAIL;
        }
    }

    return SUCCESS;
}


VOID CShmMgr::Dump()
{
    TRACE_STACK("CShmMgr::Dump()");

    Snapshot();

    T_ChannelSnapshot *ptChannel = NULL;

    {
        TRACE_STACK("CtrlChannel");

        ptChannel = &m_tCtrlChannel;

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                   "Recv : InitFlag : %d, Status : %d, "
                   "ProdM[%u, %u], ConsM[%u, %u], "
                   "ProdQ[%u, %u], ConsQ[%u, %u], "
                   "MallocCount : %lu, FreeCount : %lu, "
                   "EnqueueCount : %lu, DequeueCount : %lu\n",
                   ptChannel->tRecv.bInitFlag,
                   ptChannel->tRecv.bStatus,
                   ptChannel->tRecv.dwProdHeadM,
                   ptChannel->tRecv.dwProdTailM,
                   ptChannel->tRecv.dwConsHeadM,
                   ptChannel->tRecv.dwConsTailM,
                   ptChannel->tRecv.dwProdHeadQ,
                   ptChannel->tRecv.dwProdTailQ,
                   ptChannel->tRecv.dwConsHeadQ,
                   ptChannel->tRecv.dwConsTailQ,
                   ptChannel->tRecv.lwMallocCount,
                   ptChannel->tRecv.lwFreeCount,
                   ptChannel->tRecv.lwEnqueueCount,
                   ptChannel->tRecv.lwDequeueCount);

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                   "Send : InitFlag : %d, Status : %d, "
                   "ProdM[%u, %u], ConsM[%u, %u], "
                   "ProdQ[%u, %u], ConsQ[%u, %u], "
                   "MallocCount : %lu, FreeCount : %lu, "
                   "EnqueueCount : %lu, DequeueCount : %lu\n",
                   ptChannel->tSend.bInitFlag,
                   ptChannel->tSend.bStatus,
                   ptChannel->tSend.dwProdHeadM,
                   ptChannel->tSend.dwProdTailM,
                   ptChannel->tSend.dwConsHeadM,
                   ptChannel->tSend.dwConsTailM,
                   ptChannel->tSend.dwProdHeadQ,
                   ptChannel->tSend.dwProdTailQ,
                   ptChannel->tSend.dwConsHeadQ,
                   ptChannel->tSend.dwConsTailQ,
                   ptChannel->tSend.lwMallocCount,
                   ptChannel->tSend.lwFreeCount,
                   ptChannel->tSend.lwEnqueueCount,
                   ptChannel->tSend.lwDequeueCount);

        for (WORD32 dwIndex1 = 0; dwIndex1 < BIT_NUM_OF_WORD32; dwIndex1++)
        {
            LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                       "%10d  %15lu  %15lu  %15lu  %15lu\n",
                       (1 << dwIndex1),
                       ptChannel->tRecv.alwStatM[dwIndex1],
                       ptChannel->tRecv.alwStatQ[dwIndex1],
                       ptChannel->tSend.alwStatM[dwIndex1],
                       ptChannel->tSend.alwStatQ[dwIndex1]);
        }

        for (WORD32 dwIndex = 0; dwIndex < E_SHM_MALLOC_POINT_NUM; dwIndex++)
        {
            if (0 != ptChannel->tRecv.alwMallocPoint[dwIndex])
            {
                LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                           "SHM_RECV : Point = %2d, Malloc = %15lu, Free = %15lu\n",
                           dwIndex,
                           ptChannel->tRecv.alwMallocPoint[dwIndex],
                           ptChannel->tRecv.alwFreePoint[dwIndex]);
            }

            if (0 != ptChannel->tSend.alwMallocPoint[dwIndex])
            {
                LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                           "SHM_SEND : Point = %2d, Malloc = %15lu, Free = %15lu\n",
                           dwIndex,
                           ptChannel->tSend.alwMallocPoint[dwIndex],
                           ptChannel->tSend.alwFreePoint[dwIndex]);
            }
        }
    }

    for (WORD32 dwIndex = 0; dwIndex < m_dwChannelNum; dwIndex++)
    {
        TRACE_STACK("DataChannel");

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                   "Channel : %d\n",
                   dwIndex);

        ptChannel = &(m_atDataChannel[dwIndex]);

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                   "Recv : InitFlag : %d, Status : %d, "
                   "ProdM[%u, %u], ConsM[%u, %u], "
                   "ProdQ[%u, %u], ConsQ[%u, %u], "
                   "MallocCount : %lu, FreeCount : %lu, "
                   "EnqueueCount : %lu, DequeueCount : %lu\n",
                   ptChannel->tRecv.bInitFlag,
                   ptChannel->tRecv.bStatus,
                   ptChannel->tRecv.dwProdHeadM,
                   ptChannel->tRecv.dwProdTailM,
                   ptChannel->tRecv.dwConsHeadM,
                   ptChannel->tRecv.dwConsTailM,
                   ptChannel->tRecv.dwProdHeadQ,
                   ptChannel->tRecv.dwProdTailQ,
                   ptChannel->tRecv.dwConsHeadQ,
                   ptChannel->tRecv.dwConsTailQ,
                   ptChannel->tRecv.lwMallocCount,
                   ptChannel->tRecv.lwFreeCount,
                   ptChannel->tRecv.lwEnqueueCount,
                   ptChannel->tRecv.lwDequeueCount);

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                   "Send : InitFlag : %d, Status : %d, "
                   "ProdM[%u, %u], ConsM[%u, %u], "
                   "ProdQ[%u, %u], ConsQ[%u, %u], "
                   "MallocCount : %lu, FreeCount : %lu, "
                   "EnqueueCount : %lu, DequeueCount : %lu\n",
                   ptChannel->tSend.bInitFlag,
                   ptChannel->tSend.bStatus,
                   ptChannel->tSend.dwProdHeadM,
                   ptChannel->tSend.dwProdTailM,
                   ptChannel->tSend.dwConsHeadM,
                   ptChannel->tSend.dwConsTailM,
                   ptChannel->tSend.dwProdHeadQ,
                   ptChannel->tSend.dwProdTailQ,
                   ptChannel->tSend.dwConsHeadQ,
                   ptChannel->tSend.dwConsTailQ,
                   ptChannel->tSend.lwMallocCount,
                   ptChannel->tSend.lwFreeCount,
                   ptChannel->tSend.lwEnqueueCount,
                   ptChannel->tSend.lwDequeueCount);

        for (WORD32 dwIndex1 = 0; dwIndex1 < BIT_NUM_OF_WORD32; dwIndex1++)
        {
            LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                       "%10d  %15lu  %15lu  %15lu  %15lu\n",
                       (1 << dwIndex1),
                       ptChannel->tRecv.alwStatM[dwIndex1],
                       ptChannel->tRecv.alwStatQ[dwIndex1],
                       ptChannel->tSend.alwStatM[dwIndex1],
                       ptChannel->tSend.alwStatQ[dwIndex1]);
        }

        for (WORD32 dwIndex = 0; dwIndex < E_SHM_MALLOC_POINT_NUM; dwIndex++)
        {
            if (0 != ptChannel->tRecv.alwMallocPoint[dwIndex])
            {
                LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                           "SHM_RECV : Point = %2d, Malloc = %15lu, Free = %15lu\n",
                           dwIndex,
                           ptChannel->tRecv.alwMallocPoint[dwIndex],
                           ptChannel->tRecv.alwFreePoint[dwIndex]);
            }

            if (0 != ptChannel->tSend.alwMallocPoint[dwIndex])
            {
                LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                           "SHM_SEND : Point = %2d, Malloc = %15lu, Free = %15lu\n",
                           dwIndex,
                           ptChannel->tSend.alwMallocPoint[dwIndex],
                           ptChannel->tSend.alwFreePoint[dwIndex]);
            }
        }
    }
}


VOID CShmMgr::Printf()
{
    printf("m_eRole : %d, m_dwChannelNum : %d\n",
           m_eRole,
           m_dwChannelNum);

    Snapshot();

    T_ChannelSnapshot *ptChannel = NULL;

    {
        printf("===========================================================\n");

        printf("CtrlChannel : \n");

        ptChannel = &m_tCtrlChannel;

        printf("Recv : InitFlag : %d, Status : %d, "
               "ProdM[%u, %u], ConsM[%u, %u], "
               "ProdQ[%u, %u], ConsQ[%u, %u], "
               "MallocCount : %lu, FreeCount : %lu, "
               "EnqueueCount : %lu, DequeueCount : %lu\n",
               ptChannel->tRecv.bInitFlag,
               ptChannel->tRecv.bStatus,
               ptChannel->tRecv.dwProdHeadM,
               ptChannel->tRecv.dwProdTailM,
               ptChannel->tRecv.dwConsHeadM,
               ptChannel->tRecv.dwConsTailM,
               ptChannel->tRecv.dwProdHeadQ,
               ptChannel->tRecv.dwProdTailQ,
               ptChannel->tRecv.dwConsHeadQ,
               ptChannel->tRecv.dwConsTailQ,
               ptChannel->tRecv.lwMallocCount,
               ptChannel->tRecv.lwFreeCount,
               ptChannel->tRecv.lwEnqueueCount,
               ptChannel->tRecv.lwDequeueCount);

        printf("Send : InitFlag : %d, Status : %d, "
               "ProdM[%u, %u], ConsM[%u, %u], "
               "ProdQ[%u, %u], ConsQ[%u, %u], "
               "MallocCount : %lu, FreeCount : %lu, "
               "EnqueueCount : %lu, DequeueCount : %lu\n",
               ptChannel->tSend.bInitFlag,
               ptChannel->tSend.bStatus,
               ptChannel->tSend.dwProdHeadM,
               ptChannel->tSend.dwProdTailM,
               ptChannel->tSend.dwConsHeadM,
               ptChannel->tSend.dwConsTailM,
               ptChannel->tSend.dwProdHeadQ,
               ptChannel->tSend.dwProdTailQ,
               ptChannel->tSend.dwConsHeadQ,
               ptChannel->tSend.dwConsTailQ,
               ptChannel->tSend.lwMallocCount,
               ptChannel->tSend.lwFreeCount,
               ptChannel->tSend.lwEnqueueCount,
               ptChannel->tSend.lwDequeueCount);

        for (WORD32 dwIndex1 = 0; dwIndex1 < BIT_NUM_OF_WORD32; dwIndex1++)
        {
            printf("%10u  %15lu  %15lu  %15lu  %15lu\n",
                   (1 << dwIndex1),
                   ptChannel->tRecv.alwStatM[dwIndex1],
                   ptChannel->tRecv.alwStatQ[dwIndex1],
                   ptChannel->tSend.alwStatM[dwIndex1],
                   ptChannel->tSend.alwStatQ[dwIndex1]);
        }

        for (WORD32 dwIndex = 0; dwIndex < E_SHM_MALLOC_POINT_NUM; dwIndex++)
        {
            if (0 != ptChannel->tRecv.alwMallocPoint[dwIndex])
            {
                printf("SHM_RECV : Point = %2d, Malloc = %15lu, Free = %15lu\n",
                       dwIndex,
                       ptChannel->tRecv.alwMallocPoint[dwIndex],
                       ptChannel->tRecv.alwFreePoint[dwIndex]);
            }

            if (0 != ptChannel->tSend.alwMallocPoint[dwIndex])
            {
                printf("SHM_SEND : Point = %2d, Malloc = %15lu, Free = %15lu\n",
                       dwIndex,
                       ptChannel->tSend.alwMallocPoint[dwIndex],
                       ptChannel->tSend.alwFreePoint[dwIndex]);
            }
        }

        printf("===========================================================\n");
    }

    for (WORD32 dwIndex = 0; dwIndex < m_dwChannelNum; dwIndex++)
    {
        printf("===========================================================\n");

        printf("DataChannel : %d\n", dwIndex);

        ptChannel = &(m_atDataChannel[dwIndex]);

        printf("Recv : InitFlag : %d, Status : %d, "
               "ProdM[%u, %u], ConsM[%u, %u], "
               "ProdQ[%u, %u], ConsQ[%u, %u], "
               "MallocCount : %lu, FreeCount : %lu, "
               "EnqueueCount : %lu, DequeueCount : %lu\n",
               ptChannel->tRecv.bInitFlag,
               ptChannel->tRecv.bStatus,
               ptChannel->tRecv.dwProdHeadM,
               ptChannel->tRecv.dwProdTailM,
               ptChannel->tRecv.dwConsHeadM,
               ptChannel->tRecv.dwConsTailM,
               ptChannel->tRecv.dwProdHeadQ,
               ptChannel->tRecv.dwProdTailQ,
               ptChannel->tRecv.dwConsHeadQ,
               ptChannel->tRecv.dwConsTailQ,
               ptChannel->tRecv.lwMallocCount,
               ptChannel->tRecv.lwFreeCount,
               ptChannel->tRecv.lwEnqueueCount,
               ptChannel->tRecv.lwDequeueCount);

        printf("Send : InitFlag : %d, Status : %d, "
               "ProdM[%u, %u], ConsM[%u, %u], "
               "ProdQ[%u, %u], ConsQ[%u, %u], "
               "MallocCount : %lu, FreeCount : %lu, "
               "EnqueueCount : %lu, DequeueCount : %lu\n",
               ptChannel->tSend.bInitFlag,
               ptChannel->tSend.bStatus,
               ptChannel->tSend.dwProdHeadM,
               ptChannel->tSend.dwProdTailM,
               ptChannel->tSend.dwConsHeadM,
               ptChannel->tSend.dwConsTailM,
               ptChannel->tSend.dwProdHeadQ,
               ptChannel->tSend.dwProdTailQ,
               ptChannel->tSend.dwConsHeadQ,
               ptChannel->tSend.dwConsTailQ,
               ptChannel->tSend.lwMallocCount,
               ptChannel->tSend.lwFreeCount,
               ptChannel->tSend.lwEnqueueCount,
               ptChannel->tSend.lwDequeueCount);

        for (WORD32 dwIndex1 = 0; dwIndex1 < BIT_NUM_OF_WORD32; dwIndex1++)
        {
            printf("%10u  %15lu  %15lu  %15lu  %15lu\n",
                   (1 << dwIndex1),
                   ptChannel->tRecv.alwStatM[dwIndex1],
                   ptChannel->tRecv.alwStatQ[dwIndex1],
                   ptChannel->tSend.alwStatM[dwIndex1],
                   ptChannel->tSend.alwStatQ[dwIndex1]);
        }

        for (WORD32 dwIndex = 0; dwIndex < E_SHM_MALLOC_POINT_NUM; dwIndex++)
        {
            if (0 != ptChannel->tRecv.alwMallocPoint[dwIndex])
            {
                printf("SHM_RECV : Point = %2d, Malloc = %15lu, Free = %15lu\n",
                       dwIndex,
                       ptChannel->tRecv.alwMallocPoint[dwIndex],
                       ptChannel->tRecv.alwFreePoint[dwIndex]);
            }

            if (0 != ptChannel->tSend.alwMallocPoint[dwIndex])
            {
                printf("SHM_SEND : Point = %2d, Malloc = %15lu, Free = %15lu\n",
                       dwIndex,
                       ptChannel->tSend.alwMallocPoint[dwIndex],
                       ptChannel->tSend.alwFreePoint[dwIndex]);
            }
        }

        printf("===========================================================\n");
    }
}


CShmChannel * CShmMgr::InitOamChannel(T_ShmJsonCfg &rtCfg)
{
    T_ShmChannelParam &rtParam   = rtCfg.tOamChannel;
    T_ShmChannel      &rtChannel = m_pMetaHead->tOamChannel;
    CShmChannel       *pChannel  = NULL;
    BYTE              *pMem      = NULL;
    WORD32             dwResult  = INVALID_DWORD;

    switch (m_eRole)
    {
    case E_SHM_MASTER :
        {
            pMem = (BYTE *)(&(rtChannel.aucMaster[0]));
        }
        break ;

    case E_SHM_SLAVE :
        {
            pMem = (BYTE *)(&(rtChannel.aucSlave[0]));
        }
        break ;

    case E_SHM_OBSERVER :
        {
            pMem = (BYTE *)(&(rtChannel.aucObserver[0]));
        }
        break ;

    default :
        return NULL;
    }

    pChannel = new (pMem) CShmChannel();
    dwResult = pChannel->Initialize(m_eRole,
                                    &rtChannel,
                                    m_pMemPool,
                                    rtParam.dwSendNodeNum,
                                    rtParam.dwSendNodeSize,
                                    rtParam.dwRecvNodeNum,
                                    rtParam.dwRecvNodeSize);
    if (SUCCESS != dwResult)
    {
        return NULL;
    }

    return pChannel;
}


CShmChannel * CShmMgr::InitCtrlChannel(T_ShmJsonCfg &rtCfg)
{
    T_ShmChannelParam &rtParam   = rtCfg.tCtrlChannel;
    T_ShmChannel      &rtChannel = m_pMetaHead->tCtrlChannel;
    CShmChannel       *pChannel  = NULL;
    BYTE              *pMem      = NULL;
    WORD32             dwResult  = INVALID_DWORD;

    switch (m_eRole)
    {
    case E_SHM_MASTER :
        {
            pMem = (BYTE *)(&(rtChannel.aucMaster[0]));
        }
        break ;

    case E_SHM_SLAVE :
        {
            pMem = (BYTE *)(&(rtChannel.aucSlave[0]));
        }
        break ;

    case E_SHM_OBSERVER :
        {
            pMem = (BYTE *)(&(rtChannel.aucObserver[0]));
        }
        break ;

    default :
        return NULL;
    }

    pChannel = new (pMem) CShmChannel();
    dwResult = pChannel->Initialize(m_eRole,
                                    &rtChannel,
                                    m_pMemPool,
                                    rtParam.dwSendNodeNum,
                                    rtParam.dwSendNodeSize,
                                    rtParam.dwRecvNodeNum,
                                    rtParam.dwRecvNodeSize);
    if (SUCCESS != dwResult)
    {
        return NULL;
    }

    return pChannel;
}


CShmChannel * CShmMgr::InitDataChannel(WORD32 dwChannelID, T_ShmJsonCfg &rtCfg)
{
    T_ShmChannelParam &rtParam   = rtCfg.atChannel[dwChannelID];
    T_ShmChannel      &rtChannel = m_pMetaHead->atChannel[dwChannelID];
    CShmChannel       *pChannel  = NULL;
    BYTE              *pMem      = NULL;
    WORD32             dwResult  = INVALID_DWORD;

    switch (m_eRole)
    {
    case E_SHM_MASTER :
        {
            pMem = (BYTE *)(&(rtChannel.aucMaster[0]));
        }
        break ;

    case E_SHM_SLAVE :
        {
            pMem = (BYTE *)(&(rtChannel.aucSlave[0]));
        }
        break ;

    case E_SHM_OBSERVER :
        {
            pMem = (BYTE *)(&(rtChannel.aucObserver[0]));
        }
        break ;

    default :
        return NULL;
    }

    pChannel = new (pMem) CShmChannel();
    dwResult = pChannel->Initialize(m_eRole,
                                    &rtChannel,
                                    m_pMemPool,
                                    rtParam.dwSendNodeNum,
                                    rtParam.dwSendNodeSize,
                                    rtParam.dwRecvNodeNum,
                                    rtParam.dwRecvNodeSize);
    if (SUCCESS != dwResult)
    {
        return NULL;
    }

    return pChannel;
}


