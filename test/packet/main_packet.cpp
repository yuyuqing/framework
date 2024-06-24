

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
    /* 接收从RLC发来的上行报文 */
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

    /* 模拟从CU侧收到下行报文后主动向RLC投递该报文 */
    WORD32 Polling();

protected :
    WORD32  m_dwCellIdx;
};


class CTestRlcApp : public CAppInterface
{
public :
    /* 接收从GTPU发来的下行报文, 转发给DMAC; 接收DMAC发来的上行报文, 转发给GTPU */
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
    /* 接收RLC发来的下行报文, 转发上行报文给RLC */
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


/* 接收从RLC发来的上行报文 */
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


/* 模拟从CU侧收到下行报文后主动向RLC投递该报文 */
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


/* 接收从GTPU发来的下行报文, 转发给DMAC; 接收DMAC发来的上行报文, 转发给GTPU */
WORD32 CTestRlcApp::RecvPDU(VOID *pArg, VOID *pBuf)
{
    WORD32              dwNum  = 0;
    CTestRlcApp        *pApp   = (CTestRlcApp *)pArg;
    T_TestPacketBuffer *pTBBuf = (T_TestPacketBuffer *)pBuf;

    if ( (TEST_CHECK_PARAM_1 == pTBBuf->dwPduNum)
      && (TEST_CHECK_PARAM_2 == pTBBuf->dwTbSize))
    {
        /* 下行 */
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
        /* 上行 */
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


typedef CBaseList<T_TestPacketBuffer, TEST_NODE_NUM>  CPacketBufferList;


int main(int argc, char **argv)
{
    WORD32             dwIndex = 0;
    CPacketBufferList  cList;
    CTokenBucket       cTokenBucket(10000, 100);

    cList.Initialize();

    T_TestPacketBuffer *pBuf1 = cList.CreateTail();
    T_TestPacketBuffer *pBuf2 = cList.CreateTail();
    T_TestPacketBuffer *pBuf3 = cList.CreateTail();
    T_TestPacketBuffer *pBuf4 = cList.CreateHead();
    T_TestPacketBuffer *pBuf5 = cList.Malloc(dwIndex);
    T_TestPacketBuffer *pBuf6 = cList.Malloc(dwIndex);
    T_TestPacketBuffer *pBuf7 = cList.Malloc(dwIndex);
    T_TestPacketBuffer *pBuf8 = cList.Malloc(dwIndex);

    cList.Remove(pBuf2);
    cList.Remove(pBuf1);

    cList.InsertHead(pBuf5);
    cList.InsertHead(pBuf6);
    cList.InsertTail(pBuf7);
    cList.InsertTail(pBuf8);

    cList.Remove(pBuf4);
    cList.Remove(pBuf3);

    cList.Remove(pBuf7);
    cList.Remove(pBuf6);
    cList.Remove(pBuf5);
    cList.Remove(pBuf8);

    CInitList::GetInstance()->InitComponent((WORD32)E_PROC_DU);

    g_pOamApp->NotifyOamStartUP();

    sleep(5);

    CInitList::Destroy();

    return SUCCESS;
}


