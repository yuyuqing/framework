

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <fstream>

#include "base_list.h"
#include "base_token_bucket.h"
#include "base_init_component.h"
#include "base_app_cntrl.h"
#include "base_oam_app.h"


#define TEST_CHECK_PARAM_1       ((WORD32)(1))
#define TEST_CHECK_PARAM_2       ((WORD32)(2))
#define TEST_CHECK_PARAM_3       ((BYTE)(3))
#define TEST_CHECK_PARAM_4       ((WORD32)(4))
#define TEST_CHECK_PARAM_5       ((WORD16)(5))
#define TEST_CHECK_PARAM_6       ((WORD16)(6))


typedef struct tagT_TestPacketBuffer
{
    volatile WORD32  dwPduNum;
    volatile WORD32  dwTbSize;
}T_TestPacketBuffer;



class CTestEgtpApp : public CAppInterface
{
public :
    /* ���մ�RLC���������б��� */
    static WORD32 RecvPDU(VOID *pArg, VOID *pBuf);

public :
    CTestEgtpApp ()
        : CAppInterface(E_APP_EGTP)
    {
        m_dwCellIdx = INVALID_DWORD;
    }

    virtual ~CTestEgtpApp() {}

    WORD32 Init()
    {
        TRACE_STACK("CTestEgtpApp::Init()");

        if ((FALSE == m_bAssocFlag) || (1 != m_dwAssocNum))
        {
            assert(0);
        }

        m_dwCellIdx  = m_adwAssocID[0] - 1;

        RegisterPacketFunc((PMsgCallBack)(&CTestEgtpApp::RecvPDU));

        return SUCCESS;
    }

    WORD32 DeInit()
    {
        TRACE_STACK("CTestEgtpApp::DeInit()");
        return SUCCESS;
    }

    WORD32 Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen)
    {
        TRACE_STACK("CTestEgtpApp::Exit()");
        return SUCCESS;
    }

    /* ģ���CU���յ����б��ĺ�������RLCͶ�ݸñ��� */
    WORD32 Polling();

protected :
    WORD32  m_dwCellIdx;
};


class CTestRlcApp : public CAppInterface
{
public :
    /* ���մ�GTPU���������б���, ת����DMAC; ����DMAC���������б���, ת����GTPU */
    static WORD32 RecvPDU(VOID *pArg, VOID *pBuf);

public :
    CTestRlcApp ()
        : CAppInterface(E_APP_RLC)
    {
        m_dwCellIdx = INVALID_DWORD;
    }

    virtual ~CTestRlcApp() {}

    WORD32 Init()
    {
        TRACE_STACK("CTestRlcApp::Init()");

        if ((FALSE == m_bAssocFlag) || (1 != m_dwAssocNum))
        {
            assert(0);
        }

        m_dwCellIdx = m_adwAssocID[0] - 1;

        RegisterPacketFunc((PMsgCallBack)(&CTestRlcApp::RecvPDU));

        return SUCCESS;
    }

    WORD32 DeInit()
    {
        TRACE_STACK("CTestRlcApp::DeInit()");
        return SUCCESS;
    }

    WORD32 Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen)
    {
        TRACE_STACK("CTestRlcApp::Exit()");
        return SUCCESS;
    }

protected :
    WORD32  m_dwCellIdx;
};


class CTestDMacApp : public CAppInterface
{
public :
    /* ����RLC���������б���, ת�����б��ĸ�RLC */
    static WORD32 RecvPDU(VOID *pArg, VOID *pBuf)
    {
        WORD32              dwNum  = 0;
        CTestDMacApp       *pApp   = (CTestDMacApp *)pArg;
        T_TestPacketBuffer *pTBBuf = (T_TestPacketBuffer *)pBuf;

        if ( (TEST_CHECK_PARAM_1 == pTBBuf->dwPduNum)
          && (TEST_CHECK_PARAM_2 == pTBBuf->dwTbSize))
        {
            pTBBuf->dwPduNum = TEST_CHECK_PARAM_2;
            pTBBuf->dwTbSize = TEST_CHECK_PARAM_1;

            CTestRlcApp *pRlcApp = (CTestRlcApp *)(g_pAppCntrl->GetApp(E_APP_RLC, (pApp->m_dwCellIdx + 1)));
            if (NULL == pRlcApp)
            {
                return FAIL;
            }

            dwNum = pRlcApp->SendPacket(pBuf);
            if (0 == dwNum)
            {
                assert(0);
            }
        }
        else
        {
            assert(0);
        }

        return SUCCESS;
    }

public :
    CTestDMacApp ()
        : CAppInterface(E_APP_DMAC)
    {
        m_dwCellIdx = INVALID_DWORD;
    }

    virtual ~CTestDMacApp() {}

    WORD32 Init()
    {
        TRACE_STACK("CTestDMacApp::Init()");

        if ((FALSE == m_bAssocFlag) || (1 != m_dwAssocNum))
        {
            assert(0);
        }

        m_dwCellIdx  = m_adwAssocID[0] - 1;

        RegisterPacketFunc((PMsgCallBack)(&CTestDMacApp::RecvPDU));

        return SUCCESS;
    }

    WORD32 DeInit()
    {
        TRACE_STACK("CTestDMacApp::DeInit()");
        return SUCCESS;
    }

    WORD32 Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen)
    {
        TRACE_STACK("CTestDMacApp::Exit()");
        return SUCCESS;
    }

protected :
    WORD32  m_dwCellIdx;
};


/* ���մ�RLC���������б��� */
WORD32 CTestEgtpApp::RecvPDU(VOID *pArg, VOID *pBuf)
{
    CTestEgtpApp       *pApp     = (CTestEgtpApp *)pArg;
    T_TestPacketBuffer *pTBBuf   = (T_TestPacketBuffer *)pBuf;
    WORD32              dwResult = INVALID_DWORD;

    if ( (TEST_CHECK_PARAM_2 != pTBBuf->dwPduNum)
      || (TEST_CHECK_PARAM_1 != pTBBuf->dwTbSize)
      || (NULL == pApp))
    {
        assert(0);
    }

    dwResult = CMemPools::GetInstance()->Free((BYTE *)pBuf);
    if (SUCCESS != dwResult)
    {
        assert(0);
    }

    return SUCCESS;
}


/* ģ���CU���յ����б��ĺ�������RLCͶ�ݸñ��� */
WORD32 CTestEgtpApp::Polling()
{
    CTestRlcApp *pRlcApp = (CTestRlcApp *)(g_pAppCntrl->GetApp(E_APP_RLC, (m_dwCellIdx + 1)));
    if (NULL == pRlcApp)
    {
        return FAIL;
    }

    WORD32              dwNum = 0;
    T_TestPacketBuffer *pBuf  = NULL;

    pBuf = (T_TestPacketBuffer *)(CMemPools::GetInstance()->Malloc(
                                      (m_dwCellIdx + 1),
                                      sizeof(T_TestPacketBuffer),
                                      E_MP_MALLOC_POINT_00));
    if (NULL == pBuf)
    {
        return FAIL;
    }

    pBuf->dwPduNum = TEST_CHECK_PARAM_1;
    pBuf->dwTbSize = TEST_CHECK_PARAM_2;

    dwNum = pRlcApp->SendPacket((VOID *)pBuf);
    if (0 == dwNum)
    {
        CMemPools::GetInstance()->Free((BYTE *)pBuf);
    }

    return SUCCESS;
}


/* ���մ�GTPU���������б���, ת����DMAC; ����DMAC���������б���, ת����GTPU */
WORD32 CTestRlcApp::RecvPDU(VOID *pArg, VOID *pBuf)
{
    WORD32              dwNum  = 0;
    CTestRlcApp        *pApp   = (CTestRlcApp *)pArg;
    T_TestPacketBuffer *pTBBuf = (T_TestPacketBuffer *)pBuf;

    if ( (TEST_CHECK_PARAM_1 == pTBBuf->dwPduNum)
      && (TEST_CHECK_PARAM_2 == pTBBuf->dwTbSize))
    {
        /* ���� */
        CTestDMacApp *pDMacApp = (CTestDMacApp *)(g_pAppCntrl->GetApp(E_APP_DMAC, (pApp->m_dwCellIdx + 1)));
        if (NULL == pDMacApp)
        {
            return FAIL;
        }

        dwNum = pDMacApp->SendPacket(pBuf);
        if (0 == dwNum)
        {
            assert(0);
        }
    }
    else if ( (TEST_CHECK_PARAM_2 == pTBBuf->dwPduNum)
           && (TEST_CHECK_PARAM_1 == pTBBuf->dwTbSize))
    {
        /* ���� */
        CTestEgtpApp *pEgtpApp = (CTestEgtpApp *)(g_pAppCntrl->GetApp(E_APP_EGTP, (pApp->m_dwCellIdx + 1)));
        if (NULL == pEgtpApp)
        {
            return FAIL;
        }

        dwNum = pEgtpApp->SendPacket(pBuf);
        if (0 == dwNum)
        {
            assert(0);
        }
    }
    else
    {
        assert(0);
    }
    
    return SUCCESS;
}


DEFINE_APP(CTestEgtpApp);
DEFINE_APP(CTestRlcApp);
DEFINE_APP(CTestDMacApp);


#define TEST_NODE_NUM    ((WORD32)(1024))


typedef CBaseDataContainer<T_TestPacketBuffer, TEST_NODE_NUM, TRUE>  CPacketBufferContainer;
typedef CBaseList<T_TestPacketBuffer, TEST_NODE_NUM, TRUE>    CPacketBufferList;
typedef CShareList<T_TestPacketBuffer, TEST_NODE_NUM, TRUE>   CPacketBufferShareList;


int main(int argc, char **argv)
{
    WORD32                  dwIndex = 0;
    CPacketBufferContainer  cContainer;
    CPacketBufferList       cList;
    CPacketBufferShareList  cList1(cContainer);
    CPacketBufferShareList  cList2(cContainer);

    cContainer.Initialize();
    cList.Initialize();

    T_TestPacketBuffer *pBuf01 = cList.CreateTail();
    T_TestPacketBuffer *pBuf02 = cList.CreateTail();
    T_TestPacketBuffer *pBuf03 = cList.CreateTail();
    T_TestPacketBuffer *pBuf04 = cList.CreateHead();
    T_TestPacketBuffer *pBuf05 = cList.Malloc(dwIndex);
    T_TestPacketBuffer *pBuf06 = cList.Malloc(dwIndex);
    T_TestPacketBuffer *pBuf07 = cList.Malloc(dwIndex);
    T_TestPacketBuffer *pBuf08 = cList.Malloc(dwIndex);

    cList.Remove(pBuf02);
    cList.Remove(pBuf01);

    cList.InsertHead(pBuf05);
    cList.InsertHead(pBuf06);
    cList.InsertTail(pBuf07);
    cList.InsertTail(pBuf08);

    T_TestPacketBuffer *pHead = cList.GetHead();
    T_TestPacketBuffer *pTail = cList.GetTail();
    T_TestPacketBuffer *pNext = cList.Next(pHead);
    T_TestPacketBuffer *pPrev = cList.Prev(pTail);

    pNext = cList.Next(pNext);
    pPrev = cList.Prev(pPrev);

    cList.Remove(pBuf04);
    cList.Remove(pBuf03);

    cList.Remove(pBuf07);
    cList.Remove(pBuf06);
    cList.Remove(pBuf05);
    cList.Remove(pBuf08);

    T_TestPacketBuffer *pBuf11 = cList1.CreateTail();
    T_TestPacketBuffer *pBuf12 = cList1.CreateTail();
    T_TestPacketBuffer *pBuf13 = cList1.CreateTail();
    T_TestPacketBuffer *pBuf14 = cList1.CreateHead();
    T_TestPacketBuffer *pBuf15 = cContainer.Malloc(dwIndex);
    T_TestPacketBuffer *pBuf16 = cContainer.Malloc(dwIndex);
    T_TestPacketBuffer *pBuf17 = cContainer.Malloc(dwIndex);
    T_TestPacketBuffer *pBuf18 = cContainer.Malloc(dwIndex);

    cList1.Remove(pBuf12);
    cList1.Remove(pBuf11);

    cList1.InsertHead(pBuf15);
    cList1.InsertHead(pBuf16);
    cList1.InsertTail(pBuf17);
    cList1.InsertTail(pBuf18);

    cList1.Remove(pBuf14);
    cList1.Remove(pBuf13);

    cList1.Remove(pBuf17);
    cList1.Remove(pBuf16);
    cList1.Remove(pBuf15);
    cList1.Remove(pBuf18);

    T_TestPacketBuffer *pBuf21 = cList2.CreateTail();
    T_TestPacketBuffer *pBuf22 = cList2.CreateTail();
    T_TestPacketBuffer *pBuf23 = cList2.CreateTail();
    T_TestPacketBuffer *pBuf24 = cList2.CreateHead();
    T_TestPacketBuffer *pBuf25 = cContainer.Malloc(dwIndex);
    T_TestPacketBuffer *pBuf26 = cContainer.Malloc(dwIndex);
    T_TestPacketBuffer *pBuf27 = cContainer.Malloc(dwIndex);
    T_TestPacketBuffer *pBuf28 = cContainer.Malloc(dwIndex);

    cList2.Remove(pBuf22);
    cList2.Remove(pBuf21);

    cList2.InsertHead(pBuf25);
    cList2.InsertHead(pBuf26);
    cList2.InsertTail(pBuf27);
    cList2.InsertTail(pBuf28);

    cList2.Remove(pBuf24);
    cList2.Remove(pBuf23);

    cList2.Remove(pBuf27);
    cList2.Remove(pBuf26);
    cList2.Remove(pBuf25);
    cList2.Remove(pBuf28);

    CInitList::GetInstance()->InitComponent((WORD32)E_PROC_DU);

    g_pOamApp->NotifyOamStartUP();

    sleep(5);

    CTokenBucket cTokenBucket(10000, 100);

    CInitList::Destroy();

    return SUCCESS;
}


