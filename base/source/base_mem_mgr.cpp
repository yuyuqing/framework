

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "base_sort.h"
#include "base_mem_mgr.h"
#include "base_log.h"


CMemMgr * CMemMgr::s_pInstance = NULL;
BOOL      CMemMgr::s_bMaster   = TRUE;


static struct flock g_tWriteLock = {
    .l_type   = F_WRLCK,
    .l_whence = SEEK_SET,
    .l_start  = offsetof(T_MemMetaHead, lwMasterLock),
    .l_len    = sizeof(WORD64),
};


CDataZone::CDataZone ()
{
    m_dwZoneNum = 0;
}


CDataZone::~CDataZone()
{
}


WORD32 CDataZone::Initialize()
{
    m_cList.Initialize();

    for (WORD32 dwIndex = 0; dwIndex < DATA_ZONE_NUM; dwIndex++)
    {
        T_DataZone &rtData = (T_DataZone &)(m_cList[dwIndex]);

        memset(&rtData, 0x00, sizeof(rtData));

        rtData.dwThreadIdx    = INVALID_DWORD;
        rtData.dwTimerInnerID = 0;
        rtData.dwThreadType   = INVALID_DWORD;
        rtData.dwThreadID     = 0;
        rtData.pLogMemPool    = NULL;
        rtData.pLogRing       = NULL;
        rtData.pLogger        = NULL;
        rtData.pThread        = NULL;
    }

    return SUCCESS;
}


/* 注册线程专属数据区(支持被重复调用, 初始化日志线程和定时器线程后均会回调) */
WORD32 CDataZone::RegistZone(WORD32         dwThreadType, 
                             CB_RegistZone  pFunc, 
                             VOID          *pThread)
{
    WORD32      dwThreadIdx = INVALID_DWORD;
    WORD32      dwResult    = INVALID_DWORD;
    T_DataZone *ptData      = NULL;

    if (NULL == m_pSelfThreadZone)
    {
        dwThreadIdx = m_dwZoneNum.fetch_add(1, std::memory_order_relaxed);
        if (dwThreadIdx >= DATA_ZONE_NUM)
        {
            assert(0);
        }

        ptData = (T_DataZone *)(m_cList[dwThreadIdx]);
        if (NULL == ptData)
        {
            assert(0);
        }

        ptData->dwThreadIdx    = dwThreadIdx;
        ptData->dwTimerInnerID = 0;
        ptData->dwThreadType   = dwThreadType;
        ptData->dwThreadID     = (WORD32)gettid();
        ptData->pLogMemPool    = NULL;
        ptData->pLogRing       = NULL;
        ptData->pLogger        = NULL;
        ptData->pThread        = NULL;
        ptData->tThreadID      = pthread_self();

        pthread_getname_np(ptData->tThreadID, ptData->aucName, THREAD_NAME_LEN);

        m_pSelfThreadZone      = ptData;
        m_dwSelfRingID         = ptData->dwThreadIdx;
        m_dwSelfTimerInnerID   = ptData->dwTimerInnerID;
        m_dwSelfThreadType     = ptData->dwThreadType;
        m_dwSelfThreadID       = ptData->dwThreadID;
    }
    else
    {
        dwThreadIdx = m_pSelfThreadZone->dwThreadIdx;
        ptData      = m_pSelfThreadZone;
    }

    if (unlikely(NULL != pFunc))
    {
        /* 初始化线程专属数据区 */
        dwResult = (*pFunc) (*ptData, pThread);
        if (SUCCESS != dwResult)
        {
            assert(0);
        }
    }

    return dwThreadIdx;
}


/* 去注册线程专属数据区 */
WORD32 CDataZone::RemoveZone(WORD32         dwThreadIdx, 
                             CB_RemoveZone  pFunc,
                             VOID          *pThread)
{
    if (unlikely(NULL == pFunc))
    {
        assert(0);
    }

    if (dwThreadIdx >= DATA_ZONE_NUM)
    {
        assert(0);
    }

    T_DataZone *ptData = (T_DataZone *)(m_cList[dwThreadIdx]);
    if (NULL == ptData)
    {
        assert(0);
    }

    if (dwThreadIdx != ptData->dwThreadIdx)
    {
        assert(0);
    }

    /* 释放线程专属数据区 */
    WORD32 dwResult = (*pFunc) (*ptData, pThread);
    if (SUCCESS != dwResult)
    {
        assert(0);
    }

    ptData->dwThreadIdx    = INVALID_DWORD;
    ptData->dwTimerInnerID = 0;
    ptData->dwThreadType   = INVALID_DWORD;
    ptData->pLogMemPool    = NULL;
    ptData->pLogRing       = NULL;
    ptData->pLogger        = NULL;
    ptData->pThread        = NULL;

    m_pSelfThreadZone = NULL;

    return SUCCESS;
}


VOID CDataZone::Dump()
{
    TRACE_STACK("CDataZone::Dump()");

    WORD32 dwZoneNum = m_dwZoneNum.load(std::memory_order_relaxed);

    for (WORD32 dwIndex = 0; dwIndex < dwZoneNum; dwIndex++)
    {
        T_DataZone *ptData = (T_DataZone *)(m_cList[dwIndex]);
        if (NULL == ptData)
        {
            continue ;
        }

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
                   "Index : %3d, ThrdIdx : %3d, TimerInnerID : %d, "
                   "ThrdType : %2d, ThrdID : %d, LogMemPool : %15lu, LogRing : %15lu, "
                   "Logger : %15lu, Thread : %15lu, TThrdID : %ld, Name : %s, \n",
                   dwIndex,
                   ptData->dwThreadIdx,
                   ptData->dwTimerInnerID,
                   ptData->dwThreadType,
                   ptData->dwThreadID,
                   (WORD64)(ptData->pLogMemPool),
                   (WORD64)(ptData->pLogRing),
                   (WORD64)(ptData->pLogger),
                   (WORD64)(ptData->pThread),
                   (WORD64)(ptData->tThreadID),
                   ptData->aucName);
    }
}


CMemMgr * CMemMgr::CreateMemMgr(WORD32      dwProcID,
                                BYTE        ucMemType,
                                BYTE        ucPageNum,
                                WORD32      dwMemSize,
                                const CHAR *pDir,
                                BOOL        bMaster)
{
    if (NULL != s_pInstance)
    {
        return s_pInstance;
    }

    s_bMaster = bMaster;

    ucPageNum = MIN(ucPageNum, PAGE_NUM);

    if (ucMemType >= E_MEM_HEAP_TYPE)
    {
        ucMemType  = E_MEM_HEAP_TYPE;
    }
    else
    {
        ucMemType  = E_MEM_HUGEPAGE_TYPE;
    }

    T_MemMetaHead *ptHead = CreateMetaMemory(dwProcID,
                                             ucMemType,
                                             ucPageNum,
                                             dwMemSize,
                                             pDir,
                                             bMaster);
    if (NULL == ptHead)
    {
        assert(0);
    }

    if (s_bMaster)
    {
        s_pInstance = new ((BYTE *)(ptHead->lwHugeAddr)) CMemMgr();
        s_pInstance->Initialize(ptHead,
                                ptHead->lwHugeAddr,
                                ptHead->lwShareMemSize);
    }
    else
    {
        s_pInstance = (CMemMgr *)(ptHead->lwHugeAddr);
    }

    return s_pInstance;
}


CMemMgr * CMemMgr::GetInstance()
{
    return s_pInstance;
}


VOID CMemMgr::Destroy()
{
    T_MemMetaHead *ptHead = NULL;

    if (NULL != s_pInstance)
    {
        ptHead = s_pInstance->m_pMetaHead;

        delete s_pInstance;

        if (NULL != ptHead)
        {
            WORD32 dwMetaSize = (WORD32)(ptHead->lwMetaSize);

            CleanHuge(*ptHead);
            munmap(ptHead, dwMetaSize);
        }

        s_pInstance = NULL;
    }
}


VOID CMemMgr::LockGlobal(T_MemMetaHead &rtHead)
{
    SWORD32 iValue = 0;

    while (!__atomic_compare_exchange_n(&(rtHead.iGlobalLock), 
                                        &iValue,
                                        1, 0,
                                        __ATOMIC_ACQUIRE,
                                        __ATOMIC_RELAXED))
    {
        while (__atomic_load_n(&(rtHead.iGlobalLock), __ATOMIC_RELAXED))
        {
    #ifdef ARCH_ARM64
            asm volatile("yield" ::: "memory");
    #else
            _mm_pause();
    #endif
        }

        iValue = 0;
    }
}


VOID CMemMgr::UnLockGlobal(T_MemMetaHead &rtHead)
{
    __atomic_store_n(&(rtHead.iGlobalLock), 0, __ATOMIC_RELEASE);
}


VOID * CMemMgr::GetVirtualArea(VOID *pAddr, WORD32 dwSize)
{
    if (NULL == pAddr)
    {
        return (VOID *)(s_lwVirBassAddr);
    }

    WORD64 lwNextAddr = ((WORD64)pAddr) + dwSize;

    return (VOID *)(ROUND_UP(lwNextAddr, s_lwAlign1G));
}


T_MemMetaHead * CMemMgr::CreateMetaMemory(WORD32      dwProcID,
                                          BYTE        ucMemType,
                                          BYTE        ucPageNum,
                                          WORD32      dwMemSize,
                                          const CHAR *pDir,
                                          BOOL        bMaster)
{
    WORD32         dwResult   = INVALID_DWORD;
    WORD32         dwStartPos = ROUND_UP(sizeof(T_MemMetaHead), s_lwAlign2M);
    WORD32         dwMetaSize = 0;
    SWORD32        iFileID    = -1;
    WORD64         lwMemSize  = 0;
    T_MemMetaHead *ptMetaHead = NULL;

    if (E_MEM_HUGEPAGE_TYPE == ucMemType)
    {
        /* 大页内存单独开辟内存空间, 不占用MemMeta元内存区空间 */
        lwMemSize  = ucPageNum;
        lwMemSize  = lwMemSize * HUGEPAGE_FILE_SIZE;

        dwMetaSize = dwStartPos;
    }
    else
    {
        /* Heap内存不单独开辟内存空间, 直接占用MemMeta元内存区空间 */
        lwMemSize  = dwMemSize;                       /* 单位MB */
        lwMemSize  = lwMemSize * 1024 * 1024;         /* 单位Byte */
        lwMemSize  = ROUND_UP(lwMemSize, s_lwAlign2M);

        if ((lwMemSize + dwStartPos) >= s_lwMaxSize)
        {
            return NULL;
        }

        dwMetaSize = (WORD32)(lwMemSize + dwStartPos);
    }

    if (bMaster)
    {
        ptMetaHead = Create(dwMetaSize, iFileID);
    }
    else
    {
        ptMetaHead = Attach(dwMetaSize, iFileID);
    }

    if (NULL == ptMetaHead)
    {
        return NULL;
    }

    dwResult = InitContext(bMaster,
                           dwMetaSize,
                           lwMemSize,
                           ucMemType,
                           ucPageNum,
                           dwProcID,
                           pDir,
                           ptMetaHead);
    if (SUCCESS != dwResult)
    {
        return NULL;
    }

    return ReAttach(ptMetaHead, dwMetaSize, iFileID);
}


T_MemMetaHead * CMemMgr::Create(WORD32 dwSize, SWORD32 &rdwFileID)
{
    SWORD32        iFileID    = rdwFileID;
    SWORD32        iResult    = -1;
    VOID          *pMemAddr   = NULL;
    T_MemMetaHead *ptMetaHead = NULL;

    if (iFileID < 0)
    {
        iFileID = open(MEM_META_ZONE_PATH, O_RDWR | O_CREAT, 0600);
        if (iFileID < 0)
        {
            return NULL;
        }
    }

    iResult = ftruncate(iFileID, dwSize);
    if (iResult < 0)
    {
        close(iFileID);
        return NULL;
    }

    /* 判断是否有多个主进程打开共享内存 */
    iResult = fcntl(iFileID, F_SETLK, &g_tWriteLock);
    if (iResult < 0)
    {
        close(iFileID);
        return NULL;
    }

    pMemAddr = GetVirtualArea(NULL, dwSize);
    if (NULL == pMemAddr)
    {
        close(iFileID);
        return NULL;
    }

    ptMetaHead = (T_MemMetaHead *)mmap(pMemAddr,
                                       dwSize,
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED | MAP_FIXED,
                                       iFileID,
                                       0);
    if (MAP_FAILED == ptMetaHead)
    {
        close(iFileID);
        return NULL;
    }

    rdwFileID = iFileID;

    return ptMetaHead;
}


T_MemMetaHead * CMemMgr::Attach(WORD32 dwSize, SWORD32 &rdwFileID)
{
    SWORD32  iFileID = rdwFileID;

    if (iFileID < 0)
    {
        iFileID = open(MEM_META_ZONE_PATH, O_RDWR);
        if (iFileID < 0)
        {
            return NULL;
        }
    }

    T_MemMetaHead *ptMetaHead = (T_MemMetaHead *)mmap(NULL,
                                                      dwSize,
                                                      PROT_READ | PROT_WRITE,
                                                      MAP_SHARED,
                                                      iFileID,
                                                      0);
    if (MAP_FAILED == ptMetaHead)
    {
        close(iFileID);
        return NULL;
    }

    rdwFileID = iFileID;

    return ptMetaHead;
}


T_MemMetaHead * CMemMgr::ReAttach(T_MemMetaHead *ptHead,
                                  WORD32         dwSize,
                                  SWORD32        iFileID)
{
    if ((NULL == ptHead) || (iFileID < 0))
    {
        return NULL;
    }

    VOID *pMemAddr = (VOID *)(ptHead->lwMetaAddr);

    munmap(ptHead, dwSize);

    ptHead = (T_MemMetaHead *)mmap(pMemAddr,
                                   dwSize,
                                   PROT_READ | PROT_WRITE,
                                   MAP_SHARED,
                                   iFileID,
                                   0);
    close(iFileID);

    if ( (MAP_FAILED == ptHead)
      || (pMemAddr != ptHead))
    {
        if (MAP_FAILED != ptHead)
        {
            munmap(ptHead, dwSize);
        }

        return NULL;
    }

    return ptHead;
}


WORD32 CMemMgr::InitContext(BOOL           bMaster,
                            WORD32         dwMetaSize,
                            WORD64         lwMemSize,
                            BYTE           ucMemType,
                            BYTE           ucPageNum,
                            WORD32         dwProcID,
                            const CHAR    *pDir,
                            T_MemMetaHead *ptHead)
{
    WORD32 dwResult    = INVALID_DWORD;
    WORD32 dwWaitCount = 0;

    if (bMaster)
    {
        ptHead->lwMagic        = s_lwMagicValue;
        ptHead->dwVersion      = s_dwVersion;
        ptHead->dwHeadSize     = sizeof(T_MemMetaHead);
        ptHead->lwMasterLock   = 0;
        ptHead->iGlobalLock    = 0;
        ptHead->bInitFlag      = FALSE;
        ptHead->iMLock         = 0;
        ptHead->dwHugeNum      = 0;
        ptHead->lwMetaAddr     = 0;
        ptHead->lwMetaSize     = dwMetaSize;
        ptHead->lwHugeAddr     = 0;
        ptHead->lwShareMemSize = lwMemSize;

        LockGlobal(*ptHead);

        /* 对共享内存做初始化... */
        if (E_MEM_HUGEPAGE_TYPE == ucMemType)
        {
            dwResult = InitHugePage(ptHead, pDir, dwProcID, ucPageNum);
        }
        else
        {
            ptHead->lwHugeAddr = ((WORD64)ptHead) + s_lwAlign2M;
            dwResult = SUCCESS;
        }

        if (SUCCESS != dwResult)
        {
            UnLockGlobal(*ptHead);
            return FAIL;
        }

        ptHead->lwMetaAddr = (WORD64)(ptHead);
        ptHead->bInitFlag  = TRUE;

        UnLockGlobal(*ptHead);
    }
    else
    {
        do
        {
            if ( (s_lwMagicValue        != ptHead->lwMagic)
              || (s_dwVersion           != ptHead->dwVersion)
              || (sizeof(T_MemMetaHead) != ptHead->dwHeadSize))
            {
                sleep(1);

                dwWaitCount++;
                if (dwWaitCount >= MEM_META_SLAVE_INIT_WAIT)
                {
                    return FAIL;
                }
            }
            else
            {
                LockGlobal(*ptHead);

                if (TRUE == ptHead->bInitFlag)
                {
                    /* 对共享内存做绑定... */
                    if (E_MEM_HUGEPAGE_TYPE == ucMemType)
                    {
                        dwResult = AttachHugePage(ptHead, ucPageNum);
                    }
                    else
                    {
                        /* 执行空操作, 主进程会将虚拟地址内容填充好 */
                        dwResult = SUCCESS;
                    }

                    UnLockGlobal(*ptHead);

                    if (SUCCESS != dwResult)
                    {
                        return FAIL;
                    }

                    break ;
                }
                else
                {
                    UnLockGlobal(*ptHead);
                    usleep(100);
                }
            }
        } while(TRUE);
    }

    return SUCCESS;
}


WORD32 CMemMgr::CheckHugePageDir(const CHAR *pDir)
{
    WORD32      dwResult   = INVALID_DWORD;
    const CHAR *pHugetlbfs = "hugetlbfs";
    CHAR        aucBuf[HUGE_MOUNTS_LINE_LEN]  = {0,};

    CString<HUGE_MOUNTS_LINE_LEN> cPath(pDir);
    if (0 == cPath.Length())
    {
        return FAIL;
    }

    FILE *pFile = fopen(HUGE_MOUNTS_DIR, "r");
    if (NULL == pFile)
    {
        return FAIL;
    }

    while (fgets(aucBuf, HUGE_MOUNTS_LINE_LEN, pFile))
    {
        CString<HUGE_MOUNTS_LINE_LEN> cStr(aucBuf);

        WORD32 dwPos = cStr.Find(pHugetlbfs);
        if (dwPos > HUGE_MOUNTS_LINE_LEN)
        {
            memset(aucBuf, 0x00, sizeof(aucBuf));
            continue ;
        }

        dwPos = cStr.Find(cPath.toByte());
        if (dwPos > HUGE_MOUNTS_LINE_LEN)
        {
            memset(aucBuf, 0x00, sizeof(aucBuf));
            continue ;
        }

        dwResult = SUCCESS;
        break ;
    }

    fclose(pFile);

    return dwResult;
}


WORD32 CMemMgr::GetHugePageDir(WORD32      dwProcID,
                               WORD32      dwPos,
                               const CHAR *pBaseDir,
                               CHAR       *pDstDir,
                               WORD32      dwLen)
{
    const CHAR *pFileName  = "/huge_data_";

    CString<HUGE_MOUNTS_LINE_LEN> cDir(pBaseDir);

    cDir += pFileName;
    cDir += s_aucFileName[dwProcID];
    cDir += '_';
    cDir += (CHAR)(dwPos+ '0');

    cDir.Copy((BYTE *)pDstDir, 0, cDir.Length());

    return SUCCESS;
}


WORD32 CMemMgr::InitHugePage(T_MemMetaHead *ptHead,
                             const CHAR    *pDir,
                             WORD32         dwProcID,
                             BYTE           ucPageNum)
{
    WORD32 dwResult = INVALID_DWORD;

    if (0 == ucPageNum)
    {
        return FAIL;
    }

    dwResult = CheckHugePageDir(pDir);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    for (WORD32 dwIndex = 0; dwIndex < ucPageNum; dwIndex++)
    {
        dwResult = CreateHugePage(ptHead, pDir, dwProcID, dwIndex);
        if (SUCCESS != dwResult)
        {
            return FAIL;
        }
    }

    ptHead->lwHugeAddr = ptHead->atHugeInfo[0].lwHugeAddr;

    return SUCCESS;
}


WORD32 CMemMgr::AttachHugePage(T_MemMetaHead *ptHead, WORD16 wPageNum)
{
    WORD32 dwResult = INVALID_DWORD;

    if (0 == wPageNum)
    {
        return FAIL;
    }

    for (WORD32 dwIndex = 0; dwIndex < wPageNum; dwIndex++)
    {
        dwResult = LinkHugePage(ptHead, dwIndex);
        if (SUCCESS != dwResult)
        {
            return FAIL;
        }
    }

    return SUCCESS;
}


WORD32 CMemMgr::CreateHugePage(T_MemMetaHead *ptHead,
                               const CHAR    *pDir,
                               WORD32         dwProcID,
                               WORD32         dwPos)
{
    if (dwPos != ptHead->dwHugeNum)
    {
        return FAIL;
    }

    CHAR  aucHugeDir[HUGE_PATH_LEN] = {0, };
    VOID *pLastAddr = (0 == dwPos) ? 
        ((VOID *)ptHead) : ((VOID *)(ptHead->atHugeInfo[dwPos - 1].lwHugeAddr));

    VOID   *pAddr    = GetVirtualArea((VOID *)pLastAddr, sizeof(T_MemMetaHead));
    VOID   *pVirAddr = NULL;
    WORD32  dwResult = GetHugePageDir(dwProcID,
                                      dwPos,
                                      pDir,
                                      aucHugeDir,
                                      sizeof(aucHugeDir));
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    struct stat tStat;

    SWORD32 iRet = stat(aucHugeDir, &tStat);
    if ((iRet < 0) && (errno != ENOENT))
    {
        return FAIL;
    }

    if (0 == iRet)
    {
        unlink(aucHugeDir);
    }

    SWORD32 iFileID = open(aucHugeDir, O_CREAT | O_RDWR, 0600);
    if (iFileID < 0)
    {
        return FAIL;
    }

    if (ftruncate(iFileID, s_lwAlign1G) < 0)
    {
        close(iFileID);
        unlink(aucHugeDir);
        return FAIL;
    }

    pVirAddr = mmap(pAddr,
                    s_lwAlign1G,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_POPULATE | MAP_FIXED,
                    iFileID,
                    0);
    if ((MAP_FAILED == pVirAddr) || (pAddr != pVirAddr))
    {
        close(iFileID);
        unlink(aucHugeDir);
        return FAIL;
    }

    /* 不改变共享内存的值, 仅触发缺页中断 */
    *(volatile int *)pAddr = *(volatile int *)pAddr;

    ptHead->atHugeInfo[dwPos].iPrimaryFileID   = iFileID;
    ptHead->atHugeInfo[dwPos].iSecondaryFileID = -1;
    ptHead->atHugeInfo[dwPos].lwHugeAddr       = (WORD64)pAddr;

    memcpy(ptHead->atHugeInfo[dwPos].aucHugePath, aucHugeDir, HUGE_PATH_LEN);

    ptHead->dwHugeNum++;

    return SUCCESS;
}


WORD32 CMemMgr::LinkHugePage(T_MemMetaHead *ptHead, WORD32 dwPos)
{
    CHAR  aucHugeDir[HUGE_PATH_LEN] = {0, };
    VOID *pAddr    = (VOID *)(ptHead->atHugeInfo[dwPos].lwHugeAddr);
    VOID *pVirAddr = NULL;

    memcpy(aucHugeDir, ptHead->atHugeInfo[dwPos].aucHugePath, HUGE_PATH_LEN);

    struct stat tStat;

    SWORD32 iRet = stat(aucHugeDir, &tStat);
    if ((iRet < 0) && (errno != ENOENT))
    {
        return FAIL;
    }

    SWORD32 iFileID = open(aucHugeDir, O_RDWR, 0600);
    if (iFileID < 0)
    {
        return FAIL;
    }

    pVirAddr = mmap(pAddr,
                    s_lwAlign1G,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_POPULATE | MAP_FIXED,
                    iFileID,
                    0);
    if ((MAP_FAILED == pVirAddr) || (pAddr != pVirAddr))
    {
        close(iFileID);
        return FAIL;
    }

    ptHead->atHugeInfo[dwPos].iSecondaryFileID = iFileID;

    return SUCCESS;
}


WORD32 CMemMgr::CleanHuge(T_MemMetaHead &rtHead)
{
    WORD32 dwHugeNum = rtHead.dwHugeNum;

    for (WORD32 dwIndex = 0; dwIndex < dwHugeNum; dwIndex++)
    {
        T_MemMetaHuge &rtHuge = rtHead.atHugeInfo[dwIndex];
        munmap((VOID *)(rtHuge.lwHugeAddr), s_lwAlign1G);

        if (s_bMaster)
        {
            close(rtHuge.iPrimaryFileID);
        }
        else
        {
            close(rtHuge.iSecondaryFileID);
        }
    }

    return SUCCESS;
}


CMemMgr::CMemMgr ()
{
    m_pMetaHead       = NULL;
    m_lwBaseAddr      = 0;
    m_lwMemSize       = 0;
    m_lwDataZoneAddr  = 0;
    m_lwCentralAddr   = 0;
    m_pDataZone       = NULL;
    m_pCentralMemPool = NULL;
}


CMemMgr::~CMemMgr()
{
    if (NULL != m_pDataZone)
    {
        delete m_pDataZone;
    }

    if (NULL != m_pCentralMemPool)
    {
        delete m_pCentralMemPool;
    }
}


WORD32 CMemMgr::Initialize(T_MemMetaHead *ptHead,
                           WORD64         lwBaseAddr,
                           WORD64         lwMemSize)
{
    m_pMetaHead  = ptHead;
    m_lwBaseAddr = lwBaseAddr;
    m_lwMemSize  = lwMemSize;

    WORD64  lwAlign   = CACHE_SIZE;
    WORD64  lwSrcAddr = m_lwBaseAddr + sizeof(CMemMgr);
    WORD64  lwDstAddr = ROUND_UP(lwSrcAddr, lwAlign);

    m_lwDataZoneAddr = lwDstAddr;
    lwSrcAddr        = lwDstAddr + s_dwDataZoneSize;
    m_lwCentralAddr  = ROUND_UP(lwSrcAddr, lwAlign);

    WORD32 dwResult = InitDataZone();
    if (SUCCESS != dwResult)
    {
        assert(0);
    }

    dwResult = InitCentralMemPool();
    if (SUCCESS != dwResult)
    {
        assert(0);
    }

    return SUCCESS;
}


VOID CMemMgr::Dump()
{
    TRACE_STACK("CMemMgr::Dump()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "m_pMetaHead = %lu, m_lwBaseAddr = %lu, m_lwMemSize = %lu, "
               "m_lwDataZoneAddr = %lu, m_lwCentralAddr = %lu, "
               "m_pDataZone = %lu, m_pCentralMemPool = %lu\n",
               (WORD64)m_pMetaHead,
               m_lwBaseAddr,
               m_lwMemSize,
               m_lwDataZoneAddr,
               m_lwCentralAddr,
               (WORD64)m_pDataZone,
               (WORD64)m_pCentralMemPool);

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "lwMagic : %lu, dwVersion : %u, dwHeadSize : %u, "
               "lwMasterLock : %lu, iGlobalLock : %d, bInitFlag : %d, "
               "iMLock : %d, dwHugeNum : %d, iPrimaryFileID : %d, "
               "iSecondaryFileID : %d, lwHugeAddr : %lu, aucHugePath : %s, "
               "lwMetaAddr : %lu, lwMetaSize : %lu, lwHugeAddr : %lu, "
               "lwShareMemSize : %lu\n",
               m_pMetaHead->lwMagic,
               m_pMetaHead->dwVersion,
               m_pMetaHead->dwHeadSize,
               m_pMetaHead->lwMasterLock,
               m_pMetaHead->iGlobalLock,
               m_pMetaHead->bInitFlag,
               m_pMetaHead->iMLock,
               m_pMetaHead->dwHugeNum,
               m_pMetaHead->atHugeInfo[0].iPrimaryFileID,
               m_pMetaHead->atHugeInfo[0].iSecondaryFileID,
               m_pMetaHead->atHugeInfo[0].lwHugeAddr,
               m_pMetaHead->atHugeInfo[0].aucHugePath,
               m_pMetaHead->lwMetaAddr,
               m_pMetaHead->lwMetaSize,
               m_pMetaHead->lwHugeAddr,
               m_pMetaHead->lwShareMemSize);

    m_pDataZone->Dump();
    m_pCentralMemPool->Dump();
}


WORD32 CMemMgr::InitDataZone()
{
    BYTE *pMem = (BYTE *)(m_lwDataZoneAddr);

    new (pMem) CDataZone();
    m_pDataZone = (CDataZone *)pMem;

    return m_pDataZone->Initialize();
}


WORD32 CMemMgr::InitCentralMemPool()
{
    WORD64  lwSize   = 0;
    VOID   *pOriAddr = (VOID *)(m_lwCentralAddr + s_dwMemPoolSize);
    BYTE   *pMem     = (BYTE *)(m_lwCentralAddr);

    lwSize = m_lwMemSize - ((WORD64)s_dwMemPoolSize) - (m_lwCentralAddr - m_lwBaseAddr);

    new (pMem) CCentralMemPool();
    m_pCentralMemPool = (CCentralMemPool *)pMem;

    return m_pCentralMemPool->Initialize(pOriAddr, lwSize);
}


