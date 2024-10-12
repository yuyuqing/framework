

#ifndef _BASE_TIMER_WRAPPER_H_
#define _BASE_TIMER_WRAPPER_H_


/* ��C���� */
#ifdef __cplusplus
extern "C"{
#endif


#include "pub_typedef.h"
#include "pub_global_def.h"


typedef struct tagT_TimerParam
{
    WORD32   dwID;
    WORD32   dwExtendID;
    WORD32   dwTransID;
    WORD32   dwResvID;
    VOID    *pContext;
    VOID    *pUserData;
}T_TimerParam;


/* ��ʱ����ʱ�ص����� 
 * dwID       : ��ʱ��ʱ�����
 * dwExtendID : ��ʱ��ʱ�����
 * pContext   : ��ʱ��ʱ�����
 */
using PTimerCallBack = WORD32 (*)(WORD32 dwKey, T_TimerParam *ptParam);


/* ������ʱ����Ϣ */
typedef struct tagT_StartTimerMessage
{
    WORD64          lwMicroSec;  /* ����StartTimer�ӿ�ʱ�ľ���ʱ��(��λ΢��) */
    WORD32          dwTick;      /* �ȴ���ʱ���(��λ����) */
    WORD32          dwTimerID;   /* ��ʱ��ID, ��ҵ���̷߳���, ȫ��Ψһ */

    PTimerCallBack  pFunc;       /* �ص�������ַ */

    /* �������� */
    WORD32          dwID;
    WORD32          dwExtendID;
    WORD32          dwTransID;
    WORD32          dwResvID;
    VOID           *pContext;
    VOID           *pUserData;
}T_StartTimerMessage;


/* ֹͣ��ʱ����Ϣ */
typedef struct tagT_StopTimerMessage
{
    WORD32          dwTimerID;     /* ��ʱ��ID, ��ҵ���̷߳���, ȫ��Ψһ */
}T_StopTimerMessage;


/* ���ö�ʱ����Ϣ */
typedef struct tagT_ResetTimerMessage
{
    WORD64          lwMicroSec;  /* ����StartTimer�ӿ�ʱ�ľ���ʱ��(��λ΢��) */
    WORD32          dwTick;      /* �ȴ���ʱ���(��λ����) */
    WORD32          dwTimerID;   /* ��ʱ��ID, ��ҵ���̷߳���, ȫ��Ψһ */
}T_ResetTimerMessage;


typedef struct tagT_TimerSlotIndMessage
{
    WORD16          wSFN;
    BYTE            ucSlot;
    BYTE            ucResved;
    WORD32          dwResved;
}T_TimerSlotIndMessage;


/* ��ʱ����ʱ��Ϣ */
typedef struct tagT_TimeOutMessage
{
    WORD32          dwTimerID;
    WORD32          dwID;
    WORD32          dwExtendID;
    WORD32          dwTransID;
    WORD32          dwResvID;
    VOID           *pContext;
    VOID           *pUserData;
}T_TimeOutMessage;


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
                  VOID           *pUserData);


/* ͣ��ʱ���ӿ� */
WORD32 StopTimer(WORD32 dwTimerID);


/* ������ʱ���ӿ�, �ڵ�ǰϵͳʱ�������, ����ʱ��ʱ������dwTick(��λms) */
WORD32 ResetTimer(WORD32 dwTimerID, WORD32 dwTick);


/* ��ʱ���̷߳���SlotInd��Ϣ */
WORD32 SendSlotIndToTimer(WORD16 wSFN, BYTE ucSlot);


/* ��ʱ����NGP�ڴ��ע��(ԭ�� : ��ʱ���ص�ҵ����ʱ, ��Ҫ��NGP���ڴ���������ڴ�) */
WORD32 StartOam();

/* ֪ͨOAM�ӳ��˳�(�ӳ�dwTickʱ��, ��λ:ms) */
WORD32 StopOam(WORD32 dwTick);


VOID SetGlobalSFN(WORD16 wSFN, BYTE ucSlot);
VOID GetGlobalSFN(WORD16 &rwSFN, BYTE &rucSlot);


WORD32 GetCpuFreq();


#ifdef __cplusplus
    }
#endif


#endif


