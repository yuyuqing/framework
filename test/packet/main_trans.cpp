

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <fstream>

#include "base_data_list.h"
#include "base_init_component.h"
#include "base_app_cntrl.h"
#include "base_oam_app.h"
#include "base_trans.h"
#include "base_bitset.h"
#include "base_numeral_generate.h"


class CTestTransApp : public CAppInterface
{
public :
    CTestTransApp ()
        : CAppInterface(E_APP_TEST)
    {
    }

    virtual ~CTestTransApp() {}

    WORD32 Init()
    {
        return SUCCESS;
    }

    WORD32 DeInit()
    {
        return SUCCESS;
    }

    WORD32 Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen)
    {
        return SUCCESS;
    }

    VOID TimeOutTrans(const VOID *pIn, WORD32 dwLen)
    {
        if (unlikely((NULL == pIn) || (dwLen != sizeof(T_TimerParam))))
        {
            return ;
        }
        
        T_TimerParam  *ptParam   = (T_TimerParam *)pIn;
        
        WORD32           dwID       = ptParam->dwID;
        WORD32           dwExtendID = ptParam->dwExtendID;
        WORD64           lwTransIDH = ptParam->dwTransID;
        WORD64           lwTransIDL = ptParam->dwResvID;
        WORD64           lwTransID  = (lwTransIDH << 32) | (lwTransIDL);
        CBaseTrans      *pTrans     = (CBaseTrans *)(ptParam->pContext);
        CBaseTransStep  *pStep      = (CBaseTransStep *)(ptParam->pUserData);

        if ( (0 == dwID)
          && (0 == dwExtendID)
          && (0 == lwTransID)
          && (NULL != pTrans)
          && (NULL != pStep))
        {
        }
    }
};


class CTestStep1 : public CBaseTransStepTpl<1, 11>
{
public :
    CTestStep1 (CBaseTrans *pTrans, WORD32 dwStepID)
        : CBaseTransStepTpl<1, 11> (pTrans, dwStepID)
    {
    }

    virtual ~CTestStep1()
    {
    }

    virtual WORD32 Execute();
    virtual WORD32 ProcMsg(VOID *pIn, WORD32 dwMsgLen);
};


WORD32 CTestStep1::Execute()
{
    return SUCCESS;
}


WORD32 CTestStep1::ProcMsg(VOID *pIn, WORD32 dwMsgLen)
{
    return SUCCESS;
}


class CTestStep2 : public CBaseTransStepTpl<2, 12>
{
public :
    CTestStep2 (CBaseTrans *pTrans, WORD32 dwStepID)
        : CBaseTransStepTpl<2, 12> (pTrans, dwStepID)
    {
    }

    virtual ~CTestStep2()
    {
    }

    virtual WORD32 Execute();
    virtual WORD32 ProcMsg(VOID *pIn, WORD32 dwMsgLen);
};


WORD32 CTestStep2::Execute()
{
    return SUCCESS;
}


WORD32 CTestStep2::ProcMsg(VOID *pIn, WORD32 dwMsgLen)
{
    return SUCCESS;
}


class CTestStep3 : public CBaseTransStepTpl<3, 13>
{
public :
    CTestStep3 (CBaseTrans *pTrans, WORD32 dwStepID)
        : CBaseTransStepTpl<3, 13> (pTrans, dwStepID)
    {
    }

    virtual ~CTestStep3()
    {
    }

    virtual WORD32 Execute();
    virtual WORD32 ProcMsg(VOID *pIn, WORD32 dwMsgLen);
};


WORD32 CTestStep3::Execute()
{
    return SUCCESS;
}


WORD32 CTestStep3::ProcMsg(VOID *pIn, WORD32 dwMsgLen)
{
    return SUCCESS;
}


class CTestTrans1 : public CTransTpl<CTestStep1>
{
public :
    CTestTrans1 (CAppInterface *pApp,
                 WORD32         dwUpperID,
                 WORD32         dwExtendID,
                 WORD64         lwTransID,
                 CCBObject     *pObj,
                 PCBFUNC        pFunc)
        : CTransTpl<CTestStep1> (pApp, dwUpperID, dwExtendID, lwTransID, pObj, pFunc)
    {
    }

    virtual ~CTestTrans1()
    {
    }

    virtual WORD32 Abort();
    virtual WORD32 Finish();
};


WORD32 CTestTrans1::Abort()
{
    return SUCCESS;
}


WORD32 CTestTrans1::Finish()
{
    return SUCCESS;
}


class CTestTrans2 : public CTransTpl<CTestStep2, CTestStep1>
{
public :
    CTestTrans2 (CAppInterface *pApp,
                 WORD32         dwUpperID,
                 WORD32         dwExtendID,
                 WORD64         lwTransID,
                 CCBObject     *pObj,
                 PCBFUNC        pFunc)
        : CTransTpl<CTestStep2,
                    CTestStep1> (pApp, dwUpperID, dwExtendID, lwTransID, pObj, pFunc)
    {
    }

    virtual ~CTestTrans2()
    {
    }

    /* 事务异常结束时, 通过重载Abort函数做善后处理(如 : 重启事务) */
    virtual WORD32 Abort()
    {
        return SUCCESS;
    }

    /* 事务正常结束时, 通过重载Finish函数做专属处理 */
    virtual WORD32 Finish()
    {
        return SUCCESS;
    }
};


class CTestTrans3 : public CTransTpl<CTestStep3, CTestStep2, CTestStep1>
{
public :
    CTestTrans3 (CAppInterface *pApp,
                 WORD32         dwUpperID,
                 WORD32         dwExtendID,
                 WORD64         lwTransID,
                 CCBObject     *pObj,
                 PCBFUNC        pFunc)
        : CTransTpl<CTestStep3,
                    CTestStep2,
                    CTestStep1> (pApp, dwUpperID, dwExtendID, lwTransID, pObj, pFunc)
    {
    }

    virtual ~CTestTrans3()
    {
    }

    /* 事务异常结束时, 通过重载Abort函数做善后处理(如 : 重启事务) */
    virtual WORD32 Abort()
    {
        return SUCCESS;
    }

    /* 事务正常结束时, 通过重载Finish函数做专属处理 */
    virtual WORD32 Finish()
    {
        return SUCCESS;
    }
};


int main(int argc, char **argv)
{
    CTestTransApp  cApp;

    CTestTrans1    cTrans1(&cApp, 0, 0, 0, &cApp, (PCBFUNC)(&CTestTransApp::TimeOutTrans));
    CTestTrans2    cTrans2(&cApp, 0, 0, 0, &cApp, (PCBFUNC)(&CTestTransApp::TimeOutTrans));
    CTestTrans3    cTrans3(&cApp, 0, 0, 0, &cApp, (PCBFUNC)(&CTestTransApp::TimeOutTrans));

    cTrans1.Execute();
    cTrans2.Execute();
    cTrans3.Execute();

    return SUCCESS;
}


