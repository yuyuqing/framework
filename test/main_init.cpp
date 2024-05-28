

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <fstream>

#include "base_config_file.h"


int main(int argc, char **argv)
{
    CBaseConfigFile *pConfig = CBaseConfigFile::GetInstance();

    pConfig->ParseFile();

    CBaseConfigFile::Destroy();

    return SUCCESS;
}


