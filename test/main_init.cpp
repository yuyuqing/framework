

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <fstream>

#include "base_json_parse.h"


int main(int argc, char **argv)
{
    std::ifstream cFile;
    cFile.open("base.json");

    CJsonReader  *pReader = new CJsonReader();
    CJsonValue    cRoot;

    if (!pReader->Parse(cFile, cRoot))
    {
        cFile.close();
        return FAIL;
    }

    cFile.close();

    return SUCCESS;
}


