

#include "dpdk_bb_traffic_ctrl.h"
#include "dpdk_bb_handler.h"

#include "base_log.h"


DEFINE_TRAFFIC(CCtrlTraffic);


WORD32 CCtrlTraffic::RecvPacket(const CHAR *pBuf,
                                WORD32      dwPayloadSize,
                                WORD16      wDevID,
                                WORD16      wFAPICellID)
{
    return SUCCESS;
}


WORD32 CCtrlTraffic::SendPacket(const CHAR *pBuf,
                                WORD32      dwPayloadSize,
                                WORD16      wDevID,
                                WORD16      wFAPICellID)
{
    return SUCCESS;
}


CCtrlTraffic::CCtrlTraffic (const T_TrafficParam &rtParam)
    : CBaseTraffic(E_FAPI_CTRL_TRAFFIC, rtParam)
{
#ifdef PICOCOM_FAPI
    m_tCallBackInfo.readHandle  = CCtrlTraffic::RecvPacket;
    m_tCallBackInfo.writeHandle = CCtrlTraffic::SendPacket;
#endif
}


CCtrlTraffic::~CCtrlTraffic()
{
}


WORD32 CCtrlTraffic::Initialize()
{
    TRACE_STACK("CCtrlTraffic::Initialize()");

    g_pBBHandler->RegistCellTraffic((WORD16)m_dwBindCellID,
                                    (WORD16)m_dwDeviceID,
                                    (WORD16)m_dwQueueID,
                                    (WORD16)m_dwFAPICellID,
                                    NULL,
                                    (CBaseTraffic *)this);

#ifdef PICOCOM_FAPI
    WORD32 dwResult = pcxxCtrlOpen(&m_tCallBackInfo,
                                   (WORD16)m_dwDeviceID,
                                   (WORD16)m_dwFAPICellID);
    assert(0 == dwResult);
#endif

    return SUCCESS;
}


