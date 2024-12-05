

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
    CShmChannel *pChannel = (CShmChannel *)pArg;

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
    T_ShmJsonCfg tParam;
    tParam.bCreateFlag  = TRUE;
    tParam.dwRole       = 1;
    tParam.dwChannelNum = 4;
    tParam.atChannel[0].dwSendNodeNum  = 200;
    tParam.atChannel[0].dwSendNodeSize = 2048;
    tParam.atChannel[0].dwRecvNodeNum  = 200;
    tParam.atChannel[0].dwRecvNodeSize = 2048;
    tParam.atChannel[1].dwSendNodeNum  = 200;
    tParam.atChannel[1].dwSendNodeSize = 2048;
    tParam.atChannel[1].dwRecvNodeNum  = 200;
    tParam.atChannel[1].dwRecvNodeSize = 2048;
    tParam.atChannel[2].dwSendNodeNum  = 200;
    tParam.atChannel[2].dwSendNodeSize = 2048;
    tParam.atChannel[2].dwRecvNodeNum  = 200;
    tParam.atChannel[2].dwRecvNodeSize = 2048;
    tParam.atChannel[3].dwSendNodeNum  = 200;
    tParam.atChannel[3].dwSendNodeSize = 2048;
    tParam.atChannel[3].dwRecvNodeNum  = 200;
    tParam.atChannel[3].dwRecvNodeSize = 2048;
    tParam.tCtrlChannel.dwSendNodeNum  = 100;
    tParam.tCtrlChannel.dwSendNodeSize = 4096;
    tParam.tCtrlChannel.dwRecvNodeNum  = 100;
    tParam.tCtrlChannel.dwRecvNodeSize = 4096;
    tParam.tOamChannel.dwSendNodeNum   = 1000;
    tParam.tOamChannel.dwSendNodeSize  = 1024;
    tParam.tOamChannel.dwRecvNodeNum   = 2000;
    tParam.tOamChannel.dwRecvNodeSize  = 4096;

    g_pShmMgr = CShmMgr::CreateShmMgr((WORD32)E_PROC_CU, tParam);

    CShmChannel *pChannel     = NULL;
    CHAR         aucBuf[1024] = {0,};
    WORD32       dwLen        = 0;

    while (TRUE)
    {
        memset(aucBuf, 0x00, sizeof(aucBuf));

        printf("Client : ");
        cin >> aucBuf;

        dwLen = strlen(aucBuf);

        for (WORD32 dwIndex = 0; dwIndex < 4; dwIndex++)
        {
            pChannel = g_pShmMgr->GetDataChannel(dwIndex);

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

        for (WORD32 dwIndex = 0; dwIndex < 4; dwIndex++)
        {
            pChannel = g_pShmMgr->GetDataChannel(dwIndex);

            pChannel->Wait();
            pChannel->RecvMessage((VOID *)pChannel, (PSyncRecvMsg)(&TestRecvServer));
        }
    }

    sleep(1800);

    return SUCCESS;
}


