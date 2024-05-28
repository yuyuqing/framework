

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>

#include "pub_global_def.h"

#include "base_bitmap.h"
#include "base_init_component.h"
#include "base_thread_pool.h"
#include "base_timer_wrapper.h"

#include "dpdk_adapt_init.h"


int main(int argc, char **argv)
{
    CInitList::GetInstance()->InitComponent((WORD32)E_PROC_DU);

    TimeOutRegistMemPool(0, NULL);

    sleep(1800);

    CInitList::Destroy();

    return SUCCESS;
}


