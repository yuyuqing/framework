

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
#include "dpdk_net_ip_table.h"


int main(int argc, char **argv)
{
    T_IPNetAddr  tIP0;
    T_IPNetAddr  tIP1;
    T_IPNetAddr  tIP2;
    T_IPNetAddr  tIP3;
    T_IPNetAddr  tIP4;

    tIP0.SetIPv6("2001:0102:0304:0506:0708:090A:0B0C:0D0E");
    tIP1.SetIPv6("::1");
    tIP2.SetIPv6("2001:0102:0304::1");
    tIP3.SetIPv6("2001:102:304::1");
    tIP4.SetIPv6("2001:0102::0304:1");

    CInitList::GetInstance()->InitComponent((WORD32)E_PROC_DU);

    g_pOamApp->NotifyOamStartUP();

    sleep(5);

    CInitList::Destroy();

    return SUCCESS;
}


