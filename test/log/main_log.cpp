

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>

#include "pub_message_logger.h"

#include "base_log_wrapper.h"
#include "base_timer_wrapper.h"


typedef struct tagT_CallInfo
{
    WORD32     dwProcID;
    WORD32     dwModuleID;
    WORD32     dwLogicalID;  /* 核索引(内部唯一标识Core) */
    WORD32     dwCount;
    pthread_t  tThreadID;
    CHAR       aucName[16];
    cpu_set_t  tCpuSet;
}T_CallInfo;


#define CALL_NUM  ((BYTE)(6))


T_CallInfo  g_atCallInfo[CALL_NUM] =
{
    {E_PROC_DU, E_DU_CL_UL,  5, 0, 0, "cl"},
    {E_PROC_DU, E_DU_MAC,    6, 0, 0, "mac"},
    {E_PROC_DU, E_DU_SCH,    7, 0, 0, "sch"},
    {E_PROC_DU, E_DU_SCHL1,  8, 0, 0, "schl1"},
    {E_PROC_DU, E_DU_SCHL2,  9, 0, 0, "schl2"},
    {E_PROC_DU, E_DU_RACH,  10, 0, 0, "rach"},
};


VOID * CallFunc(VOID *pArgs)
{
    T_CallInfo *ptCallInfo = (T_CallInfo *)pArgs;

    pthread_setaffinity_np(ptCallInfo->tThreadID, 
                           sizeof(cpu_set_t), 
                           &(ptCallInfo->tCpuSet));

    LogInit_Thread();

    BOOL    bFlag   = FALSE;
    BYTE    ucTmp1  = 0;
    CHAR    ucTmp2  = -100;
    WORD16  wTmp3   = 0;
    SWORD16 swTmp4  = -2000;
    WORD32  dwTmp5  = 0;
    SWORD32 sdwTmp6 = -30000;

    WORD32 dwSN = 0;
    
    while (TRUE)
    {
        ucTmp1++;
        ucTmp2++;
        wTmp3++;
        swTmp4++;
        dwTmp5++;
        sdwTmp6++;
        
        dwSN++;

        FAST_LOG_PRINTF(ptCallInfo->dwModuleID,
                        1,
                        E_LOG_LEVEL_INFO,
                        TRUE,
                        "LogVprintf, Logical_CHANNEL , "
                        "bFlag = %d, ucTmp1 : %u, ucTmp2 : %d, wTmp3 : %u, "
                        "swTmp4 : %d, dwTmp5 : %u, sdwTmp6 : %d, dwCount : %d\n",
                        bFlag,
                        ucTmp1,
                        ucTmp2, 
                        wTmp3,
                        swTmp4,
                        dwTmp5,
                        sdwTmp6,
                        dwSN);

        dwSN++;

        FAST_LOG_PRINTF(ptCallInfo->dwModuleID,
                        1,
                        E_LOG_LEVEL_INFO,
                        TRUE,
                        "LogVprintf, Logical_CHANNEL ,"
                        "bFlag = %d, ucTmp1 : %u, ucTmp2 : %d, wTmp3 : %u, "
                        "swTmp4 : %d, dwTmp5 : %u, dwCount : %d\n",
                        bFlag,
                        ucTmp1,
                        ucTmp2, 
                        wTmp3,
                        swTmp4,
                        dwTmp5,
                        dwSN);

        dwSN++;

        FAST_LOG_PRINTF(ptCallInfo->dwModuleID,
                        1,
                        E_LOG_LEVEL_INFO,
                        TRUE,
                        "LogVprintf, Logical_CHANNEL ,"
                        "bFlag = %d, ucTmp1 : %u, ucTmp2 : %d, dwCount : %d\n",
                        bFlag,
                        ucTmp1,
                        ucTmp2, 
                        dwSN);

        dwSN++;

        FAST_LOG_PRINTF(ptCallInfo->dwModuleID,
                        0xFFFF,
                        E_LOG_LEVEL_WARN,
                        TRUE,
                        "------------TestLog------------dwCount : %d\n",
                        dwSN);

        if (dwSN >= 16000000)
        {
            break ;
        }
    }

    LogExit_Thread();

    return NULL;
}


WORD32 CreateThread(T_CallInfo *ptCallInfo)
{
    CPU_ZERO(&(ptCallInfo->tCpuSet));
    CPU_SET(ptCallInfo->dwLogicalID, &(ptCallInfo->tCpuSet));

    if (0 != pthread_create(&(ptCallInfo->tThreadID), 
                            NULL, 
                            CallFunc, 
                            (VOID *)ptCallInfo))
    {
        return FAIL;
    }

    pthread_setname_np(ptCallInfo->tThreadID, ptCallInfo->aucName);

    return SUCCESS;
}


int main()
{
    LogInit_Process(E_PROC_DU, NULL);

    StartOam();

#if 1
    T_LogSetGlobalSwitch tMsg1;
    T_LogSetWritePeriod  tMsg2;
    T_LogSetModuleSwitch tMsg3;

    tMsg1.dwProcID = E_PROC_DU;
    tMsg1.bSwitch  = TRUE;
    SendMessageToLogThread(EV_BASE_LOG_SET_GLOBAL_SWITCH_ID, 
                           (const VOID *)(&tMsg1), 
                           sizeof(tMsg1));

    tMsg2.ucPeriod = 0;
    SendMessageToLogThread(EV_BASE_LOG_SET_WRITE_PERIOD_ID, 
                           (const VOID *)(&tMsg2), 
                           sizeof(tMsg2));

    tMsg3.ucModuleID = (BYTE)E_DU_SCH;
    tMsg3.ucCellNum  = 4;
    tMsg3.ucLevelNum = 7;
    tMsg3.bSwitch    = TRUE;

    SendMessageToLogThread(EV_BASE_LOG_SET_MODULE_SWITCH_ID, 
                           (const VOID *)(&tMsg3), 
                           sizeof(tMsg3));    
#endif

    for (WORD32 dwIndex = 0; dwIndex < CALL_NUM; dwIndex++)
    {
        CreateThread(&(g_atCallInfo[dwIndex]));
    }

    for (WORD32 dwIndex = 0; dwIndex < CALL_NUM; dwIndex++)
    {
        pthread_join(g_atCallInfo[dwIndex].tThreadID, NULL);
    }

    LogExit_Process();

    return SUCCESS;
}


