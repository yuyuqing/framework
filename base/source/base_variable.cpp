

#include "base_variable.h"


__thread T_DataZone *m_pSelfThreadZone = NULL;


__thread WORD32 m_dwSelfRingID       = 0;
__thread WORD32 m_dwSelfTimerInnerID = 0;
__thread WORD32 m_dwSelfThreadType   = 0;
__thread WORD32 m_dwSelfThreadID     = 0;


WORD32 g_dwMainThreadIdx = INVALID_DWORD;
WORD32 g_dwLogAppID      = INVALID_DWORD;


const BYTE s_aucFileName[E_PROC_NUM][LOG_FILE_NAME_LEN] = 
{
    "scs",        /* E_PROC_SCS */
    "oam",        /* E_PROC_OAM */
    "cu",         /* E_PROC_CU */
    "du",         /* E_PROC_DU */
    "l1",         /* E_PROC_L1 */
    "dhcp",       /* E_PROC_DHCP */
    "tr069",      /* E_PROC_TR069 */
    "monitor",    /* E_PROC_MONITOR */
    "main",       /* E_PROC_FT */
};


CGlobalClock  *g_pGlobalClock = NULL;
CLogInfo      *g_pLogger      = NULL;
COamApp       *g_pOamApp      = NULL;
CTimerApp     *g_pTimerApp    = NULL;
CBaseThread   *g_pLogThread   = NULL;
CThreadCntrl  *g_pThreadPool  = NULL;
CShmMgr       *g_pShmMgr      = NULL;


