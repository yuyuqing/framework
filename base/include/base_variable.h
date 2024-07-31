

#ifndef _BASE_VARIABLE_H_
#define _BASE_VARIABLE_H_


#include <atomic>
#include <pthread.h>

#include "pub_global_def.h"


#define THREAD_NAME_LEN              ((WORD32)(24))


/* 按线程维护专属数据区 */
typedef struct tagT_DataZone
{
    WORD32      dwThreadIdx;     /* 用于标识线程专属下标(发送方线程, 例如:日志&定时器Ring队列) */
    WORD32      dwTimerInnerID;  /* 用于分配定时器(线程专属ID) */
    WORD32      dwThreadType;    /* 用于标识Worker线程类型(线程池创建时才有效) */
    WORD32      dwThreadID;      /* 调用gettid()获取的线程ID, 用于标识线程ID */

    VOID       *pLogMemPool;     /* 日志内存池 */
    VOID       *pLogRing;        /* 日志Ring */
    VOID       *pLogger;         /* 日志Logger(用于开关控制) */

    VOID       *pThread;         /* Worker线程实例(线程池创建时才有效) */

    pthread_t   tThreadID;       /* 调用pthread_self()获取的线程ID */
    CHAR        aucName[THREAD_NAME_LEN];
}T_DataZone;


extern __thread T_DataZone *m_pSelfThreadZone;
extern __thread WORD32      m_dwSelfRingID;
extern __thread WORD32      m_dwSelfTimerInnerID;
extern __thread WORD32      m_dwSelfThreadType;
extern __thread WORD32      m_dwSelfThreadID;
extern __thread WORD16      m_wSelfStackInc;


extern WORD32 g_dwMainThreadIdx;
extern WORD32 g_dwLogAppID;


extern const BYTE s_aucFileName[E_PROC_NUM][LOG_FILE_NAME_LEN];


class CGlobalClock;
class CLogInfo;
class COamApp;
class CTimerApp;
class CBaseThread;
class CAppCntrl;
class CThreadCntrl;
class CShmMgr;


extern CGlobalClock  *g_pGlobalClock;
extern CLogInfo      *g_pLogger;
extern COamApp       *g_pOamApp;
extern CTimerApp     *g_pTimerApp;
extern CBaseThread   *g_pLogThread;
extern CAppCntrl     *g_pAppCntrl;
extern CThreadCntrl  *g_pThreadPool;
extern CShmMgr       *g_pShmMgr;


#endif


