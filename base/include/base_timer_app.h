

#ifndef _BASE_TIMER_APP_H_
#define _BASE_TIMER_APP_H_


#include "base_tree.h"
#include "base_app_interface.h"
#include "base_timer_wrapper.h"


typedef struct tagT_TimerMeasure
{
    std::atomic<WORD64>    lwStartCount;     /* 业务进程调用启动定时器接口计数 */
    std::atomic<WORD64>    lwStartMFail;     /* 业务进程调用启动定时器接口内存分配失败计数 */
    std::atomic<WORD64>    lwStartQFail;     /* 业务进程调用启动定时器接口入队列失败计数 */
    std::atomic<WORD64>    lwStopCount;      /* 业务进程调用停止定时器接口计数 */
    std::atomic<WORD64>    lwStopMFail;      /* 业务进程调用停止定时器接口内存分配失败计数 */
    std::atomic<WORD64>    lwStopQFail;      /* 业务进程调用停止定时器接口入队列失败计数 */
    std::atomic<WORD64>    lwResetCount;     /* 业务进程调用重置定时器接口计数 */
    std::atomic<WORD64>    lwResetMFail;     /* 业务进程调用重置定时器接口内存分配失败计数 */
    std::atomic<WORD64>    lwResetQFail;     /* 业务进程调用重置定时器接口入队列失败计数 */
    std::atomic<WORD64>    lwSlotCount;      /* 业务调用发送SlotInd消息接口计数 */
    std::atomic<WORD64>    lwSlotMFail;      /* 业务调用发送SlotInd消息接口分配内存失败计数 */
    std::atomic<WORD64>    lwSlotQFail;      /* 业务调用发送SlotInd消息接口入队列失败计数 */

    WORD64  lwStartMsgCount;                 /* 定时器线程接收到启动定时器消息计数 */
    WORD64  lwCreateFailCount;               /* 定时器线程接收到启动定时器消息注册定时器失败计数 */
    WORD64  lwStopMsgCount;                  /* 定时器线程接收到停止定时器消息计数 */
    WORD64  lwNotFindCount;                  /* 定时器线程接收到停止定时器消息查找定时器失败计数 */
    WORD64  lwResetMsgCount;                 /* 定时器线程接收到重置定时器消息计数 */
    WORD64  lwResetNotFindCount;             /* 定时器线程接收到重置定时器消息查找定时器失败计数 */
    WORD64  lwSlotMsgCount;                  /* 收到SlotTTI消息计数 */
    WORD64  lwSlotMsgMissCount;              /* 收到SlotTTI消息不连续计数 */
    WORD64  lwTimeOutCount;                  /* 定时器超时次数统计 */
}T_TimerMeasure;


class CTimerTreeNode : public CCBObject
{
public :
    CTimerTreeNode ();
    virtual ~CTimerTreeNode();

    WORD32 Initialize(WORD32          dwTimerID,
                      WORD32          dwKey,
                      CTimerNode     *pTimer,
                      PTimerCallBack  pFunc);

    VOID TimeOut(const VOID *pIn, WORD32 dwLen);

    WORD32 GetTimerID();

    CTimerNode * GetTimer();

protected :
    WORD32          m_dwTimerID;
    WORD32          m_dwKey;
    CTimerNode     *m_pTimer;
    PTimerCallBack  m_pCBFunc;
};


inline WORD32 CTimerTreeNode::GetTimerID()
{
    return m_dwTimerID;
}


inline CTimerNode * CTimerTreeNode::GetTimer()
{
    return m_pTimer;
}


#define TIMER_TREE_POWER_NUM        ((WORD32)(12))

typedef CBTreeTpl<CTimerTreeNode, WORD32, TIMER_TREE_POWER_NUM>  CTimerTree;


class CTimerApp : public CAppInterface
{
public :
    friend class CTimerTreeNode;

public :
    CTimerApp ();
    virtual ~CTimerApp();

    WORD32 InitApp();

    WORD32 Start();
    WORD32 Init();
    WORD32 DeInit();
    WORD32 Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen);

    /* 处理启动定时器消息 */
    VOID ProcStartTimer(const VOID *pIn, WORD32 dwLen);

    /* 处理停止定时器消息 */
    VOID ProcStopTimer(const VOID *pIn, WORD32 dwLen);

    /* 处理重置定时器消息 */
    VOID ProcResetTimer(const VOID *pIn, WORD32 dwLen);

    /* 处理SlotInd消息 */
    VOID ProcSlotInd(const VOID *pIn, WORD32 dwLen);

    /* 向CTimerApp发送启动定时器消息 */
    WORD32 CreateTimer(WORD32          dwTick,
                       PTimerCallBack  pFunc,
                       WORD32          dwID,
                       WORD32          dwExtendID,
                       WORD32          dwTransID,
                       WORD32          dwResvID,
                       VOID           *pContext,
                       VOID           *pUserData);

    /* 向CTimerApp发送停止定时器消息 */
    WORD32 RemoveTimer(WORD32 dwTimerID);

    /* 向CTimerApp发送重置定时器消息 */
    WORD32 ResetTimer(WORD32 dwTimerID, WORD32 dwTick);

    /* 向CTimerApp发送SlotTti消息 */
    WORD32 NotifySlotInd(WORD16 wSFN, BYTE ucSlot);

    /* 周期性输出系统维测 */
    VOID DumpMeasure(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpTimerMeas(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpPhyRecv(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpClApp(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpUlRecv(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpSchApp(const VOID *pIn, WORD32 dwLen);

    WORD32 InnerDelete(WORD32 dwKey);

    T_PhyRecvCellAtomicMeasure      & GetPhyRecvMeasure(WORD32 dwCellIdx);
    T_ClAppDlFapiCellAtomicMeasure  & GetClAppMeasure(WORD32 dwCellIdx);
    T_SchAppAtomicMeasure           & GetSchAppMeasure();
    T_UlRecvAtomicMeasure           & GetUlRecvMeasure();

protected :
    WORD32 InnerCreate(WORD32          dwKey,
                       WORD64          lwMicroSec,
                       WORD32          dwTick,
                       PTimerCallBack  pFunc,
                       WORD32          dwID,
                       WORD32          dwExtendID,
                       WORD32          dwTransID,
                       WORD32          dwResvID,
                       VOID           *pContext,
                       VOID           *pUserData);

protected :
    CTimerTree                  m_cTree;
    CB_RegistMemPool            m_pRegistMemPoolFunc;  /* 向NGP内存池注册线程信息 */

    WORD64                      m_lwSlotCount;
    WORD16                      m_wSFN;
    BYTE                        m_ucSlot;

    BYTE                        m_ucMeasMinute;  /* 维测定时器时长(单位:分钟) */

    T_TimerMeasure              m_tMeas;

    T_PhyRecvAtomicMeasure      m_tPhyRecvMeasure;
    T_ClAppDlFapiAtomicMeasure  m_tClAppMeasure;
    T_SchAppAtomicMeasure       m_tSchAppMeasure;
    T_UlRecvAtomicMeasure       m_tUlRecvMeasure;
};


inline T_PhyRecvCellAtomicMeasure & CTimerApp::GetPhyRecvMeasure(WORD32 dwCellIdx)
{
    return m_tPhyRecvMeasure.atCell[dwCellIdx];
}


inline T_ClAppDlFapiCellAtomicMeasure & CTimerApp::GetClAppMeasure(WORD32 dwCellIdx)
{
    return m_tClAppMeasure.atCell[dwCellIdx];
}


inline T_SchAppAtomicMeasure & CTimerApp::GetSchAppMeasure()
{
    return m_tSchAppMeasure;
}


inline T_UlRecvAtomicMeasure & CTimerApp::GetUlRecvMeasure()
{
    return m_tUlRecvMeasure;
}


#endif


