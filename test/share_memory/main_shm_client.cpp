

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
#include "base_shm_mgr.h"


using namespace std;


WORD32 TestRecvServer(VOID *pArg, const VOID *ptMsg, WORD32 dwLen)
{
    CChannelTpl *pChannel = (CChannelTpl *)pArg;

    WORD32 dwStrLen = strlen((CHAR *)(ptMsg));

    if (dwLen != (dwStrLen + 2))
    {
        assert(0);
    }

    printf("Recv : %s\n", (const CHAR *)ptMsg);

    pChannel->Free((BYTE *)ptMsg);

    return SUCCESS;
}


int main(int argc, char **argv)
{
    CMemMgr *pMemMgr = CMemMgr::CreateMemMgr((WORD32)E_PROC_DU,
                                             (BYTE)E_MEM_HUGEPAGE_TYPE,
                                             1,
                                             1024 * 1024 * 1024,
                                             "/dev/hugepages",
                                             TRUE);

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
           "lwShareMemSize : %lu\n",
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
           pMetaHead->lwShareMemSize);

    WORD32 *pdwValue = (WORD32 *)(pCentralMemPool->Malloc(sizeof(WORD32)));
    *pdwValue = 123456789;

    printf("pdwValue address is %lu\n", (WORD64)pdwValue);

    BYTE   *pMem     = pCentralMemPool->Malloc(sizeof(CShmMgr));
    WORD32  dwResult = INVALID_DWORD;

    assert(NULL != pMem);

    g_pShmMgr = CShmMgr::GetInstance(pMem);
    dwResult  = g_pShmMgr->Initialize(FALSE, 4, 14, pCentralMemPool);

    assert(SUCCESS == dwResult);

    CHAR         aucBuf[1024] = {0,};
    WORD32       dwLen        = 0;
    CChannelTpl *pChannel     = NULL;

    while (TRUE)
    {
        memset(aucBuf, 0x00, sizeof(aucBuf));

        printf("Client : ");
        cin >> aucBuf;

        dwLen = strlen(aucBuf);

        for (WORD32 dwIndex = 0; dwIndex <= 4; dwIndex++)
        {
            pChannel = g_pShmMgr->GetChannel(dwIndex);

            BYTE *pBuf = pChannel->Malloc((dwLen + 1), E_SHM_MALLOC_POINT_01);
            if (NULL == pBuf)
            {
                return FAIL;
            }

            memcpy(pBuf, aucBuf, dwLen);
            pBuf[dwLen] = 0;

            pChannel->SendMessage((VOID *)pBuf);

            pChannel->Post();
        }

        for (WORD32 dwIndex = 0; dwIndex <= 4; dwIndex++)
        {
            pChannel = g_pShmMgr->GetChannel(dwIndex);

            pChannel->Wait();
            pChannel->RecvMessage((VOID *)pChannel, (PSyncRecvMsg)(&TestRecvServer));
        }
    }

    sleep(1800);

    return SUCCESS;
}



