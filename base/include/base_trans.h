

#ifndef _BASE_TRANS_H_
#define _BASE_TRANS_H_


#include <assert.h>

#include "base_app_interface.h"


typedef struct tagT_TransStepValue
{
    BYTE  aucStep[DOUBLE_CACHE_SIZE];
}T_TransStepValue;


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

    enum { TRANS_STEP_WAIT_TICK = 10};  /* 10ms */

    typedef CBaseDataContainer<T_TransStepValue, MAX_TRANS_STEP_NUM, FALSE> CTransStepList;

public :
    CBaseTrans (CAppInterface *pApp,
                WORD32         dwUpperID,
                WORD32         dwExtendID,
                WORD64         lwTransID,
                CCBObject     *pObj,
                PCBFUNC        pFunc);
    virtual ~CBaseTrans();

    virtual WORD32 Initialize();

    virtual WORD32 Execute();

    virtual WORD32 WaitTimeOut(WORD64 lwTransID, CBaseTransStep *pStep);

    virtual WORD32 RecvMsg(WORD32 dwMsgID, VOID *pIn, WORD32 dwMsgLen);

    /* 事务异常结束时, 通过重载Abort函数做善后处理(如 : 重启事务) */
    virtual WORD32 Abort() = 0;

    /* 事务正常结束时, 通过重载Finish函数做专属处理 */
    virtual WORD32 Finish() = 0;

    CAppInterface * GetApp();
    CCBObject * GetObject();

    WORD32 GetUpperID();
    WORD64 GetTransID();
    WORD32 GetStepNum();

    BOOL IsFinish();

    /* 分配内存用于实例化Step; 无需考虑内存回收(伴随Trans销毁自动回收) */
    BYTE * Mallc();

    WORD32 RegisterStep(CBaseTransStep *pStep);

protected :
    CBaseTransStep * GetCurStep();

protected :
    CAppInterface   *m_pApp;
    WORD32           m_dwUpperID;    /* 用于标识高层业务ID(如 : CellID) */
    WORD32           m_dwExtendID;   /* 用于标识高层业务ID(如 : UEID) */
    WORD64           m_lwTransID;
    CCBObject       *m_pObj;
    PCBFUNC          m_pTimeOutFunc;

    WORD32           m_dwTimerID;
    E_TransState     m_eTransState;
    WORD32           m_dwCurStepID;
    WORD32           m_dwStepNum;
    CBaseTransStep  *m_apStep[MAX_TRANS_STEP_NUM];

    CTransStepList   m_cList;
};


inline CAppInterface * CBaseTrans::GetApp()
{
    return m_pApp;
}


inline CCBObject * CBaseTrans::GetObject()
{
    return m_pObj;
}


inline WORD32 CBaseTrans::GetUpperID()
{
    return m_dwUpperID;
}


inline WORD64 CBaseTrans::GetTransID()
{
    return m_lwTransID;
}


inline WORD32 CBaseTrans::GetStepNum()
{
    return m_dwStepNum;
}


inline BOOL CBaseTrans::IsFinish()
{
    return (E_TRANS_COMPLETE == m_eTransState);
}


/* 分配内存用于实例化Step; 无需考虑内存回收(伴随Trans销毁自动回收) */
inline BYTE * CBaseTrans::Mallc()
{
    T_TransStepValue *ptValue = (T_TransStepValue *)(m_cList.Malloc());
    if (NULL == ptValue)
    {
        return NULL;
    }

    return ptValue->aucStep;
}


inline WORD32 CBaseTrans::RegisterStep(CBaseTransStep *pStep)
{
    if (m_dwStepNum >= MAX_TRANS_STEP_NUM)
    {
        return FAIL;
    }

    m_apStep[m_dwStepNum++] = pStep;

    return SUCCESS;
}


inline CBaseTransStep * CBaseTrans::GetCurStep()
{
    if (m_dwCurStepID >= m_dwStepNum)
    {
        return NULL;
    }

    return m_apStep[m_dwCurStepID];
}


class CBaseTransStep : public CBaseData, public CCBObject
{
public :
    friend class CBaseTrans;

public :
    CBaseTransStep (CBaseTrans *pTrans,
                    WORD32      dwStepID,
                    WORD32      dwSendMsgID,
                    WORD32      dwRecvMsgID);
    virtual ~CBaseTransStep();

    virtual WORD32 Execute() = 0;

    virtual WORD32 ProcMsg(VOID *pIn, WORD32 dwMsgLen) = 0;

    WORD32 GetStepID();

protected :
    CBaseTrans       *m_pTrans;

    WORD32            m_dwStepID;     /* 当前StepID */
    WORD32            m_dwSendMsgID;  /* Step发送消息号 */
    WORD32            m_dwRecvMsgID;  /* Step等待接收消息号 */
};


inline WORD32 CBaseTransStep::GetStepID()
{
    return m_dwStepID;
}


template <WORD32 SendMsgID, WORD32 RecvMsgID>
class CBaseTransStepTpl : public CBaseTransStep
{
public :
    CBaseTransStepTpl (CBaseTrans *pTrans, WORD32 dwStepID)
        : CBaseTransStep(pTrans,
                         dwStepID,
                         SendMsgID,
                         RecvMsgID)
    {
    }

    virtual ~CBaseTransStepTpl() {}
};


/* T : 具体的CBaseTransStepTpl派生类 */
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


template <typename... Ts> class CTransTpl;
template <> class CTransTpl<> : public CBaseTrans
{
protected :
    using CBaseTrans::Initialize;

public :
    CTransTpl (CAppInterface *pApp,
               WORD32         dwUpperID,
               WORD32         dwExtendID,
               WORD64         lwTransID,
               CCBObject     *pObj,
               PCBFUNC        pFunc)
        : CBaseTrans(pApp, dwUpperID, dwExtendID, lwTransID, pObj, pFunc)
    {
        CBaseTrans::Initialize();
    }

    virtual ~CTransTpl() {}
};


template <typename T, typename... Ts>
class CTransTpl<T, Ts...> : public CTransTpl<Ts...>
{
protected :
    using CBaseTrans::Initialize;

public :
    CTransTpl (CAppInterface *pApp,
               WORD32         dwUpperID,
               WORD32         dwExtendID,
               WORD64         lwTransID,
               CCBObject     *pObj,
               PCBFUNC        pFunc)
        : CTransTpl<Ts...>(pApp, dwUpperID, dwExtendID, lwTransID, pObj, pFunc)
    {
        Constructor<T> ();
    }

    virtual ~CTransTpl() {}

protected :
    template <typename CurType>
    VOID Constructor()
    {
        using ValType = CTransStepTpl<CurType>;

        CurType *pStep = ValType::CreateStep(this);
        assert(NULL != pStep);
    }
};


#if 0
/* Ts : 具体的CBaseTransStepTpl派生类 */
template <typename... Ts>
class CTransTpl : public CBaseTrans
{
protected :
    using CBaseTrans::Initialize;

public :
    CTransTpl (CAppInterface *pApp,
               WORD32         dwUpperID,
               WORD32         dwExtendID,
               WORD64         lwTransID,
               CCBObject     *pObj,
               PCBFUNC        pFunc)
        : CBaseTrans(pApp, dwUpperID, dwExtendID, lwTransID, pObj, pFunc)
    {
        CBaseTrans::Initialize();
        Constructor<0, Ts...>();
    }

    virtual ~CTransTpl() {}

protected :
    template <WORD32 IDX, typename T, typename... Tails>
    VOID Constructor()
    {
        using ValType = CTransStepTpl<T>;

        T *pStep = ValType::CreateStep(this);
        assert(NULL != pStep);

        if constexpr (IDX == (sizeof...(Ts) - 1))
        {
            /* 结束递归 */
            return ;
        }
        else
        {
            return Constructor<IDX + 1, Tails...>();
        }
    }
};
#endif


#endif


