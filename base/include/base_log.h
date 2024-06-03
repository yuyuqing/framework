

#ifndef _BASE_LOG_H_
#define _BASE_LOG_H_


#include <fstream>
#include <string>
#include <ctime>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdarg>
#include <atomic>

#include "base_snprintf.h"
#include "base_time.h"
#include "base_mem_pool.h"
#include "base_config_file.h"
#include "base_ring_message.h"
#include "base_log_context.h"


typedef struct tagT_LogDualFile
{
    FILE     *pFile;
    CHAR      aucFileName[LOG_NAME_LEN];
}T_LogDualFile;


typedef enum tagE_LogFilePeriod
{
    E_LOG_PERIOD_05_MINUTE = 0,    /* 0 :   5分钟 */
    E_LOG_PERIOD_10_MINUTE,        /* 1 :  10分钟 */
    E_LOG_PERIOD_15_MINUTE,        /* 2 :  15分钟 */
    E_LOG_PERIOD_30_MINUTE,        /* 3 :  30分钟 */
    E_LOG_PERIOD_60_MINUTE,        /* 4 :  60分钟 */
    E_LOG_PERIOD_120_MINUTE,       /* 5 : 120分钟 */
    E_LOG_PERIOD_240_MINUTE,       /* 6 : 240分钟 */
    E_LOG_PERIOD_480_MINUTE,       /* 7 : 480分钟 */
    E_LOG_PERIOD_NUM,
}E_LogFilePeriod;


class CLogInfo : public CBaseData
{
public :
    friend class CLogContext;

    static BYTE   s_aucCstructPre[CONST_LOG_PRE_LEN];
    static BYTE   s_aucDstructPre[CONST_LOG_PRE_LEN];
    static BYTE   s_aucPrintPre[CONST_LOG_PRE_LEN];
    static WORD16 s_awPeriodMinute[E_LOG_PERIOD_NUM];

    enum { LOG_DUAL_FILE_NUM  =  2 };
    enum { LOG_WAIT_THRESHOLD = 32 };
    enum { LOG_LOCK_THRESHOLD =  2 };
    enum { LOG_LOOP_THRESHOLD =  8 };

public :
    CLogInfo ();
    virtual ~CLogInfo();

    /* 日志初始化接口
     * dwProcID  : 进程ID (指定业务进程)
     * pFileName : 日志文件名
     */
    WORD32 Initialize(WORD32 dwProcID, CHAR *pFileName);

    WORD32 FetchJsonConfig();

    /* 获取日志全局开关 */
    BOOL GetGlobalSwitch();

    /* 设置日志全局开关 */
    VOID SetGlobalSwitch(BOOL bFlag);

    /* 设置日志模块开关 */
    VOID SetModuleMask(WORD32 dwModuleID, BOOL bFlag);

    /* 清除小区和级别bitmap */
    VOID ClearExtModuleMask(WORD32 dwModuleID);

    /* 设置日志模块下的小区开关 */
    VOID SetExtModuleMask(WORD32 dwModuleID, WORD32 dwExtModuleID, BOOL bFlag);

    /* 设置日志模块下的级别开关 */
    VOID SetLevelMask(WORD32 dwModuleID, WORD32 dwLevelID, BOOL bFlag);

    /* 获取是否开启系统时钟同步标志 */
    BYTE GetSyncFlag();

    /* 获取日志维测周期 */
    WORD16 GetLogMeasure();

    /* 设置日志维测输出周期, 单位 : 分钟 */
    VOID SetLogMeasure(WORD32 dwLogMeasure);

    /* 获取日志文件切换周期(5/10/15/30/60/120/240/480分钟) */
    WORD16 GetPeriod();

    /* 设置日志文件切换周期(5/10/15/30/60/120/240/480分钟) */
    VOID SetPeriod(E_LogFilePeriod ePeriod);

    VOID SetLogFile();

    VOID OpenModuleLogMask(WORD32 dwModuleID);

    BOOL isAble();

    BOOL isAble(WORD32 dwModuleID, WORD32 dwExtModuleID, WORD32 dwLevelID);

    VOID SetPrintFlag(BOOL bFlag);

    BYTE GetPos();
    VOID SetPos(BYTE ucPos);

    FILE * GetFileID();

    WORD32 GetProcID();

    WORD16 GetWaitThreshold();

    WORD16 GetLockThreshold();

    WORD16 GetLoopThrehold();

    CModuleLogInfo * GetLogModule(WORD32 dwModuleID);

    VOID Trace(CLogMemPool *pMemPool,
               WORD32  dwModuleID,
               WORD32  dwExtModuleID,
               WORD32  dwLevelID,
               BOOL    bForced,
               CHAR   *pchPrtInfo, 
               ...);

    VOID Flush();

    WORD32 Assemble(T_LogBinaryMessage *ptMsg, CHAR *pDst, WORD32 &rdwMsgLen);

    VOID Dump();

protected :
    WORD32 InitDualFile(CHAR *pFileName);

private : 
    std::atomic<BOOL>     m_bGlobalSwitch;     /* 网管设置的日志全局开关 */    
    BOOL                  m_bEnable;           /* 日志全局开关 */    
    BOOL                  m_bPrintFlag;        /* 控制是否输出到屏幕, 用于FT */

    WORD32                m_dwProcID;          /* 进程ID(SCS/OAM/CU/DU/L1/...) */

    WORD16                m_wThresholdWait;    /* 日志策略控制, 等待消息队列长度 */
    WORD16                m_wThresholdLock;    /* 日志策略控制, 加锁循环次数 */
    WORD16                m_wThresholdLoop;    /* 日志策略控制, 循环最大次数后丢弃 */

    CBaseBitMap32         m_cModuleBitMap;     /* 模块bit掩码, 用于控制模块级别的日志输出 */
    CModuleLogInfo        m_aModule[E_LOG_MODULE_NUM];

    WORD16                m_wStackInc;         /* 用于提供给CLogContext控制输出格式 */

    BYTE                  m_ucSyncFlag;        /* 用于控制是否开启系统时钟同步 */
    WORD16                m_wLogMeasure;       /* 用于控制日志维测输出周期, 单位 : 分钟 */
    std::atomic<WORD16>   m_wPeriodMinute;     /* 5/10/15/30/60/120/240/480分钟, 日志文件切换间隔 */

    CHAR                  m_aucPath[LOG_FILE_NAME_LEN]; 
    std::atomic<BYTE>     m_ucPos;             /* 标识当前有效的日志文件 */
    T_LogDualFile         m_atDualFile[LOG_DUAL_FILE_NUM];
};


inline BOOL CLogInfo::isAble()
{
    return m_bEnable;
}


inline BOOL CLogInfo::isAble(WORD32 dwModuleID, WORD32 dwExtModuleID, WORD32 dwLevelID)
{
    /* 优先判断日志总开关 */
    if (!m_bEnable)
    {
        return FALSE;
    }

    if (unlikely(dwModuleID >= E_LOG_MODULE_NUM))
    {
        return FALSE;
    }

    __builtin_prefetch(&(m_aModule[dwModuleID]), 1, 2);

    if (!(m_cModuleBitMap & dwModuleID))
    {
        return FALSE;
    }

    if ( (m_aModule[dwModuleID].m_cExtModuleBitMap & dwExtModuleID)
      && (m_aModule[dwModuleID].m_cLevelBitMap & dwLevelID))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


inline WORD16 CLogInfo::GetWaitThreshold()
{
    return m_wThresholdWait;
}


inline WORD16 CLogInfo::GetLockThreshold()
{
    return m_wThresholdLock;
}


inline WORD16 CLogInfo::GetLoopThrehold()
{
    return m_wThresholdLoop;
}


inline CModuleLogInfo * CLogInfo::GetLogModule(WORD32 dwModuleID)
{
    if (dwModuleID >= E_LOG_MODULE_NUM)
    {
        return NULL;
    }

    return &(m_aModule[dwModuleID]);
}


#define LOG_THRESHOLD_LOW                    ((WORD32)(20))
#define LOG_THRESHOLD_HIGH                   ((WORD32)(16))
#define LOG_THREAD_ID                        ((WORD32)(0))


#define TRACE_STACK(STR)                                                      \
    CLogContext __log__((CHAR *)(__FILE__),                                   \
                        __LINE__,                                             \
                        const_cast<CHAR *>(STR),                              \
                        (CLogInfo *)(m_pSelfThreadZone->pLogger))


#define LOG_VPRINT(ModuleID, ExtModuleID, Level, Flag, INFOSTR, ...)          \
    do                                                                        \
    {                                                                         \
        CLogInfo *__pLogInfo__ = (CLogInfo *)(m_pSelfThreadZone->pLogger);    \
        __pLogInfo__->Trace((CLogMemPool *)(m_pSelfThreadZone->pLogMemPool),  \
                            (ModuleID),                                       \
                            (ExtModuleID),                                    \
                            (WORD32)(Level),                                  \
                            Flag,                                             \
                            (CHAR *)INFOSTR,                                  \
                            ##__VA_ARGS__);                                   \
    } while(0) 


#define DO_LOG_COMMON(ModuleID, CellID, LevelID, Forced, PRing)                              \
    if (unlikely((NULL == pMemPool) || (NULL == pLogInfo) || (NULL == PRing)))               \
    {                                                                                        \
        return ;                                                                             \
    }                                                                                        \
    if (FALSE == pLogInfo->isAble(ModuleID, CellID, LevelID))                                \
    {                                                                                        \
        return ;                                                                             \
    }                                                                                        \
    WORD64 lwStartCycle = GetCycle();                                                        \
    __builtin_prefetch(pMemPool, 1, 3);                                                      \
    pModule = pLogInfo->GetLogModule(ModuleID);                                              \
    WORD64 lwTotalCount = pModule->m_lwTotalCount.fetch_add(1, std::memory_order_relaxed);   \
    WORD32 dwFreeCount  = PRing->FreeCount();                                                \
    WORD32 dwWaitCount  = pLogInfo->GetWaitThreshold();                                      \
    if ((dwFreeCount <= LOG_THRESHOLD_HIGH) && (LOG_THREAD_ID != m_dwSelfThreadID))          \
    {                                                                                        \
        do                                                                                   \
        {                                                                                    \
            dwWaitCount--;                                                                   \
            dwFreeCount = PRing->FreeCount();                                                \
        }while((dwFreeCount <= LOG_THRESHOLD_LOW) && (dwWaitCount));                         \
        if (0 == dwWaitCount)                                                                \
        {                                                                                    \
            pModule->m_lwForcedCount++;                                                      \
            return ;                                                                         \
        }                                                                                    \
    }                                                                                        \
    pLogBuffer = pMemPool->Malloc(MAX_LOG_STR_LEN, pLogInfo->GetLockThreshold());            \
    if (NULL == pLogBuffer)                                                                  \
    {                                                                                        \
        pModule->m_lwMallocFail++;                                                           \
        return ;                                                                             \
    }                                                                                        \
    __builtin_prefetch(pLogBuffer, 1, 1)


#define LOG_PREFIX_PLACEHOLDER    ((WORD32)(83))
#define LOG_YEAR_POS              ((WORD32)(0))
#define LOG_YEAR_PLACEHOLDER      ((WORD32)(4))
#define LOG_MONTH_POS             ((WORD32)(5))
#define LOG_MONTH_PLACEHOLDER     ((WORD32)(2))
#define LOG_DAY_POS               ((WORD32)(8))
#define LOG_DAY_PLACEHOLDER       ((WORD32)(2))
#define LOG_HOUR_POS              ((WORD32)(11))
#define LOG_HOUR_PLACEHOLDER      ((WORD32)(2))
#define LOG_MINUT_POS             ((WORD32)(14))
#define LOG_MINUT_PLACEHOLDER     ((WORD32)(2))
#define LOG_SECOND_POS            ((WORD32)(17))
#define LOG_SECOND_PLACEHOLDER    ((WORD32)(2))
#define LOG_MICROS_POS            ((WORD32)(20))
#define LOG_MICROS_PLACEHOLDER    ((WORD32)(6))
#define LOG_MODULE_POS            ((WORD32)(28))
#define LOG_MODULE_PLACEHOLDER    ((WORD32)(18))
#define LOG_LEVEL_POS             ((WORD32)(47))
#define LOG_LEVEL_PLACEHOLDER     ((WORD32)(6))
#define LOG_CELL_POS              ((WORD32)(61))
#define LOG_SN_POS                ((WORD32)(68))
#define LOG_SN_PLACEHOLDER        ((WORD32)(12))

extern const CHAR s_aucLogPrefix[LOG_PREFIX_PLACEHOLDER];

#define DO_LOG_PREFIX(DWSize, PLogBuffer, LWSec, DWMicro, PModuleStr, PLevelStr, UCCell, LWTotal)       \
        do                                                                                              \
        {                                                                                               \
            tm      tTMNow;                                                                             \
            time_t  tSec   = LWSec;                                                                     \
            LocalTime(&tSec, tTMNow);                                                                   \
            WORD16  WYear  = (WORD16)(tTMNow.tm_year + BaseTimeYear);                                   \
            BYTE    UCMon  = (BYTE)(tTMNow.tm_mon + 1);                                                 \
            BYTE    UCDay  = (BYTE)(tTMNow.tm_mday);                                                    \
            BYTE    UCHour = (BYTE)(tTMNow.tm_hour);                                                    \
            BYTE    UCMin  = (BYTE)(tTMNow.tm_min);                                                     \
            BYTE    UCSec  = (BYTE)(tTMNow.tm_sec);                                                     \
            CHAR   *pTmp = PLogBuffer;                                                                  \
            memcpy(pTmp, s_aucLogPrefix, LOG_PREFIX_PLACEHOLDER);                                       \
            IntToStr(pTmp + LOG_YEAR_POS,   WYear,     E_DECIMAL_10);                                   \
            IntToStr(pTmp + LOG_MONTH_POS,  UCMon,     E_DECIMAL_10, LOG_MONTH_PLACEHOLDER,  '0');      \
            IntToStr(pTmp + LOG_DAY_POS,    UCDay,     E_DECIMAL_10, LOG_DAY_PLACEHOLDER,    '0');      \
            IntToStr(pTmp + LOG_HOUR_POS,   UCHour,    E_DECIMAL_10, LOG_HOUR_PLACEHOLDER,   '0');      \
            IntToStr(pTmp + LOG_MINUT_POS,  UCMin,     E_DECIMAL_10, LOG_MINUT_PLACEHOLDER,  '0');      \
            IntToStr(pTmp + LOG_SECOND_POS, UCSec,     E_DECIMAL_10, LOG_SECOND_PLACEHOLDER, '0');      \
            IntToStr(pTmp + LOG_MICROS_POS, DWMicro,   E_DECIMAL_10, LOG_MICROS_PLACEHOLDER, '0');      \
            memcpy(pTmp + LOG_MODULE_POS, PModuleStr, LOG_MODULE_PLACEHOLDER);                          \
            memcpy(pTmp + LOG_LEVEL_POS,  PLevelStr,  LOG_LEVEL_PLACEHOLDER);                           \
            *(pTmp + LOG_CELL_POS) = UCCell;                                                            \
            IntToStr(pTmp + LOG_SN_POS,     LWTotal,   E_DECIMAL_10, LOG_SN_PLACEHOLDER,     ' ');      \
            DWSize = (LOG_PREFIX_PLACEHOLDER - 1);                                                      \
        } while(0)


#endif


