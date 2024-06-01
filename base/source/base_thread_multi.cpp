

#include "base_thread_multi.h"


WORD32 CMultiThread::ProcMultiMessage(VOID *pObj, VOID *pMsg)
{
    CMultiThread   *pThread = (CMultiThread *)pObj;
    T_BaseMessage  *ptMsg   = (T_BaseMessage *)pMsg;
    WORD32          dwMsgID = ptMsg->dwMsgID;

    ptMsg->lwEndCycle = GetCycle();

    pThread->StatisticMsgQ(pThread->m_tMeasure.tMultiMsgQStat, 
                           dwMsgID, 
                           ptMsg->lwStartCycle, 
                           ptMsg->lwEndCycle);

    pThread->Dispatch(dwMsgID, 
                      ptMsg, 
                      sizeof(T_BaseMessage) + ptMsg->wMsgLen);

    pThread->m_pMsgMemPool->Free((BYTE *)pMsg);

    return SUCCESS;
}


CMultiThread::CMultiThread (const T_ThreadParam &rtParam)
    : CWorkThread(rtParam)
{
    m_eType   = NRT_THREAD;
    m_eClass  = E_THREAD_MULTI;
}


CMultiThread::~CMultiThread ()
{
}


/* 创建线程实例后执行初始化(在主线程栈空间执行) */
WORD32 CMultiThread::Initialize()
{
    TRACE_STACK("CMultiThread::Initialize()");

    WORD32 dwResult = CWorkThread::Initialize();
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    m_cMultiRing.Initialize();

    return SUCCESS;
}


VOID CMultiThread::DoRun()
{
    TRACE_STACK("CMultiThread::DoRun()");

    WORD64 lwTick           = THREAD_TICK_GRANULARITY;
    WORD32 dwProcHNum       = 0;
    WORD32 dwProcLNum       = 0;
    WORD32 dwProcDNum       = 0;
    WORD32 dwRemainLNum     = 0;
    WORD32 dwRemainDNum     = 0;
    WORD64 lwTimeStampStart = 0;
    WORD64 lwTimeStampProcH = 0;
    WORD64 lwTimeStampProcL = 0;
    WORD64 lwTimeStampProcD = 0;
    WORD64 lwTimeStampEnd   = 0;
    WORD64 lwTimeOutNum     = 0;

    while (TRUE)
    {
        while (m_MsgRingH.isEmpty()
            && m_MsgRingL.isEmpty()
            && m_MsgRingD.isEmpty()
            && m_cMultiRing.isEmpty())
        {
            /* 计算超时任务(处理完所有超时定时器) */
            Execute(lwTimeOutNum);
            m_cSemaphore.TimeWait(lwTick);
        }

        lwTimeStampStart  = GetCycle();
        m_dwCyclePer100NS = g_pGlobalClock->GetCyclePer100NS();

        /* 优先处理完所有高优先级队列中的消息 */
        dwProcHNum = m_MsgRingH.Dequeue(this, &CPollingThread::ProcMessageHP);

        lwTimeStampProcH = GetCycle();

        /* 每次循环固定从低优先级队列中取指定数量的消息 */
        dwRemainLNum = m_MsgRingL.Dequeue(m_dwCBNum, 
                                          this, 
                                          &CPollingThread::ProcMessageLP, 
                                          dwProcLNum);

        lwTimeStampProcL = GetCycle();

        /* 从数据队列接收用户面报文 */
        dwRemainDNum = m_MsgRingD.Dequeue(m_dwPacketCBNum, dwProcDNum);

        /* 从MultiRing接收业务消息 */
        dwRemainDNum = m_cMultiRing.Dequeue(m_dwMultiCBNum, 
                                            this, 
                                            &CMultiThread::ProcMultiMessage,
                                            dwProcDNum);

        lwTimeStampProcD = GetCycle();

        /* 执行元定时器操作(最多处理指定门限个数超时定时器) */
        Execute(lwTimeOutNum, m_dwTimerThresh);

        lwTimeStampEnd = GetCycle();

        Statistic(dwProcHNum, dwProcLNum, dwProcDNum, 
                  dwRemainLNum, dwRemainDNum, 
                  lwTimeStampStart, 
                  lwTimeStampProcH, lwTimeStampProcL, lwTimeStampProcD, 
                  lwTimeStampEnd);

        /* 线程退出 */
        if (unlikely(isCancel()))
        {
            LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_DEBUG, TRUE, 
                       "Thread Cancel\n");
            break ;
        }
    }
}


