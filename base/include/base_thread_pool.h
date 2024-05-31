

#ifndef _BASE_THREAD_POOL_H_
#define _BASE_THREAD_POOL_H_


#include "base_app_cntrl.h"


class CThreadPool
{
public :
    template <class T>
    static WORD32 DefThread(const CHAR *pName);

public :
    CThreadPool ();
    virtual ~CThreadPool();

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

protected :
    T_ThreadDefInfo * Define(const CHAR *pName);

    T_ThreadDefInfo * FindDef(const CHAR *pName);

    WORD32 FetchJsonConfig();

    T_ThreadInfo * CreateThread(WORD32           dwThreadID,
                                WORD32           dwLogicalID,
                                WORD32           dwPolicy,
                                WORD32           dwPriority,
                                WORD32           dwStakcSize,
                                WORD32           dwCBNum,
                                WORD32           dwPacketCBNum,
                                WORD32           dwMultiCBNum,
                                WORD32           dwTimerThresh,
                                BOOL             bAloneLog,
                                T_ThreadDefInfo *ptDefInfo);

    T_ThreadInfo * Create();

    WORD32 LoadApp(T_ThreadInfo &rtThread);

protected :    
    WORD32           m_dwDefNum;
    T_ThreadDefInfo  m_atDefInfo[MAX_WORKER_NUM];

    WORD32           m_dwThreadNum;
    T_ThreadInfo     m_atThreadInfo[MAX_WORKER_NUM];

    CCentralMemPool *m_pMemInterface;    /* Central内存池 */
};


inline CBaseThread * CThreadPool::operator[] (WORD32 dwIndex)
{
    if (unlikely(dwIndex >= m_dwThreadNum))
    {
        return NULL;
    }

    return m_atThreadInfo[dwIndex].pWorker;
}


inline T_ThreadInfo * CThreadPool::FindThreadInfo(WORD32 dwThreadID)
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


inline CBaseThread * CThreadPool::FindThread(WORD32 dwThreadID)
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


inline CBaseThread * CThreadPool::FindThread(E_AppClass  eDstClass,
                                             WORD32      dwDstAssocID,
                                             WORD32     &rdwDstAppID,
                                             WORD32     &rdwDstThreadID)
{
    CAppCntrl *pAppCntrl = CAppCntrl::GetInstance();

    rdwDstAppID    = pAppCntrl->GetAppIDByAppClass(eDstClass, dwDstAssocID);
    rdwDstThreadID = pAppCntrl->GetThreadIDByAppID(rdwDstAppID);

    return FindThread(rdwDstThreadID);
}


inline WORD32 CThreadPool::GetThreadNum()
{
    return m_dwThreadNum;
}


class CThreadCntrl : public CSingleton<CThreadCntrl>
{
public :
    CThreadCntrl ();
    virtual ~CThreadCntrl();

    WORD32 Initialize(CCentralMemPool *pMemInterface, WORD32 dwProcID);

    CThreadPool * GetThreadPool();

    VOID Dump();

protected :
    CThreadPool         m_cThreadPool;
    CCentralMemPool    *m_pMemInterface;    /* Central内存池 */
};


inline CThreadPool * CThreadCntrl::GetThreadPool()
{
    return &m_cThreadPool;
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
    CAppCntrl *pAppCntrl = CAppCntrl::GetInstance();

    WORD32 dwDstAppID    = pAppCntrl->GetAppIDByAppClass(eDstClass, dwDstAssocID);
    WORD32 dwDstThreadID = pAppCntrl->GetThreadIDByAppID(dwDstAppID);

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
    CAppCntrl *pAppCntrl = CAppCntrl::GetInstance();

    WORD32 dwDstAppID    = pAppCntrl->GetAppIDByAppClass(eDstClass, dwDstAssocID);
    WORD32 dwDstThreadID = pAppCntrl->GetThreadIDByAppID(dwDstAppID);

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
    CAppCntrl *pAppCntrl = CAppCntrl::GetInstance();

    WORD32 dwDstAppID    = pAppCntrl->GetAppIDByAppClass(eDstClass, dwDstAssocID);
    WORD32 dwDstThreadID = pAppCntrl->GetThreadIDByAppID(dwDstAppID);

    return SendMultiRingMsgToApp(dwDstThreadID,
                                 dwDstAppID,
                                 dwSrcAppID,
                                 dwMsgID,
                                 wLen,
                                 ptMsg);
}


#define DEFINE_THREAD(T)    \
    WORD32 __attribute__((used)) __dwThread_##T##_ = CThreadPool::DefThread<T>(#T)


template <class T>
WORD32 CThreadPool::DefThread(const CHAR *pName)
{
    CThreadPool *pPool = CThreadCntrl::GetInstance()->GetThreadPool();
    if (NULL == pPool)
    {
        return FAIL;
    }

    T_ThreadDefInfo *ptInfo = pPool->Define(pName);
    if (NULL == ptInfo)
    {
        return FAIL;
    }

    ptInfo->pCreateFunc  = (PCreateThread)(&CDecorateDataV<T, T_ThreadParam>::Create);
    ptInfo->pDestroyFunc = (PDestroyThread)(&CDecorateDataV<T, T_ThreadParam>::Destroy);
    ptInfo->dwMemSize    = sizeof(CDecorateDataV<T, T_ThreadParam>);

    return SUCCESS;
}


#endif


