

#include <string.h>

#include "base_sort.h"
#include "base_variable.h"
#include "base_init_component.h"
#include "base_thread_pool.h"


WORD32 CInitList::CreateFunc(const T_InitFunc *ptFunc)
{
    CInitList *pList = CInitList::GetInstance();

    const T_InitFunc *pFunc = pList->Create(ptFunc);
    if (NULL == pFunc)
    {
        return FAIL;
    }
    
    return SUCCESS;
}


CInitList::CInitList ()
{
    m_pRegistMemPoolFunc = NULL;

    m_dwNum = 0;
    memset(m_atFunc, 0x00, sizeof(m_atFunc));

    m_pMemMgr    = NULL;
    m_pClock     = NULL;
    m_pMemPools  = NULL;
}


CInitList::~CInitList ()
{
    /* 退出所有业务线程 */
    if (NULL != g_pThreadPool)
    {
        g_pThreadPool->JoinThreads();
    }

    /* 先退出所有线程, 再回收资源, 避免日志线程访问野指针 */
    if (NULL != g_pLogThread)
    {
        g_pLogThread->Cancel();
        g_pLogThread->Join();
    }

    for (WORD32 dwIndex = m_dwNum; dwIndex > 0; dwIndex--)
    {
        T_InitFunc *ptFunc = &(m_atFunc[dwIndex-1]);

        if (NULL != ptFunc->pExitFunc)
        {
            (*(ptFunc->pExitFunc)) ((VOID *)ptFunc);
        }
    }

    /* 销毁APP实例, 回收AppInterface相关实例及其持有的内存数据区 */
    CAppCntrl::Destroy();

    m_pRegistMemPoolFunc = NULL;
    m_dwNum = 0;
    memset(m_atFunc, 0x00, sizeof(m_atFunc));

    DestroyGlobalData();
}


const T_InitFunc * CInitList::Create(const T_InitFunc *ptFunc)
{
    if ((NULL == ptFunc) || (m_dwNum >= MAX_INIT_FUNC_NUM))
    {
        return NULL;
    }

    T_InitFunc *pInitFunc = &(m_atFunc[m_dwNum]);

    m_dwNum++;

    *pInitFunc = *(const_cast<T_InitFunc *> (ptFunc));

    return pInitFunc;
}


const T_InitFunc * CInitList::Find(CHAR * pName)
{
    if (NULL == pName)
    {
        return NULL;
    }

    WORD32 dwLen = strlen(pName);

    for (WORD32 dwIndex = 0; dwIndex < m_dwNum; dwIndex++)
    {
        T_InitFunc *ptFunc = &(m_atFunc[dwIndex]);

        if (strlen(ptFunc->pName) == dwLen)
        {
            if (0 == memcmp(pName, ptFunc->pName, dwLen))
            {
                return ptFunc;
            }
        }
    }

    return NULL;
}


WORD32 CInitList::InitComponent(WORD32 dwProcID, CB_RegistMemPool pFunc)
{
    if (dwProcID >= E_PROC_NUM)
    {
        assert(0);
    }

    m_pRegistMemPoolFunc = pFunc;

    WORD32 dwResult = InitGlobalData(dwProcID);
    if (SUCCESS != dwResult)
    {
        assert(0);
    }

    CCentralMemPool *pCentralMemPool = m_pMemMgr->GetCentralMemPool();

    BYTE *pMem = pCentralMemPool->Malloc(sizeof(CAppCntrl));
    if (NULL == pMem)
    {
        assert(0);
    }

    CAppCntrl *pAppCntrl = CAppCntrl::GetInstance(pMem);
    pAppCntrl->Initialize(pCentralMemPool);

    /* 从小到大排序 */
    HeapSort<T_InitFunc, WORD32> (m_atFunc, m_dwNum,
                                  (&T_InitFunc::dwLevel),
                                  (PComparePFunc<T_InitFunc, WORD32>)(& GreaterV<T_InitFunc, WORD32>));

    T_InitFunc *ptFunc   = NULL;

    for (WORD32 dwIndex = 0; dwIndex < m_dwNum; dwIndex++)
    {
        m_atFunc[dwIndex].pMemInterface = pCentralMemPool;

        ptFunc = &(m_atFunc[dwIndex]);

        dwResult = (*(ptFunc->pInitFunc)) (dwProcID, (VOID *)ptFunc);
        if (SUCCESS != dwResult)
        {
            return FAIL;
        }
    }

    usleep(1000);

    Dump();

    return SUCCESS;
}


VOID CInitList::Dump()
{
    TRACE_STACK("CInitList::Dump()");

    CCpuCoreInfo::GetInstance()->Dump();
    g_pAppCntrl->Dump();

    if (NULL != m_pMemMgr)
    {
        m_pMemMgr->Dump();
    }

    if (NULL != m_pMemPools)
    {
        m_pMemPools->Dump();
    }
}


WORD32 CInitList::InitGlobalData(WORD32 dwProcID)
{
    /* 初始化系统核资源 */
    CCpuCoreInfo::GetInstance()->Initialize();

    /* 解析base.json配置文件 */
    CBaseConfigFile *pConfig = CBaseConfigFile::GetInstance();
    pConfig->ParseFile();

    T_MemJsonCfg &rtMemCfg = pConfig->GetMemJsonCfg();

    m_pMemMgr = CMemMgr::CreateMemMgr(dwProcID,
                                      rtMemCfg.ucMemType,
                                      rtMemCfg.ucPageNum,
                                      rtMemCfg.dwMemSize,
                                      rtMemCfg.aucHugeDir,
                                      TRUE);

    CCentralMemPool *pCentralMemPool = m_pMemMgr->GetCentralMemPool();

    BYTE *pMem = pCentralMemPool->Malloc(sizeof(CGlobalClock));
    if (NULL == pMem)
    {
        return FAIL;
    }

    /* 创建并初始化全局变量g_pGlobalClock */
    m_pClock = CGlobalClock::GetInstance(pMem);
    if (NULL != m_pClock)
    {
        g_pGlobalClock = m_pClock;
    }
    else
    {
        return FAIL;
    }

    /* 初始化Block/Trunk MemPools内存池 */
    m_pMemPools = CMemPools::GetInstance(pCentralMemPool);

    /* 注册线程专属数据区(主线程) */
    CDataZone *pDataZone = m_pMemMgr->GetDataZone();
    g_dwMainThreadIdx = pDataZone->RegistZone((WORD32)INVALID_THREAD_TYPE, NULL, NULL);

    return SUCCESS;
}


WORD32 CInitList::DestroyGlobalData()
{
    CDataZone *pDataZone = m_pMemMgr->GetDataZone();
    pDataZone->RemoveZone(g_dwMainThreadIdx,
                          (CB_RemoveZone)(&RemoveThreadZone),
                          NULL);

    CCentralMemPool *pCentralMemPool = m_pMemMgr->GetCentralMemPool();

    if (NULL != m_pMemPools)
    {
        CMemPools::Destroy();
        m_pMemPools = NULL;
    }

    /* 销毁全局数据g_pGlobalClock */
    if (NULL != m_pClock)
    {
        CGlobalClock::Destroy();
        pCentralMemPool->Free((BYTE *)m_pClock);
        m_pClock       = NULL;
        g_pGlobalClock = NULL;
    }

    CMemMgr::Destroy();
    m_pMemMgr = NULL;

    CBaseConfigFile::Destroy();
    CCpuCoreInfo::Destroy();
    CFactoryApp::Destroy();
    CFactoryThread::Destroy();

    return SUCCESS;
}


