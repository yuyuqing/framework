

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <fstream>

#include "base_list.h"
#include "base_init_component.h"
#include "base_app_cntrl.h"
#include "base_oam_app.h"
#include "base_trans.h"


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
};


class CTestStep1 : public CBaseTransStepTpl<1, 11, 5>
{
public :
    CTestStep1 (CBaseTrans *pTrans, WORD32 dwStepID)
        : CBaseTransStepTpl<1, 11, 5> (pTrans, dwStepID)
    {
    }

    virtual ~CTestStep1()
    {
    }

    WORD32 ProcMsg(VOID *pIn, WORD16 wMsgLen)
    {
        return SUCCESS;
    }

    WORD32 ProcFail()
    {
        return SUCCESS;
    }

    WORD32 WaitTimeOut()
    {
        return SUCCESS;
    }
};


class CTestStep2 : public CBaseTransStepTpl<2, 12, 5>
{
public :
    CTestStep2 (CBaseTrans *pTrans, WORD32 dwStepID)
        : CBaseTransStepTpl<2, 12, 5> (pTrans, dwStepID)
    {
    }

    virtual ~CTestStep2()
    {
    }

    WORD32 ProcMsg(VOID *pIn, WORD16 wMsgLen)
    {
        return SUCCESS;
    }

    WORD32 ProcFail()
    {
        return SUCCESS;
    }

    WORD32 WaitTimeOut()
    {
        return SUCCESS;
    }
};


typedef CTransTpl<CTestStep1>              CTestTrans1;
typedef CTransTpl<CTestStep1, CTestStep2>  CTestTrans2;


int main(int argc, char **argv)
{
    CTestTransApp  cApp;
    CTestTrans1    cTrans1(&cApp, 1);
    CTestTrans2    cTrans2(&cApp, 2);

    return SUCCESS;
}


