

#ifndef _BASE_CONFIG_FILE_H_
#define _BASE_CONFIG_FILE_H_


#include <fstream>

#include "pub_global_def.h"

#include "base_string.h"
#include "base_util.h"
#include "base_singleton_tpl.h"


#define LOG_JSON_BITMAP_LEN          ((WORD32)(BITMAP_BYTE_NUM_02 * BIT_NUM_PER_BYTE))

#define DEFAULT_LOG_FILE_SIZE        ((WORD64)(100 * BYTE_NUM_PER_MB))

#define MAX_HUGE_DIR_LEN             ((WORD32)(56))
#define MAX_BLOCK_NUM                ((WORD32)(32))
#define MAX_POOL_NUM                 ((WORD32)(8))
#define MAX_WORKER_NUM               ((WORD32)(16))
#define MAX_APP_NUM                  ((WORD32)(64))
#define WORKER_NAME_LEN              ((WORD32)(32))
#define APP_NAME_LEN                 ((WORD32)(32))
#define MAX_APP_NUM_PER_THREAD       ((WORD32)(12))
#define MAX_ASSOCIATE_NUM_PER_APP    ((WORD32)(16))


#define EAL_ARG_NUM                  ((WORD32)(16))
#define EAL_ARG_LEN                  ((WORD32)(64))
#define DEV_NAME_LEN                 ((WORD32)(32))
#define MAX_DEV_PORT_NUM             ((WORD32)(8))
#define IPV4_STRING_LEN              ((WORD32)(16))
#define IPV6_STRING_LEN              ((WORD32)(40))
#define MAX_DEV_IP_NUM               ((WORD32)(2))
#define MAX_DEV_VLAN_NUM             ((WORD32)(16))
#define BB_TRAFFIC_NAME_LEN          ((WORD32)(32))
#define MAX_BB_TRAFFIC_NUM           ((WORD32)(8))
#define BB_TRAFFIC_CELL_INVALID      ((WORD32)(255))


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
    CHAR              aucHugeDir[MAX_HUGE_DIR_LEN];
    T_MemJsonPool     atPool[MAX_POOL_NUM];
}T_MemJsonCfg;


typedef struct tagT_LogJsonModule
{
    CHAR    aucName[LOG_MODULE_LEN];
    WORD32  dwModuleID;    /* 根据名称查询所得, 不需要在json文件中定义 */
    BOOL    bSwitch;
    BYTE    ucCellNum;     /* 取值为json中定义的小区列表数量 */
    BYTE    ucLevelNum;    /* 取值为json中定义的级别列表数量 */
    WORD32  adwCellID[LOG_JSON_BITMAP_LEN];
    WORD32  adwLevelID[LOG_JSON_BITMAP_LEN];
}T_LogJsonModule;


typedef struct tagT_AppJsonCfg
{
    CHAR       aucName[APP_NAME_LEN];
    WORD32     dwAppID;
    WORD32     dwEventBegin;
    BOOL       bAssocFlag;
    WORD32     dwAssocNum;
    WORD32     adwAssocID[MAX_ASSOCIATE_NUM_PER_APP];
}T_AppJsonCfg;


typedef struct tagT_LogJsonCfg
{
    WORD32           dwCoreID;
    WORD32           dwPolicy;
    WORD32           dwPriority;
    WORD32           dwStackSize;
    WORD32           dwCBNum;
    WORD32           dwPacketCBNum;
    WORD32           dwMultiCBNum;
    WORD32           dwTimerThresh;
    CHAR             aucPath[LOG_FILE_NAME_LEN];
    BOOL             bMeasSwitch;
    BOOL             bGlobalSwitch;
    WORD32           dwSyncFlag;         /* 周期性同步系统时钟启动标志 */
    WORD32           dwLogMeasure;       /* 日志维测输出时长(单位 : 分钟) */
    WORD32           dwWriteFilePeriod;  /* 取值(0/1/2/3/4/5/6), 对应(5/10/15/30/60/120/240分钟) */
    WORD16           wThresholdWait;     /* 日志策略 */
    WORD16           wThresholdLock;     /* 日志策略 */    
    WORD16           wThresholdLoop;     /* 日志策略 */
    WORD64           lwMaxFileSize;      /* 单个文件最大长度 */
    WORD32           dwModuleNum;        /* 取值为json中定义的模块列表数量 */
    WORD32           dwAppNum;           /* 日志线程绑定的App数量 */
    T_AppJsonCfg     atApp[MAX_APP_NUM_PER_THREAD];
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
    WORD32        dwMultiCBNum;
    WORD32        dwTimerThresh;
    BOOL          bAloneLog;
    WORD32        dwAppNum;
    T_AppJsonCfg  atApp[MAX_APP_NUM_PER_THREAD];
}T_ThreadJsonCfg;


typedef struct tagT_ThreadPoolJsonCfg
{
    WORD32           dwWorkerNum;
    T_ThreadJsonCfg  atWorker[MAX_WORKER_NUM];
}T_ThreadPoolJsonCfg;


typedef struct tagT_DpdkEthIPJsonCfg
{
    WORD32               dwIPType;
    CHAR                 aucIpv4Addr[IPV4_STRING_LEN];
    CHAR                 aucIpv4GW[IPV4_STRING_LEN];
    CHAR                 aucIpv6Addr[IPV6_STRING_LEN];
    CHAR                 aucIpv6GW[IPV6_STRING_LEN];
}T_DpdkEthIPJsonCfg;


typedef struct tagT_DpdkEthVlanJsonCfg
{
    WORD32               dwVlanID;
    WORD32               dwPriority;
    T_DpdkEthIPJsonCfg   tIP;
}T_DpdkEthVlanJsonCfg;


typedef struct tagT_DpdkEthDevJsonCfg
{
    WORD32               dwDeviceID;
    WORD32               dwLinkType;    /* 上联网络类型(0:Access, 1:Trunk, 2:Hybrid) */
    WORD32               dwIpNum;
    WORD32               dwVlanNum;
    T_DpdkEthIPJsonCfg   atIP[MAX_DEV_IP_NUM];
    T_DpdkEthVlanJsonCfg atVlan[MAX_DEV_VLAN_NUM];
}T_DpdkEthDevJsonCfg;


typedef struct tagT_DpdkBBFapiTrafficJsonCfg
{
    CHAR    aucType[BB_TRAFFIC_NAME_LEN];
    WORD32  dwTrafficID;
    WORD32  dwFAPICell;
    WORD32  dwBindCell;
}T_DpdkBBFapiTrafficJsonCfg;


typedef struct tagT_DpdkBBDevJsonCfg
{
    WORD32                      dwDeviceID;
    WORD32                      dwQueueID;
    WORD32                      dwFapiTrafficNum;
    T_DpdkBBFapiTrafficJsonCfg  atFAPITraffic[MAX_BB_TRAFFIC_NUM];
}T_DpdkBBDevJsonCfg;


typedef struct tagT_DpdkDevJsonCfg
{
    CHAR          aucType[DEV_NAME_LEN];
    CHAR          aucAddr[DEV_NAME_LEN];
    WORD32        dwDeviceID;
    WORD32        dwPortID;
    WORD32        dwQueueNum;
    WORD32        dwMBufNum;
    WORD32        dwMBufCacheSize;
    WORD32        dwMBufPrivSize;
    WORD32        dwMBufRoomSize;
    WORD32        dwRxDescNum;
    WORD32        dwTxDescNum;
}T_DpdkDevJsonCfg;


typedef struct tagT_DpdkJsonCfg
{
    BOOL                 bInitFlag;
    WORD32               dwArgNum;
    WORD32               dwDevNum;
    WORD32               dwBBNum;
    WORD32               dwEthNum;
    CHAR                 aucArgs[EAL_ARG_NUM][EAL_ARG_LEN];
    T_DpdkDevJsonCfg     atDevice[MAX_DEV_PORT_NUM];
    T_DpdkBBDevJsonCfg   atBBDev[MAX_DEV_PORT_NUM];
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

    T_DpdkBBDevJsonCfg  * GetBBDevJsonCfg(WORD32 dwDeviceID);
    T_DpdkEthDevJsonCfg * GetEthDevJsonCfg(WORD32 dwDeviceID);

protected :
    T_RootJsonCfg    m_tRootConfig;
};


#endif


