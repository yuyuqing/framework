

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

    /* 处理启动定时器消息 */
    VOID ProcStartTimer(const VOID *pIn, WORD32 dwLen);

    /* 处理停止定时器消息 */
    VOID ProcStopTimer(const VOID *pIn, WORD32 dwLen);

    /* 处理重置定时器消息 */
    VOID ProcResetTimer(const VOID *pIn, WORD32 dwLen);

    /* 处理SlotInd消息 */
    VOID ProcSlotInd(const VOID *pIn, WORD32 dwLen);

    /* 向CTimerApp发送启动定时器消息 */
    WORD32 CreateTimer(WORD32          dwTick,
                       PTimerCallBack  pFunc,
                       WORD32          dwID,
                       WORD32          dwExtendID,
                       WORD32          dwTransID,
                       WORD32          dwResvID,
                       VOID           *pContext,
                       VOID           *pUserData);

    /* 向CTimerApp发送停止定时器消息 */
    WORD32 RemoveTimer(WORD32 dwTimerID);

    /* 向CTimerApp发送重置定时器消息 */
    WORD32 ResetTimer(WORD32 dwTimerID, WORD32 dwTick);

    /* 向CTimerApp发送SlotTti消息 */
    WORD32 NotifySlotInd(WORD16 wSFN, BYTE ucSlot);

protected :
};


#endif


