

#include "base_init_component.h"
#include "base_thread_log.h"


WORD32 ExitLog(VOID *pArg)
{
    /* 销毁日志线程(清除所有线程专属的LogMemPool + LogRing资源) */
    CLogThread::Destroy();

    return SUCCESS;
}


WORD32 InitLog(WORD32 dwProcID, VOID *pArg)
{
    T_InitFunc *ptInitFunc = (T_InitFunc *)pArg;
    ptInitFunc->pExitFunc  = &ExitLog;

    T_LogJsonCfg &rJsonCfg = CBaseConfigFile::GetInstance()->GetLogJsonCfg();

    T_ThreadParam tParam;
    tParam.dwProcID      = dwProcID;
    tParam.dwThreadID    = LOG_THREAD_ID;
    tParam.dwLogicalID   = rJsonCfg.dwCoreID;
    tParam.dwPolicy      = rJsonCfg.dwPolicy;
    tParam.dwPriority    = rJsonCfg.dwPriority;
    tParam.dwStackSize   = rJsonCfg.dwStackSize;
    tParam.dwCBNum       = rJsonCfg.dwCBNum;
    tParam.dwPacketCBNum = rJsonCfg.dwPacketCBNum;
    tParam.dwMultiCBNum  = rJsonCfg.dwMultiCBNum;
    tParam.dwTimerThresh = rJsonCfg.dwTimerThresh;
    tParam.bAloneLog     = TRUE;
    tParam.pMemPool      = ptInitFunc->pMemInterface;

    /* 创建并初始化全局变量g_pLogThread */
    CLogThread *pLogThread = CLogThread::CreateInstance(tParam);

    TRACE_STACK("InitLog()");

    pLogThread->GenerateThrdName("log");
    pLogThread->Start();

    return SUCCESS;
}
INIT_EXPORT(InitLog, 0);


/* 处理普通接口+Fast接口日志 */
WORD32 CLogThread::RecvLog(VOID *pObj, VOID *pMsg)
{
    CLogThread     *pThread  = (CLogThread *)pObj;
    T_LogBufHeader *ptHeader = (T_LogBufHeader *)((WORD64)(pMsg) - sizeof(T_LogBufHeader));
    CLogMemPool    *pMemPool = (CLogMemPool *)(ptHeader->lwAddr);

    if (0 == ptHeader->ucLogType)
    {
        /* 普通接口日志 */
        pThread->WriteBack((FILE *)(ptHeader->lwFile), 
                           (CHAR *)pMsg, 
                           ptHeader->wMsgLen);
    }
    else
    {
        /* Fast接口日志 */
        pThread->Assemble((FILE *)(ptHeader->lwFile),
                          (T_LogBinaryMessage *)(pMsg));
    }

    if (likely(NULL != pMemPool))
    {
        pMemPool->Free((BYTE *)pMsg);
    }

    return SUCCESS;
}


CLogThread::CLogThread (const T_ThreadParam &rtParam)
    : CSingletonThread<CLogThread, CLogMemPool>(rtParam)
{
    m_dwFileNum    = 0;
    m_lwEnqueCount = 0;
    m_bMeasSwitch  = FALSE;
    m_dwFileSize   = 0;

    memset(&m_tFastMeasure, 0x00, sizeof(m_tFastMeasure));
    memset(m_atLogFile,     0x00, sizeof(m_atLogFile));    
    memset(m_aucAssemble,   0x00, sizeof(m_aucAssemble));
}


CLogThread::~CLogThread()
{
    /* 销毁主线程CLogInfo对象实例 */
    if (NULL != g_pLogger)
    {
        delete g_pLogger;
        m_rCentralMemPool.Free((BYTE *)g_pLogger);
        g_pLogger = NULL;
    }

    for (WORD32 dwIndex = 0; dwIndex < m_dwFileNum; dwIndex++)
    {
        fclose(m_atLogFile[dwIndex].pFile);
    }

    m_dwFileNum    = 0;
    m_lwEnqueCount = 0;
    m_bMeasSwitch  = FALSE;
    m_dwFileSize   = 0;

    memset(&m_tFastMeasure, 0x00, sizeof(m_tFastMeasure));
    memset(m_atLogFile,     0x00, sizeof(m_atLogFile));    
    memset(m_aucAssemble,   0x00, sizeof(m_aucAssemble));
}


/* 创建线程实例后执行初始化(在主线程栈空间执行) */
WORD32 CLogThread::Initialize()
{
    T_LogJsonCfg &rJsonCfg = CBaseConfigFile::GetInstance()->GetLogJsonCfg();

    m_bMeasSwitch        = rJsonCfg.bMeasSwitch;
    m_tBindInfo.dwAppNum = rJsonCfg.dwAppNum;

    for (WORD32 dwIndex = 0; dwIndex < m_tBindInfo.dwAppNum; dwIndex++)
    {
        T_AppJsonCfg &rtInfo = m_tBindInfo.atApp[dwIndex];
        T_AppJsonCfg &rtCfg  = rJsonCfg.atApp[dwIndex];

        memcpy(&rtInfo, &rtCfg, sizeof(T_AppJsonCfg));
    }

    WORD32 dwResult = CSingletonThread<CLogThread, CLogMemPool>::Initialize();
    if (SUCCESS != dwResult)
    {
        assert(0);
    }

    g_pLogThread = this;

    /* 创建并初始化全局变量g_pLogger */
    g_pLogger = InitLogInfo(m_rCentralMemPool,
                            m_dwProcID,
                            (CHAR *)(s_aucFileName[m_dwProcID]));
    if (NULL == g_pLogger)
    {
        assert(0);
    }

    /* 注册线程专属数据区(主线程) */
    CDataZone *pDataZone = CMemMgr::GetInstance()->GetDataZone();
    WORD32 dwThreadIdx = pDataZone->RegistZone((WORD32)INVALID_THREAD_TYPE,
                                               (CB_RegistZone)(&RegistMainLog),
                                               NULL);
    if (dwThreadIdx != g_dwMainThreadIdx)
    {
        assert(0);
    }

    return SUCCESS;
}


WORD32 CLogThread::RegistLogFile(FILE     *pFile,
                                 CLogInfo *pLogger,
                                 BYTE      ucPos,
                                 WORD32    dwFileSize)
{
    if (NULL == pFile)
    {
        return FAIL;
    }

    CGuardLock<CSpinLock> cGuard(m_cLock);

    if (m_dwFileNum >= LOG_FILE_NUM)
    {
        fclose(pFile);
        return FAIL;
    }

    m_atLogFile[m_dwFileNum].ucFilePos    = ucPos;   /* 标识文件(0或1) */
    m_atLogFile[m_dwFileNum].dwFileSize   = dwFileSize;
    m_atLogFile[m_dwFileNum].pFile        = pFile;
    m_atLogFile[m_dwFileNum].lwWriteTotal = 0;
    m_atLogFile[m_dwFileNum].pLogger      = pLogger;

    m_dwFileNum++;

    return SUCCESS;
}


WORD32 CLogThread::Cancel()
{
    g_pLogThread->SendLPMsgToApp(g_dwLogAppID,
                                 0,
                                 EV_BASE_APP_SHUTDOWN_ID,
                                 0, NULL);

    return SUCCESS;
}


/* 普通日志接口 */
WORD32 CLogThread::NormalWrite(FILE    *pFile,
                               WORD16   wModuleID,
                               WORD16   wLen,
                               CHAR    *pchPrtInfo,
                               WORD16   wLoopThreshold,
                               WORD64   lwAddr,
                               CMultiMessageRing::CSTRing *pRing)
{
    if ( (NULL == pchPrtInfo) 
      || (wLen >= LOG_MEM_BUF_SIZE - sizeof(T_LogBufHeader)))
    {
        return FAIL;
    }

    /* 如果是日志线程自己打印日志, 则直接写回, 避免对信号量的操作 */
    if (lwAddr == (WORD64)m_pLogMemPool)
    {
        WriteBackSelf(pFile, pchPrtInfo, wLen);
        m_pLogMemPool->Free((BYTE *)pchPrtInfo);
        return SUCCESS;
    }

    T_LogBufHeader *ptHeader = (T_LogBufHeader *)((WORD64)(pchPrtInfo) - sizeof(T_LogBufHeader));

    ptHeader->lwAddr    = lwAddr;
    ptHeader->lwFile    = (WORD64)pFile;
    ptHeader->ucLogType = 0;
    ptHeader->wModuleID = wModuleID;
    ptHeader->wMsgLen   = wLen;

    WORD32 dwNum = pRing->Enqueue((VOID *)pchPrtInfo, wLoopThreshold);
    if (dwNum > 0)
    {
        m_cSemaphore.Post();
        m_lwEnqueCount++;

        return SUCCESS;
    }
    else
    {
        return FAIL;
    }
}


/* Fast日志接口 */
WORD32 CLogThread::FastWrite(FILE               *pFile,
                             WORD16              wModuleID,
                             T_LogBinaryMessage *ptMsg,
                             WORD16              wLoopThreshold,
                             WORD64              lwAddr,
                             CMultiMessageRing::CSTRing *pRing)
{
    if (NULL == ptMsg)
    {
        return FAIL;
    }

    T_LogBufHeader *ptHeader = (T_LogBufHeader *)((WORD64)(ptMsg) - sizeof(T_LogBufHeader));

    ptHeader->lwAddr    = lwAddr;
    ptHeader->lwFile    = (WORD64)pFile;
    ptHeader->ucLogType = 1;
    ptHeader->wModuleID = wModuleID;
    ptHeader->wMsgLen   = MAX_LOG_STR_LEN;

    WORD32 dwNum = pRing->Enqueue((VOID *)ptMsg, wLoopThreshold);
    if (dwNum > 0)
    {
        m_cSemaphore.Post();
        m_lwEnqueCount++;
        
        return SUCCESS;
    }
    else
    {
        return FAIL;
    }
}


WORD32 CLogThread::Flush()
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwFileNum; dwIndex++)
    {
        if (0 != m_atLogFile[dwIndex].ucFilePos)
        {
            continue ;
        }

        m_atLogFile[dwIndex].pLogger->Flush();
    }

    return SUCCESS;
}


VOID CLogThread::Dump()
{
    TRACE_STACK("CLogThread::Dump()");

    WORD64 lwEnqueCount = m_lwEnqueCount.load(std::memory_order_relaxed);

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "lwEnqueCount : %lu, FastCount : %lu, Total(Unit 0.1us) : %lu\n",
               lwEnqueCount,
               m_tFastMeasure.lwCount,
               m_tFastMeasure.lwRingTotalTime);

    for (WORD32 dwIndex1 = 0; dwIndex1 < BIT_NUM_OF_WORD32; dwIndex1++)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                   "%10d  %12u\n",
                   (1 << dwIndex1),
                   m_tFastMeasure.adwStat[dwIndex1]);
    }

    g_pLogger->Dump();

    /* 当采用线程池框架创建线程时: 
     * 由于各个线程可以创建线程专属的CLogInfo, 各个模块的打印信息不在g_pLogger中统计
     */
    for (WORD32 dwIndex = 0; dwIndex < m_dwFileNum; dwIndex++)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                   "pLogger %p, pFile %p, ucFilePos %d, lwWriteTotal %ld\n",
                   m_atLogFile[dwIndex].pLogger,
                   m_atLogFile[dwIndex].pFile,
                   m_atLogFile[dwIndex].ucFilePos,
                   m_atLogFile[dwIndex].lwWriteTotal);

        if (m_atLogFile[dwIndex].pLogger == g_pLogger)
        {
            continue ;
        }

        if (m_atLogFile[dwIndex].lwWriteTotal > 0)
        {
            m_atLogFile[dwIndex].pLogger->Dump();
        }
    }
}


VOID CLogThread::DoRun()
{
    TRACE_STACK("CLogThread::DoRun()");

    WORD64 lwTick           = THREAD_TICK_GRANULARITY;
    WORD32 dwResult         = 0;
    WORD32 dwProcHNum       = 0;
    WORD32 dwProcLNum       = 0;
    WORD32 dwProcDNum       = 0;
    WORD32 dwRemainLNum     = 0;
    WORD32 dwRemainDNum     = 0;
    WORD64 lwTimeStampStart = 0;
    WORD64 lwTimeStampProcH = 0;
    WORD64 lwTimeStampProcL = 0;
    WORD64 lwTimeStampProcD = 0;
    WORD64 lwTimeStampEnd   = 0;
    WORD64 lwTimeOutNum     = 0;

    m_pLogger->SetGlobalSwitch(m_bMeasSwitch);
    m_pLogger->OpenModuleLogMask(E_BASE_FRAMEWORK);

    while (TRUE)
    {
        while (m_MsgRingH.isEmpty()
            && m_MsgRingL.isEmpty()
            && m_MsgRingD.isEmpty()
            && m_cMultiRing.isEmpty())
        {
            /* 计算超时任务(处理完所有超时定时器) */
            Execute(lwTimeOutNum);
            m_cSemaphore.TimeWait(lwTick);
        }

        lwTimeStampStart  = GetCycle();
        m_dwCyclePer100NS = g_pGlobalClock->GetCyclePer100NS();

        /* 优先处理完所有高优先级队列中的消息 */
        dwProcHNum = m_MsgRingH.Dequeue(this, &CPollingThread::ProcMessageHP);

        lwTimeStampProcH = GetCycle();

        /* 每次循环固定从低优先级队列中取指定数量的消息 */
        dwRemainLNum = m_MsgRingL.Dequeue(m_dwCBNum, 
                                          this, 
                                          &CPollingThread::ProcMessageLP, 
                                          dwProcLNum);

        lwTimeStampProcL = GetCycle();

        /* 从数据队列接收用户面报文 */
        dwRemainDNum = m_MsgRingD.Dequeue(m_dwPacketCBNum, dwProcDNum);

        /* 从MultiRing接收业务消息 */
        dwRemainDNum = m_cMultiRing.Dequeue(m_dwMultiCBNum, 
                                            this, 
                                            &CLogThread::RecvLog,
                                            dwProcDNum);

        lwTimeStampProcD = GetCycle();

        /* 执行元定时器操作(最多处理指定门限个数超时定时器) */
        Execute(lwTimeOutNum, m_dwTimerThresh);

        lwTimeStampEnd = GetCycle();

        Statistic(dwProcHNum, dwProcLNum, dwProcDNum, 
                  dwRemainLNum, dwRemainDNum, 
                  lwTimeStampStart, 
                  lwTimeStampProcH, lwTimeStampProcL, lwTimeStampProcD, 
                  lwTimeStampEnd);

        /* 线程退出 */
        if (unlikely(isCancel()))
        {
            /* 将消息队列中残留日志全部处理完 */
            m_MsgRingL.Dequeue(this, &CLogThread::RecvLog);
            m_MsgRingD.Dequeue();
            m_cMultiRing.Dequeue(this, &CLogThread::RecvLog);

            g_pOamApp->Dump();

            g_pLogger->Flush();
            m_pLogger->Flush();

            break ;
        }
    }
}


/* 日志线程输出日志落盘接口 */
WORD32 CLogThread::WriteBackSelf(FILE    *pFile, 
                                 CHAR    *ptMsg, 
                                 WORD32   dwMsgLen)
{
    T_LogFileInfo *ptFile = Find(pFile);
    if (NULL == ptFile)
    {
        return FAIL;
    }

    fwrite(ptMsg, dwMsgLen, 1, pFile);

    ptFile->lwWriteTotal += dwMsgLen;
    m_dwFileSize         += dwMsgLen;

    /* 控制日志线程的日志文件大小 */
    if (m_dwFileSize >= ptFile->dwFileSize)
    {
        m_dwFileSize = 0;
        fseek(ptFile->pFile, 0, SEEK_SET);
    }

    return SUCCESS;
}


/* 业务线程输出日志落盘接口 */
WORD32 CLogThread::WriteBack(FILE    *pFile, 
                             CHAR    *ptMsg, 
                             WORD32   dwMsgLen)
{
    T_LogFileInfo *ptFile = Find(pFile);
    if (NULL == ptFile)
    {
        return FAIL;
    }

    fwrite(ptMsg, dwMsgLen, 1, pFile);

    ptFile->lwWriteTotal += dwMsgLen;

    return SUCCESS;
}


WORD32 CLogThread::Assemble(FILE *pFile, T_LogBinaryMessage *ptMsg)
{
    WORD32 dwMsgLen   = 0;
    WORD64 lwCycle    = ptMsg->alwParams[2];
    WORD32 dwTimeUsed = 0;

    m_dwCyclePer100NS = g_pGlobalClock->GetCyclePer100NS();

    /* 将Cycle转换为0.1us */
    dwTimeUsed = TRANSFER_CYCLE_TO_100NS2(lwCycle, m_dwCyclePer100NS);

    m_pLogger->Assemble(ptMsg, m_aucAssemble, dwMsgLen);

    m_tFastMeasure.lwCount++;
    m_tFastMeasure.lwRingTotalTime += dwTimeUsed;
    m_tFastMeasure.adwStat[base_bsr_uint32(dwTimeUsed)]++;

    return WriteBack(pFile, m_aucAssemble, dwMsgLen);
}


T_LogFileInfo * CLogThread::Find(FILE *pFile)
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwFileNum; dwIndex++)
    {
        if (pFile == m_atLogFile[dwIndex].pFile)
        {
            return &(m_atLogFile[dwIndex]);
        }
    }

    return NULL;
}


