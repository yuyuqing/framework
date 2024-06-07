

#ifndef _BASE_THREAD_SINGLETON_H_
#define _BASE_THREAD_SINGLETON_H_


#include "base_thread_multi.h"
#include "base_app_cntrl.h"


typedef struct tagT_BindAppInfo
{
    WORD32        dwAppNum;
    T_AppJsonCfg  atApp[MAX_APP_NUM_PER_THREAD];
}T_BindAppInfo;


/* TH为单例线程类, 例如: CThreadLog/CThreadTimer
 * TM的类型必须是CObjectMemPool<>的派生类, 例如: CLogMemPool/CTimerMemPool 
 */
template<class TH, class TM>
class CSingletonThread : public CMultiThread, public CBaseData
{
public :
    static TH * CreateInstance(const T_ThreadParam &rtParam);
    static TH * GetInstance();

    static VOID Destroy();

public :
    CSingletonThread (const T_ThreadParam &rtParam);
    virtual ~CSingletonThread();

    /* 创建线程实例后执行初始化(在主线程栈空间执行) */
    virtual WORD32 Initialize();

    /* 在RegistZone时调用(即, 在业务线程创建后, 在业务线程的栈空间调用) */
    virtual CObjMemPoolInterface * CreateSTMemPool();

    /* 通过线程专属内存池+线程专属Ring队列发送消息给线程 */
    WORD32 SendExclusiveMsgToThread(WORD32 dwMsgID, WORD16 wLen, const VOID *ptMsg);

protected :
    WORD32 BindApp();

protected :
    CMultiMemPool<TM>    m_cMemPools;
    T_BindAppInfo        m_tBindInfo;

private :
    static TH *s_pInstance; 
};


template<class TH, class TM>
TH * CSingletonThread<TH, TM>::s_pInstance = NULL;


template<class TH, class TM>
TH * CSingletonThread<TH, TM>::CreateInstance(const T_ThreadParam &rtParam)
{
    if ((NULL != s_pInstance) || (NULL == rtParam.pMemPool))
    {
        assert(0);
    }

    BYTE *pMem = rtParam.pMemPool->Malloc(sizeof(TH));
    if (NULL == pMem)
    {
        assert(0);
    }

    s_pInstance = new (pMem) TH(rtParam);

    s_pInstance->Initialize();
    s_pInstance->BindApp();

    return s_pInstance;
}


template<class TH, class TM>
inline TH * CSingletonThread<TH, TM>::GetInstance()
{
    if (NULL != s_pInstance)
    {
        return s_pInstance;
    }

    return NULL;
}


template<class TH, class TM>
VOID CSingletonThread<TH, TM>::Destroy()
{
    if (NULL != s_pInstance)
    {
        CCentralMemPool &rCentralMemPool = s_pInstance->m_rCentralMemPool;
        delete s_pInstance;
        rCentralMemPool.Free((BYTE *)s_pInstance);
    }

    s_pInstance = NULL;
}


template<class TH, class TM>
CSingletonThread<TH, TM>::CSingletonThread (const T_ThreadParam &rtParam)
    : CMultiThread(rtParam),
      m_cMemPools(*(rtParam.pMemPool))
{
    memset(&m_tBindInfo, 0x00, sizeof(m_tBindInfo));
}


template<class TH, class TM>
CSingletonThread<TH, TM>::~CSingletonThread()
{
}


/* 创建线程实例后执行初始化(在主线程栈空间执行) */
template<class TH, class TM>
WORD32 CSingletonThread<TH, TM>::Initialize()
{
    WORD32 dwResult = CMultiThread::Initialize();
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    m_cMemPools.Initialize();

    return SUCCESS;
}


/* 在RegistZone时调用(即, 在业务线程创建后, 在业务线程的栈空间调用) */
template<class TH, class TM>
CObjMemPoolInterface * CSingletonThread<TH, TM>::CreateSTMemPool()
{
    return m_cMemPools.CreateSTPool();
}


/* 通过线程专属内存池+线程专属Ring队列发送消息给线程(在业务线程栈空间调用) */
template<class TH, class TM>
WORD32 CSingletonThread<TH, TM>::SendExclusiveMsgToThread(WORD32      dwMsgID,
                                                          WORD16      wLen,
                                                          const VOID *ptMsg)
{
    CMultiMessageRing *pRing = GetMultiRing();
    if (unlikely(NULL == pRing))
    {
        return 0;
    }

    BYTE *pBuffer = m_cMemPools.Malloc(wLen, INVALID_WORD);
    if (NULL == pBuffer)
    {
        return 0;
    }

    if ((NULL != ptMsg) && (0 != wLen))
    {
        memcpy(pBuffer, ptMsg, wLen);
    }

    WORD32 dwNum = pRing->Enqueue((VOID *)pBuffer, INVALID_WORD);
    if (dwNum > 0)
    {
        Notify();
    }
    else
    {
        m_cMemPools.Free(pBuffer);
    }

    return dwNum;
}


template<class TH, class TM>
WORD32 CSingletonThread<TH, TM>::BindApp()
{
    CBaseThread   *pWorker   = (CBaseThread *)this;
    T_AppJsonCfg  *ptAppCfg  = NULL;
    T_AppInfo     *ptAppInfo = NULL;
    CAppInterface *pAppInst  = NULL;

    for (WORD32 dwIndex = 0; dwIndex < m_tBindInfo.dwAppNum; dwIndex++)
    {
        ptAppCfg  = &(m_tBindInfo.atApp[dwIndex]);
        ptAppInfo = g_pAppCntrl->Create(ptAppCfg->aucName,
                                        pWorker->GetThreadID(),
                                        ptAppCfg);
        if (NULL == ptAppInfo)
        {
            continue ;
        }

        pAppInst = ptAppInfo->pAppState->GetAppInst();

        pAppInst->PreInit(pWorker, ptAppInfo);
        pWorker->LoadApp(ptAppInfo);
    }

    return SUCCESS;
}


#endif


