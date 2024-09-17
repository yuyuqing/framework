

#ifndef _BASE_TIMER_APP_H_
#define _BASE_TIMER_APP_H_


#include "base_tree.h"
#include "base_app_interface.h"
#include "base_timer_wrapper.h"


typedef struct tagT_TimerMeasure
{
    std::atomic<WORD64>    lwStartCount;     /* ҵ����̵���������ʱ���ӿڼ��� */
    std::atomic<WORD64>    lwStartMFail;     /* ҵ����̵���������ʱ���ӿ��ڴ����ʧ�ܼ��� */
    std::atomic<WORD64>    lwStartQFail;     /* ҵ����̵���������ʱ���ӿ������ʧ�ܼ��� */
    std::atomic<WORD64>    lwStopCount;      /* ҵ����̵���ֹͣ��ʱ���ӿڼ��� */
    std::atomic<WORD64>    lwStopMFail;      /* ҵ����̵���ֹͣ��ʱ���ӿ��ڴ����ʧ�ܼ��� */
    std::atomic<WORD64>    lwStopQFail;      /* ҵ����̵���ֹͣ��ʱ���ӿ������ʧ�ܼ��� */
    std::atomic<WORD64>    lwResetCount;     /* ҵ����̵������ö�ʱ���ӿڼ��� */
    std::atomic<WORD64>    lwResetMFail;     /* ҵ����̵������ö�ʱ���ӿ��ڴ����ʧ�ܼ��� */
    std::atomic<WORD64>    lwResetQFail;     /* ҵ����̵������ö�ʱ���ӿ������ʧ�ܼ��� */
    std::atomic<WORD64>    lwSlotCount;      /* ҵ����÷���SlotInd��Ϣ�ӿڼ��� */
    std::atomic<WORD64>    lwSlotMFail;      /* ҵ����÷���SlotInd��Ϣ�ӿڷ����ڴ�ʧ�ܼ��� */
    std::atomic<WORD64>    lwSlotQFail;      /* ҵ����÷���SlotInd��Ϣ�ӿ������ʧ�ܼ��� */

    WORD64  lwStartMsgCount;                 /* ��ʱ���߳̽��յ�������ʱ����Ϣ���� */
    WORD64  lwCreateFailCount;               /* ��ʱ���߳̽��յ�������ʱ����Ϣע�ᶨʱ��ʧ�ܼ��� */
    WORD64  lwStopMsgCount;                  /* ��ʱ���߳̽��յ�ֹͣ��ʱ����Ϣ���� */
    WORD64  lwNotFindCount;                  /* ��ʱ���߳̽��յ�ֹͣ��ʱ����Ϣ���Ҷ�ʱ��ʧ�ܼ��� */
    WORD64  lwResetMsgCount;                 /* ��ʱ���߳̽��յ����ö�ʱ����Ϣ���� */
    WORD64  lwResetNotFindCount;             /* ��ʱ���߳̽��յ����ö�ʱ����Ϣ���Ҷ�ʱ��ʧ�ܼ��� */
    WORD64  lwSlotMsgCount;                  /* �յ�SlotTTI��Ϣ���� */
    WORD64  lwSlotMsgMissCount;              /* �յ�SlotTTI��Ϣ���������� */
    WORD64  lwTimeOutCount;                  /* ��ʱ����ʱ����ͳ�� */
}T_TimerMeasure;


class CTimerTreeNode : public CCBObject
{
public :
    CTimerTreeNode ();
    virtual ~CTimerTreeNode();

    WORD32 Initialize(WORD32          dwTimerID,
                      WORD32          dwKey,
                      CTimerNode     *pTimer,
                      PTimerCallBack  pFunc);

    VOID TimeOut(const VOID *pIn, WORD32 dwLen);

    WORD32 GetTimerID();

    CTimerNode * GetTimer();

protected :
    WORD32          m_dwTimerID;
    WORD32          m_dwKey;
    CTimerNode     *m_pTimer;
    PTimerCallBack  m_pCBFunc;
};


inline WORD32 CTimerTreeNode::GetTimerID()
{
    return m_dwTimerID;
}


inline CTimerNode * CTimerTreeNode::GetTimer()
{
    return m_pTimer;
}


#define TIMER_TREE_POWER_NUM        ((WORD32)(12))

typedef CBTreeTpl<CTimerTreeNode, WORD32, TIMER_TREE_POWER_NUM>  CTimerTree;


class CTimerApp : public CAppInterface
{
public :
    friend class CTimerTreeNode;

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

    /* ���������ϵͳά�� */
    VOID DumpMeasure(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpTimerMeas(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpPhyRecv(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpClApp(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpUlRecv(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpSchApp(const VOID *pIn, WORD32 dwLen);

    WORD32 InnerDelete(WORD32 dwKey);

    T_PhyRecvCellAtomicMeasure      & GetPhyRecvMeasure(WORD32 dwCellIdx);
    T_ClAppDlFapiCellAtomicMeasure  & GetClAppMeasure(WORD32 dwCellIdx);
    T_SchAppAtomicMeasure           & GetSchAppMeasure();
    T_UlRecvAtomicMeasure           & GetUlRecvMeasure();

protected :
    WORD32 InnerCreate(WORD32          dwKey,
                       WORD64          lwMicroSec,
                       WORD32          dwTick,
                       PTimerCallBack  pFunc,
                       WORD32          dwID,
                       WORD32          dwExtendID,
                       WORD32          dwTransID,
                       WORD32          dwResvID,
                       VOID           *pContext,
                       VOID           *pUserData);

protected :
    CTimerTree                  m_cTree;
    CB_RegistMemPool            m_pRegistMemPoolFunc;  /* ��NGP�ڴ��ע���߳���Ϣ */

    WORD64                      m_lwSlotCount;
    WORD16                      m_wSFN;
    BYTE                        m_ucSlot;

    BYTE                        m_ucMeasMinute;  /* ά�ⶨʱ��ʱ��(��λ:����) */

    T_TimerMeasure              m_tMeas;

    T_PhyRecvAtomicMeasure      m_tPhyRecvMeasure;
    T_ClAppDlFapiAtomicMeasure  m_tClAppMeasure;
    T_SchAppAtomicMeasure       m_tSchAppMeasure;
    T_UlRecvAtomicMeasure       m_tUlRecvMeasure;
};


inline T_PhyRecvCellAtomicMeasure & CTimerApp::GetPhyRecvMeasure(WORD32 dwCellIdx)
{
    return m_tPhyRecvMeasure.atCell[dwCellIdx];
}


inline T_ClAppDlFapiCellAtomicMeasure & CTimerApp::GetClAppMeasure(WORD32 dwCellIdx)
{
    return m_tClAppMeasure.atCell[dwCellIdx];
}


inline T_SchAppAtomicMeasure & CTimerApp::GetSchAppMeasure()
{
    return m_tSchAppMeasure;
}


inline T_UlRecvAtomicMeasure & CTimerApp::GetUlRecvMeasure()
{
    return m_tUlRecvMeasure;
}


#endif


