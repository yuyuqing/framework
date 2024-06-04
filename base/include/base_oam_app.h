

#ifndef _BASE_OAM_APP_H_
#define _BASE_OAM_APP_H_


#include "base_tree_array.h"
#include "base_app_interface.h"


typedef enum tagE_OamTaskType
{
    E_OAM_TASK_SYNC_ID = 0,
    E_OAM_MEASURE_ID,
    E_DPDK_MEASURE_ID,
}E_OamTaskType;


typedef struct tagT_OamRegistCallBack
{
    WORD32     dwTaskID;    /* 注册任务ID(E_LogTaskType枚举, 取值唯一) */
    WORD32     dwPeriod;    /* 任务超时周期(单位:ms) */
    CCBObject *pObjAddr;    /* CObject对象地址 */
    PCBFUNC    pFuncAddr;   /* PCBFUNC函数地址 */
    WORD64     lwUsrData;   /* 回调函数入参 */
}T_OamRegistCallBack;


typedef struct tagT_OamRemoveCallBack
{
    WORD32     dwTaskID;    /* 注册任务ID(E_LogTaskType枚举, 取值唯一) */
}T_OamRemoveCallBack;


#define OAM_CB_TABLE_NODE_POWER_NUM        ((WORD32)(10))


class COamCBNode
{
public :
    COamCBNode();
    virtual ~COamCBNode();

    WORD32 Initialize(WORD32     dwInstID,
                      WORD32     dwTaskID,
                      CCBObject *pObj,
                      PCBFUNC    pFunc,
                      WORD64     lwUserData);

public :
    WORD32      m_dwInstID;    /* 在CB表中的索引 */
    WORD32      m_dwTaskID;    /* 任务ID, CB表的主键 */
    CCBObject  *m_pObj;        /* 任务周期性超时回调对象地址 */
    PCBFUNC     m_pFunc;       /* 任务周期性超时回调函数地址 */
    WORD64      m_lwUserData;  /* 任务周期性超时回调出参 */
    WORD32      m_dwTimerID;   /* 保存定时器ID */
};


typedef CBTreeArray<COamCBNode, WORD32, OAM_CB_TABLE_NODE_POWER_NUM>  COamCBTable;


#define DISABLE_SYNC_FLAG       ((BYTE)(0))
#define ENABLE_SYNC_FLAG        ((BYTE)(1))


class COamApp : public CAppInterface
{
public :
    enum { SYNC_PERIOD            = 30000 };  /* 30秒 = 30000ms */
    enum { TIMER_SYNC_INTERVAL    =  5000 };
    enum { TIMER_TIMEOUT_INTERVAL =    20 };
    enum { DELAY_INIT_APPS_TICK   =     5 };

public :
    COamApp ();
    virtual ~COamApp();

    WORD32 InitApp();

    WORD32 Start();
    WORD32 Init();
    WORD32 DeInit();
    WORD32 Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen);

    /* 通知COamApp上电, 启动5ms定时器(超时后通知其它APP上电), 在main进程中调用 */
    WORD32 NotifyOamStartUP();

    /* 延迟通知其它APP上电(在COamApp上电后启动5ms定时器) */
    VOID TimeOutStartUpAllApps(const VOID *pIn, WORD32 dwLen);

    VOID ProcGlobalSwitchMsg(const VOID *pIn, WORD32 dwLen);

    VOID ProcWritePeriodMsg(const VOID *pIn, WORD32 dwLen);

    VOID ProcModuleSwitchMsg(const VOID *pIn, WORD32 dwLen);

    /* 处理注册定时回调函数的消息 */
    VOID ProcCBRegistMsg(const VOID *pIn, WORD32 dwLen);

    /* 处理去注册定时回调函数的消息 */
    VOID ProcCBRemoveMsg(const VOID *pIn, WORD32 dwLen);

    /* 针对注册定时回调函数, 周期性执行回调 */
    VOID CallBack(const VOID *pIn, WORD32 dwLen);

    /* 向其它应用或线程提供消息接口, 用以支持注册定时回调 */
    WORD32 SendRegistCBMsg(WORD32     dwTaskID,
                           WORD32     dwPeriod,
                           CCBObject *pObj,
                           PCBFUNC    pFunc,
                           VOID      *pUsrData);

    /* 向其它应用或线程提供消息接口, 用以支持去注册定时回调 */
    WORD32 SendRemoveCBMsg(WORD32 dwTaskID);

    /* 发送周期性执行是时钟同步任务的SYNC消息 */
    VOID SyncClock(const VOID *pIn, WORD32 dwLen);

    /* 发送周期性输出系统维测任务的MEAS消息 */
    VOID DumpMeasure(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpThreadMeas(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpAppMeas(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpMsgQueueMeas(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpMsgMemPoolMeas(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpMemPoolsMeas(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutMemMgrDump(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutLogMeas(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutLogFlush(const VOID *pIn, WORD32 dwLen);

    VOID TimeOutSwitch(const VOID *pIn, WORD32 dwLen);

protected :
    WORD32 CalcTick(WORD64 lwSeconds, WORD64 lwMicroSec, WORD32 dwPeriod);

    /* 计算切换日志文件的时间点, 并启动绝对定时器 */
    WORD32 DoLogTask(WORD64 lwSeconds,
                     WORD64 lwMicroSec,
                     WORD64 lwCycle,
                     WORD32 dwSwitchPrd);

    /* 切换日志文件 */
    WORD32 SwitchLogFile(BYTE ucPos);

protected :
    COamCBTable           m_cCBTable;      /* OAM周期性任务表 */

    BYTE                  m_ucPos;         /* 保持和g_pLogger的主备文件Pos一致 */
    BYTE                  m_ucMeasMinute;  /* 维测定时器时长(单位:分钟) */
    WORD16                m_wSwitchPrd;    /* 日志文件切换周期(单位:分钟) */

    BYTE                  m_ucSyncFlag;    /* 是否启动系统时钟标志 */
    BYTE                  m_ucThrdNum;
    BYTE                  m_ucAppNum;

    /* 线程级别维测信息 */    
    CBaseThread          *m_apThread[MAX_WORKER_NUM];
    T_ThreadMeasure       m_atThrdMeasure[MAX_WORKER_NUM];
    T_ThreadRingMeasure   m_atThreadRingMeasure[MAX_WORKER_NUM];

    /* APP级别维测信息 */    
    CAppInterface        *m_apApp[MAX_APP_NUM];
    T_AppMeasure          m_atAppMeasure[MAX_APP_NUM];

    /* Block/Trun MemPools内存池维测信息 */    
    T_MemMeasure          m_tMemPoolMeasure;
};


#endif


