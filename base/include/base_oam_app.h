

#ifndef _BASE_THREAD_LOG_APP_H_
#define _BASE_THREAD_LOG_APP_H_


#include "base_tree_array.h"
#include "base_app_interface.h"


typedef enum tagE_OamTaskType
{
    E_OAM_TASK_SYNC_ID = 0,
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


class COamApp : public CAppInterface
{
public :
    enum { TIMER_SYNC_INTERVAL    = 5000 };
    enum { TIMER_TIMEOUT_INTERVAL =    5 };

public :
    COamApp ();
    virtual ~COamApp();

    WORD32 InitApp();

    WORD32 Start();
    WORD32 Init();
    WORD32 DeInit();
    WORD32 Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen);

    /* ֪ͨ����APP�ϵ� */
    WORD32 InitAllApps();

    VOID ProcGlobalSwitchMsg(const VOID *pIn, WORD32 dwLen);

    VOID ProcWritePeriodMsg(const VOID *pIn, WORD32 dwLen);

    VOID ProcModuleSwitchMsg(const VOID *pIn, WORD32 dwLen);

    /* ����ע�ᶨʱ�ص���������Ϣ */
    VOID ProcCBRegistMsg(const VOID *pIn, WORD32 dwLen);

    /* ����ȥע�ᶨʱ�ص���������Ϣ */
    VOID ProcCBRemoveMsg(const VOID *pIn, WORD32 dwLen);

    VOID CallBack(const VOID *pIn, WORD32 dwLen);

    /* ������Ӧ�û��߳��ṩ��Ϣ�ӿ�, ����֧��ע�ᶨʱ�ص� */
    WORD32 SendRegistCBMsg(WORD32     dwTaskID,
                           WORD32     dwPeriod,
                           CCBObject *pObj,
                           PCBFUNC    pFunc,
                           VOID      *pUsrData);

    /* ������Ӧ�û��߳��ṩ��Ϣ�ӿ�, ����֧��ȥע�ᶨʱ�ص� */
    WORD32 SendRemoveCBMsg(WORD32 dwTaskID);

    VOID SyncClock(const VOID *pIn, WORD32 dwLen);
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


