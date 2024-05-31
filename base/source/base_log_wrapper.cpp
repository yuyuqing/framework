

#include <assert.h>

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


VOID LogPrintf(WORD32      dwModuleID,
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


