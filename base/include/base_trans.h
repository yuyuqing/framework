

#ifndef _BASE_TRANS_H_
#define _BASE_TRANS_H_


#include <assert.h>

#include "base_app_interface.h"


typedef struct tagT_TransStepValue
{
    BYTE  aucStep[QUATRA_CACHE_SIZE];
}T_TransStepValue;


typedef enum tagE_TransStepState
{
    E_TRANS_STEP_INVALID = 0,
    E_TRANS_STEP_INIT,
    E_TRANS_STEP_WAIT,
    E_TRANS_STEP_RECVED,
    E_TRANS_STEP_COMPLETE,
}E_TransStepState;


typedef enum tagE_TransState
{
    E_TRANS_INVALID = 0,
    E_TRANS_INIT,
    E_TRANS_WAIT,
    E_TRANS_COMPLETE,
    E_TRANS_FAIL,
}E_TransState;


#define MAX_TRANS_STEP_NUM    ((WORD16)(16))


class CBaseTransStep;


class CBaseTrans : public CBaseData, public CCBObject
{
public :
    friend class CBaseTransStep;

    typedef CBaseDataContainer<T_TransStepValue, MAX_TRANS_STEP_NUM, FALSE> CTransStepList;

public :
    CBaseTrans (CAppInterface *pApp, WORD64 lwTransID);
    virtual ~CBaseTrans();

    virtual WORD32 Initialize();

    WORD64 GetTransID();
    WORD32 GetStepNum();

    WORD32 RegisterStep(CBaseTransStep *pStep);

    CBaseTransStep * GetCurStep();

    WORD32 Wait(WORD32 dwStepID);
    WORD32 NextStep(WORD32 dwStepID);
    WORD32 Fail(WORD32 dwStepID);

    /* �ȴ���Ӧ��Ϣ��ʱ */
    VOID WaitTimeOut(const VOID *pIn, WORD32 dwLen);

    BOOL IsFinish();

    /* �����ڴ�����ʵ����Step; ���迼���ڴ����(����Trans�����Զ�����) */
    BYTE * Mallc();

protected :
    CAppInterface   *m_pApp;
    WORD64           m_lwTransID;
    WORD32           m_dwTimerID;
    E_TransState     m_eTransState;
    WORD32           m_dwCurStepID;

    WORD32           m_dwStepNum;
    CBaseTransStep  *m_apStep[MAX_TRANS_STEP_NUM];

    CTransStepList   m_cList;
};


class CBaseTransStep : public CBaseData, public CCBObject
{
public :
    enum { TRANS_STEP_WAIT_TICK = 5};  /* 5ms */

public :
    CBaseTransStep (CBaseTrans *pTrans,
                    WORD32      dwStepID,
                    WORD32      dwSendMsgID,
                    WORD32      dwRecvMsgID,
                    WORD32      dwWaitTick);
    virtual ~CBaseTransStep();

    WORD32 SendMsg(E_AppClass  eDstClass,
                   WORD32      dwDstAssocID,
                   WORD16      wLen,
                   const VOID *ptMsg);

    WORD32 RecvMsg(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen);

    WORD32 GetTransID();
    WORD32 GetStepID();
    WORD32 GetWaitTick();

    /* ��RecvMsg���� */
    virtual WORD32 ProcMsg(VOID *pIn, WORD16 wMsgLen) = 0;

    /* ����ProcMsgʧ�ܺ�, ����Զ�����ProcFail()�ӿ� */
    virtual WORD32 ProcFail() = 0;

    /* �ȴ���Ϣ��ʱ, ����Զ�����Stepʵ����WaitTimeOut()�ӿ� */
    virtual WORD32 WaitTimeOut() = 0;

protected :
    CBaseTrans       *m_pTrans;

    WORD32            m_dwTransID;    /* ����ID, ��ǰTrans������Step��ͬ */
    WORD32            m_dwStepID;     /* ��ǰStepID */
    E_TransStepState  m_eState;
    WORD32            m_dwSendMsgID;  /* Step������Ϣ�� */
    WORD32            m_dwRecvMsgID;  /* Step�ȴ�������Ϣ�� */
    WORD32            m_dwWaitTick;   /* Step�ȴ�ʱ��, ��λ : ms */
};


template <WORD32 SendMsgID, WORD32 RecvMsgID, WORD32 WaitTick>
class CBaseTransStepTpl : public CBaseTransStep
{
public :
    CBaseTransStepTpl (CBaseTrans *pTrans, WORD32 dwStepID)
        : CBaseTransStep(pTrans,
                         dwStepID,
                         SendMsgID,
                         RecvMsgID,
                         WaitTick)
    {
    }

    virtual ~CBaseTransStepTpl() {}
};


/* T : �����CBaseTransStepTpl������ */
template <class T>
class CTransStepTpl
{
public :
    static T * CreateStep(CBaseTrans *pTrans)
    {
        BYTE *pMem = pTrans->Mallc();
        if ((NULL == pMem) || (sizeof(T_TransStepValue) < sizeof(T)))
        {
            assert(0);
        }

        T *pStep = new (pMem) T(pTrans, pTrans->GetStepNum());

        pTrans->RegisterStep(pStep);

        return pStep;
    }

public :
    virtual ~CTransStepTpl() {}

protected :
    CTransStepTpl () {}

    CTransStepTpl (CTransStepTpl &) = delete;
    CTransStepTpl (const CTransStepTpl &) = delete;
    CTransStepTpl & operator= (const CTransStepTpl &) = delete;
};


/* Ts : �����CBaseTransStepTpl������ */
template <typename... Ts>
class CTransTpl : public CBaseTrans
{
protected :
    using CBaseTrans::Initialize;

public :
    CTransTpl<Ts...>(CAppInterface *pApp, WORD64 lwTransID)
        : CBaseTrans(pApp, lwTransID)
    {
        CBaseTrans::Initialize();
        Constructor<0, Ts...>();
    }

    virtual ~CTransTpl() {}

protected :
    template <WORD32 IDX, typename TVal>
    VOID Create()
    {
        using ValType = CTransStepTpl<TVal>;

        TVal *pStep = ValType::CreateStep(this);
        assert(NULL != pStep);
    }

    template <WORD32 IDX, typename CurType, typename... Tails>
    VOID Constructor()
    {
        Create<IDX, CurType>();

        if constexpr (IDX == (sizeof...(Ts) - 1))
        {
            /* �����ݹ� */
            return ;
        }
        else
        {
            return Constructor<IDX + 1, Tails...>();
        }
    }
};


#endif


