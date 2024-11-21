

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <fstream>

#include "base_list.h"
#include "base_init_component.h"
#include "base_app_cntrl.h"
#include "base_oam_app.h"


typedef struct tagT_TestVector
{
    WORD64  lwTest1;
    WORD64  lwTest2;
}T_TestVector;


int main(int argc, char **argv)
{
    std::vector<T_TestVector>    cVector1;
    std::vector<T_TestVector *>  cVector2;
    std::vector<T_TestVector *>  cVector3;

    std::vector<T_TestVector>::iterator    cIT1;
    std::vector<T_TestVector *>::iterator  cIT2;
    std::vector<T_TestVector *>::iterator  cIT3;

    T_TestVector tTest0 = {0, 0};
    T_TestVector tTest1 = {1, 1};
    T_TestVector tTest2 = {2, 2};
    T_TestVector tTest3 = {0, 3};

    cVector1.push_back(tTest0);
    cVector1.push_back(tTest1);
    cVector1.push_back(tTest2);
    cVector1.push_back(tTest3);

    cVector2.push_back(&tTest0);
    cVector2.push_back(&tTest1);
    cVector2.push_back(&tTest2);
    cVector2.push_back(&tTest3);

    cVector3 = cVector2;

    tTest3.lwTest1 = 3;

    WORD32 dwSize1 = cVector1.size();
    WORD32 dwSize2 = cVector2.size();

    cIT1 = cVector1.begin();
    cIT2 = cVector2.begin();
    cIT3 = cVector3.begin();

    for (WORD32 dwIndex = 0; dwIndex < 4; dwIndex++)
    {
        T_TestVector &rtTmp1 = *cIT1;
        T_TestVector *ptTmp2 = *cIT2;
        T_TestVector *ptTmp3 = *cIT3;

        cIT1++;
        cIT2++;
        cIT3++;
    }

    return 0;
}


