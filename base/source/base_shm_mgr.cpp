

#include <assert.h>

#include "base_shm_mgr.h"
#include "base_init_component.h"
#include "base_variable.h"
#include "base_log.h"
#include "base_config_file.h"


WORD32 ExitShmMgr(VOID *pArg)
{
    T_InitFunc      *ptInitFunc      = (T_InitFunc *)pArg;
    CCentralMemPool *pCentralMemPool = ptInitFunc->pMemInterface;

    CShmMgr::Destroy();

    if (NULL != g_pShmMgr)
    {
        pCentralMemPool->Free((BYTE *)g_pShmMgr);
        g_pShmMgr = NULL;
    }

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

    CCentralMemPool *pCentralMemPool = ptInitFunc->pMemInterface;

    BYTE *pMem = pCentralMemPool->Malloc(sizeof(CShmMgr));
    if (NULL == pMem)
    {
        return FAIL;
    }

    g_pShmMgr = CShmMgr::GetInstance(pMem);
    if (NULL == g_pShmMgr)
    {
        assert(0);
    }

    WORD32 dwResult = g_pShmMgr->Initialize(rJsonCfg.bMaster,
                                            rJsonCfg.dwChannelNum,
                                            rJsonCfg.dwPowerNum,
                                            pCentralMemPool);
    if (SUCCESS != dwResult)
    {
        assert(0);
    }

    return SUCCESS;
}
INIT_EXPORT(InitShmMgr, 1);


CShmMgr::CShmMgr ()
{
    m_bMaster      = FALSE;
    m_dwChannelNum = 0;
    m_dwPowerNum   = 0;
    m_pMemPool     = NULL;

    for (WORD32 dwIndex = 0; dwIndex < MAX_CHANNEL_NUM; dwIndex++)
    {
        m_apChannel[dwIndex] = NULL;
    }

    memset(m_atChannel, 0x00, sizeof(m_atChannel));
}


CShmMgr::~CShmMgr()
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwChannelNum; dwIndex++)
    {
        if (NULL != m_apChannel[dwIndex])
        {
            delete m_apChannel[dwIndex];
            m_pMemPool->Free((BYTE *)(m_apChannel[dwIndex]));
            m_apChannel[dwIndex] = NULL;
        }
    }

    m_bMaster      = FALSE;
    m_dwChannelNum = 0;
    m_dwPowerNum   = 0;
    m_pMemPool     = NULL;

    memset(m_atChannel, 0x00, sizeof(m_atChannel));
}


WORD32 CShmMgr::Initialize(BOOL             bMaster,
                           WORD32           dwChannelNum,
                           WORD32           dwPowerNum,
                           CCentralMemPool *pCentralMemPool)
{
    //TRACE_STACK("CShmMgr::Initialize()");

    if ( (dwChannelNum > MAX_CHANNEL_NUM)
      || (dwPowerNum < E_ShmPowerNum_14)
      || (dwPowerNum > E_ShmPowerNum_18)
      || (NULL == pCentralMemPool))
    {
        return FAIL;
    }

    BYTE *pBuf = NULL;

    for (WORD32 dwIndex = 0; dwIndex < dwChannelNum; dwIndex++)
    {
        pBuf = NULL;
        pBuf = pCentralMemPool->Malloc(SHM_CHANNEL_SIZE);
        if (NULL == pBuf)
        {
            return FAIL;
        }

        m_apChannel[dwIndex] = CreateChannel(bMaster,
                                             dwPowerNum,
                                             pBuf,
                                             s_dwMasterKey + dwIndex,
                                             s_dwSlaveKey + dwIndex);
        if (NULL == m_apChannel[dwIndex])
        {
            return FAIL;
        }
    }

    m_bMaster      = bMaster;
    m_dwChannelNum = dwChannelNum;
    m_dwPowerNum   = dwPowerNum;
    m_pMemPool     = pCentralMemPool;

    return SUCCESS;
}


VOID CShmMgr::Printf()
{
    printf("m_dwChannelNum : %d, m_dwPowerNum : %d\n",
           m_dwChannelNum,
           m_dwPowerNum);

    Snapshot();

    T_ChannelSnapshot *ptChannel = NULL;

    for (WORD32 dwIndex = 0; dwIndex < m_dwChannelNum; dwIndex++)
    {
        printf("===========================================================\n");

        printf("Channel : %d\n", dwIndex);

        ptChannel = &(m_atChannel[dwIndex]);

        printf("Recv : InitFlag : %d, GLock : %d, ULock : %d, Status : %d, "
               "ProdM[%u, %u], ConsM[%u, %u], "
               "ProdQ[%u, %u], ConsQ[%u, %u], "
               "MallocCount : %lu, FreeCount : %lu, "
               "EnqueueCount : %lu, DequeueCount : %lu\n",
               ptChannel->tRecv.bInitFlag,
               ptChannel->tRecv.iGlobalLock,
               ptChannel->tRecv.iUserLock,
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

        printf("Send : InitFlag : %d, GLock : %d, ULock : %d, Status : %d, "
               "ProdM[%u, %u], ConsM[%u, %u], "
               "ProdQ[%u, %u], ConsQ[%u, %u], "
               "MallocCount : %lu, FreeCount : %lu, "
               "EnqueueCount : %lu, DequeueCount : %lu\n",
               ptChannel->tSend.bInitFlag,
               ptChannel->tSend.iGlobalLock,
               ptChannel->tSend.iUserLock,
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


VOID CShmMgr::Dump()
{
    TRACE_STACK("CShmMgr::Dump()");

    Snapshot();

    T_ChannelSnapshot *ptChannel = NULL;

    for (WORD32 dwIndex = 0; dwIndex < m_dwChannelNum; dwIndex++)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                   "Channel : %d\n",
                   dwIndex);

        ptChannel = &(m_atChannel[dwIndex]);

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                   "Recv : InitFlag : %d, GLock : %d, ULock : %d, Status : %d, "
                   "ProdM[%u, %u], ConsM[%u, %u], "
                   "ProdQ[%u, %u], ConsQ[%u, %u], "
                   "MallocCount : %lu, FreeCount : %lu, "
                   "EnqueueCount : %lu, DequeueCount : %lu\n",
                   ptChannel->tRecv.bInitFlag,
                   ptChannel->tRecv.iGlobalLock,
                   ptChannel->tRecv.iUserLock,
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
                   "Send : InitFlag : %d, GLock : %d, ULock : %d, Status : %d, "
                   "ProdM[%u, %u], ConsM[%u, %u], "
                   "ProdQ[%u, %u], ConsQ[%u, %u], "
                   "MallocCount : %lu, FreeCount : %lu, "
                   "EnqueueCount : %lu, DequeueCount : %lu\n",
                   ptChannel->tSend.bInitFlag,
                   ptChannel->tSend.iGlobalLock,
                   ptChannel->tSend.iUserLock,
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


CChannelTpl * CShmMgr::CreateChannel(BOOL    bMaster,
                                     WORD32  dwPowerNum,
                                     BYTE   *pBuf,
                                     WORD32  dwKeyS,
                                     WORD32  dwKeyR)
{
    //TRACE_STACK("CShmMgr::CreateChannel()");

    if (bMaster)
    {
        return CreateMaster(dwPowerNum, pBuf, dwKeyS, dwKeyR);
    }
    else
    {
        return CreateSlave(dwPowerNum, pBuf, dwKeyS, dwKeyR);
    }
}


CChannelTpl * CShmMgr::CreateMaster(WORD32  dwPowerNum,
                                    BYTE   *pBuf,
                                    WORD32  dwKeyS,
                                    WORD32  dwKeyR)
{
    //TRACE_STACK("CShmMgr::CreateMaster()");

    CChannelTpl *pChannel = NULL;

    switch (dwPowerNum)
    {
    case E_ShmPowerNum_14 :
        {
            pChannel = new (pBuf) CShmChannelM16();
        }
        break ;

    case E_ShmPowerNum_15 :
        {
            pChannel = new (pBuf) CShmChannelM17();
        }
        break ;

    case E_ShmPowerNum_16 :
        {
            pChannel = new (pBuf) CShmChannelM18();
        }
        break ;

    case E_ShmPowerNum_17 :
        {
            pChannel = new (pBuf) CShmChannelM19();
        }
        break ;

    case E_ShmPowerNum_18 :
        {
            pChannel = new (pBuf) CShmChannelM20();
        }
        break ;

    default :
        break ;
    }

    if (NULL == pChannel)
    {
        return NULL;
    }

    WORD32 dwResult = pChannel->Initialize(dwKeyS, dwKeyR);
    if (SUCCESS != dwResult)
    {
        return NULL;
    }

    return pChannel;
}


CChannelTpl * CShmMgr::CreateSlave(WORD32  dwPowerNum,
                                   BYTE   *pBuf,
                                   WORD32  dwKeyS,
                                   WORD32  dwKeyR)
{
    //TRACE_STACK("CShmMgr::CreateSlave()");

    CChannelTpl *pChannel = NULL;

    switch (dwPowerNum)
    {
    case E_ShmPowerNum_14 :
        {
            pChannel = new (pBuf) CShmChannelS16();
        }
        break ;

    case E_ShmPowerNum_15 :
        {
            pChannel = new (pBuf) CShmChannelS17();
        }
        break ;

    case E_ShmPowerNum_16 :
        {
            pChannel = new (pBuf) CShmChannelS18();
        }
        break ;

    case E_ShmPowerNum_17 :
        {
            pChannel = new (pBuf) CShmChannelS19();
        }
        break ;

    case E_ShmPowerNum_18 :
        {
            pChannel = new (pBuf) CShmChannelS20();
        }
        break ;

    default :
        break ;
    }

    if (NULL == pChannel)
    {
        return NULL;
    }

    WORD32 dwResult = pChannel->Initialize(dwKeyS, dwKeyR);
    if (SUCCESS != dwResult)
    {
        return NULL;
    }

    return pChannel;
}


