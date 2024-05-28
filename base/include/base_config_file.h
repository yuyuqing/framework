

#ifndef _BASE_CONFIG_FILE_H_
#define _BASE_CONFIG_FILE_H_


#include <fstream>

#include "pub_global_def.h"

#include "base_string.h"
#include "base_util.h"
#include "base_singleton_tpl.h"


#define LOG_JSON_BITMAP_LEN          ((WORD32)(BITMAP_BYTE_NUM_02 * BIT_NUM_PER_BYTE))

#define DEFAULT_LOG_FILE_SIZE        ((WORD64)(100 * BYTE_NUM_PER_MB))


#define MAX_BLOCK_NUM                ((WORD32)(32))
#define MAX_POOL_NUM                 ((WORD32)(8))
#define MAX_WORKER_NUM               ((WORD32)(16))
#define MAX_APP_NUM                  ((WORD32)(64))
#define WORKER_NAME_LEN              ((WORD32)(32))
#define APP_NAME_LEN                 ((WORD32)(32))
#define MAX_APP_NUM_PER_THREAD       ((WORD32)(12))
#define MAX_ASSOCIATE_NUM_PER_APP    ((WORD32)(16))

#define EAL_CORE_ARG_LEN             ((WORD32)(64))
#define MAX_DEV_PORT_NUM             ((WORD32)(4))
#define IPV4_STRING_LEN              ((WORD32)(16))
#define IPV6_STRING_LEN              ((WORD32)(40))
#define MAX_DEV_IP_NUM               ((WORD32)(2))
#define MAX_DEV_VLAN_NUM             ((WORD32)(16))


extern const BYTE s_aucModule[E_LOG_MODULE_NUM][LOG_MODULE_LEN];


typedef struct tagT_MemJsonBlock
{
    WORD32    dwTrunkSize;
    WORD32    dwPowerNum;
}T_MemJsonBlock;


typedef struct tagT_MemJsonPool
{
    WORD32          dwPoolID;
    WORD32          dwBlockNum;
    T_MemJsonBlock  atBlock[MAX_BLOCK_NUM];
}T_MemJsonPool;


typedef struct tagT_MemJsonCfg
{
    BYTE              ucMemType;   /* 0:ShareMem; 1:HugePage; 2:Heap */
    BYTE              ucPageNum;
    BYTE              ucPoolNum;
    BYTE              ucResved;
    WORD32            dwMemSize;
    T_MemJsonPool     atPool[MAX_POOL_NUM];
}T_MemJsonCfg;


typedef struct tagT_LogJsonModule
{
    CHAR    aucName[LOG_MODULE_LEN];
    WORD32  dwModuleID;    /* �������Ʋ�ѯ����, ����Ҫ��json�ļ��ж��� */
    BOOL    bSwitch;
    BYTE    ucCellNum;     /* ȡֵΪjson�ж����С���б����� */
    BYTE    ucLevelNum;    /* ȡֵΪjson�ж���ļ����б����� */
    WORD32  adwCellID[LOG_JSON_BITMAP_LEN];
    WORD32  adwLevelID[LOG_JSON_BITMAP_LEN];
}T_LogJsonModule;


typedef struct tagT_LogJsonCfg
{
    WORD32           dwCoreID;
    WORD32           dwPolicy;
    WORD32           dwPriority;
    WORD32           dwStackSize;
    CHAR             aucPath[LOG_FILE_NAME_LEN];
    BOOL             bMeasSwitch;
    BOOL             bGlobalSwitch;
    WORD32           dwLogMeasure;       /* ��־ά�����ʱ��(��λ : ����) */
    WORD32           dwWriteFilePeriod;  /* ȡֵ(0/1/2/3/4/5/6), ��Ӧ(5/10/15/30/60/120/240����) */
    WORD16           wThresholdWait;     /* ��־���� */
    WORD16           wThresholdLock;     /* ��־���� */    
    WORD16           wThresholdLoop;     /* ��־���� */
    WORD32           dwModuleNum;        /* ȡֵΪjson�ж����ģ���б����� */
    WORD64           lwMaxFileSize;      /* �����ļ���󳤶� */
    T_LogJsonModule  atModule[E_LOG_MODULE_NUM];
}T_LogJsonCfg;


typedef struct tagT_TimerJsonCfg
{
    BOOL             bCreateFlag;
    WORD32           dwCoreID;
    WORD32           dwPolicy;
    WORD32           dwPriority;
    WORD32           dwStackSize;
    WORD32           dwServTimeLen;
    WORD32           dwAppTimeLen;
    BOOL             bAloneLog;
}T_TimerJsonCfg;


typedef struct tagT_ShmJsonCfg
{
    BOOL             bCreateFlag;
    BOOL             bMaster;
    WORD32           dwChannelNum;
    WORD32           dwPowerNum;
}T_ShmJsonCfg;


typedef struct tagT_AppJsonCfg
{
    CHAR       aucName[APP_NAME_LEN];
    WORD32     dwAppID;
    WORD32     dwEventBegin;
    BOOL       bAssocFlag;
    WORD32     dwAssocNum;
    WORD32     adwAssocID[MAX_ASSOCIATE_NUM_PER_APP];
}T_AppJsonCfg;


typedef struct tagT_ThreadJsonCfg
{
    CHAR          aucType[WORKER_NAME_LEN];
    CHAR          aucName[WORKER_NAME_LEN];
    WORD32        dwThreadID;
    WORD32        dwLogicalID;
    WORD32        dwPolicy;
    WORD32        dwPriority;
    WORD32        dwStackSize;
    WORD32        dwCBNum;
    WORD32        dwPacketCBNum;
    BOOL          bAloneLog;
    WORD32        dwAppNum;
    T_AppJsonCfg  atApp[MAX_APP_NUM_PER_THREAD];
}T_ThreadJsonCfg;


typedef struct tagT_ThreadPoolJsonCfg
{
    WORD32           dwWorkerNum;
    T_ThreadJsonCfg  atWorker[MAX_WORKER_NUM];
}T_ThreadPoolJsonCfg;


typedef struct tagT_DpdkEthVlanJsonCfg
{
    WORD32               dwVlanID;
    WORD32               dwPriority;
    CHAR                 aucIpv4Addr[IPV4_STRING_LEN];
    CHAR                 aucIpv4GW[IPV4_STRING_LEN];
}T_DpdkEthVlanJsonCfg;


typedef struct tagT_DpdkEthIPJsonCfg
{
    WORD32               dwIPType;
    CHAR                 aucIpv4Addr[IPV4_STRING_LEN];
    CHAR                 aucIpv4GW[IPV4_STRING_LEN];
    CHAR                 aucIpv6Addr[IPV6_STRING_LEN];
    CHAR                 aucIpv6GW[IPV6_STRING_LEN];
}T_DpdkEthIPJsonCfg;


typedef struct tagT_DpdkEthDevJsonCfg
{
    WORD32               dwPort;
    WORD32               dwQueueNum;
    WORD32               dwLinkType;    /* ������������(0:Access, 1:Trunk, 2:Hybrid) */
    WORD32               dwIpNum;
    WORD32               dwVlanNum;
    T_DpdkEthIPJsonCfg   atIP[MAX_DEV_IP_NUM];
    T_DpdkEthVlanJsonCfg atVlan[MAX_DEV_VLAN_NUM];
}T_DpdkEthDevJsonCfg;


typedef struct tagT_DpdkJsonCfg
{
    BOOL                 bInitFlag;
    WORD32               dwEthNum;
    CHAR                 aucCoreArg[EAL_CORE_ARG_LEN];
    T_DpdkEthDevJsonCfg  atEthDev[MAX_DEV_PORT_NUM];
}T_DpdkJsonCfg;


typedef struct tagT_RootJsonCfg
{
    T_MemJsonCfg         tMemConfig;
    T_LogJsonCfg         tLogConfig;
    T_TimerJsonCfg       tTimerConfig;
    T_ShmJsonCfg         tShmConfig;
    T_ThreadPoolJsonCfg  tWorkerConfig;
    T_DpdkJsonCfg        tDpdkConfig;
}T_RootJsonCfg;


class CBaseConfigFile : public CSingleton<CBaseConfigFile>
{
public :
    CBaseConfigFile ();
    virtual ~CBaseConfigFile();

    WORD32 ParseFile();

    T_MemJsonCfg        & GetMemJsonCfg();
    T_LogJsonCfg        & GetLogJsonCfg();
    T_TimerJsonCfg      & GetTimerJsonCfg();
    T_ShmJsonCfg        & GetShmJsonCfg();
    T_ThreadPoolJsonCfg & GetWorkerJsonCfg();
    T_DpdkJsonCfg       & GetDpdkJsonCfg();
    T_RootJsonCfg       & GetRootJsonCfg();

protected :
    T_RootJsonCfg    m_tRootConfig;
};


#endif


