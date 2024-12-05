

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

    T_ShmJsonCfg tParam;
    tParam.bCreateFlag  = TRUE;
    tParam.dwRole       = 2;
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

    while (TRUE)
    {
        printf("Options[ 0 : Exit, 1 : PrintShmMgr ]\n");
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
            PrintShmMgr();
            break ;

        default :
            break ;
        }
    }

    return SUCCESS;
}


