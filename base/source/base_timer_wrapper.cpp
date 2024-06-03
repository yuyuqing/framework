

#include "base_util.h"
#include "base_timer_wrapper.h"
#include "base_oam_app.h"
#include "base_timer_app.h"


/* ��ʱ���ӿ� : ��ʱ��ö�ʱ���Զ�ɾ��, �����ٵ���StopTimerɾ��
   dwTick     : ��ʱʱ��, ��λms
   pFunc      : �ص�����
   dwID       : �ص�����
   dwExtendID : �ص�����
   dwTransID  : �ص�����
   dwResvID   : �ص�����
   pContext   : �ص�����
   pUserData  : �ص�����
   ����ֵ     : dwTimerID, ��ʱ��ID(KillTimerʱ��Ҫ�õ�)
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


/* ֹͣ��ʱ���ӿ� 
 * dwTimerID : StartTimer�ӿڷ��صĶ�ʱ��ID
 */
WORD32 StopTimer(WORD32 dwTimerID)
{
    if (unlikely((NULL == m_pSelfThreadZone) || (NULL == g_pTimerApp)))
    {
        return FAIL;
    }

    return g_pTimerApp->RemoveTimer(dwTimerID);
}


/* ������ʱ���ӿ�, �ڵ�ǰϵͳʱ�������, ����ʱ��ʱ������dwTick(��λms) 
 * dwTimerID : StartTimer�ӿڷ��صĶ�ʱ��ID
 * dwTick    : ����ʱ��(��λms)
 */
WORD32 ResetTimer(WORD32 dwTimerID, WORD32 dwTick)
{
    if (unlikely((NULL == m_pSelfThreadZone) || (NULL == g_pTimerApp)))
    {
        return FAIL;
    }

    return g_pTimerApp->ResetTimer(dwTimerID, dwTick);
}


/* ��ʱ���̷߳���SlotInd��Ϣ */
WORD32 SendSlotIndToTimer(WORD16 wSFN, BYTE ucSlot)
{
    if (unlikely((NULL == m_pSelfThreadZone) || (NULL == g_pTimerApp)))
    {
        return FAIL;
    }

    return g_pTimerApp->NotifySlotInd(wSFN, ucSlot);
}


/* ��ʱ����NGP�ڴ��ע��(ԭ�� : ��ʱ���ص�ҵ����ʱ, ��Ҫ��NGP���ڴ���������ڴ�)
 * ͬʱ֪ͨ����App�ϵ�
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


