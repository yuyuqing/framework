

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <iostream>

#include "pub_global_def.h"

#include "base_init_component.h"
#include "base_thread_pool.h"
#include "base_shm_mgr.h"


#define TEST_MEM_SIZE    ((WORD32)(4 * 1024 * 1024))

BYTE             *g_pTestMem     = NULL;
CCentralMemPool  *g_pTestMemPool = NULL;


VOID CreateShmMgr()
{
    if (NULL != g_pTestMem)
    {
        return ;
    }

    g_pTestMem = (BYTE *)(malloc(TEST_MEM_SIZE));

    WORD64  lwSize     = TEST_MEM_SIZE;
    WORD64  lwAlign    = CACHE_SIZE;
    WORD64  lwAddr     = (WORD64)g_pTestMem;
    WORD64  lwBegin    = ROUND_UP(lwAddr, lwAlign);
    WORD64  lwPoolSize = ROUND_UP(sizeof(CCentralMemPool), CACHE_SIZE);
    BYTE   *pMem       = (BYTE *)(lwBegin);
    VOID   *pOriAddr   = (VOID *)(lwBegin + lwPoolSize);

    lwSize = lwSize - (lwBegin - lwAddr) - lwPoolSize;

    new (pMem) CCentralMemPool();
    g_pTestMemPool = (CCentralMemPool *)pMem;
    g_pTestMemPool->Initialize(pOriAddr, lwSize);

    WORD32  dwResult = INVALID_DWORD;
    BYTE   *pShmMem  = g_pTestMemPool->Malloc(sizeof(CShmMgr));

    assert(NULL != pShmMem);
    assert(NULL == g_pShmMgr);

    g_pShmMgr = CShmMgr::GetInstance(pShmMem);
    dwResult  = g_pShmMgr->Initialize(FALSE, 4, 14, g_pTestMemPool);

    assert(SUCCESS == dwResult);
}


VOID DeleteShmMgr()
{
    if (NULL != g_pShmMgr)
    {
        CShmMgr::Destroy();
        g_pShmMgr = NULL;
    }

    if (NULL != g_pTestMemPool)
    {
        delete g_pTestMemPool;
    }

    if (NULL != g_pTestMem)
    {
        free(g_pTestMem);
        g_pTestMem = NULL;
    }
}


VOID PrintDataZone(CDataZone &rDataZone)
{
    rDataZone.Printf();
}


VOID PrintAppCntrl(WORD64 lwAppCntrlAddr)
{
    CAppCntrl *pCntrl = (CAppCntrl *)lwAppCntrlAddr;
    pCntrl->Printf();
}


VOID PrintThreadCntrl(WORD64 lwThreadCntrlAddr)
{
    CThreadCntrl *pCntrl = (CThreadCntrl *)lwThreadCntrlAddr;
    pCntrl->Printf();
}


VOID PrintShmMgr()
{
    g_pShmMgr->Printf();
}


int main(int argc, char **argv)
{
    pthread_t  tThreadID;
    cpu_set_t  tCpuSet;

    memset(&tThreadID, 0x00, sizeof(tThreadID));
    CPU_ZERO(&tCpuSet);

    /* ¹Ì¶¨°ó¶¨ºË1 */
    CPU_SET(1, &tCpuSet);
    tThreadID = pthread_self();
    pthread_setaffinity_np(tThreadID, sizeof(cpu_set_t), &tCpuSet);

    CMemMgr *pMemMgr = CMemMgr::CreateMemMgr((WORD32)E_PROC_DU,
                                             (BYTE)E_MEM_HUGEPAGE_TYPE,
                                             1,
                                             1024 * 1024 * 1024,
                                             "/dev/hugepages",
                                             FALSE);

    T_MemMetaHead   *pMetaHead       = pMemMgr->GetMetaHead();
    CDataZone       *pDataZone       = pMemMgr->GetDataZone();
    CCentralMemPool *pCentralMemPool = pMemMgr->GetCentralMemPool();

    WORD64  lwMetaAddr     = pMetaHead->lwMetaAddr;
    WORD64  lwHugeAddr     = pMetaHead->lwHugeAddr;
    WORD64  lwDataZoneAddr = (WORD64)pDataZone;
    WORD64  lwCtrlMemPool  = (WORD64)pCentralMemPool;
    WORD64  lwMemPools     = pMetaHead->lwMemPools;
    WORD64  lwAppCntrlAddr = pMetaHead->lwAppCntrlAddr;
    WORD64  lwThdCntrlAddr = pMetaHead->lwThreadCntrlAddr;

    printf("lwMagic : %lu, dwVersion : %u, dwHeadSize : %u, "
           "lwMasterLock : %lu, iGlobalLock : %d, bInitFlag : %d, "
           "iMLock : %d, dwHugeNum : %d, iPrimaryFileID : %d, "
           "iSecondaryFileID : %d, lwHugeAddr : %lu, aucHugePath : %s, "
           "lwMetaAddr : %lu, lwMetaSize : %lu, lwHugeAddr : %lu, "
           "lwShareMemSize : %lu, lwAppCntrlAddr : %lu, "
           "lwThreadCntrlAddr : %lu, lwMemPools : %lu\n",
           pMetaHead->lwMagic,
           pMetaHead->dwVersion,
           pMetaHead->dwHeadSize,
           pMetaHead->lwMasterLock,
           pMetaHead->iGlobalLock,
           pMetaHead->bInitFlag,
           pMetaHead->iMLock,
           pMetaHead->dwHugeNum,
           pMetaHead->atHugeInfo[0].iPrimaryFileID,
           pMetaHead->atHugeInfo[0].iSecondaryFileID,
           pMetaHead->atHugeInfo[0].lwHugeAddr,
           pMetaHead->atHugeInfo[0].aucHugePath,
           pMetaHead->lwMetaAddr,
           pMetaHead->lwMetaSize,
           pMetaHead->lwHugeAddr,
           pMetaHead->lwShareMemSize,
           pMetaHead->lwAppCntrlAddr,
           pMetaHead->lwThreadCntrlAddr,
           pMetaHead->lwMemPools);

    printf("lwMetaAddr     = %lu\n", lwMetaAddr);
    printf("lwHugeAddr     = %lu\n", lwHugeAddr);
    printf("lwDataZoneAddr = %lu\n", lwDataZoneAddr);
    printf("lwCtrlMemPool  = %lu\n", lwCtrlMemPool);
    printf("lwMemPools     = %lu\n", lwMemPools);
    printf("lwAppCntrlAddr = %lu\n", lwAppCntrlAddr);
    printf("lwThdCntrlAddr = %lu\n", lwThdCntrlAddr);

    CreateShmMgr();

    WORD32 dwOptions = 0;

    while (TRUE)
    {
        printf("Options[ 0 : Exit, 1 : PrintDataZone, 2 : PrintApps, "
               "3 : PrintThreads, 4 : PrintShmMgr ]\n");
        printf("Input[0/1/2/3] : ");
        std::cin >> dwOptions;

        switch (dwOptions)
        {
        case 0 :
            DeleteShmMgr();
            return SUCCESS;

        case 1 : 
            PrintDataZone(*pDataZone);
            break ;

        case 2 : 
            PrintAppCntrl(pMetaHead->lwAppCntrlAddr);
            break ;

        case 3 : 
            PrintThreadCntrl(pMetaHead->lwThreadCntrlAddr);
            break ;

        case 4 :
            PrintShmMgr();
            break ;

        default :
            break ;
        }
    }

    return SUCCESS;
}


