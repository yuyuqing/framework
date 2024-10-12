

#ifndef _BASE_TIMER_WRAPPER_H_
#define _BASE_TIMER_WRAPPER_H_


/* 与C兼容 */
#ifdef __cplusplus
extern "C"{
#endif


#include "pub_typedef.h"
#include "pub_global_def.h"


typedef struct tagT_TimerParam
{
    WORD32   dwID;
    WORD32   dwExtendID;
    WORD32   dwTransID;
    WORD32   dwResvID;
    VOID    *pContext;
    VOID    *pUserData;
}T_TimerParam;


/* 定时器超时回调处理 
 * dwID       : 起定时器时的入参
 * dwExtendID : 起定时器时的入参
 * pContext   : 起定时器时的入参
 */
using PTimerCallBack = WORD32 (*)(WORD32 dwKey, T_TimerParam *ptParam);


/* 启动定时器消息 */
typedef struct tagT_StartTimerMessage
{
    WORD64          lwMicroSec;  /* 调用StartTimer接口时的绝对时间(单位微秒) */
    WORD32          dwTick;      /* 等待超时间隔(单位毫秒) */
    WORD32          dwTimerID;   /* 定时器ID, 由业务线程分配, 全局唯一 */

    PTimerCallBack  pFunc;       /* 回调函数地址 */

    /* 出参内容 */
    WORD32          dwID;
    WORD32          dwExtendID;
    WORD32          dwTransID;
    WORD32          dwResvID;
    VOID           *pContext;
    VOID           *pUserData;
}T_StartTimerMessage;


/* 停止定时器消息 */
typedef struct tagT_StopTimerMessage
{
    WORD32          dwTimerID;     /* 定时器ID, 由业务线程分配, 全局唯一 */
}T_StopTimerMessage;


/* 重置定时器消息 */
typedef struct tagT_ResetTimerMessage
{
    WORD64          lwMicroSec;  /* 调用StartTimer接口时的绝对时间(单位微秒) */
    WORD32          dwTick;      /* 等待超时间隔(单位毫秒) */
    WORD32          dwTimerID;   /* 定时器ID, 由业务线程分配, 全局唯一 */
}T_ResetTimerMessage;


typedef struct tagT_TimerSlotIndMessage
{
    WORD16          wSFN;
    BYTE            ucSlot;
    BYTE            ucResved;
    WORD32          dwResved;
}T_TimerSlotIndMessage;


/* 定时器超时消息 */
typedef struct tagT_TimeOutMessage
{
    WORD32          dwTimerID;
    WORD32          dwID;
    WORD32          dwExtendID;
    WORD32          dwTransID;
    WORD32          dwResvID;
    VOID           *pContext;
    VOID           *pUserData;
}T_TimeOutMessage;


/* 起定时器接口 : 超时后该定时器自动删除, 无需再调用StopTimer删除
   dwTick     : 超时时长, 单位ms
   pFunc      : 回调函数
   dwID       : 回调出参
   dwExtendID : 回调出参
   dwTransID  : 回调出参
   dwResvID   : 回调出参
   pContext   : 回调出参
   pUserData  : 回调出参
   返回值     : dwTimerID, 定时器ID(KillTimer时需要用到)
 */
WORD32 StartTimer(WORD32          dwTick,
                  PTimerCallBack  pFunc,
                  WORD32          dwID,
                  WORD32          dwExtendID,
                  WORD32          dwTransID,
                  WORD32          dwResvID,
                  VOID           *pContext,
                  VOID           *pUserData);


/* 停定时器接口 */
WORD32 StopTimer(WORD32 dwTimerID);


/* 重启定时器接口, 在当前系统时间基础上, 将定时器时长后延dwTick(单位ms) */
WORD32 ResetTimer(WORD32 dwTimerID, WORD32 dwTick);


/* 向定时器线程发送SlotInd消息 */
WORD32 SendSlotIndToTimer(WORD16 wSFN, BYTE ucSlot);


/* 超时后向NGP内存池注册(原因 : 定时器回调业务函数时, 需要从NGP的内存池中申请内存) */
WORD32 StartOam();

/* 通知OAM延迟退出(延迟dwTick时长, 单位:ms) */
WORD32 StopOam(WORD32 dwTick);


VOID SetGlobalSFN(WORD16 wSFN, BYTE ucSlot);
VOID GetGlobalSFN(WORD16 &rwSFN, BYTE &rucSlot);


WORD32 GetCpuFreq();


#ifdef __cplusplus
    }
#endif


#endif


