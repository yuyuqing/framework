

#ifndef _PUB_MESSAGE_LOGGER_H_
#define _PUB_MESSAGE_LOGGER_H_


/* ��C���� */
#ifdef __cplusplus
extern "C"{
#endif


#include "pub_event.h"
#include "pub_global_def.h"


#define MAX_LOG_LEVEL_NUM                           ((BYTE)(8))
#define MAX_CELL_PER_GNB                            ((WORD32)(4))


typedef struct tagT_LogSetGlobalSwitch
{
    WORD32   dwProcID;    /* ����ID, ���̼����� */
    BOOL     bSwitch;     /* ��־ȫ�ֿ���; TRUE is open, FALSE is close */
}T_LogSetGlobalSwitch;


typedef struct tagT_LogSetWritePeriod
{
    BYTE     ucPeriod;    /* BU������, 0/1/2/3/4 : 5/10/15/30/60���� */
}T_LogSetWritePeriod;


typedef struct tagT_LogSetModuleSwitch
{
    BYTE     ucModuleID;    /* ģ��ID */
    BYTE     ucCellNum;     /* ָ��С���б�; bSwitchΪFALSEʱ���� */
    BYTE     ucLevelNum;    /* ָ�������б�; bSwitchΪFALSEʱ���� */
    BYTE     aucResved;

    BOOL     bSwitch;       /* ģ�鼶��־���� */

    /* Cell MOID, ����bSwitchΪTRUEʱ��Ч, ��ʶ��ǰ��ЩС��������־��ӡ */
    WORD16   awCellID[MAX_CELL_PER_GNB];

    /* ��־����,  ����bSwitchΪTRUEʱ��Ч, ��ʶ��ǰ��Щ��������־��ӡ */
    BYTE     aucLevelID[MAX_LOG_LEVEL_NUM];
}T_LogSetModuleSwitch;


#ifdef __cplusplus
}
#endif


#endif


