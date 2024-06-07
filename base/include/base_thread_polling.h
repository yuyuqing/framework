

#ifndef _BASE_THREAD_POLLING_H_
#define _BASE_THREAD_POLLING_H_


#include "base_app_interface.h"


typedef struct tagT_AppContext
{
    T_AppInfo   *pAppInfo;
    BOOL         bTimerFlag;  /* 1 : 启动定时器 */
}T_AppContext;


class CPollingThread : public CBaseThread
{
public :
    friend class CThreadCntrl;

    enum { MAX_EVENT_NUM  = 64 };
    enum { MAX_LOOP_COUNT = 10000 };
    enum { PACKET_RING_BURST_NUM = 16 };

    /* 高优先级队列消息回调 */
    static WORD32 ProcMessageHP(VOID *pObj, VOID *pMsg);

    /* 低优先级队列消息回调 */
    static WORD32 ProcMessageLP(VOID *pObj, VOID *pMsg);

public :
    CPollingThread (const T_ThreadParam &rtParam);
    virtual ~CPollingThread();

    /* 创建线程实例后执行初始化(在主线程栈空间执行) */
    virtual WORD32 Initialize();

    /* 在创建线程实例并执行初始化后加载App(在主线程栈空间执行) */
    WORD32 LoadApp(T_AppInfo *ptAppInfo);

    virtual VOID Run(CBaseThread *pArg);

    /* 注册消息回调函数 */
    WORD32 Register(WORD32     dwMsgID,
                    CCBObject *pObj,
                    PCBFUNC    pFunc);

    VOID ProcCancelMsg(const VOID *pIn, WORD32 dwLen);

    VOID ProcResetAppMsg(const VOID *pIn, WORD32 dwLen);

    virtual CMessageRing * GetMsgRingH();

    virtual CMessageRing * GetMsgRingL();

    virtual CDataPlaneRing * GetMsgRingD();

    CAppInterface * GetAppInst(E_AppClass eClass, WORD32 dwAssocID = INVALID_DWORD);

    VOID Dump();

protected :
    /* 线程启动后启动App(在线程栈空间执行, 在进入线程主循环之前调用) */
    WORD32 StartApp();

    /* 线程启动后进入主循环 */
    virtual VOID DoRun();

    /* 向App分发消息 */
    WORD32 Dispatch(WORD32 dwMsgID, T_BaseMessage *pMsg, WORD16 wMsgLen);

    WORD32 Polling();

    T_AppContext * FindAppCtx(WORD32 dwAppID);

    /* 统计线程消息队列的测量指标 */
    VOID StatisticMsgQ(T_AtomicMsgMeasure &rtMeasure,
                       WORD32              dwMsgID, 
                       WORD64              lwStartCycle, 
                       WORD64              lwEndCycle);

    /* 统计线程执行的测量指标 */
    VOID Statistic(WORD32 dwProcHNum,
                   WORD32 dwProcLNum,
                   WORD32 dwProcDNum,
                   WORD32 dwRemainLNum,
                   WORD32 dwRemainDNum,
                   WORD64 lwTimeStampStart,
                   WORD64 lwTimeStampProcH,
                   WORD64 lwTimeStampProcL,
                   WORD64 lwTimeStampProcD,
                   WORD64 lwTimeStampEnd);

protected :
    CMessageRing                 m_MsgRingH;       /* 高优先级队列 */
    CMessageRing                 m_MsgRingL;       /* 低优先级队列 */
    CDataPlaneRing               m_MsgRingD;       /* 报文队列 */
    CEventMap<MAX_EVENT_NUM>     m_EventMap;

    WORD32                       m_dwCBNum;        /* 低优先级队列每次循环最大回调次数 */
    WORD32                       m_dwPacketCBNum;  /* 报文队列每次循环最大回调次数 */
    WORD32                       m_dwMultiCBNum;   /* 多队列每次循环最大回调次数(针对每个STRing) */
    WORD32                       m_dwTimerThresh;  /* 定时器队列一次最大处理的超时定时器数量门限 */
    WORD32                       m_dwCyclePer100NS;

    WORD32                       m_dwAppNum;
    T_AppContext                 m_atAppCtx[MAX_APP_NUM_PER_THREAD];
};


/* 高优先级消息队列回调 */
inline WORD32 CPollingThread::ProcMessageHP(VOID *pObj, VOID *pMsg)
{
    CPollingThread *pThread = (CPollingThread *)pObj;
    T_BaseMessage  *ptMsg   = (T_BaseMessage *)pMsg;
    WORD32          dwMsgID = ptMsg->dwMsgID;

    ptMsg->lwEndCycle = GetCycle();

    pThread->StatisticMsgQ(pThread->m_tMeasure.tHPMsgQStat, 
                           dwMsgID, 
                           ptMsg->lwStartCycle, 
                           ptMsg->lwEndCycle);

    if (dwMsgID < EV_BASE_THREAD_END)
    {
        pThread->m_EventMap.ProcessEvent(dwMsgID, 
                                         pMsg, 
                                         sizeof(T_BaseMessage) + ptMsg->wMsgLen);
    }
    else
    {
        pThread->Dispatch(dwMsgID, 
                          ptMsg, 
                          sizeof(T_BaseMessage) + ptMsg->wMsgLen);
    }

    pThread->m_pMsgMemPool->Free((BYTE *)pMsg);

    return SUCCESS;
}


/* 低优先级消息队列回调 */
inline WORD32 CPollingThread::ProcMessageLP(VOID *pObj, VOID *pMsg)
{
    CPollingThread *pThread = (CPollingThread *)pObj;
    T_BaseMessage  *ptMsg   = (T_BaseMessage *)pMsg;
    WORD32          dwMsgID = ptMsg->dwMsgID;

    ptMsg->lwEndCycle = GetCycle();

    pThread->StatisticMsgQ(pThread->m_tMeasure.tLPMsgQStat, 
                           dwMsgID, 
                           ptMsg->lwStartCycle, 
                           ptMsg->lwEndCycle);

    if (dwMsgID < EV_BASE_THREAD_END)
    {
        pThread->m_EventMap.ProcessEvent(dwMsgID, 
                                         pMsg, 
                                         sizeof(T_BaseMessage) + ptMsg->wMsgLen);
    }
    else
    {
        pThread->Dispatch(dwMsgID, 
                          ptMsg, 
                          sizeof(T_BaseMessage) + ptMsg->wMsgLen);
    }

    pThread->m_pMsgMemPool->Free((BYTE *)pMsg);

    return SUCCESS;
}


inline CMessageRing * CPollingThread::GetMsgRingH()
{
    return &m_MsgRingH;
}


inline CMessageRing * CPollingThread::GetMsgRingL()
{
    return &m_MsgRingL;
}


inline CDataPlaneRing * CPollingThread::GetMsgRingD()
{
    return &m_MsgRingD;
}


inline WORD32 CPollingThread::Dispatch(WORD32         dwMsgID,
                                       T_BaseMessage *pMsg, 
                                       WORD16         wMsgLen)
{
    T_AppInfo     *ptAppInfo = NULL;
    CAppState     *pState    = NULL;
    E_AppState     eState    = APP_STATE_NULL;
    CAppInterface *pAppInst  = NULL;

    for (WORD32 dwIndex = 0; dwIndex < m_dwAppNum; dwIndex++)
    {
        ptAppInfo = m_atAppCtx[dwIndex].pAppInfo;

        if (pMsg->dwDstAppID == ptAppInfo->dwAppID)
        {
            pState   = ptAppInfo->pAppState;
            eState   = pState->GetState();
            pAppInst = pState->GetAppInst();

            pAppInst->StatisticMsgQ(dwMsgID, 
                                    pMsg->lwStartCycle, 
                                    pMsg->lwEndCycle);

            pState->Enter(eState, 
                          dwMsgID, 
                          pMsg->aValue, 
                          pMsg->wMsgLen,
                          &(ptAppInfo->pAppState),
                          ptAppInfo->aucAppState);
            break ;
        }
    }

    return SUCCESS;
}


inline WORD32 CPollingThread::Polling()
{
    T_AppInfo *ptAppInfo = NULL;

    for (WORD32 dwIndex = 0; dwIndex < m_dwAppNum; dwIndex++)
    {
        ptAppInfo = m_atAppCtx[dwIndex].pAppInfo;
        ptAppInfo->pAppState->Polling();
    }

    return SUCCESS;
}


inline T_AppContext * CPollingThread::FindAppCtx(WORD32 dwAppID)
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwAppNum; dwIndex++)
    {
        if (dwAppID == m_atAppCtx[dwIndex].pAppInfo->dwAppID)
        {
            return &(m_atAppCtx[dwIndex]);
        }
    }

    return NULL;
}


inline VOID CPollingThread::StatisticMsgQ(T_AtomicMsgMeasure &rtMeasure,
                                          WORD32              dwMsgID, 
                                          WORD64              lwStartCycle, 
                                          WORD64              lwEndCycle)
{   
    rtMeasure.lwMsgCount++;

    WORD64 lwTimeStat = lwEndCycle - lwStartCycle;
    WORD32 dwTimeUsed = TRANSFER_CYCLE_TO_100NS2(lwTimeStat, m_dwCyclePer100NS);
    WORD32 dwMaxUsed  = rtMeasure.dwMsgQMaxUsed.load(std::memory_order_relaxed);

    if (dwTimeUsed > dwMaxUsed)
    {
        rtMeasure.dwQMaxMsgID   = dwMsgID;
        rtMeasure.dwMsgQMaxUsed = dwTimeUsed;
    }

    rtMeasure.lwMsgQTotalTime += dwTimeUsed;
    rtMeasure.adwStat[base_bsr_uint32(dwTimeUsed)]++;
}


inline VOID CPollingThread::Statistic(WORD32 dwProcHNum,
                                      WORD32 dwProcLNum,
                                      WORD32 dwProcDNum,
                                      WORD32 dwRemainLNum,
                                      WORD32 dwRemainDNum,
                                      WORD64 lwTimeStampStart,
                                      WORD64 lwTimeStampProcH,
                                      WORD64 lwTimeStampProcL,
                                      WORD64 lwTimeStampProcD,
                                      WORD64 lwTimeStampEnd)
{
#ifdef THREAD_MEASURE
    WORD64 lwTimeStat1     = lwTimeStampProcH - lwTimeStampStart;
    WORD64 lwTimeStat2     = lwTimeStampProcL - lwTimeStampProcH;
    WORD64 lwTimeStat3     = lwTimeStampProcD - lwTimeStampProcL;
    WORD64 lwTimeStat4     = lwTimeStampEnd   - lwTimeStampProcD;
    WORD64 lwTimeStat5     = lwTimeStampEnd   - lwTimeStampStart;
    WORD32 dwTimeUsedH     = TRANSFER_CYCLE_TO_100NS2(lwTimeStat1, m_dwCyclePer100NS);
    WORD32 dwTimeUsedL     = TRANSFER_CYCLE_TO_100NS2(lwTimeStat2, m_dwCyclePer100NS);
    WORD32 dwTimeUsedD     = TRANSFER_CYCLE_TO_100NS2(lwTimeStat3, m_dwCyclePer100NS);
    WORD32 dwTimeUsedT     = TRANSFER_CYCLE_TO_100NS2(lwTimeStat4, m_dwCyclePer100NS);
    WORD32 dwTimeUsedA     = TRANSFER_CYCLE_TO_100NS2(lwTimeStat5, m_dwCyclePer100NS);
    WORD32 dwMaxRemainLNum = m_tMeasure.dwMaxRemainLNum.load(std::memory_order_relaxed);
    WORD32 dwMaxRemainDNum = m_tMeasure.dwMaxRemainDNum.load(std::memory_order_relaxed);

    m_tMeasure.lwLoop++;
    m_tMeasure.lwProcHNum += dwProcHNum;
    m_tMeasure.lwProcLNum += dwProcLNum;
    m_tMeasure.lwProcDNum += dwProcDNum;

    if (dwRemainLNum > dwMaxRemainLNum)
    {
        m_tMeasure.dwMaxRemainLNum = dwRemainLNum;
    }

    if (dwRemainDNum > dwMaxRemainDNum)
    {
        m_tMeasure.dwMaxRemainDNum = dwRemainDNum;
    }

    m_tMeasure.lwTimeUsedTotalH += dwTimeUsedH;
    m_tMeasure.lwTimeUsedTotalL += dwTimeUsedL;
    m_tMeasure.lwTimeUsedTotalD += dwTimeUsedD;
    m_tMeasure.lwTimeUsedTotalT += dwTimeUsedT;
    m_tMeasure.lwTimeUsedTotalA += dwTimeUsedA;

    m_tMeasure.alwStatH[base_bsr_uint32(dwTimeUsedH)]++;
    m_tMeasure.alwStatL[base_bsr_uint32(dwTimeUsedL)]++;
    m_tMeasure.alwStatD[base_bsr_uint32(dwTimeUsedD)]++;
    m_tMeasure.alwStatT[base_bsr_uint32(dwTimeUsedT)]++;
    m_tMeasure.alwStatA[base_bsr_uint32(dwTimeUsedA)]++;
#endif
}


#endif


