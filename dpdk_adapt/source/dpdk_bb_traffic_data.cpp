

#include "dpdk_bb_traffic_data.h"
#include "dpdk_bb_handler.h"

#include "base_log.h"


DEFINE_TRAFFIC(CDataTraffic);


WORD32 CDataTraffic::SendPacket(const CHAR *pBuf,
                                WORD32      dwPayloadSize,
                                WORD16      wDevID,
                                WORD16      wFAPICellID)
{
    return SUCCESS;
}


CDataTraffic::CDataTraffic (const T_TrafficParam &rtParam)
    : CBaseTraffic(E_FAPI_DATA_TRAFFIC, rtParam)
{
#ifdef PICOCOM_FAPI
    m_tCallBackInfo.readHandle  = NULL;
    m_tCallBackInfo.writeHandle = CDataTraffic::SendPacket;
#endif
}


CDataTraffic::~CDataTraffic()
{
}


WORD32 CDataTraffic::Initialize()
{
    TRACE_STACK("CDataTraffic::Initialize()");

    g_pBBHandler->RegistCellTraffic((WORD16)m_dwBindCellID,
                                    (WORD16)m_dwDeviceID,
                                    (WORD16)m_dwQueueID,
                                    (WORD16)m_dwFAPICellID,
                                    (CBaseTraffic *)this,
                                    NULL);

#ifdef PICOCOM_FAPI
    WORD32 dwResult = pcxxDataOpen(&m_tCallBackInfo,
                                   (WORD16)m_dwDeviceID,
                                   (WORD16)m_dwFAPICellID);
    assert(0 == dwResult);
#endif

    return SUCCESS;
}


