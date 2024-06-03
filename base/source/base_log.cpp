

#include "base_string.h"
#include "base_thread_log.h"


BYTE   CLogInfo::s_aucCstructPre[]  = {'>', '>', '>', '>'};
BYTE   CLogInfo::s_aucDstructPre[]  = {'<', '<', '<', '<'};
BYTE   CLogInfo::s_aucPrintPre[]    = {' ', ' ', ' ', ' '};
WORD16 CLogInfo::s_awPeriodMinute[] = {  5,  10,  15,  30,  60, 120, 240, 480};


const CHAR s_aucLogPrefix[LOG_PREFIX_PLACEHOLDER] =
{
      0,   0,   0,   0, '-',   0,   0, '-',   0,   0, ' ', 
      0,   0, ':',   0,   0, ':',   0,   0, '.',   0,   0,   0,   0,   0,   0, ' ',
    '[',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, ':',
      0,   0,   0,   0,   0,   0, ']', ' ', '[', 'C', 'e', 'l', 'l', ':',   0,
    ']', ' ', '[', 'S', 'N', ' ',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, ']', ' ', 0
};


CLogInfo::CLogInfo ()
{
    m_bGlobalSwitch  = FALSE;
    m_bEnable        = FALSE;
    m_bPrintFlag     = FALSE;
    m_dwProcID       = INVALID_DWORD;
    m_wThresholdWait = LOG_WAIT_THRESHOLD;
    m_wThresholdLock = LOG_LOCK_THRESHOLD;
    m_wThresholdLoop = LOG_LOOP_THRESHOLD;
    m_wStackInc      = 0;
    m_ucSyncFlag     = 0;
    m_wLogMeasure    = s_awPeriodMinute[E_LOG_PERIOD_05_MINUTE];
    m_wPeriodMinute  = s_awPeriodMinute[E_LOG_PERIOD_05_MINUTE];
    m_ucPos          = 0;

    memset(m_aucPath,          0x00, sizeof(m_aucPath));
    memset(&(m_atDualFile[0]), 0x00, sizeof(m_atDualFile));
}


CLogInfo::~CLogInfo()
{
    /* 此处不执行close日志文件, 交由ClogThread析构时close */
    m_bGlobalSwitch  = FALSE;
    m_bEnable        = FALSE;
    m_bPrintFlag     = FALSE;
    m_dwProcID       = INVALID_DWORD;
    m_wThresholdWait = LOG_WAIT_THRESHOLD;
    m_wThresholdLock = LOG_LOCK_THRESHOLD;
    m_wThresholdLoop = LOG_LOOP_THRESHOLD;
    m_wStackInc      = 0;
    m_ucSyncFlag     = 0;
    m_wLogMeasure    = s_awPeriodMinute[E_LOG_PERIOD_05_MINUTE];
    m_wPeriodMinute  = s_awPeriodMinute[E_LOG_PERIOD_05_MINUTE];
    m_ucPos          = 0;

    memset(m_aucPath,          0x00, sizeof(m_aucPath));
    memset(&(m_atDualFile[0]), 0x00, sizeof(m_atDualFile));
}


WORD32 CLogInfo::InitDualFile(CHAR *pFileName)
{
    if (NULL == pFileName)
    {
        return FAIL;
    }

    WORD32 dwResult = 0;

    CLogThread *pLogThread = static_cast<CLogThread *>(g_pLogThread);

    for (BYTE ucIndex = 0; ucIndex < LOG_DUAL_FILE_NUM; ucIndex++)
    {
        CString<LOG_NAME_LEN> cName(m_aucPath);

        CHAR ucNum = ucIndex + '0';

        cName += pFileName;
        cName += '_';
        cName += ucNum;
        cName += (CHAR *)".log";

        memcpy(m_atDualFile[ucIndex].aucFileName, 
               cName.toChar(), 
               cName.Length());

        m_atDualFile[ucIndex].pFile = fopen(m_atDualFile[ucIndex].aucFileName, "wb");
        if (NULL == m_atDualFile[ucIndex].pFile)
        {
            return FAIL;
        }

        dwResult = pLogThread->RegistLogFile(m_atDualFile[ucIndex].pFile, 
                                             this,
                                             ucIndex,
                                             CLogThread::LOG_FILE_SIZE);
        if (SUCCESS != dwResult)
        {
            return FAIL;
        }
    }

    return SUCCESS;
}


WORD32 CLogInfo::Initialize(WORD32 dwProcID, CHAR *pFileName)
{
    m_dwProcID = dwProcID;

    FetchJsonConfig();

    WORD32 dwResult = InitDualFile(pFileName);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    /* 初始化日志时, 设置当前写日志文件 */
    SetLogFile();

    return SUCCESS;
}


WORD32 CLogInfo::FetchJsonConfig()
{
    T_LogJsonCfg &rJsonCfg = CBaseConfigFile::GetInstance()->GetLogJsonCfg();

    memcpy(m_aucPath, rJsonCfg.aucPath, LOG_FILE_NAME_LEN);

    SetGlobalSwitch(rJsonCfg.bGlobalSwitch);
    SetLogMeasure(rJsonCfg.dwLogMeasure);
    SetPeriod((E_LogFilePeriod)(rJsonCfg.dwWriteFilePeriod));

    m_ucSyncFlag     = (rJsonCfg.dwSyncFlag) ? 1 : 0;
    m_wThresholdWait = (rJsonCfg.wThresholdWait) ? rJsonCfg.wThresholdWait : LOG_WAIT_THRESHOLD;
    m_wThresholdLock = (rJsonCfg.wThresholdLock) ? rJsonCfg.wThresholdLock : LOG_LOCK_THRESHOLD;
    m_wThresholdLoop = (rJsonCfg.wThresholdLoop) ? rJsonCfg.wThresholdLoop : LOG_LOOP_THRESHOLD;

    T_LogJsonModule *ptModule = NULL;

    for (WORD32 dwIndex = 0; dwIndex < rJsonCfg.dwModuleNum; dwIndex++)
    {
        ptModule = &(rJsonCfg.atModule[dwIndex]);

        SetModuleMask(ptModule->dwModuleID, ptModule->bSwitch);

        for (WORD32 dwIndex1 = 0; dwIndex1 < ptModule->ucCellNum; dwIndex1++)
        {
            SetExtModuleMask(ptModule->dwModuleID,
                             ptModule->adwCellID[dwIndex1],
                             TRUE);
        }

        for (WORD32 dwIndex1 = 0; dwIndex1 < ptModule->ucLevelNum; dwIndex1++)
        {
            SetLevelMask(ptModule->dwModuleID,
                         ptModule->adwLevelID[dwIndex1],
                         TRUE);
        }
    }

    return SUCCESS;
}


BOOL CLogInfo::GetGlobalSwitch()
{
    m_bEnable = m_bGlobalSwitch.load(std::memory_order_relaxed);
    return m_bEnable;
}


VOID CLogInfo::SetGlobalSwitch(BOOL bFlag)
{
    m_bGlobalSwitch.store(bFlag, std::memory_order_relaxed);
    m_bEnable = m_bGlobalSwitch.load(std::memory_order_relaxed);
}


VOID CLogInfo::SetModuleMask(WORD32 dwModuleID, BOOL bFlag)
{
    if (bFlag)
    {
        m_cModuleBitMap.SetBitMap(dwModuleID);
    }
    else
    {
        m_cModuleBitMap.RemoveBitMap(dwModuleID);
    }
}


VOID CLogInfo::ClearExtModuleMask(WORD32 dwModuleID)
{
    if (dwModuleID >= E_LOG_MODULE_NUM)
    {
        return ;
    }

    m_aModule[dwModuleID].m_cExtModuleBitMap.ClearBitMap();
    m_aModule[dwModuleID].m_cLevelBitMap.ClearBitMap();
}


VOID CLogInfo::SetExtModuleMask(WORD32 dwModuleID, 
                                WORD32 dwExtModuleID, 
                                BOOL   bFlag)
{
    if (dwModuleID >= E_LOG_MODULE_NUM)
    {
        return ;
    }

    if (bFlag)
    {
        m_aModule[dwModuleID].m_cExtModuleBitMap.SetBitMap(dwExtModuleID);
    }
    else
    {
        m_aModule[dwModuleID].m_cExtModuleBitMap.RemoveBitMap(dwExtModuleID);
    }    
}


VOID CLogInfo::SetLevelMask(WORD32 dwModuleID, WORD32 dwLevelID, BOOL bFlag)
{
    if (dwModuleID >= E_LOG_MODULE_NUM)
    {
        return ;
    }

    if (bFlag)
    {
        m_aModule[dwModuleID].m_cLevelBitMap.SetBitMap(dwLevelID);
    }
    else
    {
        m_aModule[dwModuleID].m_cLevelBitMap.RemoveBitMap(dwLevelID);
    }
}


/* 获取是否开启系统时钟同步标志 */
BYTE CLogInfo::GetSyncFlag()
{
    return m_ucSyncFlag;
}


WORD16 CLogInfo::GetLogMeasure()
{
    return m_wLogMeasure;
}


VOID CLogInfo::SetLogMeasure(WORD32 dwLogMeasure)
{
    if (0 == dwLogMeasure)
    {
        dwLogMeasure = s_awPeriodMinute[E_LOG_PERIOD_05_MINUTE];
    }

    m_wLogMeasure = (WORD16)(dwLogMeasure);
}


WORD16 CLogInfo::GetPeriod()
{
    WORD16 wPeriod = m_wPeriodMinute.load(std::memory_order_relaxed);

    return wPeriod;
}


VOID CLogInfo::SetPeriod(E_LogFilePeriod ePeriod)
{
    if (ePeriod >= E_LOG_PERIOD_NUM)
    {
        ePeriod = E_LOG_PERIOD_480_MINUTE;
    }

    m_wPeriodMinute.store(s_awPeriodMinute[ePeriod], std::memory_order_relaxed);
}


VOID CLogInfo::SetLogFile()
{
    WORD16 wPeriodMinute = GetPeriod();

    tm     tTime;
    WORD64 lwMicroSec = 0;
    WORD64 lwCycle    = 0;

    g_pGlobalClock->GetTime3(lwMicroSec, lwCycle);

    time_t  tSec   = lwMicroSec / 1000000;
    LocalTime(&tSec, tTime);
    WORD32  dwHour = (WORD32)(tTime.tm_hour);
    WORD32  dwMin  = (WORD32)(tTime.tm_min);
    WORD32  dwSec  = (WORD32)(tTime.tm_sec);
    WORD32  dwMil  = (WORD32)((lwMicroSec / 1000) % 1000);

    WORD32 dwTimeStamp = ((((dwHour * 60) + dwMin) * 60) + dwSec) * 1000 
                       + dwMil + LOG_PERIOD_AHEAD;
    WORD32 dwPeriodMil = wPeriodMinute * 60000;

    BYTE ucPos = (BYTE)((dwTimeStamp / dwPeriodMil) % CLogInfo::LOG_DUAL_FILE_NUM);

    m_ucPos.store(ucPos, std::memory_order_relaxed);
}


VOID CLogInfo::OpenModuleLogMask(WORD32 dwModuleID)
{
    if (dwModuleID >= E_LOG_MODULE_NUM)
    {
        return ;
    }

    SetModuleMask(dwModuleID, TRUE);

    for (WORD32 dwIndex1 = 0; 
         dwIndex1 < ((CBaseBitMap02::BITMAP_BYTE_NUM) * BIT_NUM_PER_BYTE); 
         dwIndex1++)
    {
        SetExtModuleMask(dwModuleID, dwIndex1, TRUE);
    }

    for (WORD32 dwIndex2 = 0; dwIndex2 < E_LOG_LEVEL_NUM; dwIndex2++)
    {
        SetLevelMask(dwModuleID, dwIndex2, TRUE);
    }
}


VOID CLogInfo::SetPrintFlag(BOOL bFlag)
{
    m_bPrintFlag = bFlag;
}


BYTE CLogInfo::GetPos()
{
    BYTE ucPos = m_ucPos.load(std::memory_order_relaxed);

    return ucPos;
}


VOID CLogInfo::SetPos(BYTE ucPos)
{
    BYTE ucPosMod = ucPos % LOG_DUAL_FILE_NUM;

    if (NULL != m_atDualFile[ucPosMod].pFile)
    {
        //ftruncate(fileno(m_atDualFile[ucPosMod].pFile), 0);
        //fflush(m_atDualFile[ucPosMod].pFile);
        fseek(m_atDualFile[ucPosMod].pFile, 0, SEEK_SET);
    }

    m_ucPos.store(ucPos, std::memory_order_relaxed);
}


FILE * CLogInfo::GetFileID()
{
    BYTE ucPos = m_ucPos.load(std::memory_order_relaxed);

    return m_atDualFile[ucPos % LOG_DUAL_FILE_NUM].pFile;
}


WORD32 CLogInfo::GetProcID()
{
    return m_dwProcID;
}


VOID CLogInfo::Trace(CLogMemPool *pMemPool,
                     WORD32       dwModuleID,
                     WORD32       dwExtModuleID,
                     WORD32       dwLevelID,
                     BOOL         bForced,
                     CHAR        *pchPrtInfo, 
                     ...)
{
    if (unlikely((NULL == pchPrtInfo) 
              || (NULL == m_pSelfThreadZone)))
    {
        return ;
    }

    CLogThread     *pLogThread = static_cast<CLogThread *>(g_pLogThread);
    CLogInfo       *pLogInfo   = this;
    CModuleLogInfo *pModule    = NULL;
    BYTE           *pLogBuffer = NULL;
    WORD16          wStachInc  = 0;

    CMultiMessageRing::CSTRing *pRing = (CMultiMessageRing::CSTRing *)(m_pSelfThreadZone->pLogRing);

    DO_LOG_COMMON(dwModuleID, dwExtModuleID, dwLevelID, bForced, pRing);

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
                  ((CHAR)((dwExtModuleID == 0xFFFF) ? 'X' : (dwExtModuleID  + '0'))),
                  lwTotalCount);

    wStachInc = (0 == m_wStackInc) ?
                    0 : (m_wStackInc - CONST_LOG_PRE_LEN);
    if (wStachInc > 0)
    {
        memset((pLogBuffer + dwLen), 
               ' ', 
               wStachInc);
    }

    memcpy((pLogBuffer + dwLen + wStachInc), 
           CLogInfo::s_aucPrintPre, 
           CONST_LOG_PRE_LEN);

    wStachInc += CONST_LOG_PRE_LEN;

    va_list  tParamList;
    va_start(tParamList, pchPrtInfo);
    base_vsnprintf((CHAR *)(pLogBuffer + dwLen + wStachInc), 
                   (MAX_LOG_STR_LEN - dwLen - wStachInc), 
                   ' ', 
                   pchPrtInfo, 
                   tParamList); 
    va_end(tParamList);

    /* TraceMe打印 */    
    WORD32 dwResult = pLogThread->NormalWrite(GetFileID(),
                                              (WORD16)dwModuleID,
                                              strlen((CHAR *)pLogBuffer),
                                              (CHAR *)pLogBuffer,
                                              bForced ? m_wThresholdLoop : 0,
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


VOID CLogInfo::Flush()
{
    BYTE ucPos = m_ucPos.load(std::memory_order_relaxed);

    if (NULL != m_atDualFile[ucPos].pFile)
    {
        fflush(m_atDualFile[ucPos].pFile);
    }
}


WORD32 CLogInfo::Assemble(T_LogBinaryMessage *ptMsg, CHAR *pDst, WORD32 &rdwMsgLen)
{
    /* 添加日期时间/日志级别信息 */
    /* 添加日期时间/日志级别信息 */
    DO_LOG_PREFIX(rdwMsgLen, 
                  ((CHAR *)pDst),
                  (ptMsg->lwSeconds),
                  (ptMsg->dwMicroSec),
                  ((CHAR *)(s_aucModule[ptMsg->wModuleID])),
                  ((CHAR *)(s_aucLevel[ptMsg->ucLevelID])),
                  (ptMsg->ucCell),
                  (ptMsg->lwTotalCount));

    *(pDst + rdwMsgLen) = '[';
    rdwMsgLen++;

    memcpy(pDst + rdwMsgLen, ptMsg->aucFile, ptMsg->ucFileLen);
    rdwMsgLen += ptMsg->ucFileLen;

    rdwMsgLen += base_assemble((CHAR *)(pDst + rdwMsgLen), 
                               (MAX_LOG_STR_LEN - rdwMsgLen), 
                               ptMsg->wStrLen,
                               ptMsg->aucFormat, 
                               ptMsg->ucParamNum,
                               ptMsg->alwParams); 

    return SUCCESS;
}


VOID CLogInfo::Dump()
{
    TRACE_STACK("CLogInfo::Dump()");

    BYTE   ucPos         = m_ucPos.load(std::memory_order_relaxed);
    WORD16 wPeriod       = m_wPeriodMinute.load(std::memory_order_relaxed);
    WORD64 lwTotalCount  = 0;
    WORD64 lwForcedCount = 0;
    WORD64 lwMallocFail  = 0;
    WORD64 lwEnqueueFail = 0;
    WORD64 lwSuccCount   = 0;

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "File0 : %s, File1 : %s, Period : %d, m_ucPos : %d, "
               "m_wThresholdWait : %d, m_wThresholdLock : %d, m_wThresholdLoop : %d\n",
               m_atDualFile[0 % LOG_DUAL_FILE_NUM].aucFileName,
               m_atDualFile[1 % LOG_DUAL_FILE_NUM].aucFileName,
               wPeriod,
               ucPos,
               m_wThresholdWait,
               m_wThresholdLock,
               m_wThresholdLoop);

    for (WORD32 dwIndex = 0; dwIndex < E_LOG_MODULE_NUM; dwIndex++)
    {
        if (FALSE == (m_cModuleBitMap & dwIndex))
        {
            continue ;
        }

        lwTotalCount  = m_aModule[dwIndex].m_lwTotalCount.load(std::memory_order_relaxed);
        lwForcedCount = m_aModule[dwIndex].m_lwForcedCount.load(std::memory_order_relaxed);
        lwMallocFail  = m_aModule[dwIndex].m_lwMallocFail.load(std::memory_order_relaxed);
        lwEnqueueFail = m_aModule[dwIndex].m_lwEnqueueFail.load(std::memory_order_relaxed);
        lwSuccCount   = m_aModule[dwIndex].m_lwSuccCount.load(std::memory_order_relaxed);

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                   "%s, lwTotalCount : %ld, lwForcedCount : %ld, "
                   "lwMallocFail : %ld, m_lwEnqueueFail : %ld, m_lwSuccCount : %d\n",
                   (BYTE *)(s_aucModule[dwIndex]),
                   lwTotalCount,
                   lwForcedCount,
                   lwMallocFail,
                   lwEnqueueFail,
                   lwSuccCount);

        if (0 == lwTotalCount)
        {
            continue ;
        }

        m_aModule[dwIndex].m_cExtModuleBitMap.Dump("CellBitMap  ");
        m_aModule[dwIndex].m_cLevelBitMap.Dump("LevelBitMap ");
    }

    m_cModuleBitMap.Dump("ModuleBitMap");
}


