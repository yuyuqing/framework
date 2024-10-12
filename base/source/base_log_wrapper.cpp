

#include <assert.h>

#include "base_init_component.h"
#include "base_thread_log.h"


const BYTE s_aucLevel[E_LOG_LEVEL_NUM][LOG_LEVEL_LEN] = 
{
    "Trace ",    /* Trace  */
    "Debug ",    /* Debug  */
    "Info  ",    /* Info   */
    "Warn  ",    /* Warn   */
    "Error ",    /* Error  */
    "Assert",    /* Assert */
    "Fatal ",    /* Fatal  */
};


const BYTE s_aucColor[E_LOG_LEVEL_NUM][LOG_LEVEL_LEN] =
{
    WHITE,    /* Trace */
    MAGENTA,  /* Debug */
    GREEN,    /* Info */
    YELLOW,   /* Warn */
    RED,      /* Error */
    CYAN,     /* Assert */
    RED,      /* Fatal */
};


#define LOG_FILE_LIN_PLACEHOLDER    ((BYTE)(17))

static const CHAR s_aucFLine[LOG_FILE_LIN_PLACEHOLDER] = 
{
    ':', '%', 'd', ']', ' ',
    '[', '%', 'd', ']', ' ',
    '[', 'T', ':', '%', 'u', ']', ' ',
};


/* 当业务线程不是通过CThreadPool创建时, 通过调用本接口完成注册线程专属数据区 */
WORD32 RegistThreadZone(T_DataZone &rtThreadZone, VOID *pThread)
{
    CMultiMessageRing::CSTRing *pLogRing = InitLogRing(m_dwSelfRingID);
    CLogMemPool *pLogMemPool = InitLogMemPool();

    rtThreadZone.pLogMemPool   = (VOID *)pLogMemPool;
    rtThreadZone.pLogRing      = (VOID *)pLogRing;
    rtThreadZone.pLogger       = g_pLogger;
    rtThreadZone.pThread       = NULL;

    return SUCCESS;
}


/* 在主进程入口处调用 */
WORD32 LogInit_Process(WORD32 dwProcID, CB_RegistMemPool pFunc)
{
    CInitList::GetInstance()->InitComponent(dwProcID, pFunc);

    usleep(1000);

    return SUCCESS;
}


WORD32 LogExit_Process()
{
    CInitList::Destroy();

    return SUCCESS;
}


/* 在线程入口处调用(不通过框架线程池创建的线程实例) */
WORD32 LogInit_Thread()
{
    CDataZone *pDataZone = CMemMgr::GetInstance()->GetDataZone();

    /* 注册线程专属数据区(不通过框架线程池创建的线程实例) */
    pDataZone->RegistZone((WORD32)INVALID_THREAD_TYPE,
                          (CB_RegistZone)(&RegistThreadZone),
                          NULL);

    return SUCCESS;
}


/* 线程退出时调用(不通过框架线程池创建的线程实例) */
WORD32 LogExit_Thread()
{
    /* 等待日志线程消费完日志 */
    usleep(1000);

    CDataZone *pDataZone = CMemMgr::GetInstance()->GetDataZone();

    pDataZone->RemoveZone(m_pSelfThreadZone->dwThreadIdx,
                          (CB_RemoveZone)(&RemoveThreadZone),
                          NULL);

    return SUCCESS;
};


WORD32 SendMessageToLogThread(WORD32 dwMsgID, const VOID *ptMsg, WORD16 wLen)
{
    return g_pLogThread->SendLPMsgToApp(g_dwLogAppID,
                                        0,
                                        dwMsgID,
                                        wLen, ptMsg);
}


VOID LogAssert()
{
    g_pLogThread->SendLPMsgToApp(g_dwLogAppID,
                                 0,
                                 EV_BASE_APP_SHUTDOWN_ID,
                                 0, NULL);
    usleep(1000);
    assert(0);
}


/* 普通日志打印接口 */
VOID LogPrintf(WORD32        dwModuleID,
               WORD32        dwCellID,
               WORD32        dwLevelID,
               BOOL          bForced,
               const CHAR   *pchPrtInfo,
               ...)
{
    if (unlikely((NULL == pchPrtInfo) || (NULL == m_pSelfThreadZone)))
    {
        return ;
    }

    CLogThread     *pLogThread = (CLogThread *)g_pLogThread;
    CLogMemPool    *pMemPool   = (CLogMemPool *)(m_pSelfThreadZone->pLogMemPool);
    CLogInfo       *pLogInfo   = (CLogInfo *)(m_pSelfThreadZone->pLogger);
    CModuleLogInfo *pModule    = NULL;
    BYTE           *pLogBuffer = NULL;

    CMultiMessageRing::CSTRing *pRing = (CMultiMessageRing::CSTRing *)(m_pSelfThreadZone->pLogRing);

    DO_LOG_COMMON(dwModuleID, dwCellID, dwLevelID, bForced, pRing);

    WORD64 lwMicroSec = 0;

    g_pGlobalClock->GetTime3(lwMicroSec, lwStartCycle);

    /* 添加日期时间/日志级别信息 */
    WORD32 dwLen = 0;
    DO_LOG_PREFIX(dwLen,
                  ((CHAR *)pLogBuffer),
                  (lwMicroSec / 1000000),
                  ((WORD32)(lwMicroSec % 1000000)),
                  ((CHAR *)(s_aucModule[dwModuleID])),
                  ((CHAR *)(s_aucLevel[dwLevelID])),
                  ((CHAR)((dwCellID == 0xFFFF) ? 'X' : (dwCellID  + '0'))),
                  lwTotalCount);

    va_list  tParamList;
    va_start(tParamList, pchPrtInfo);
#ifndef STD_LOG
    base_vsnprintf((CHAR *)(pLogBuffer + dwLen),
                   (MAX_LOG_STR_LEN - dwLen),
                   ' ',
                   pchPrtInfo,
                   tParamList);
#else
    vsnprintf((CHAR *)(pLogBuffer + dwLen),
              (MAX_LOG_STR_LEN - dwLen),
              pchPrtInfo,
              tParamList);
#endif
    va_end(tParamList);

/*#if DEBUG_NR5G
    printf(GREEN "%s" GRAY, pLogBuffer);
#endif*/

    /* TraceMe打印 */
    WORD32 dwResult = pLogThread->NormalWrite(pLogInfo->GetFileID(),
                                              (WORD16)dwModuleID,
                                              strlen((CHAR *)pLogBuffer),
                                              (CHAR *)pLogBuffer,
                                              bForced ? pLogInfo->GetLoopThrehold() : 0,
                                              (WORD64)pMemPool,
                                              pRing);
    if (SUCCESS != dwResult)
    {
        pModule->m_lwEnqueueFail++;
        pMemPool->Free(pLogBuffer);
    }
    else
    {
        pModule->m_lwSuccCount++;
    }
}


/* Fast日志打印接口 */
VOID FastLogPrintf(WORD32      dwModuleID,
                   WORD32      dwCellID,
                   WORD32      dwLevelID,
                   BOOL        bForced,
                   const CHAR *pFile,
                   WORD32      dwLine,
                   BYTE        ucParamNum,
                   const CHAR *pchPrtInfo,
                   ...)
{
    if (unlikely( (NULL == pFile) 
               || (NULL == pchPrtInfo) 
               || (NULL == m_pSelfThreadZone)))
    {
        return ;
    }

    __builtin_prefetch((m_pSelfThreadZone->pLogger), 0, 3);

    CLogThread     *pLogThread = static_cast<CLogThread *>(g_pLogThread);
    CLogMemPool    *pMemPool   = (CLogMemPool *)(m_pSelfThreadZone->pLogMemPool);
    CLogInfo       *pLogInfo   = (CLogInfo *)(m_pSelfThreadZone->pLogger);
    CModuleLogInfo *pModule    = NULL;
    BYTE           *pLogBuffer = NULL;

    CMultiMessageRing::CSTRing *pRing = (CMultiMessageRing::CSTRing *)(m_pSelfThreadZone->pLogRing);

    DO_LOG_COMMON(dwModuleID, dwCellID, dwLevelID, bForced, pRing);

    WORD64 lwMicroSec = 0;
    WORD64 lwCycle    = 0;

    g_pGlobalClock->GetTime3(lwMicroSec, lwCycle);

    T_LogBinaryMessage *ptMsg = (T_LogBinaryMessage *)pLogBuffer;

    /* 预取ptMsg->lwTotalCount + ptMsg->alwParams[0~6]数据 */
    __builtin_prefetch((((CHAR *)pLogBuffer) + 64U), 1, 1);
    
    ptMsg->lwSeconds  = lwMicroSec / 1000000;
    ptMsg->dwMicroSec = (WORD32)(lwMicroSec % 1000000);
    ptMsg->wModuleID  = (WORD16)dwModuleID;
    ptMsg->ucLevelID  = (BYTE)dwLevelID;
    ptMsg->ucCell     = (CHAR)((dwCellID == 0xFFFF) ? 'X' : (dwCellID  + '0'));
    ptMsg->ucFileLen  = MIN(strlen(pFile), (__FILE_NAME_LEN__ - 1));
    ptMsg->ucParamNum = MIN(ucParamNum + 3, LOG_MAX_PARAM_NUM);
    ptMsg->wStrLen    = MIN(strlen(pchPrtInfo), (MAX_LOG_STR_LEN - LOG_FILE_LIN_PLACEHOLDER - sizeof(T_LogBinaryMessage) - 1));

    memcpy(ptMsg->aucFile, pFile, ptMsg->ucFileLen);
    ptMsg->aucFile[ptMsg->ucFileLen] = 0;

    ptMsg->lwTotalCount  = lwTotalCount;
    ptMsg->alwParams[0]  = dwLine;
    
    va_list  tParamList;
    va_start(tParamList, pchPrtInfo);

    /* 预取ptMsg->alwParams[7~30]数据 */
    __builtin_prefetch((((CHAR *)pLogBuffer) + 128U), 1, 0);
    __builtin_prefetch((((CHAR *)pLogBuffer) + 192U), 1, 0);
    __builtin_prefetch((((CHAR *)pLogBuffer) + 256U), 1, 0);

    for (WORD32 dwIndex = 3; 
         dwIndex < ptMsg->ucParamNum; 
         dwIndex++)
    {
        ptMsg->alwParams[dwIndex] = va_arg(tParamList, WORD64);
    }

    /* 预取ptMsg->aucFormat[]数据, 预取192字节 */
    __builtin_prefetch((((CHAR *)pLogBuffer) + 320U), 1, 0);
    __builtin_prefetch((((CHAR *)pLogBuffer) + 384U), 1, 0);
    __builtin_prefetch((((CHAR *)pLogBuffer) + 448U), 1, 0);
    va_end(tParamList);

    memcpy(ptMsg->aucFormat, s_aucFLine, LOG_FILE_LIN_PLACEHOLDER);

    memcpy(ptMsg->aucFormat + LOG_FILE_LIN_PLACEHOLDER, pchPrtInfo, ptMsg->wStrLen);

    ptMsg->wStrLen += LOG_FILE_LIN_PLACEHOLDER;
    ptMsg->aucFormat[ptMsg->wStrLen - 1] = '\n'; 
    ptMsg->aucFormat[ptMsg->wStrLen]     = 0;

    ptMsg->alwParams[1]  = m_dwSelfThreadID;

    /* TraceMe打印 */    
    WORD32 dwResult = pLogThread->FastWrite(pLogInfo->GetFileID(),
                                            (WORD16)dwModuleID,
                                            ptMsg,
                                            bForced ? pLogInfo->GetLoopThrehold() : 0,
                                            (WORD64)pMemPool,
                                            pRing);
    if (SUCCESS != dwResult)
    {
        pModule->m_lwEnqueueFail++;
        pMemPool->Free(pLogBuffer);
    }
    else
    {
        ptMsg->alwParams[2] = GetCycle() - lwStartCycle;
        pModule->m_lwSuccCount++;
    }
}


VOID TestGdbMemory()
{
    pthread_t  tThreadID;
    cpu_set_t  tCpuSet;

    memset(&tThreadID, 0x00, sizeof(tThreadID));
    CPU_ZERO(&tCpuSet);

    /* 固定绑定核1 */
    CPU_SET(1, &tCpuSet);
    tThreadID = pthread_self();
    pthread_setaffinity_np(tThreadID, sizeof(cpu_set_t), &tCpuSet);

    CMemMgr *pMemMgr = CMemMgr::CreateMemMgr((WORD32)E_PROC_DU,
                                             (BYTE)E_MEM_HUGEPAGE_TYPE,
                                             1,
                                             1024 * 1024 * 1024,
                                             "/dev/hugepages",
                                             FALSE);

    T_MemMetaHead   *pMetaHead       = pMemMgr->GetMetaHead();
    CDataZone       *pDataZone       = pMemMgr->GetDataZone();
    CCentralMemPool *pCentralMemPool = pMemMgr->GetCentralMemPool();

    WORD64  lwMetaAddr     = pMetaHead->lwMetaAddr;
    WORD64  lwHugeAddr     = pMetaHead->lwHugeAddr;
    WORD64  lwDataZoneAddr = (WORD64)pDataZone;
    WORD64  lwCtrlMemPool  = (WORD64)pCentralMemPool;
    WORD64  lwMemPools     = pMetaHead->lwMemPools;
    WORD64  lwAppCntrlAddr = pMetaHead->lwAppCntrlAddr;
    WORD64  lwThdCntrlAddr = pMetaHead->lwThreadCntrlAddr;

    printf("lwMagic : %lu, dwVersion : %u, dwHeadSize : %u, "
           "lwMasterLock : %lu, iGlobalLock : %d, bInitFlag : %d, "
           "iMLock : %d, dwHugeNum : %d, iPrimaryFileID : %d, "
           "iSecondaryFileID : %d, lwHugeAddr : %lu, aucHugePath : %s, "
           "lwMetaAddr : %lu, lwMetaSize : %lu, lwHugeAddr : %lu, "
           "lwShareMemSize : %lu, lwAppCntrlAddr : %lu, "
           "lwThreadCntrlAddr : %lu, lwMemPools : %lu\n",
           pMetaHead->lwMagic,
           pMetaHead->dwVersion,
           pMetaHead->dwHeadSize,
           pMetaHead->lwMasterLock,
           pMetaHead->iGlobalLock,
           pMetaHead->bInitFlag,
           pMetaHead->iMLock,
           pMetaHead->dwHugeNum,
           pMetaHead->atHugeInfo[0].iPrimaryFileID,
           pMetaHead->atHugeInfo[0].iSecondaryFileID,
           pMetaHead->atHugeInfo[0].lwHugeAddr,
           pMetaHead->atHugeInfo[0].aucHugePath,
           pMetaHead->lwMetaAddr,
           pMetaHead->lwMetaSize,
           pMetaHead->lwHugeAddr,
           pMetaHead->lwShareMemSize,
           pMetaHead->lwAppCntrlAddr,
           pMetaHead->lwThreadCntrlAddr,
           pMetaHead->lwMemPools);

    printf("lwMetaAddr     = %lu\n", lwMetaAddr);
    printf("lwHugeAddr     = %lu\n", lwHugeAddr);
    printf("lwDataZoneAddr = %lu\n", lwDataZoneAddr);
    printf("lwCtrlMemPool  = %lu\n", lwCtrlMemPool);
    printf("lwMemPools     = %lu\n", lwMemPools);
    printf("lwAppCntrlAddr = %lu\n", lwAppCntrlAddr);
    printf("lwThdCntrlAddr = %lu\n", lwThdCntrlAddr);
}


