

#include "base_thread.h"
#include "base_mem_mgr.h"


CLogInfo * InitLogInfo(CCentralMemPool &rCentralMemPool, 
                       WORD32           dwProcID, 
                       CHAR            *pFileName)
{
    BYTE *pMem = rCentralMemPool.Malloc(sizeof(CLogInfo));
    if (NULL == pMem)
    {
        return NULL;
    }

    CLogInfo *pLogger = new (pMem) CLogInfo();
    pLogger->Initialize(dwProcID, pFileName);

    return pLogger;
}


CMultiMessageRing::CSTRing * InitLogRing(WORD32 dwRingID)
{
    if (NULL == g_pLogThread)
    {
        return NULL;
    }

    CMultiMessageRing *pRings = g_pLogThread->GetMultiRing();
    if (NULL == pRings)
    {
        return NULL;
    }

    return pRings->CreateSTRing(dwRingID);
}


CLogMemPool * InitLogMemPool(WORD32 dwRingID)
{
    if (NULL == g_pLogThread)
    {
        return NULL;
    }

    return (CLogMemPool *)(g_pLogThread->CreateSTMemPool(dwRingID));
}


/* 分配主线程专属内存池资源(专用于日志打印的LogMemPool)
 * 分配主线程专属Ring队列资源(专用于日志打印的LogRing)
 * 主线程CLogInfo对象(用于日志开关控制)
 * 主线程日志相关专属资源均由g_pLogThread负责分配
 */
WORD32 RegistMainLog(T_DataZone &rtThreadZone, VOID *pThread)
{
    CMultiMessageRing::CSTRing *pRing = InitLogRing(m_dwSelfRingID);
    if (NULL == pRing)
    {
        return FAIL;
    }

    CLogMemPool *pMemPool = InitLogMemPool(m_dwSelfRingID);
    if (NULL == pMemPool)
    {
        return FAIL;
    }

    rtThreadZone.pLogMemPool = (VOID *)pMemPool;
    rtThreadZone.pLogRing    = (VOID *)pRing;
    rtThreadZone.pLogger     = g_pLogger;
    rtThreadZone.pThread     = NULL;

    return SUCCESS;
}


CCpuCoreInfo::CCpuCoreInfo ()
{
    m_dwCoreNum = 0;
    memset(m_atCore, 0x00, sizeof(m_atCore));
}


CCpuCoreInfo::~CCpuCoreInfo()
{
    m_dwCoreNum = 0;
    memset(m_atCore, 0x00, sizeof(m_atCore));
}


WORD32 CCpuCoreInfo::Initialize()
{
    WORD32 dwCoreNum = sysconf(_SC_NPROCESSORS_CONF);

    for (WORD32 dwIndex = 0;
         ((dwIndex < MAX_CPU_CORE_NUM) && (dwIndex < dwCoreNum));
         dwIndex++)
    {
        CPU_ZERO(&(m_atCore[dwIndex].tCpuSet));

        if (FALSE == CpuDetected(dwIndex))
        {
            continue ;
        }

        CPU_SET(dwIndex, &(m_atCore[dwIndex].tCpuSet));

        m_atCore[dwIndex].dwLogicalID = m_dwCoreNum;
        m_atCore[dwIndex].dwSocketID  = SocketID(dwIndex);
        m_atCore[dwIndex].dwCoreID    = CoreID(dwIndex);

        m_dwCoreNum++;
    }

    return SUCCESS;
}


VOID CCpuCoreInfo::Dump()
{
    TRACE_STACK("CCpuCoreInfo::Dump()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "m_dwCoreNum : %d\n",
               m_dwCoreNum);

    for (WORD32 dwIndex = 0; dwIndex < m_dwCoreNum; dwIndex++)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                   "dwLogicalID : %2d, dwSocketID : %2d, dwCoreID : %2d\n",
                   m_atCore[dwIndex].dwLogicalID,
                   m_atCore[dwIndex].dwSocketID,
                   m_atCore[dwIndex].dwCoreID);
    }
}


/* 线程专属日志内存池资源(用于日志打印的LogMemPool)
 * 线程专属日志Ring队列资源(用于日志打印的LogRing)
 * 线程专属CLogInfo对象(用于日志开关控制)
 * 线程专属定时器内存池资源(用于定时器的TimerMemPool)
 * 线程专属定时器Ring队列资源(用于定时器的TimerRing)
 */
WORD32 CBaseThread::RegistThread(T_DataZone &rtThreadZone, VOID *pThread)
{
    CBaseThread *pInst = static_cast<CBaseThread *>(pThread);
    if (NULL == pInst)
    {
        return FAIL;
    }

    /* 初始化线程日志 */
    pInst->InitLogger(pInst->m_dwProcID);

    rtThreadZone.pLogMemPool = (VOID *)(pInst->m_pLogMemPool);
    rtThreadZone.pLogRing    = (VOID *)(pInst->m_pLogRing);
    rtThreadZone.pLogger     = (VOID *)(pInst->m_pLogger);
    rtThreadZone.pThread     = (VOID *)(pInst);

    return SUCCESS;
}


/* 线程专属数据区内存池资源会在Singleton线程对象销毁时回收, 
 * 线程专属数据区Ring队列资源会在Singleton线程对象销毁时回收
 * CLogInfo实例在线程对象销毁时回收
 * 因而无需在业务线程执行RemoveThread时做资源回收操作
 */
WORD32 CBaseThread::RemoveThread(T_DataZone &rtThreadZone, VOID *pThread)
{
    CBaseThread *pInst = static_cast<CBaseThread *>(pThread);
    if (NULL == pInst)
    {
        return FAIL;
    }

    pInst->m_pLogMemPool = NULL;
    pInst->m_pLogRing    = NULL;

    return SUCCESS;
}


VOID * CBaseThread::CallBack(VOID *pThread)
{
    CBaseThread *pInst = static_cast<CBaseThread *>(pThread);
    if (NULL == pInst)
    {
        return NULL;
    }

    CDataZone *pDataZone = CMemMgr::GetInstance()->GetDataZone();
    if (NULL == pDataZone)
    {
        return NULL;
    }

    pInst->SetThreadName((const CHAR *)(pInst->m_aucThrdName));

    WORD32 dwThreadIdx = INVALID_DWORD;

    /* 注册线程专属数据区 */
    dwThreadIdx = pDataZone->RegistZone((WORD32)(pInst->m_eType),
                                        (CB_RegistZone)(&CBaseThread::RegistThread),
                                        pThread);

    /* 设置CPU核亲和性 */
    pInst->SetAffinity(pInst->m_tThreadID, pInst->m_tCpuSet);

    /* 初始化线程级测量 */
    pInst->InitMeasure();

    pInst->Run(pInst);

    /* 删除线程专属数据区 */
    pDataZone->RemoveZone(dwThreadIdx,
                          (CB_RemoveZone)(&CBaseThread::RemoveThread),
                          pThread);

    return NULL;
}


CBaseThread::CBaseThread (const T_ThreadParam &rtParam)
    : m_rCentralMemPool(*(rtParam.pMemPool))
{
    memset(m_aucName,     0x00, sizeof(m_aucName));
    memset(m_aucThrdName, 0x00, sizeof(m_aucThrdName));
    memset(&m_tThreadID,  0x00, sizeof(m_tThreadID));
    CPU_ZERO(&m_tCpuSet);

    if ( (rtParam.dwPolicy != SCHED_FIFO)
      && (rtParam.dwPolicy != SCHED_RR))
    {
        m_dwPolicy = SCHED_OTHER;
    }
    else
    {
        m_dwPolicy = rtParam.dwPolicy;
    }

    m_dwProcID      = rtParam.dwProcID;
    m_eType         = NRT_THREAD;
    m_eClass        = E_THREAD_INVALID;
    m_dwThreadID    = rtParam.dwThreadID;
    m_dwLogicalID   = rtParam.dwLogicalID;
    m_dwSocketID    = 0;
    m_dwCoreID      = 0;
    m_dwPriority    = rtParam.dwPriority;
    m_dwStackSize   = rtParam.dwStackSize;
    m_bAloneLog     = rtParam.bAloneLog;
    m_bCancel       = FALSE;
    m_pMsgMemPool   = NULL;
    m_pLogger       = NULL;
    m_pLogMemPool   = NULL;
    m_pLogRing      = NULL;
}


CBaseThread::~CBaseThread()
{
    //pthread_cancel(m_tThreadID);

    /* 销毁CLogInfo实例 */
    if ((NULL != m_pLogger) && (TRUE == m_bAloneLog))
    {
        delete m_pLogger;
        m_rCentralMemPool.Free((BYTE *)m_pLogger);
    }

    memset(m_aucName,     0x00, sizeof(m_aucName));
    memset(m_aucThrdName, 0x00, sizeof(m_aucThrdName));
    memset(&m_tThreadID,  0x00, sizeof(m_tThreadID));

    CPU_ZERO(&m_tCpuSet);

    m_dwThreadID  = INVALID_DWORD;
    m_dwLogicalID = 0;
    m_dwSocketID  = 0;
    m_dwCoreID    = 0;
    m_dwPolicy    = 0;
    m_dwPriority  = 0;
    m_dwStackSize = 0;
    m_bAloneLog   = FALSE;
    m_bCancel     = FALSE;

    if (NULL != m_pMsgMemPool)
    {
        delete m_pMsgMemPool;
        m_rCentralMemPool.Free((BYTE *)m_pMsgMemPool);
    }

    m_pLogger       = NULL;
    m_pLogMemPool   = NULL;
    m_pLogRing      = NULL;
}


/* 创建线程实例后执行初始化(在主线程栈空间执行) */
WORD32 CBaseThread::Initialize()
{
    BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CMsgMemPool));
    if (NULL == pMem)
    {
        return FAIL;
    }

    m_pMsgMemPool = new (pMem) CMsgMemPool(m_rCentralMemPool);
    m_pMsgMemPool->Initialize();

    return SUCCESS;
}


WORD32 CBaseThread::Start()
{
    TRACE_STACK("CBaseThread::Start()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_DEBUG, TRUE,
               "m_dwThreadID : %d, m_dwLogicalID : %d\n", 
               m_dwThreadID,
               m_dwLogicalID);

    WORD32 dwResult = INVALID_DWORD;

    CCpuCoreInfo *pCoreInfo = CCpuCoreInfo::GetInstance();
    T_CoreConfig *ptCore    = (*pCoreInfo)(m_dwLogicalID);
    if (NULL != ptCore)
    {
        m_dwSocketID = ptCore->dwSocketID;
        m_dwCoreID   = ptCore->dwCoreID;
    
        memcpy(&m_tCpuSet, &(ptCore->tCpuSet), sizeof(m_tCpuSet));
    }

    /* 设置线程优先级, 需root用户权限 */
    if ( (SCHED_OTHER != m_dwPolicy) 
      && (TRUE == IsRoot()))
    {
        dwResult = SchedulePriority();
        if (SUCCESS != dwResult)
        {
            return FAIL;
        }
    }
    else
    {
        if (0 != pthread_create(&m_tThreadID, NULL, CallBack, this))
        {
            return FAIL;
        }
    }

    return SUCCESS;
}


/* 必须是root用户, 否则无法设置SCHED_RR线程属性 */
WORD32 CBaseThread::SchedulePriority()
{
    TRACE_STACK("CBaseThread::SchedulePriority()");

    SWORD32             iResult = 0;
    pthread_attr_t      tAttr;
    struct sched_param  tParam;

    memset(&tParam, 0x00, sizeof(tParam));

    iResult = pthread_attr_init(&tAttr);
    if (0 != iResult)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_WARN, TRUE,
                   "pthread_attr_init : %d\n",
                   iResult);
    }

    iResult = pthread_attr_setstacksize(&tAttr, m_dwStackSize);
    if (0 != iResult)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_WARN, TRUE,
                   "pthread_attr_setstacksize : %d\n",
                   iResult);
    }

    iResult = pthread_attr_setinheritsched(&tAttr, PTHREAD_EXPLICIT_SCHED);
    if (0 != iResult)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_WARN, TRUE,
                   "pthread_attr_setinheritsched : %d\n",
                   iResult);
    }

    iResult = pthread_attr_setscope(&tAttr, PTHREAD_SCOPE_SYSTEM);
    if (0 != iResult)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_WARN, TRUE,
                   "pthread_attr_setscope : %d\n",
                   iResult);
    }

    iResult = pthread_attr_setschedpolicy(&tAttr, m_dwPolicy);
    if (0 != iResult)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_WARN, TRUE,
                   "pthread_attr_setschedpolicy : %d\n",
                   iResult);
    }

    tParam.sched_priority = m_dwPriority;

    iResult = pthread_attr_setschedparam(&tAttr, &tParam);
    if (0 != iResult)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_WARN, TRUE,
                   "pthread_attr_setschedparam : %d\n",
                   iResult);
    }

    if (0 != pthread_create(&m_tThreadID, &tAttr, CallBack, this))
    {
        assert(0);
    }

    iResult = pthread_attr_destroy(&tAttr);
    if (0 != iResult)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_WARN, TRUE,
                   "pthread_attr_destroy : %d\n",
                   iResult);
    }

    return SUCCESS;
}


SWORD32 CBaseThread::SetAffinity(pthread_t &rtThreadID, cpu_set_t &rtCpuSet)
{
    return pthread_setaffinity_np(rtThreadID, sizeof(rtCpuSet), &rtCpuSet);
}


/* 为线程设置名称, 方便ssh后台观察 */
SWORD32 CBaseThread::SetThreadName(const CHAR *pName)
{
    return pthread_setname_np(m_tThreadID, pName);
}


/* 主线程被阻塞, 监测子线程状态, 等到子线程退出后, 回收子线程资源 */
WORD32 CBaseThread::Join()
{
    return pthread_join(m_tThreadID, NULL);
}


/* 分离子线程, 子线程退出后, 子线程资源立即自动回收 */
WORD32 CBaseThread::Detach()
{
    return pthread_detach(m_tThreadID);
}


WORD32 CBaseThread::Cancel()
{
    SWORD32 sdwResult = pthread_cancel(m_tThreadID);
    if (0 != sdwResult)
    {
        return FAIL;
    }
    else
    {
        SetCancel(TRUE);
        return SUCCESS;
    }
}


CMessageRing * CBaseThread::GetMsgRingH()
{
    return NULL;
}


CMessageRing * CBaseThread::GetMsgRingL()
{
    return NULL;
}


CDataPlaneRing * CBaseThread::GetMsgRingD()
{
    return NULL;
}


CMultiMessageRing * CBaseThread::GetMultiRing()
{
    return NULL;
}


CTimerRepo * CBaseThread::GetTimerRepo()
{
    return NULL;
}


CObjMemPoolInterface * CBaseThread::CreateSTMemPool(WORD32 dwRingID)
{
    return NULL;
}


VOID CBaseThread::Dump()
{
    TRACE_STACK("CBaseThread::Dump()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "ThreadID : %3d, LogicalID : %2d, SocketID : %2d, CoreID : %2d, "
               "Policy : %d, Priority : %2d, StackSize : %d, Name : %s\n",
               m_dwThreadID,
               m_dwLogicalID,
               m_dwSocketID,
               m_dwCoreID,
               m_dwPolicy,
               m_dwPriority,
               m_dwStackSize,
               m_aucName);
}


/* 更新定时器线程+线程池线程日志文件配置 */
VOID CBaseThread::UpdateGlobalSwitch(BOOL bFlag)
{
    if (m_pLogger == g_pLogger)
    {
        return ;
    }

    m_pLogger->SetGlobalSwitch(bFlag);
}


/* 更新定时器线程+线程池线程日志文件配置 */
VOID CBaseThread::UpdatePeriod(E_LogFilePeriod ePeriod)
{
    if (m_pLogger == g_pLogger)
    {
        return ;
    }

    m_pLogger->SetPeriod(ePeriod);
    //m_pLogger->SetLogFile();
}


/* 更新定时器线程+线程池线程日志文件配置 */
VOID CBaseThread::UpdateModuleSwitch(const T_LogSetModuleSwitch &rtMsg)
{
    if (m_pLogger == g_pLogger)
    {
        return ;
    }

    BOOL bGlobalSwitch = m_pLogger->GetGlobalSwitch();

    /* 在设置模块级别的日志配置时, 先关闭日志总开关, 待模块级设置完成再恢复 */
    if (bGlobalSwitch)
    {
        m_pLogger->SetGlobalSwitch(FALSE);
    }

    m_pLogger->SetModuleMask((WORD32)(rtMsg.ucModuleID), rtMsg.bSwitch);
    m_pLogger->ClearExtModuleMask((WORD32)(rtMsg.ucModuleID));

    for (WORD32 dwIndex1 = 0; dwIndex1 < rtMsg.ucCellNum; dwIndex1++)
    {
        m_pLogger->SetExtModuleMask((WORD32)(rtMsg.ucModuleID),
                                    (WORD32)(rtMsg.awCellID[dwIndex1]),
                                    TRUE);
    }

    for (WORD32 dwIndex1 = 0; dwIndex1 < rtMsg.ucLevelNum; dwIndex1++)
    {
        m_pLogger->SetLevelMask((WORD32)(rtMsg.ucModuleID),
                                (WORD32)(rtMsg.aucLevelID[dwIndex1]),
                                TRUE);
    }

    /* 恢复日志总开关配置 */
    if (bGlobalSwitch)
    {
        m_pLogger->SetGlobalSwitch(bGlobalSwitch);
    }
}


VOID CBaseThread::GetMeasure(T_ThreadMeasure &rtMeasure)
{
    rtMeasure.dwThreadID        = m_tMeasure.dwThreadID;
    rtMeasure.wThreadID         = m_tMeasure.wThreadID;
    rtMeasure.ucThreadClass     = m_tMeasure.ucThreadClass;
    rtMeasure.ucLogicalID       = m_tMeasure.ucLogicalID;

    rtMeasure.lwLoop            = m_tMeasure.lwLoop.load(std::memory_order_relaxed);
    rtMeasure.lwProcHNum        = m_tMeasure.lwProcHNum.load(std::memory_order_relaxed);
    rtMeasure.lwProcLNum        = m_tMeasure.lwProcLNum.load(std::memory_order_relaxed);
    rtMeasure.lwProcDNum        = m_tMeasure.lwProcDNum.load(std::memory_order_relaxed);
    rtMeasure.dwMaxRemainLNum   = m_tMeasure.dwMaxRemainLNum.load(std::memory_order_relaxed);
    rtMeasure.dwMaxRemainDNum   = m_tMeasure.dwMaxRemainDNum.load(std::memory_order_relaxed);

    rtMeasure.lwTimeUsedTotalH  = m_tMeasure.lwTimeUsedTotalH.load(std::memory_order_relaxed);
    rtMeasure.lwTimeUsedTotalL  = m_tMeasure.lwTimeUsedTotalL.load(std::memory_order_relaxed);
    rtMeasure.lwTimeUsedTotalD  = m_tMeasure.lwTimeUsedTotalD.load(std::memory_order_relaxed);
    rtMeasure.lwTimeUsedTotalT  = m_tMeasure.lwTimeUsedTotalT.load(std::memory_order_relaxed);
    rtMeasure.lwTimeUsedTotalA  = m_tMeasure.lwTimeUsedTotalA.load(std::memory_order_relaxed);

    for (WORD32 dwIndex = 0; dwIndex < BIT_NUM_OF_WORD32; dwIndex++)
    {
        rtMeasure.alwStatH[dwIndex] = m_tMeasure.alwStatH[dwIndex].load(std::memory_order_relaxed);;
        rtMeasure.alwStatL[dwIndex] = m_tMeasure.alwStatL[dwIndex].load(std::memory_order_relaxed);;
        rtMeasure.alwStatD[dwIndex] = m_tMeasure.alwStatD[dwIndex].load(std::memory_order_relaxed);;
        rtMeasure.alwStatT[dwIndex] = m_tMeasure.alwStatT[dwIndex].load(std::memory_order_relaxed);;
        rtMeasure.alwStatA[dwIndex] = m_tMeasure.alwStatA[dwIndex].load(std::memory_order_relaxed);;
    }

    rtMeasure.tHPMsgQStat.lwMsgCount      = m_tMeasure.tHPMsgQStat.lwMsgCount.load(std::memory_order_relaxed);
    rtMeasure.tHPMsgQStat.dwQMaxMsgID     = m_tMeasure.tHPMsgQStat.dwQMaxMsgID.load(std::memory_order_relaxed);
    rtMeasure.tHPMsgQStat.dwMsgQMaxUsed   = m_tMeasure.tHPMsgQStat.dwMsgQMaxUsed.load(std::memory_order_relaxed);
    rtMeasure.tHPMsgQStat.lwMsgQTotalTime = m_tMeasure.tHPMsgQStat.lwMsgQTotalTime.load(std::memory_order_relaxed);

    for (WORD32 dwIndex = 0; dwIndex < BIT_NUM_OF_WORD32; dwIndex++)
    {
        rtMeasure.tHPMsgQStat.alwStat[dwIndex] = m_tMeasure.tHPMsgQStat.alwStat[dwIndex].load(std::memory_order_relaxed);
    }

    rtMeasure.tLPMsgQStat.lwMsgCount      = m_tMeasure.tLPMsgQStat.lwMsgCount.load(std::memory_order_relaxed);
    rtMeasure.tLPMsgQStat.dwQMaxMsgID     = m_tMeasure.tLPMsgQStat.dwQMaxMsgID.load(std::memory_order_relaxed);
    rtMeasure.tLPMsgQStat.dwMsgQMaxUsed   = m_tMeasure.tLPMsgQStat.dwMsgQMaxUsed.load(std::memory_order_relaxed);
    rtMeasure.tLPMsgQStat.lwMsgQTotalTime = m_tMeasure.tLPMsgQStat.lwMsgQTotalTime.load(std::memory_order_relaxed);

    for (WORD32 dwIndex = 0; dwIndex < BIT_NUM_OF_WORD32; dwIndex++)
    {
        rtMeasure.tLPMsgQStat.alwStat[dwIndex] = m_tMeasure.tLPMsgQStat.alwStat[dwIndex].load(std::memory_order_relaxed);
    }

    rtMeasure.tMultiMsgQStat.lwMsgCount      = m_tMeasure.tMultiMsgQStat.lwMsgCount.load(std::memory_order_relaxed);
    rtMeasure.tMultiMsgQStat.dwQMaxMsgID     = m_tMeasure.tMultiMsgQStat.dwQMaxMsgID.load(std::memory_order_relaxed);
    rtMeasure.tMultiMsgQStat.dwMsgQMaxUsed   = m_tMeasure.tMultiMsgQStat.dwMsgQMaxUsed.load(std::memory_order_relaxed);
    rtMeasure.tMultiMsgQStat.lwMsgQTotalTime = m_tMeasure.tMultiMsgQStat.lwMsgQTotalTime.load(std::memory_order_relaxed);

    for (WORD32 dwIndex = 0; dwIndex < BIT_NUM_OF_WORD32; dwIndex++)
    {
        rtMeasure.tMultiMsgQStat.alwStat[dwIndex] = m_tMeasure.tMultiMsgQStat.alwStat[dwIndex].load(std::memory_order_relaxed);
    }

    m_pMsgMemPool->GetMeasure(rtMeasure.tMsgMemStat);
}


/* 获取线程维测信息 */
VOID CBaseThread::GetRingMeasure(T_ThreadRingMeasure &rtMeasure)
{
    CMessageRing   *pRingH = GetMsgRingH();
    CMessageRing   *pRingL = GetMsgRingL();
    CDataPlaneRing *pRingD = GetMsgRingD();

    if (unlikely((NULL ==pRingH)
              || (NULL ==pRingL)
              || (NULL ==pRingD)))
    {
        return ;
    }

    pRingH->SnapShot(rtMeasure.tRingH.tProd, rtMeasure.tRingH.tCons);
    pRingL->SnapShot(rtMeasure.tRingL.tProd, rtMeasure.tRingL.tCons);
    pRingD->SnapShot(rtMeasure.tRingD.dwRingNum,
                     rtMeasure.tRingD.atProd,
                     rtMeasure.tRingD.atCons);
}


/* 生成线程名 */
VOID CBaseThread::GenerateThrdName(const CHAR *pThrdName)
{
    CString<WORKER_NAME_LEN> cStr(pThrdName);
    cStr += '_';
    cStr += (CHAR *)(s_aucFileName[m_dwProcID]);

    memcpy(m_aucThrdName, cStr.toChar(), cStr.Length());
}


/* 生成日志文件名 */
VOID CBaseThread::GenerateName(const CHAR *pProcName)
{
    static const CHAR * s_aucName = "_th_";

    WORD32 dwLen1 = strlen(pProcName);
    WORD32 dwLen2 = strlen(s_aucName);
    WORD32 dwLen  = dwLen1 + dwLen2;
    WORD32 dwID   = m_dwThreadID;
    WORD32 dwPos  = dwLen + 2;

    if (dwPos >= WORKER_NAME_LEN)
    {
        assert(0);
    }

    memcpy(m_aucName, pProcName, dwLen1);
    memcpy(m_aucName + dwLen1, s_aucName, dwLen2);

    while (dwPos >= dwLen)
    {
        m_aucName[dwPos] = (dwID % 10) + '0';
        dwID = dwID / 10;

        dwPos--;
    }
}


WORD32 CBaseThread::InitLogger(WORD32 dwProcID)
{
    GenerateName((CHAR *)(s_aucFileName[dwProcID]));

    m_pLogRing = InitLogRing(m_dwSelfRingID);
    if (NULL == m_pLogRing)
    {
        assert(0);
    }

    m_pLogMemPool = InitLogMemPool(m_dwSelfRingID);
    if (NULL == m_pLogMemPool)
    {
        assert(0);
    }

    if (m_bAloneLog)
    {
        m_pLogger = InitLogInfo(m_rCentralMemPool, dwProcID, (CHAR *)m_aucName);
        if (NULL == m_pLogger)
        {
            assert(0);
        }
    }
    else
    {
        m_pLogger = g_pLogger;
    }

    return SUCCESS;
}


WORD32 CBaseThread::InitMeasure()
{
    m_tMeasure.dwThreadID        = (WORD32)m_tThreadID;
    m_tMeasure.wThreadID         = (WORD16)m_dwThreadID;
    m_tMeasure.ucThreadClass     = (BYTE)GetThreadClass();
    m_tMeasure.ucLogicalID       = (BYTE)m_dwLogicalID;

    m_tMeasure.lwLoop            = 0;
    m_tMeasure.lwProcHNum        = 0;
    m_tMeasure.lwProcLNum        = 0;
    m_tMeasure.lwProcDNum        = 0;
    m_tMeasure.dwMaxRemainLNum   = 0;
    m_tMeasure.dwMaxRemainDNum   = 0;

    m_tMeasure.lwTimeUsedTotalH  = 0;
    m_tMeasure.lwTimeUsedTotalL  = 0;
    m_tMeasure.lwTimeUsedTotalD  = 0;
    m_tMeasure.lwTimeUsedTotalT  = 0;
    m_tMeasure.lwTimeUsedTotalA  = 0;

    for (WORD32 dwIndex = 0; dwIndex < BIT_NUM_OF_WORD32; dwIndex++)
    {
        m_tMeasure.alwStatH[dwIndex] = 0;
        m_tMeasure.alwStatL[dwIndex] = 0;
        m_tMeasure.alwStatD[dwIndex] = 0;
        m_tMeasure.alwStatT[dwIndex] = 0;
        m_tMeasure.alwStatA[dwIndex] = 0;
    }

    m_tMeasure.tHPMsgQStat.lwMsgCount              = 0;
    m_tMeasure.tHPMsgQStat.dwQMaxMsgID             = 0;
    m_tMeasure.tHPMsgQStat.dwMsgQMaxUsed           = 0;
    m_tMeasure.tHPMsgQStat.lwMsgQTotalTime         = 0;

    for (WORD32 dwIndex = 0; dwIndex < BIT_NUM_OF_WORD32; dwIndex++)
    {
        m_tMeasure.tHPMsgQStat.alwStat[dwIndex] = 0;
    }

    m_tMeasure.tLPMsgQStat.lwMsgCount              = 0;
    m_tMeasure.tLPMsgQStat.dwQMaxMsgID             = 0;
    m_tMeasure.tLPMsgQStat.dwMsgQMaxUsed           = 0;
    m_tMeasure.tLPMsgQStat.lwMsgQTotalTime         = 0;

    for (WORD32 dwIndex = 0; dwIndex < BIT_NUM_OF_WORD32; dwIndex++)
    {
        m_tMeasure.tLPMsgQStat.alwStat[dwIndex] = 0;
    }

    m_tMeasure.tMultiMsgQStat.lwMsgCount              = 0;
    m_tMeasure.tMultiMsgQStat.dwQMaxMsgID             = 0;
    m_tMeasure.tMultiMsgQStat.dwMsgQMaxUsed           = 0;
    m_tMeasure.tMultiMsgQStat.lwMsgQTotalTime         = 0;

    for (WORD32 dwIndex = 0; dwIndex < BIT_NUM_OF_WORD32; dwIndex++)
    {
        m_tMeasure.tMultiMsgQStat.alwStat[dwIndex] = 0;
    }

    m_tMeasure.tMsgMemStat.dwTypeNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < MSG_MEM_TYPE_NUM; dwIndex++)
    {
        m_tMeasure.tMsgMemStat.alwTotalSize[dwIndex] = 0;
        m_tMeasure.tMsgMemStat.alwUsedCount[dwIndex] = 0;
        m_tMeasure.tMsgMemStat.alwFreeCount[dwIndex] = 0;
    }

    return SUCCESS;
}


