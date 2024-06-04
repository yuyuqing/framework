

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


#define TEST_MAX_TIMER_NUM    ((WORD32)(64))

WORD32 g_adwTimerTick[TEST_MAX_TIMER_NUM] = {
          10,  20,  30,  40,  50,  60,  70,  80,  90,
    100, 110, 120, 130, 140, 150, 160, 170, 180, 190,
    200, 210, 220, 230, 240, 250, 260, 270, 280, 290,
    300, 310, 320, 330, 340, 350, 360, 370, 380, 390,
    400, 410, 420, 430, 440, 450, 460, 470, 480, 490,
    500, 510, 520, 530, 540, 550, 560, 570, 580, 590,
    600, 610, 620, 630, 640 };

WORD32 TestTimeOut(WORD32 dwKey, T_TimerParam *ptParam)
{
    TRACE_STACK("TestTimeOut");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_DEBUG, TRUE,
               "dwKey : %d, dwID : %d, dwExtendID : %d\n",
               dwKey,
               ptParam->dwID,
               ptParam->dwExtendID);

    StartTimer(ptParam->dwExtendID,
               (PTimerCallBack)(&TestTimeOut),
               ptParam->dwID, ptParam->dwExtendID, 0, 0, NULL, NULL);

    return SUCCESS;
}


int main(int argc, char **argv)
{
    CInitList::GetInstance()->InitComponent((WORD32)E_PROC_DU);

    g_pOamApp->NotifyOamStartUP();

    WORD32 dwTimerID = INVALID_DWORD;
    WORD32 dwTick    = 0;

    for (WORD32 dwIndex = 0; dwIndex < TEST_MAX_TIMER_NUM; dwIndex++)
    {
        dwTick    = g_adwTimerTick[dwIndex];
        dwTimerID = StartTimer(dwTick,
                               (PTimerCallBack)(&TestTimeOut),
                               dwIndex, dwTick, 0, 0, NULL, NULL);
    }

    sleep(300);

    CInitList::Destroy();

    return SUCCESS;
}



