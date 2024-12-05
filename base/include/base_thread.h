

#ifndef _BASE_THREAD_H_
#define _BASE_THREAD_H_


#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include "pub_message_base_type.h"
#include "pub_message_logger.h"


#include "base_mem_pool_msg.h"
#include "base_timer.h"
#include "base_ring_message.h"
#include "base_log.h"
#include "base_app_state.h"


#define THREAD_TIMER_NODE_NUM                ((WORD32)(4800))
typedef CTimerList<THREAD_TIMER_NODE_NUM>    CTimerRepo;


typedef enum tagE_ThreadType
{
    RT_THREAD  = 0,    /* ʵʱ�߳� */
    NRT_THREAD = 1,    /* ��ʵʱ�߳� */
    INVALID_THREAD_TYPE,
}E_ThreadType;


typedef enum tagE_ThreadClass
{
    E_THREAD_INVALID = 0,
    E_THREAD_POLLING,              /* CPollingThread */
    E_THREAD_RT,                   /* CRTThread */
    E_THREAD_WORK,                 /* CWorkThread */
    E_THREAD_MULTI,                /* CMultiThread, ���ж��ST-Ring���� */
}E_ThreadClass;


typedef struct tagT_ThreadParam
{
    WORD32           dwProcID;
    WORD32           dwThreadID;
    WORD32           dwLogicalID;
    WORD32           dwPolicy;
    WORD32           dwPriority;
    WORD32           dwStackSize;
    WORD32           dwCBNum;
    WORD32           dwPacketCBNum;
    WORD32           dwMultiCBNum;
    WORD32           dwTimerThresh;
    BOOL             bAloneLog;
    CCentralMemPool *pMemPool;
}T_ThreadParam;


/* �߳�ר���������ڴ����Դ����Singleton�̶߳�������ʱ����, 
 * �߳�ר��������Ring������Դ����Singleton�̶߳�������ʱ����
 * CLogInfoʵ�����̶߳�������ʱ����
 * ���������ҵ���߳�ִ��RemoveThreadʱ����Դ���ղ���
 */
inline WORD32 RemoveThreadZone(T_DataZone &rtThreadZone, VOID *pThread)
{
    return SUCCESS;
}


typedef struct tagT_LogicalCoreConfig
{
    WORD32     dwLogicalID;  /* �߼���ID, ϵͳΨһ��� */
    WORD32     dwSocketID;   /* �����ID */
    WORD32     dwCoreID;     /* ������ϵ�CPU��ID */

    cpu_set_t  tCpuSet;      /* CPU�׺������� */
}T_CoreConfig;


class CCpuCoreInfo : public CSingleton<CCpuCoreInfo>
{
public :
    CCpuCoreInfo ();
    ~CCpuCoreInfo();

    WORD32 Initialize();

    T_CoreConfig * operator[] (WORD32 dwIndex);

    T_CoreConfig * operator() (WORD32 dwLogicalID);

    WORD32 GetCoreNum();

    VOID Dump();

protected :
    WORD32          m_dwCoreNum;
    T_CoreConfig    m_atCore[MAX_CPU_CORE_NUM];
};


inline T_CoreConfig * CCpuCoreInfo::operator[] (WORD32 dwIndex)
{
    if (unlikely(dwIndex >= m_dwCoreNum))
    {
        return NULL;
    }

    return &(m_atCore[dwIndex]);
}


inline T_CoreConfig * CCpuCoreInfo::operator() (WORD32 dwLogicalID)
{
    T_CoreConfig *ptCore = NULL;

    for (WORD32 dwIndex = 0; dwIndex < m_dwCoreNum; dwIndex++)
    {
        if (dwLogicalID == m_atCore[dwIndex].dwLogicalID)
        {
            ptCore = &(m_atCore[dwIndex]);
            break ;
        }
    }

    return ptCore;    
}


inline WORD32 CCpuCoreInfo::GetCoreNum()
{
    return m_dwCoreNum;
}


class CBaseThread : public CCBObject
{
protected :
    friend class CThreadCntrl;

    static WORD32 RegistThread(T_DataZone &rtThreadZone, VOID *pThread);
    static WORD32 RemoveThread(T_DataZone &rtThreadZone, VOID *pThread);

    static VOID * CallBack(VOID *pThread);

public :
    enum { DEFAULT_POLICY          = SCHED_OTHER };
    enum { DEFAULT_STACK_SIZE      = 16 * 1024 };
    enum { DEFAULT_PRIORITY        = 20 };
    enum { THREAD_TICK_GRANULARITY = 500000 };    /* 500ms���� */

public :
    CBaseThread (const T_ThreadParam &rtParam);
    virtual ~CBaseThread();

    /* �����߳�ʵ����ִ�г�ʼ��(�����߳�ջ�ռ�ִ��) */
    virtual WORD32 Initialize();

    /* �ڴ����߳�ʵ��ʱ����App */
    virtual WORD32 LoadApp(T_AppInfo *ptAppInfo) = 0;

    virtual VOID Run(CBaseThread *pApp) = 0;

    WORD32 Start();

    /* ����Ϣ�ҵ��̵߳ĸ����ȼ���Ϣ����(�ڷ��ͷ��߳�ջ�ռ����) */
    WORD32 SendHPMsgToApp(WORD32      dwDstAppID,
                          WORD32      dwSrcAppID,
                          WORD32      dwMsgID,
                          WORD16      wLen,
                          const VOID *ptMsg);

    /* ����Ϣ�ҵ��̵߳ĸ����ȼ���Ϣ����(�ڷ��ͷ��߳�ջ�ռ����)--�⿽�� */
    WORD32 SendHPMsg(WORD32      dwDstAppID,
                     WORD32      dwSrcAppID,
                     WORD32      dwMsgID,
                     WORD16      wLen,
                     const VOID *ptMsg);

    /* ����Ϣ�ҵ��̵߳ĵ����ȼ���Ϣ����(�ڷ��ͷ��߳�ջ�ռ����) */
    WORD32 SendLPMsgToApp(WORD32      dwDstAppID,
                          WORD32      dwSrcAppID,
                          WORD32      dwMsgID,
                          WORD16      wLen,
                          const VOID *ptMsg);

    /* ����Ϣ�ҵ��̵߳ĵ����ȼ���Ϣ����(�ڷ��ͷ��߳�ջ�ռ����)--�⿽�� */
    WORD32 SendLPMsg(WORD32      dwDstAppID,
                     WORD32      dwSrcAppID,
                     WORD32      dwMsgID,
                     WORD16      wLen,
                     const VOID *ptMsg);

    /* ����Ϣ�ҵ��̵߳�ST-Ring����(�ڷ��ͷ��߳�ջ�ռ����) */
    WORD32 SendSTMsgToApp(WORD32      dwDstAppID,
                          WORD32      dwSrcAppID,
                          WORD32      dwMsgID,
                          WORD16      wLen,
                          const VOID *ptMsg);

    WORD32 SchedulePriority();

    SWORD32 SetAffinity(pthread_t &rtThreadID, cpu_set_t &rtCpuSet);

    /* Ϊ�߳���������, ����ssh��̨�۲� */
    virtual SWORD32 SetThreadName(const CHAR *pName);

    WORD32 Join();

    WORD32 Detach();

    virtual WORD32 Cancel();

    BOOL isCancel();

    VOID SetCancel(BOOL bFlag);

    pthread_t & GetThreadT();

    BYTE * GetName();

    E_ThreadType GetThreadType();

    E_ThreadClass GetThreadClass();

    WORD32 GetProcID();

    WORD32 GetThreadID();

    WORD32 GetLogicalID();

    CLogInfo * GetLogger();

    /* �����ȼ����� */
    virtual CMessageRing * GetMsgRingH();

    /* �����ȼ����� */
    virtual CMessageRing * GetMsgRingL();

    /* ���ݱ��Ķ��� */
    virtual CDataPlaneRing * GetMsgRingD();

    virtual CMultiMessageRing * GetMultiRing();

    virtual CTimerRepo * GetTimerRepo();

    virtual CObjMemPoolInterface * CreateSTMemPool(WORD32 dwRingID);

    virtual WORD32 Notify();

    CMsgMemPool * GetMsgMemPool();

    /* ��ȡ��Ϣ���е��ڴ�ʹ���� */
    VOID GetMsgMemMeasure(T_MsgMemMeasure &rtMeasure);

    virtual VOID Dump();

    /* �����߳���־�ļ����� */
    VOID UpdateGlobalSwitch(BOOL bFlag);

    /* �����߳���־�ļ����� */
    VOID UpdatePeriod(E_LogFilePeriod ePeriod);

    /* �����߳���־�ļ����� */
    VOID UpdateModuleSwitch(const T_LogSetModuleSwitch &rtMsg);

    /* ��ȡ�߳�ά����Ϣ */
    VOID GetMeasure(T_ThreadMeasure &rtMeasure);

    /* ��ȡ�̳߳��е���Ϣ����ά����Ϣ */
    VOID GetRingMeasure(T_ThreadRingMeasure &rtMeasure);

    /* �����߳��� */
    VOID GenerateThrdName(const CHAR *pThrdName);

protected :
    /* ������־�ļ��� */
    VOID GenerateName(const CHAR *pProcName);

    WORD32 InitLogger(WORD32 dwProcID);

    WORD32 InitMeasure();

protected :
    CCentralMemPool             &m_rCentralMemPool;

    BYTE                         m_aucThrdName[WORKER_NAME_LEN];  /* ���������߳��� */
    BYTE                         m_aucName[WORKER_NAME_LEN];      /* ����������־�ļ��� */

    pthread_t                    m_tThreadID;    /* ϵͳ���÷��ص��߳�ID */
    cpu_set_t                    m_tCpuSet;

    WORD32                       m_dwProcID;     /* ����ID */

    E_ThreadType                 m_eType;        /* �߳�����(ʵʱ/��ʵʱ) */
    E_ThreadClass                m_eClass;       /* �߳���(Polling/RT/Worker/Multi) */
    WORD32                       m_dwThreadID;   /* �ڲ���ʶ�߳�ID, Ψһ��� */
    WORD32                       m_dwLogicalID;  /* ������(�ڲ�Ψһ��ʶCore) */
    WORD32                       m_dwSocketID;   /* �����ID */
    WORD32                       m_dwCoreID;     /* ������ϵ�CPU��ID */

    WORD32                       m_dwPolicy;
    WORD32                       m_dwPriority;
    WORD32                       m_dwStackSize;
    BOOL                         m_bAloneLog;

    std::atomic<BOOL>            m_bCancel;

    CMsgMemPool                 *m_pMsgMemPool;  /* ÿ���̳߳���һ����Ϣ�����ڴ�� */

    CLogInfo                    *m_pLogger;
    CLogMemPool                 *m_pLogMemPool;
    CMultiMessageRing::CSTRing  *m_pLogRing;

    T_AtomicThreadMeasure        m_tMeasure;
};


/* ����Ϣ�ҵ��̵߳ĸ����ȼ���Ϣ����(�ڷ��ͷ��߳�ջ�ռ����) */
inline WORD32 CBaseThread::SendHPMsgToApp(WORD32      dwDstAppID,
                                          WORD32      dwSrcAppID,
                                          WORD32      dwMsgID,
                                          WORD16      wLen,
                                          const VOID *ptMsg)
{
    CMessageRing *pRing = GetMsgRingH();
    if (unlikely(NULL == pRing))
    {
        return 0;
    }

    T_BaseMessage *ptBaseMsg = (T_BaseMessage *)(m_pMsgMemPool->Malloc(sizeof(T_BaseMessage) + wLen));
    if (unlikely(NULL == ptBaseMsg))
    {
        return 0;
    }

    ptBaseMsg->dwMsgID       = dwMsgID;
    ptBaseMsg->dwSrcAppID    = dwSrcAppID;
    ptBaseMsg->dwDstAppID    = dwDstAppID;
    ptBaseMsg->dwSrcThreadID = m_dwSelfThreadID;
    ptBaseMsg->dwDstThreadID = m_dwThreadID;
    ptBaseMsg->wMsgLen       = wLen;
    ptBaseMsg->lwStartCycle  = GetCycle();
    ptBaseMsg->lwEndCycle    = 0;

    if ((NULL != ptMsg) && (0 != wLen))
    {
        memcpy(ptBaseMsg->aValue, ptMsg, wLen);
    }

    WORD32 dwNum = pRing->Enqueue((VOID *)ptBaseMsg, INVALID_WORD);
    if (dwNum > 0)
    {
        Notify();
    }
    else
    {
        m_pMsgMemPool->Free((BYTE *)ptBaseMsg);
    }

    return dwNum;
}


/* ����Ϣ�ҵ��̵߳ĸ����ȼ���Ϣ����(�ڷ��ͷ��߳�ջ�ռ����)--�⿽�� */
inline WORD32 CBaseThread::SendHPMsg(WORD32      dwDstAppID,
                                     WORD32      dwSrcAppID,
                                     WORD32      dwMsgID,
                                     WORD16      wLen,
                                     const VOID *ptMsg)
{
    T_BaseMessage *ptBaseMsg = (T_BaseMessage *)(((WORD64)ptMsg) - sizeof(T_BaseMessage));

    CMessageRing *pRing = GetMsgRingH();
    if (unlikely(NULL == pRing))
    {
        m_pMsgMemPool->Free((BYTE *)ptBaseMsg);
        return 0;
    }

    ptBaseMsg->dwMsgID       = dwMsgID;
    ptBaseMsg->dwSrcAppID    = dwSrcAppID;
    ptBaseMsg->dwDstAppID    = dwDstAppID;
    ptBaseMsg->dwSrcThreadID = m_dwSelfThreadID;
    ptBaseMsg->dwDstThreadID = m_dwThreadID;
    ptBaseMsg->wMsgLen       = wLen;
    ptBaseMsg->lwStartCycle  = GetCycle();
    ptBaseMsg->lwEndCycle    = 0;

    WORD32 dwNum = pRing->Enqueue((VOID *)ptBaseMsg, INVALID_WORD);
    if (dwNum > 0)
    {
        Notify();
    }
    else
    {
        m_pMsgMemPool->Free((BYTE *)ptBaseMsg);
    }

    return dwNum;
}


/* ����Ϣ�ҵ��̵߳ĵ����ȼ���Ϣ����(�ڷ��ͷ��߳�ջ�ռ����) */
inline WORD32 CBaseThread::SendLPMsgToApp(WORD32      dwDstAppID,
                                          WORD32      dwSrcAppID,
                                          WORD32      dwMsgID,
                                          WORD16      wLen,
                                          const VOID *ptMsg)
{
    CMessageRing *pRing = GetMsgRingL();
    if (unlikely(NULL == pRing))
    {
        return 0;
    }

    T_BaseMessage *ptBaseMsg = (T_BaseMessage *)(m_pMsgMemPool->Malloc(sizeof(T_BaseMessage) + wLen));
    if (NULL == ptBaseMsg)
    {
        return 0;
    }

    ptBaseMsg->dwMsgID       = dwMsgID;
    ptBaseMsg->dwSrcAppID    = dwSrcAppID;
    ptBaseMsg->dwDstAppID    = dwDstAppID;
    ptBaseMsg->dwSrcThreadID = m_dwSelfThreadID;
    ptBaseMsg->dwDstThreadID = m_dwThreadID;
    ptBaseMsg->wMsgLen       = wLen;
    ptBaseMsg->lwStartCycle  = GetCycle();
    ptBaseMsg->lwEndCycle    = 0;

    if ((NULL != ptMsg) && (0 != wLen))
    {
        memcpy(ptBaseMsg->aValue, ptMsg, wLen);
    }

    WORD32 dwNum = pRing->Enqueue((VOID *)ptBaseMsg, INVALID_WORD);
    if (dwNum > 0)
    {
        Notify();
    }
    else
    {
        m_pMsgMemPool->Free((BYTE *)ptBaseMsg);
    }

    return dwNum;
}


/* ����Ϣ�ҵ��̵߳ĵ����ȼ���Ϣ����(�ڷ��ͷ��߳�ջ�ռ����)--�⿽�� */
inline WORD32 CBaseThread::SendLPMsg(WORD32      dwDstAppID,
                                     WORD32      dwSrcAppID,
                                     WORD32      dwMsgID,
                                     WORD16      wLen,
                                     const VOID *ptMsg)
{
    T_BaseMessage *ptBaseMsg = (T_BaseMessage *)(((WORD64)ptMsg) - sizeof(T_BaseMessage));

    CMessageRing *pRing = GetMsgRingL();
    if (unlikely(NULL == pRing))
    {
        m_pMsgMemPool->Free((BYTE *)ptBaseMsg);
        return 0;
    }

    ptBaseMsg->dwMsgID       = dwMsgID;
    ptBaseMsg->dwSrcAppID    = dwSrcAppID;
    ptBaseMsg->dwDstAppID    = dwDstAppID;
    ptBaseMsg->dwSrcThreadID = m_dwSelfThreadID;
    ptBaseMsg->dwDstThreadID = m_dwThreadID;
    ptBaseMsg->wMsgLen       = wLen;
    ptBaseMsg->lwStartCycle  = GetCycle();
    ptBaseMsg->lwEndCycle    = 0;

    WORD32 dwNum = pRing->Enqueue((VOID *)ptBaseMsg, INVALID_WORD);
    if (dwNum > 0)
    {
        Notify();
    }
    else
    {
        m_pMsgMemPool->Free((BYTE *)ptBaseMsg);
    }

    return dwNum;
}


/* ����Ϣ�ҵ��̵߳�ST-Ring����(�ڷ��ͷ��߳�ջ�ռ����) */
inline WORD32 CBaseThread::SendSTMsgToApp(WORD32      dwDstAppID,
                                          WORD32      dwSrcAppID,
                                          WORD32      dwMsgID,
                                          WORD16      wLen,
                                          const VOID *ptMsg)
{
    CMultiMessageRing *pRing = GetMultiRing();
    if (unlikely(NULL == pRing))
    {
        return 0;
    }

    T_BaseMessage *ptBaseMsg = (T_BaseMessage *)(m_pMsgMemPool->Malloc(sizeof(T_BaseMessage) + wLen));
    if (NULL == ptBaseMsg)
    {
        return 0;
    }

    ptBaseMsg->dwMsgID       = dwMsgID;
    ptBaseMsg->dwSrcAppID    = dwSrcAppID;
    ptBaseMsg->dwDstAppID    = dwDstAppID;
    ptBaseMsg->dwSrcThreadID = m_dwSelfThreadID;
    ptBaseMsg->dwDstThreadID = m_dwThreadID;
    ptBaseMsg->wMsgLen       = wLen;
    ptBaseMsg->lwStartCycle  = GetCycle();
    ptBaseMsg->lwEndCycle    = 0;

    if ((NULL != ptMsg) && (0 != wLen))
    {
        memcpy(ptBaseMsg->aValue, ptMsg, wLen);
    }

    WORD32 dwNum = pRing->Enqueue(m_dwSelfRingID, (VOID *)ptBaseMsg, INVALID_WORD);
    if (dwNum > 0)
    {
        Notify();
    }
    else
    {
        m_pMsgMemPool->Free((BYTE *)ptBaseMsg);
    }

    return dwNum;
}


inline BOOL CBaseThread::isCancel()
{
    BOOL bCancel = m_bCancel.load(std::memory_order_relaxed);
    return bCancel;
}


inline VOID CBaseThread::SetCancel(BOOL bFlag)
{
    m_bCancel.store(bFlag, std::memory_order_relaxed);
}


inline pthread_t & CBaseThread::GetThreadT()
{
    return m_tThreadID;
}


inline BYTE * CBaseThread::GetName()
{
    return m_aucName;
}


inline E_ThreadType CBaseThread::GetThreadType()
{
    return m_eType;
}


inline E_ThreadClass CBaseThread::GetThreadClass()
{
    return m_eClass;
}


inline WORD32 CBaseThread::GetProcID()
{
    return m_dwProcID;
}


inline WORD32 CBaseThread::GetThreadID()
{
    return m_dwThreadID;
}


inline WORD32 CBaseThread::GetLogicalID()
{
    return m_dwLogicalID;
}


inline CLogInfo * CBaseThread::GetLogger()
{
    return m_pLogger;
}


inline WORD32 CBaseThread::Notify()
{
    return SUCCESS;
}


inline CMsgMemPool * CBaseThread::GetMsgMemPool()
{
    return m_pMsgMemPool;
}


/* ��ȡ��Ϣ���е��ڴ�ʹ���� */
inline VOID CBaseThread::GetMsgMemMeasure(T_MsgMemMeasure &rtMeasure)
{
    m_pMsgMemPool->GetMeasure(rtMeasure);
}


typedef struct tagT_ThreadInfo
{
    WORD32            dwThreadID;     /* �߳�ID, �ڲ�Ψһ��ʶ */
    WORD32            dwLogicalID;    /* �̰߳󶨵��߼���ID */
    WORD32            dwPolicy;       /* �̵߳��Ȳ���; SCHED_RR(2)/SCHED_FIFO(1)/SCHED_OTHER(0) */
    WORD32            dwPriority;     /* �߳����ȼ� */
    WORD32            dwStackSize;    /* �߳�ջ�ռ��С */
    WORD32            dwCBNum;        /* �����ȼ�����ÿ��ѭ�����ص����� */
    WORD32            dwPacketCBNum;  /* ���Ķ���ÿ��ѭ�����ص����� */
    WORD32            dwMultiCBNum;   /* MultiRing����ÿ��ѭ�����ص�����(���STRing) */
    WORD32            dwTimerThresh;  /* ��ʱ������ÿ��ѭ�����ص����� */
    BOOL              bAloneLog;      /* �̼߳�¼��־�Ƿ�ʹ�ö�����־�ļ� */
    PCreateProduct    pCreateFunc;    /* �̶߳���Ĵ������� */
    PDestroyProduct   pDestroyFunc;   /* �̶߳�������ٺ��� */
    CBaseThread      *pWorker;        /* �̶߳��� */
    WORD32            dwMemSize;      /* �̶߳���ռ���ڴ��С */
    WORD32            dwAppNum;       /* �̰߳󶨵�App���� */
    BYTE             *pMem;           /* �̶߳����ڴ�� */
    T_AppJsonCfg      atApp[MAX_APP_NUM_PER_THREAD];
    CHAR              aucName[WORKER_NAME_LEN];

    tagT_ThreadInfo & operator=(const tagT_ThreadInfo &rValue)
    {
        if (&rValue == this)
        {
            return *this;
        }

        this->dwThreadID    = rValue.dwThreadID;
        this->dwLogicalID   = rValue.dwLogicalID;
        this->dwPolicy      = rValue.dwPolicy;
        this->dwPriority    = rValue.dwPriority;
        this->dwStackSize   = rValue.dwStackSize;
        this->dwCBNum       = rValue.dwCBNum;
        this->dwPacketCBNum = rValue.dwPacketCBNum;
        this->dwMultiCBNum  = rValue.dwMultiCBNum;
        this->dwTimerThresh = rValue.dwTimerThresh;
        this->bAloneLog     = rValue.bAloneLog;
        this->pCreateFunc   = rValue.pCreateFunc;
        this->pDestroyFunc  = rValue.pDestroyFunc;
        this->pWorker       = rValue.pWorker;
        this->dwMemSize     = rValue.dwMemSize;
        this->dwAppNum      = MIN(rValue.dwAppNum, MAX_APP_NUM_PER_THREAD);
        this->pMem          = rValue.pMem;

        memset(this->atApp, 0x00, MAX_APP_NUM_PER_THREAD * sizeof(T_AppJsonCfg));
        memcpy(this->aucName, rValue.aucName, WORKER_NAME_LEN);

        for (WORD32 dwIndex = 0; dwIndex < this->dwAppNum; dwIndex++)
        {
            this->atApp[dwIndex].dwAppID      = rValue.atApp[dwIndex].dwAppID;
            this->atApp[dwIndex].dwEventBegin = rValue.atApp[dwIndex].dwEventBegin;
            this->atApp[dwIndex].bAssocFlag   = rValue.atApp[dwIndex].bAssocFlag;
            this->atApp[dwIndex].dwAssocNum   = rValue.atApp[dwIndex].dwAssocNum;

            memcpy((this->atApp[dwIndex].aucName),
                   (rValue.atApp[dwIndex].aucName),
                   APP_NAME_LEN);
            memcpy((this->atApp[dwIndex].adwAssocID),
                   (rValue.atApp[dwIndex].adwAssocID),
                   MAX_ASSOCIATE_NUM_PER_APP * sizeof(WORD32));
        }

        return *this;
    }
}T_ThreadInfo;


extern CLogInfo * InitLogInfo(CCentralMemPool &rCentralMemPool, 
                              WORD32           dwProcID, 
                              CHAR            *pFileName);

extern CMultiMessageRing::CSTRing * InitLogRing(WORD32 dwRingID);
extern CLogMemPool * InitLogMemPool(WORD32 dwRingID);

/* ���߳��ڴ�����־�߳�ʵ��ʱ, ͨ���ص������ӿ�ע�����߳�ר�������� */
extern WORD32 RegistMainLog(T_DataZone &rtThreadZone, VOID *pThread);


#endif


