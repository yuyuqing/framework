

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <fstream>

#include "base_init_component.h"
#include "base_oam_app.h"


int main(int argc, char **argv)
{
    CInitList::GetInstance()->InitComponent((WORD32)E_PROC_DU);

    g_pOamApp->InitAllApps();

    sleep(5);

    CInitList::Destroy();

    return SUCCESS;
}


