

#include "base_util.h"
#include "base_timer_wrapper.h"
#include "base_oam_app.h"
#include "base_timer_app.h"


/* 起定时器接口 : 超时后该定时器自动删除, 无需再调用StopTimer删除
   dwTick     : 超时时长, 单位ms
   pFunc      : 回调函数
   dwID       : 回调出参
   dwExtendID : 回调出参
   dwTransID  : 回调出参
   dwResvID   : 回调出参
   pContext   : 回调出参
   pUserData  : 回调出参
   返回值     : dwTimerID, 定时器ID(KillTimer时需要用到)
 */
WORD32 StartTimer(WORD32          dwTick,
                  PTimerCallBack  pFunc,
                  WORD32          dwID,
                  WORD32          dwExtendID,
                  WORD32          dwTransID,
                  WORD32          dwResvID,
                  VOID           *pContext,
                  VOID           *pUserData)
{
    if (unlikely((NULL == m_pSelfThreadZone) || (NULL == g_pTimerApp)))
    {
        return INVALID_DWORD;
    }

    return g_pTimerApp->CreateTimer(dwTick,
                                    pFunc,
                                    dwID,
                                    dwExtendID,
                                    dwTransID,
                                    dwResvID,
                                    pContext,
                                    pUserData);
}


/* 停止定时器接口 
 * dwTimerID : StartTimer接口返回的定时器ID
 */
WORD32 StopTimer(WORD32 dwTimerID)
{
    if (unlikely((NULL == m_pSelfThreadZone) || (NULL == g_pTimerApp)))
    {
        return FAIL;
    }

    return g_pTimerApp->RemoveTimer(dwTimerID);
}


/* 重启定时器接口, 在当前系统时间基础上, 将定时器时长后延dwTick(单位ms) 
 * dwTimerID : StartTimer接口返回的定时器ID
 * dwTick    : 重置时长(单位ms)
 */
WORD32 ResetTimer(WORD32 dwTimerID, WORD32 dwTick)
{
    if (unlikely((NULL == m_pSelfThreadZone) || (NULL == g_pTimerApp)))
    {
        return FAIL;
    }

    return g_pTimerApp->ResetTimer(dwTimerID, dwTick);
}


/* 向定时器线程发送SlotInd消息 */
WORD32 SendSlotIndToTimer(WORD16 wSFN, BYTE ucSlot)
{
    if (unlikely((NULL == m_pSelfThreadZone) || (NULL == g_pTimerApp)))
    {
        return FAIL;
    }

    return g_pTimerApp->NotifySlotInd(wSFN, ucSlot);
}


/* 超时后向NGP内存池注册(原因 : 定时器回调业务函数时, 需要从NGP的内存池中申请内存)
 * 同时通知所有App上电
 */
WORD32 StartOam()
{
    if (NULL == g_pOamApp)
    {
        return FAIL;
    }

    g_pOamApp->NotifyOamStartUP();

    return SUCCESS;
}


VOID SetGlobalSFN(WORD16 wSFN, BYTE ucSlot)
{
    g_pGlobalClock->SetSFN(wSFN, ucSlot);
}


VOID GetGlobalSFN(WORD16 &rwSFN, BYTE &rucSlot)
{
    g_pGlobalClock->GetSFN(rwSFN, rucSlot);
}


WORD32 GetCpuFreq()
{
    return g_pGlobalClock->GetCpuFreq();
}


