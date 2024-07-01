

#include "dpdk_net_interface.h"
#include "dpdk_app_eth.h"


DEFINE_APP(CEthApp);


CEthApp::CEthApp ()
    : CAppInterface(E_APP_DPDK_ETH)
{
    m_dwDeviceID = INVALID_DWORD;
    m_wPortID    = INVALID_WORD;
    m_wQueueID   = INVALID_WORD;
    m_pQueue     = NULL;
}


CEthApp::~CEthApp()
{
    m_dwDeviceID = INVALID_DWORD;
    m_wPortID    = INVALID_WORD;
    m_wQueueID   = INVALID_WORD;
    m_pQueue     = NULL;
}


WORD32 CEthApp::Init()
{
    TRACE_STACK("CEthApp::Exit()");

    if ((FALSE == m_bAssocFlag) || (1 != m_dwAssocNum) || (NULL == g_pDpdkMgr))
    {
        assert(0);
    }

    /* 如果配置为多队列, 则dwAssocID需要由DeviceID+QueueID合成 */
    WORD32 dwAssocID = m_adwAssocID[0];

    m_dwDeviceID = dwAssocID >> 2;
    m_wQueueID   = (WORD16)(dwAssocID & 0x00000003);

    CBaseDevice *pDevice = g_pDpdkMgr->FindDevice(m_dwDeviceID);
    if (NULL == pDevice)
    {
        assert(0);
    }

    m_wPortID = pDevice->GetPortID();
    m_pQueue  = pDevice->GetQueue(m_wQueueID);
    if (NULL == m_pQueue)
    {
        assert(0);
    }

    return SUCCESS;
}


WORD32 CEthApp::DeInit()
{
    TRACE_STACK("CEthApp::DeInit()");

    return SUCCESS;
}


WORD32 CEthApp::Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen)
{
    TRACE_STACK("CEthApp::Exit()");

    m_dwDeviceID = INVALID_DWORD;
    m_wPortID    = INVALID_WORD;
    m_wQueueID   = INVALID_WORD;
    m_pQueue     = NULL;

    return SUCCESS;
}


WORD32 CEthApp::Polling()
{
    return m_pQueue->RecvPacket(MBUF_BURST_NUM,
                                this,
                                (PMBufCallBack)(&CNetStack::RecvPacket));
}


