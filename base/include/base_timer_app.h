

#ifndef _BASE_TIMER_APP_H_
#define _BASE_TIMER_APP_H_


#include "base_tree_array.h"
#include "base_app_interface.h"
#include "base_timer_wrapper.h"


class CTimerApp : public CAppInterface
{
public :
    CTimerApp ();
    virtual ~CTimerApp();

    WORD32 InitApp();

    WORD32 Start();
    WORD32 Init();
    WORD32 DeInit();
    WORD32 Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen);

    /* ����������ʱ����Ϣ */
    VOID ProcStartTimer(const VOID *pIn, WORD32 dwLen);

    /* ����ֹͣ��ʱ����Ϣ */
    VOID ProcStopTimer(const VOID *pIn, WORD32 dwLen);

    /* �������ö�ʱ����Ϣ */
    VOID ProcResetTimer(const VOID *pIn, WORD32 dwLen);

    /* ����SlotInd��Ϣ */
    VOID ProcSlotInd(const VOID *pIn, WORD32 dwLen);

    /* ��CTimerApp����������ʱ����Ϣ */
    WORD32 CreateTimer(WORD32          dwTick,
                       PTimerCallBack  pFunc,
                       WORD32          dwID,
                       WORD32          dwExtendID,
                       WORD32          dwTransID,
                       WORD32          dwResvID,
                       VOID           *pContext,
                       VOID           *pUserData);

    /* ��CTimerApp����ֹͣ��ʱ����Ϣ */
    WORD32 RemoveTimer(WORD32 dwTimerID);

    /* ��CTimerApp�������ö�ʱ����Ϣ */
    WORD32 ResetTimer(WORD32 dwTimerID, WORD32 dwTick);

    /* ��CTimerApp����SlotTti��Ϣ */
    WORD32 NotifySlotInd(WORD16 wSFN, BYTE ucSlot);

protected :
};


#endif


