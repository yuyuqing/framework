

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
#include "base_numeral_generate.h"
#include "base_data_recurring.h"


class CTestData1 : public CBaseSelfData<CTestData1>
{
public :
    CTestData1 ()
    {
        m_dwTest1 = 0;
    }

    CTestData1 (WORD32 dwTest)
    {
        m_dwTest1 = dwTest;
    }

    virtual ~CTestData1()
    {
        m_dwTest1 = 0;
    }

protected :
    WORD32  m_dwTest1;
};


typedef CSelfDataContainer<CTestData1>  CTestSelfDataContainer;


int main(int argc, char **argv)
{
    CInitList::GetInstance()->InitComponent((WORD32)E_PROC_DU);

    CCentralMemPool *pCentranMemPool = CMemMgr::GetInstance()->GetCentralMemPool();

    CTestSelfDataContainer::GetInstance(pCentranMemPool, 1200);

    CTestData1 *pTest0 = new CTestData1();
    CTestData1 *pTest1 = new CTestData1(1);
    CTestData1 *pTest2 = new (2) CTestData1();
    CTestData1 *pTest3 = new (3) CTestData1(3);

    delete pTest2;
    delete pTest1;
    delete pTest3;
    delete pTest0;

    CInitList::Destroy();

    return SUCCESS;
}


