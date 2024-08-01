

#include "dpdk_app_bb.h"

#ifdef PICOCOM_FAPI
#include "pcxx_ipc.h"
#endif


CBBApp::CBBApp ()
    : CAppInterface(E_APP_DPDK_BB)
{
    m_wCellID      = INVALID_WORD;
    m_wDeviceID    = INVALID_WORD;
    m_wFapiCellID  = INVALID_WORD;
    m_pCtrlTraffic = NULL;
    m_pDataTraffic = NULL;
    m_pBBDevice    = NULL;
}


CBBApp::~CBBApp()
{
    m_wCellID      = INVALID_WORD;
    m_wDeviceID    = INVALID_WORD;
    m_wFapiCellID  = INVALID_WORD;
    m_pCtrlTraffic = NULL;
    m_pDataTraffic = NULL;
    m_pBBDevice    = NULL;
}


WORD32 CBBApp::Init()
{
    TRACE_STACK("CBBApp::Init()");

    if ((FALSE == m_bAssocFlag) || (1 != m_dwAssocNum) || (NULL == g_pDpdkMgr))
    {
        assert(0);
    }

    CBBHandler &rBBHandler = g_pDpdkMgr->GetBBHandler();

    m_wCellID      = (WORD16)(m_adwAssocID[0]);
    m_pCtrlTraffic = (CCtrlTraffic *)(rBBHandler.GetCtrlTraffic(m_wCellID));
    m_pDataTraffic = (CDataTraffic *)(rBBHandler.GetDataTraffic(m_wCellID));

    if ((NULL == m_pCtrlTraffic) || (NULL == m_pDataTraffic))
    {
        assert(0);
    }

    m_wDeviceID   = (WORD16)(m_pCtrlTraffic->GetDeviceID());
    m_wFapiCellID = (WORD16)(m_pCtrlTraffic->GetFAPICellID());
    m_pBBDevice   = g_pDpdkMgr->FindDevice(m_wDeviceID);

    if (NULL == m_pBBDevice)
    {
        assert(0);
    }

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "m_dwAppID : %d, m_wCellID : %d, m_wDeviceID : %d, "
               "m_wFapiCellID : %d, CtrlTraffic : %lu, DataTraffic : %lu, "
               "BBDevice : %lu\n",
               m_dwAppID,
               m_wCellID,
               m_wDeviceID,
               m_wFapiCellID,
               (WORD64)(m_pCtrlTraffic),
               (WORD64)(m_pDataTraffic),
               (WORD64)(m_pBBDevice));

    return SUCCESS;
}


WORD32 CBBApp::DeInit()
{
    TRACE_STACK("CBBApp::DeInit()");

    return SUCCESS;
}


WORD32 CBBApp::Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen)
{
    TRACE_STACK("CBBApp::Exit()");

    m_wCellID      = INVALID_WORD;
    m_pCtrlTraffic = NULL;
    m_pDataTraffic = NULL;
    m_pBBDevice    = NULL;

    return SUCCESS;
}


WORD32 CBBApp::Polling()
{
#ifdef PICOCOM_FAPI
    pcxxCtrlRecv(m_wDeviceID, m_wFapiCellID);
#endif

    return SUCCESS;
}


