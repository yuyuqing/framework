

#ifndef _BASE_VARIABLE_H_
#define _BASE_VARIABLE_H_


#include <atomic>
#include <pthread.h>

#include "pub_global_def.h"


#define THREAD_NAME_LEN              ((WORD32)(24))


/* ���߳�ά��ר�������� */
typedef struct tagT_DataZone
{
    WORD32      dwThreadIdx;     /* ���ڱ�ʶ�߳�ר���±�(���ͷ��߳�, ����:��־&��ʱ��Ring����) */
    WORD32      dwTimerInnerID;  /* ���ڷ��䶨ʱ��(�߳�ר��ID) */
    WORD32      dwThreadType;    /* ���ڱ�ʶWorker�߳�����(�̳߳ش���ʱ����Ч) */
    WORD32      dwThreadID;      /* ����gettid()��ȡ���߳�ID, ���ڱ�ʶ�߳�ID */

    VOID       *pLogMemPool;     /* ��־�ڴ�� */
    VOID       *pLogRing;        /* ��־Ring */
    VOID       *pLogger;         /* ��־Logger(���ڿ��ؿ���) */

    VOID       *pThread;         /* Worker�߳�ʵ��(�̳߳ش���ʱ����Ч) */

    pthread_t   tThreadID;       /* ����pthread_self()��ȡ���߳�ID */
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


