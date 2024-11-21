

#ifndef _BASE_APP_STATE_H_
#define _BASE_APP_STATE_H_


#include "pub_event.h"

#include "base_config_file.h"
#include "base_factory_tpl.h"


typedef enum tagE_AppClass
{
    E_APP_INVALID = -1,

    E_APP_OAM     = 0,    /* COamApp */
    E_APP_TIMER,          /* CTimerApp */
    E_APP_PHY_RECV,       /* CPhyRecvApp */
    E_APP_CL,             /* CClApp */
    E_APP_SCH,            /* CSchApp */
    E_APP_RLC,            /* CRlcApp */
    E_APP_DMAC,           /* CDmacApp */
    E_APP_EGTP,           /* CEgtpApp */
    E_APP_NRUP,           /* CNrupApp */
    E_APP_SCH_UCI,
    E_APP_OAM_AGENT,
    E_APP_DUMGR,
    E_APP_DUF1AP,
    E_APP_DURRM,
    E_APP_DUUE,

    E_APP_DPDK_BB,
    E_APP_DPDK_ETH,

    E_APP_TEST,           /* 用于测试 */

    E_APP_BASE,           /* CBaseApp */
    E_APP_FT,             /* CFTApp */

    E_APP_NUM,
}E_AppClass;


typedef enum tagE_AppState
{
    APP_STATE_NULL = 0,
    APP_STATE_IDLE,         /* App空闲状态 */
    APP_STATE_WORK,         /* App工作状态 */
    APP_STATE_EXIT,         /* App退出状态 */    
}E_AppState;


class CAppInterface;


class CAppState : public CBaseData
{
public :
    static WORD32 Enter(E_AppState  eState,
                        WORD32      dwMsgID, 
                        VOID       *pIn, 
                        WORD16      wMsgLen, 
                        CAppState **pState,
                        BYTE       *pData);

    WORD32 Polling();

public :
    CAppState (CAppInterface *pAppInst);
    virtual ~CAppState();

    virtual E_AppState GetState();

    virtual WORD32 Start();
    virtual WORD32 Init();
    virtual WORD32 DeInit();
    virtual WORD32 Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen);

    CAppInterface * GetAppInst();

    VOID Dump();

protected :
    E_AppState     m_eState;
    CAppInterface *m_pAppInst;
};


inline E_AppState CAppState::GetState()
{
    return m_eState;
}


inline CAppInterface * CAppState::GetAppInst()
{
    return m_pAppInst;
}


class CAppIdleState : public CAppState
{
public :
    CAppIdleState (CAppInterface *pAppInst);
    ~CAppIdleState();

    WORD32 Init();
};


class CAppWorkState : public CAppState
{
public :
    CAppWorkState (CAppInterface *pAppInst);
    ~CAppWorkState();

    WORD32 DeInit();
};


class CAppExitState : public CAppState
{
public :
    CAppExitState (CAppInterface *pAppInst);
    ~CAppExitState();

    WORD32 Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen);
};


#define SIZEOF_APP_STATE  MAX(MAX(sizeof(CAppState), sizeof(CAppIdleState)), \
                              MAX(sizeof(CAppWorkState), sizeof(CAppExitState)))


typedef struct tagT_AppInfo
{
    CHAR             aucName[APP_NAME_LEN];
    WORD32           dwAppID;         /* 内部标识的AppID */
    WORD32           dwThreadID;      /* 归属线程ID */
    WORD32           dwStartEventID;  /* 起始消息ID, 每种App处理的消息个数为APP_CB_EVENT_NUM */
    BOOL             bAssocFlag;      /* App是否存在绑定标志(与小区的绑定关系) */
    WORD32           dwAssocNum;      /* App与关联对象之间的绑定关系 */
    WORD32           adwAssocID[MAX_ASSOCIATE_NUM_PER_APP];
    WORD32           dwMemSize;
    CAppState       *pAppState;
    BYTE             aucAppState[SIZEOF_APP_STATE];
    PCreateProduct   pCreateFunc;
    PResetProduct    pResetFunc;
    PDestroyProduct  pDestroyFunc;
    BYTE            *pMem;
}T_AppInfo;


#endif


