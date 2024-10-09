

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
#include "base_bitset.h"


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


class CTestStep3 : public CBaseTransStepTpl<3, 13, 5>
{
public :
    CTestStep3 (CBaseTrans *pTrans, WORD32 dwStepID)
        : CBaseTransStepTpl<3, 13, 5> (pTrans, dwStepID)
    {
    }

    virtual ~CTestStep3()
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


typedef CTransTpl<CTestStep1>                          CTestTrans1;
typedef CTransTpl<CTestStep1, CTestStep2>              CTestTrans2;
typedef CTransTpl<CTestStep1, CTestStep2, CTestStep3>  CTestTrans3;


int main(int argc, char **argv)
{
    BOOL   bFlag   = FALSE;
    WORD32 dwCount = 0;
    WORD32 dwFirst = 0;
    WORD32 dwNext  = 0;
    WORD32 dwLast  = 0;

    CBaseBitSetTpl<3>   c3Bit0(255);
    CBaseBitSetTpl<3>   c3Bit1(255);
    CBaseBitSetTpl<31>  c31Bit(0xFFFFFFFF);
    CBaseBitSetTpl<32>  c32Bit(0xFFFFFFFE);
    CBaseBitSetTpl<33>  c33Bit;
    CBaseBitSetTpl<64>  c64Bit(0xFFFFFFFF);
    CBaseBitSetTpl<129> c129Bit(0xBBBBBBBB);

    CBaseBitSetTpl<31>  c31Bit1 = ~c31Bit;
    CBaseBitSetTpl<31>  c31Bit2 = c31Bit << 8;
    CBaseBitSetTpl<31>  c31Bit3 = c31Bit >> 12;

    c31Bit1 = c31Bit << 8;
    c31Bit1 = c31Bit >> 12;

    c64Bit <<= 3;
    c64Bit <<= 4;
    c64Bit <<= 5;
    c64Bit >>= 3;
    c64Bit >>= 4;
    c64Bit >>= 5;
    c64Bit <<= 32;
    c64Bit >>= 32;

    c3Bit0.Set(0, 0);
    c3Bit0.Set(1, 0);
    c3Bit0.Set(2, 0);

    c3Bit0 &= c3Bit1;
    c3Bit0 |= c3Bit1;
    c3Bit0 ^= c3Bit1;

    c3Bit0.Flip();
    c3Bit0.Flip(0);
    c3Bit0.Flip(1);
    c3Bit0.Flip(2);

    bFlag   = c129Bit[0];
    bFlag   = c129Bit[1];
    bFlag   = c129Bit[2];
    bFlag   = (c3Bit0 == c3Bit1);
    bFlag   = (c3Bit0 != c3Bit1);
    bFlag   = c129Bit.IsAny();
    dwCount = c129Bit.Count();
    dwFirst = c33Bit.FindFirst();
    dwNext  = c33Bit.FindNext(0);
    dwLast  = c33Bit.FindLast(0);

    c33Bit.Set(0, bFlag);
    c33Bit.Set(dwCount, bFlag);

    dwFirst = c33Bit.FindFirst();
    dwNext  = c33Bit.FindNext(0);
    dwLast  = c33Bit.FindLast(0);

    assert(TRUE  == c129Bit.LeftBit(0));
    assert(FALSE == c129Bit.LeftBit(1));
    assert(TRUE  == c129Bit.LeftBit(2));
    assert(TRUE  == c129Bit.LeftBit(10));

    dwFirst = c129Bit.FindFirst();
    dwNext  = c129Bit.FindNext(dwFirst);
    dwLast  = c129Bit.FindLast(dwFirst);

    dwNext = c129Bit.FindNext(dwLast);
    dwLast = c129Bit.FindLast(dwNext);
    dwNext = c129Bit.FindNext(dwNext);
    dwLast = c129Bit.FindLast(dwNext);

    dwNext = c129Bit.FindNext(dwLast);
    dwLast = c129Bit.FindLast(dwNext);
    dwNext = c129Bit.FindNext(dwNext);
    dwLast = c129Bit.FindLast(dwNext);

    CTestTransApp  cApp;
    CTestTrans1    cTrans1(&cApp, 1);
    CTestTrans2    cTrans2(&cApp, 2);
    CTestTrans3    cTrans3(&cApp, 3);

    return SUCCESS;
}


