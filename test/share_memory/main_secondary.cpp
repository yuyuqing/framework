

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


VOID PrintThreadCntrl(WORD64 lwThreadCntrlAddr)
{
    CThreadCntrl *pCntrl = (CThreadCntrl *)lwThreadCntrlAddr;
    pCntrl->Printf();
}


VOID PrintAppCntrl(WORD64 lwAppCntrlAddr)
{
    CAppCntrl *pCntrl = (CAppCntrl *)lwAppCntrlAddr;
    pCntrl->Printf();
}


VOID PrintDataZone(CDataZone &rDataZone)
{
    rDataZone.Printf();
}


int main(int argc, char **argv)
{
    CMemMgr *pMemMgr = CMemMgr::CreateMemMgr((WORD32)E_PROC_DU,
                                             (BYTE)E_MEM_HUGEPAGE_TYPE,
                                             1,
                                             1024 * 1024 * 1024,
                                             "/dev/hugepages",
                                             FALSE);

    T_MemMetaHead   *pMetaHead       = pMemMgr->GetMetaHead();
    CDataZone       *pDataZone       = pMemMgr->GetDataZone();
    CCentralMemPool *pCentralMemPool = pMemMgr->GetCentralMemPool();

    printf("pMetaHead = %lu, pMemMgr = %lu, pDataZone = %lu, pCentralMemPool = %lu\n",
           (WORD64)pMetaHead,
           (WORD64)pMemMgr,
           (WORD64)pDataZone,
           (WORD64)pCentralMemPool);

    printf("lwMagic : %lu, dwVersion : %u, dwHeadSize : %u, "
           "lwMasterLock : %lu, iGlobalLock : %d, bInitFlag : %d, "
           "iMLock : %d, dwHugeNum : %d, iPrimaryFileID : %d, "
           "iSecondaryFileID : %d, lwHugeAddr : %lu, aucHugePath : %s, "
           "lwMetaAddr : %lu, lwMetaSize : %lu, lwHugeAddr : %lu, "
           "lwShareMemSize : %lu, lwAppCntrlAddr : %lu, lwThreadCntrlAddr : %lu\n",
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
           pMetaHead->lwThreadCntrlAddr);

    WORD64 lwAddr = 0;

    while (TRUE)
    {
        std::cin >> lwAddr;

        PrintThreadCntrl(pMetaHead->lwThreadCntrlAddr);
        PrintAppCntrl(pMetaHead->lwAppCntrlAddr);
        PrintDataZone(*pDataZone);
    }

    return SUCCESS;
}


