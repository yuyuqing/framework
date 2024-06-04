

#ifndef _BASE_OAM_APP_H_
#define _BASE_OAM_APP_H_


#include "base_tree_array.h"
#include "base_app_interface.h"


typedef enum tagE_OamTaskType
{
    E_OAM_TASK_SYNC_ID = 0,
    E_OAM_MEASURE_ID,
    E_DPDK_MEASURE_ID,
}E_OamTaskType;


typedef struct tagT_OamRegistCallBack
{
    WORD32     dwTaskID;    /* ע������ID(E_LogTaskTypeö��, ȡֵΨһ) */
    WORD32     dwPeriod;    /* ����ʱ����(��λ:ms) */
    CCBObject *pObjAddr;    /* CObject�����ַ */
    PCBFUNC    pFuncAddr;   /* PCBFUNC������ַ */
    WORD64     lwUsrData;   /* �ص�������� */
}T_OamRegistCallBack;


typedef struct tagT_OamRemoveCallBack
{
    WORD32     dwTaskID;    /* ע������ID(E_LogTaskTypeö��, ȡֵΨһ) */
}T_OamRemoveCallBack;


#define OAM_CB_TABLE_NODE_POWER_NUM        ((WORD32)(10))


class COamCBNode
{
public :
    COamCBNode();
    virtual ~COamCBNode();

    WORD32 Initialize(WORD32     dwInstID,
                      WORD32     dwTaskID,
                      CCBObject *pObj,
                      PCBFUNC    pFunc,
                      WORD64     lwUserData);

public :
    WORD32      m_dwInstID;    /* ��CB���е����� */
    WORD32      m_dwTaskID;    /* ����ID, CB������� */
    CCBObject  *m_pObj;        /* ���������Գ�ʱ�ص������ַ */
    PCBFUNC     m_pFunc;       /* ���������Գ�ʱ�ص�������ַ */
    WORD64      m_lwUserData;  /* ���������Գ�ʱ�ص����� */
    WORD32      m_dwTimerID;   /* ���涨ʱ��ID */
};


typedef CBTreeArray<COamCBNode, WORD32, OAM_CB_TABLE_NODE_POWER_NUM>  COamCBTable;


#define DISABLE_SYNC_FLAG       ((BYTE)(0))
#define ENABLE_SYNC_FLAG        ((BYTE)(1))


class COamApp : public CAppInterface
{
public :
    enum { SYNC_PERIOD            = 30000 };  /* 30�� = 30000ms */
    enum { TIMER_SYNC_INTERVAL    =  5000 };
    enum { TIMER_TIMEOUT_INTERVAL =    20 };
    enum { DELAY_INIT_APPS_TICK   =     5 };

public :
    COamApp ();
    virtual ~COamApp();

    WORD32 InitApp();

    WORD32 Start();
    WORD32 Init();
    WORD32 DeInit();
    WORD32 Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen);

    /* ֪ͨCOamApp�ϵ�, ����5ms��ʱ��(��ʱ��֪ͨ����APP�ϵ�), ��main�����е��� */
    WORD32 NotifyOamStartUP();

    /* �ӳ�֪ͨ����APP�ϵ�(��COamApp�ϵ������5ms��ʱ��) */
    VOID TimeOutStartUpAllApps(const VOID *pIn, WORD32 dwLen);

    VOID ProcGlobalSwitchMsg(const VOID *pIn, WORD32 dwLen);

    VOID ProcWritePeriodMsg(const VOID *pIn, WORD32 dwLen);

    VOID ProcModuleSwitchMsg(const VOID *pIn, WORD32 dwLen);

    /* ����ע�ᶨʱ�ص���������Ϣ */
    VOID ProcCBRegistMsg(const VOID *pIn, WORD32 dwLen);

    /* ����ȥע�ᶨʱ�ص���������Ϣ */
    VOID ProcCBRemoveMsg(const VOID *pIn, WORD32 dwLen);

    /* ���ע�ᶨʱ�ص�����, ������ִ�лص� */
    VOID CallBack(const VOID *pIn, WORD32 dwLen);

    /* ������Ӧ�û��߳��ṩ��Ϣ�ӿ�, ����֧��ע�ᶨʱ�ص� */
    WORD32 SendRegistCBMsg(WORD32     dwTaskID,
                           WORD32     dwPeriod,
                           CCBObject *pObj,
                           PCBFUNC    pFunc,
                           VOID      *pUsrData);

    /* ������Ӧ�û��߳��ṩ��Ϣ�ӿ�, ����֧��ȥע�ᶨʱ�ص� */
    WORD32 SendRemoveCBMsg(WORD32 dwTaskID);

    /* ����������ִ����ʱ��ͬ�������SYNC��Ϣ */
    VOID SyncClock(const VOID *pIn, WORD32 dwLen);

    /* �������������ϵͳά�������MEAS��Ϣ */
    VOID DumpMeasure(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpThreadMeas(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpAppMeas(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpMsgQueueMeas(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpMsgMemPoolMeas(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutDumpMemPoolsMeas(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutMemMgrDump(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutLogMeas(const VOID *pIn, WORD32 dwLen);
    VOID TimeOutLogFlush(const VOID *pIn, WORD32 dwLen);

    VOID TimeOutSwitch(const VOID *pIn, WORD32 dwLen);

protected :
    WORD32 CalcTick(WORD64 lwSeconds, WORD64 lwMicroSec, WORD32 dwPeriod);

    /* �����л���־�ļ���ʱ���, ���������Զ�ʱ�� */
    WORD32 DoLogTask(WORD64 lwSeconds,
                     WORD64 lwMicroSec,
                     WORD64 lwCycle,
                     WORD32 dwSwitchPrd);

    /* �л���־�ļ� */
    WORD32 SwitchLogFile(BYTE ucPos);

protected :
    COamCBTable           m_cCBTable;      /* OAM����������� */

    BYTE                  m_ucPos;         /* ���ֺ�g_pLogger�������ļ�Posһ�� */
    BYTE                  m_ucMeasMinute;  /* ά�ⶨʱ��ʱ��(��λ:����) */
    WORD16                m_wSwitchPrd;    /* ��־�ļ��л�����(��λ:����) */

    BYTE                  m_ucSyncFlag;    /* �Ƿ�����ϵͳʱ�ӱ�־ */
    BYTE                  m_ucThrdNum;
    BYTE                  m_ucAppNum;

    /* �̼߳���ά����Ϣ */    
    CBaseThread          *m_apThread[MAX_WORKER_NUM];
    T_ThreadMeasure       m_atThrdMeasure[MAX_WORKER_NUM];
    T_ThreadRingMeasure   m_atThreadRingMeasure[MAX_WORKER_NUM];

    /* APP����ά����Ϣ */    
    CAppInterface        *m_apApp[MAX_APP_NUM];
    T_AppMeasure          m_atAppMeasure[MAX_APP_NUM];

    /* Block/Trun MemPools�ڴ��ά����Ϣ */    
    T_MemMeasure          m_tMemPoolMeasure;
};


#endif


