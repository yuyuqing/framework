

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


WORD32 TestRecvClient(VOID *pArg, const VOID *ptMsg, WORD32 dwLen)
{
    CShmChannel *pChannel = (CShmChannel *)pArg;

    WORD32 dwStrLen = strlen((CHAR *)(ptMsg));

    if (dwLen != (dwStrLen + 1))
    {
        assert(0);
    }

    printf("Recv : %s\n", (const CHAR *)ptMsg);

    BYTE *pBuf = pChannel->Malloc((dwLen + 1), E_SHM_MALLOC_POINT_01);
    if (NULL == pBuf)
    {
        return FAIL;
    }

    memcpy(pBuf, ptMsg, dwLen);
    pBuf[dwLen] = 0;

    pChannel->SendMessage((VOID *)pBuf);

    pChannel->Free((BYTE *)ptMsg);

    return SUCCESS;
}


int main(int argc, char **argv)
{
    T_ShmJsonCfg tParam;
    tParam.bCreateFlag  = TRUE;
    tParam.dwRole       = 0;
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

    WORD32 dwOptions = 0;
    BOOL   bRun      = TRUE;

    while (bRun)
    {
        printf("Options[ 0 : Exit, 1 : run ]\n");
        printf("Input[0/1] : ");
        std::cin >> dwOptions;

        switch (dwOptions)
        {
        case 0 :
            {
                CShmMgr::Destroy();
                g_pShmMgr = NULL;
            }
            return SUCCESS;

        case 1 :
            bRun = FALSE;
            break ;

        default :
            break ;
        }
    }

    CShmChannel *pChannel = NULL;

    while (TRUE)
    {
        for (WORD32 dwIndex = 0; dwIndex < 4; dwIndex++)
        {
            pChannel = g_pShmMgr->GetDataChannel(dwIndex);

            pChannel->Wait();
            pChannel->RecvMessage((VOID *)pChannel, (PSyncRecvMsg)(&TestRecvClient));
            pChannel->Post();
        }
    }

    sleep(1800);

    CShmMgr::Destroy();
    g_pShmMgr = NULL;

    return SUCCESS;
}


