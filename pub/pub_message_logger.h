

#ifndef _PUB_MESSAGE_LOGGER_H_
#define _PUB_MESSAGE_LOGGER_H_


/* 与C兼容 */
#ifdef __cplusplus
extern "C"{
#endif


#include "pub_event.h"
#include "pub_global_def.h"


#define MAX_LOG_LEVEL_NUM                           ((BYTE)(8))
#define MAX_CELL_PER_GNB                            ((WORD32)(4))


typedef struct tagT_LogSetGlobalSwitch
{
    WORD32   dwProcID;    /* 进程ID, 进程级粒度 */
    BOOL     bSwitch;     /* 日志全局开关; TRUE is open, FALSE is close */
}T_LogSetGlobalSwitch;


typedef struct tagT_LogSetWritePeriod
{
    BYTE     ucPeriod;    /* BU级粒度, 0/1/2/3/4 : 5/10/15/30/60分钟 */
}T_LogSetWritePeriod;


typedef struct tagT_LogSetModuleSwitch
{
    BYTE     ucModuleID;    /* 模块ID */
    BYTE     ucCellNum;     /* 指定小区列表; bSwitch为FALSE时忽略 */
    BYTE     ucLevelNum;    /* 指定级别列表; bSwitch为FALSE时忽略 */
    BYTE     aucResved;

    BOOL     bSwitch;       /* 模块级日志开关 */

    /* Cell MOID, 仅当bSwitch为TRUE时有效, 标识当前哪些小区开启日志打印 */
    WORD16   awCellID[MAX_CELL_PER_GNB];

    /* 日志级别,  仅当bSwitch为TRUE时有效, 标识当前哪些级别开启日志打印 */
    BYTE     aucLevelID[MAX_LOG_LEVEL_NUM];
}T_LogSetModuleSwitch;


#ifdef __cplusplus
}
#endif


#endif


