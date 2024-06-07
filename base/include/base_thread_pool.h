

#ifndef _BASE_THREAD_POOL_H_
#define _BASE_THREAD_POOL_H_


#include "base_app_cntrl.h"


class CFactoryThread : public CFactoryTpl<CFactoryThread>
{
public :
    CFactoryThread () {}
    virtual ~CFactoryThread() {}

    VOID Dump();
};


#define DEFINE_THREAD(V)    \
    WORD32 __attribute__((used)) __dwThread_##V##_ = CFactoryThread::DefineProduct<V, T_ThreadParam>(#V)


class CThreadCntrl : public CSingleton<CThreadCntrl>, public CBaseData
{
public :
    CThreadCntrl ();
    virtual ~CThreadCntrl();

    WORD32 Initialize(CCentralMemPool *pMemInterface, WORD32 dwProcID);

    WORD32 StartThreads();

    WORD32 JoinThreads();

    CBaseThread * operator[] (WORD32 dwIndex);

    T_ThreadInfo * FindThreadInfo(WORD32 dwThreadID);
    CBaseThread  * FindThread(WORD32 dwThreadID);
    CBaseThread  * FindThread(E_AppClass  eDstClass,
                              WORD32      dwDstAssocID,
                              WORD32     &rdwDstAppID,
                              WORD32     &rdwDstThreadID);

    WORD32 GetThreadNum();

    VOID Dump();
    VOID Printf();

protected :
    WORD32 FetchJsonConfig();

    T_ThreadInfo * CreateInfo(WORD32            dwThreadID,
                              WORD32            dwLogicalID,
                              WORD32            dwPolicy,
                              WORD32            dwPriority,
                              WORD32            dwStakcSize,
                              WORD32            dwCBNum,
                              WORD32            dwPacketCBNum,
                              WORD32            dwMultiCBNum,
                              WORD32            dwTimerThresh,
                              BOOL              bAloneLog,
                              T_ProductDefInfo *ptDefInfo);

    WORD32 LoadApp(T_ThreadInfo &rtThread);

protected :
    CCentralMemPool      *m_pMemInterface;    /* Central内存池 */
    WORD32                m_dwThreadNum;
    T_ThreadInfo          m_atThreadInfo[MAX_WORKER_NUM];
};


inline CBaseThread * CThreadCntrl::operator[] (WORD32 dwIndex)
{
    if (unlikely(dwIndex >= m_dwThreadNum))
    {
        return NULL;
    }

    return m_atThreadInfo[dwIndex].pWorker;
}


inline T_ThreadInfo * CThreadCntrl::FindThreadInfo(WORD32 dwThreadID)
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwThreadNum; dwIndex++)
    {
        if (dwThreadID == m_atThreadInfo[dwIndex].dwThreadID)
        {
            return &(m_atThreadInfo[dwIndex]);
        }
    }

    return NULL;
}


inline CBaseThread * CThreadCntrl::FindThread(WORD32 dwThreadID)
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwThreadNum; dwIndex++)
    {
        if (dwThreadID == m_atThreadInfo[dwIndex].dwThreadID)
        {
            return m_atThreadInfo[dwIndex].pWorker;
        }
    }

    return NULL;
}


inline CBaseThread * CThreadCntrl::FindThread(E_AppClass  eDstClass,
                                              WORD32      dwDstAssocID,
                                              WORD32     &rdwDstAppID,
                                              WORD32     &rdwDstThreadID)
{
    rdwDstAppID    = g_pAppCntrl->GetAppIDByAppClass(eDstClass, dwDstAssocID);
    rdwDstThreadID = g_pAppCntrl->GetThreadIDByAppID(rdwDstAppID);

    return FindThread(rdwDstThreadID);
}


inline WORD32 CThreadCntrl::GetThreadNum()
{
    return m_dwThreadNum;
}


/* 向指定APP发送高优先级消息 */
inline WORD32 SendHighPriorMsgToApp(WORD32      dwDstThreadID,
                                    WORD32      dwDstAppID,
                                    WORD32      dwSrcAppID,
                                    WORD32      dwMsgID,
                                    WORD16      wLen,
                                    const VOID *ptMsg)
{
    CBaseThread *pThread = g_pThreadPool->FindThread(dwDstThreadID);
    if (unlikely(NULL == pThread))
    {
        return FAIL;
    }

    WORD32 dwNum = pThread->SendHPMsgToApp(dwDstAppID,
                                           dwSrcAppID,
                                           dwMsgID,
                                           wLen,
                                           ptMsg);
    if (0 == dwNum)
    {
        return FAIL;
    }

    return SUCCESS;
}


/* 发送高优先级消息 */
inline WORD32 SendHighPriorMsg(E_AppClass  eDstClass,
                               WORD32      dwDstAssocID,
                               WORD32      dwSrcAppID,
                               WORD32      dwMsgID,
                               WORD16      wLen,
                               const VOID *ptMsg)
{
    WORD32 dwDstAppID    = g_pAppCntrl->GetAppIDByAppClass(eDstClass, dwDstAssocID);
    WORD32 dwDstThreadID = g_pAppCntrl->GetThreadIDByAppID(dwDstAppID);

    return SendHighPriorMsgToApp(dwDstThreadID,
                                 dwDstAppID,
                                 dwSrcAppID,
                                 dwMsgID,
                                 wLen,
                                 ptMsg);
}


/* 向指定APP发送低优先级消息 */
inline WORD32 SendLowPriorMsgToApp(WORD32      dwDstThreadID,
                                   WORD32      dwDstAppID,
                                   WORD32      dwSrcAppID,
                                   WORD32      dwMsgID,
                                   WORD16      wLen,
                                   const VOID *ptMsg)
{
    CBaseThread *pThread = g_pThreadPool->FindThread(dwDstThreadID);
    if (unlikely(NULL == pThread))
    {
        return FAIL;
    }

    WORD32 dwNum = pThread->SendLPMsgToApp(dwDstAppID,
                                           dwSrcAppID,
                                           dwMsgID,
                                           wLen,
                                           ptMsg);
    if (0 == dwNum)
    {
        return FAIL;
    }

    return SUCCESS;
}


/* 发送低优先级消息 */
inline WORD32 SendLowPriorMsg(E_AppClass  eDstClass,
                              WORD32      dwDstAssocID,
                              WORD32      dwSrcAppID,
                              WORD32      dwMsgID,
                              WORD16      wLen,
                              const VOID *ptMsg)
{
    WORD32 dwDstAppID    = g_pAppCntrl->GetAppIDByAppClass(eDstClass, dwDstAssocID);
    WORD32 dwDstThreadID = g_pAppCntrl->GetThreadIDByAppID(dwDstAppID);

    return SendLowPriorMsgToApp(dwDstThreadID,
                                dwDstAppID,
                                dwSrcAppID,
                                dwMsgID,
                                wLen,
                                ptMsg);
}


inline WORD32 SendMultiRingMsgToApp(WORD32      dwDstThreadID,
                                    WORD32      dwDstAppID,
                                    WORD32      dwSrcAppID,
                                    WORD32      dwMsgID,
                                    WORD16      wLen,
                                    const VOID *ptMsg)
{
    CBaseThread *pThread = g_pThreadPool->FindThread(dwDstThreadID);
    if (unlikely(NULL == pThread))
    {
        return FAIL;
    }

    WORD32 dwNum = pThread->SendSTMsgToApp(dwDstAppID,
                                           dwSrcAppID,
                                           dwMsgID,
                                           wLen,
                                           ptMsg);
    if (0 == dwNum)
    {
        return FAIL;
    }

    return SUCCESS;
}


inline WORD32 SendMultiRingMsg(E_AppClass  eDstClass,
                               WORD32      dwDstAssocID,
                               WORD32      dwSrcAppID,
                               WORD32      dwMsgID,
                               WORD16      wLen,
                               const VOID *ptMsg)
{
    WORD32 dwDstAppID    = g_pAppCntrl->GetAppIDByAppClass(eDstClass, dwDstAssocID);
    WORD32 dwDstThreadID = g_pAppCntrl->GetThreadIDByAppID(dwDstAppID);

    return SendMultiRingMsgToApp(dwDstThreadID,
                                 dwDstAppID,
                                 dwSrcAppID,
                                 dwMsgID,
                                 wLen,
                                 ptMsg);
}


#endif


