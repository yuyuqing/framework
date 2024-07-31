

#include "base_thread_log.h"


CLogContext::CLogContext (CHAR     *pcFileName,
                          WORD32    dwLineNum,
                          CHAR     *ptr,
                          CLogInfo *pLogInfo)
{
    m_pLogInfo   = pLogInfo;
    m_pcFileName = pcFileName;
    m_dwLineNum  = dwLineNum;
    m_wFlag      = UNABLE_FLAG;
    m_wLen       = 0;

    if (unlikely((NULL == ptr) 
              || (NULL == pLogInfo) 
              || (NULL == m_pSelfThreadZone)))
    {
        return ;
    }

    CLogThread     *pLogThread = static_cast<CLogThread *>(g_pLogThread);
    CLogMemPool    *pMemPool   = (CLogMemPool *)(m_pSelfThreadZone->pLogMemPool);
    CModuleLogInfo *pModule    = NULL;
    BYTE           *pLogBuffer = NULL;

    CMultiMessageRing::CSTRing *pRing = (CMultiMessageRing::CSTRing *)(m_pSelfThreadZone->pLogRing);

    DO_LOG_COMMON(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE, pRing);

    WORD64 lwMicroSec = 0;

    g_pGlobalClock->GetTime3(lwMicroSec, lwStartCycle);

    m_wFlag = ENABLE_FLAG;

    /* 添加日期时间/日志级别信息 */
    WORD32 dwLen = 0;
    DO_LOG_PREFIX(dwLen, 
                  ((CHAR *)pLogBuffer),
                  (lwMicroSec / 1000000),
                  ((WORD32)(lwMicroSec % 1000000)),
                  ((CHAR *)(s_aucModule[E_BASE_FRAMEWORK])),
                  ((CHAR *)(s_aucLevel[E_LOG_LEVEL_TRACE])),
                  ('X'),
                  lwTotalCount);

    memset((m_aucData + m_wLen), ' ', m_wSelfStackInc);
    m_wLen += m_wSelfStackInc;

    memcpy((m_aucData + m_wLen), CLogInfo::s_aucCstructPre, CONST_LOG_PRE_LEN);
    m_wLen += CONST_LOG_PRE_LEN;

    memcpy((m_aucData + m_wLen), ptr, strlen(ptr));
    m_wLen += strlen(ptr);

    m_wSelfStackInc += CONST_LOG_PRE_LEN;

    /* 添加文件名/代码行信息 */
    if (NULL != pcFileName)
    {
        base_snprintf((CHAR *)(m_aucData + m_wLen), 
                      MAX_LOG_STR_LEN - m_wLen, 
                      ' ', 
                      " at File: %s, Line: %d\n", 
                      pcFileName,
                      dwLineNum);
    }        
    else
    {
        sprintf((CHAR *)(m_aucData + m_wLen), " \n");
    }

    base_snprintf((CHAR *)(pLogBuffer + dwLen), 
                  MAX_LOG_STR_LEN, 
                  ' ', 
                  "%s", 
                  (CHAR *)m_aucData);

    WORD32 dwResult = pLogThread->NormalWrite(pLogInfo->GetFileID(),
                                              E_BASE_FRAMEWORK,
                                              strlen((CHAR *)pLogBuffer),
                                              (CHAR *)pLogBuffer, 
                                              pLogInfo->GetLoopThrehold(),
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


CLogContext::~CLogContext()
{
    if ((0 == m_wLen) || (UNABLE_FLAG == m_wFlag))
    {
        return ;
    }

    CLogThread     *pLogThread = static_cast<CLogThread *>(g_pLogThread);
    CLogMemPool    *pMemPool   = (CLogMemPool *)(m_pSelfThreadZone->pLogMemPool);
    CLogInfo       *pLogInfo   = m_pLogInfo;
    CModuleLogInfo *pModule    = NULL;
    BYTE           *pLogBuffer = NULL;

    CMultiMessageRing::CSTRing *pRing = (CMultiMessageRing::CSTRing *)(m_pSelfThreadZone->pLogRing);

    m_wSelfStackInc -= CONST_LOG_PRE_LEN;

    DO_LOG_COMMON(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE, pRing);

    WORD64 lwMicroSec = 0;

    g_pGlobalClock->GetTime3(lwMicroSec, lwStartCycle);

    /* 添加日期时间/日志级别信息 */
    WORD32 dwLen = 0;
    DO_LOG_PREFIX(dwLen, 
                  ((CHAR *)pLogBuffer),
                  (lwMicroSec / 1000000),
                  ((WORD32)(lwMicroSec % 1000000)),
                  ((CHAR *)(s_aucModule[E_BASE_FRAMEWORK])),
                  ((CHAR *)(s_aucLevel[E_LOG_LEVEL_TRACE])),
                  ('X'),
                  lwTotalCount);

    memcpy((m_aucData + m_wSelfStackInc), 
           CLogInfo::s_aucDstructPre, 
           CONST_LOG_PRE_LEN);

    base_snprintf((CHAR *)(pLogBuffer + dwLen), 
                  MAX_LOG_STR_LEN, 
                  ' ', 
                  "%s", 
                  (CHAR *)m_aucData);

    WORD32 dwResult = pLogThread->NormalWrite(pLogInfo->GetFileID(),
                                              E_BASE_FRAMEWORK,
                                              strlen((CHAR *)pLogBuffer),
                                              (CHAR *)pLogBuffer, 
                                              pLogInfo->GetLoopThrehold(),
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
    m_pcFileName = NULL;
}


CModuleLogInfo::CModuleLogInfo ()
{
    m_lwTotalCount   = 0;
    m_lwForcedCount  = 0;
    m_lwMallocFail   = 0;
    m_lwEnqueueFail  = 0;
    m_lwSuccCount    = 0;
}


CModuleLogInfo::~CModuleLogInfo()
{
    m_lwTotalCount   = 0;
    m_lwForcedCount  = 0;
    m_lwMallocFail   = 0;
    m_lwEnqueueFail  = 0;
    m_lwSuccCount    = 0;
}


