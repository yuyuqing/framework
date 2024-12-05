

#ifndef _BASE_THREAD_SINGLETON_H_
#define _BASE_THREAD_SINGLETON_H_


#include "base_mem_pool.h"
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
    virtual CObjMemPoolInterface * CreateSTMemPool(WORD32 dwRingID);

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
CObjMemPoolInterface * CSingletonThread<TH, TM>::CreateSTMemPool(WORD32 dwRingID)
{
    return m_cMemPools.CreateSTPool(dwRingID);
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


