

#include "dpdk_bb_handler.h"
#include "dpdk_bb_traffic_eth.h"

#include "base_log.h"


DEFINE_TRAFFIC(CEthTraffic);


CEthTraffic::CEthTraffic (const T_TrafficParam &rtParam)
    : CBaseTraffic(E_ETH_TRAFFIC, rtParam)
{
}


CEthTraffic::~CEthTraffic()
{
}


WORD32 CEthTraffic::Initialize()
{
    TRACE_STACK("CEthTraffic::Initialize()");

    g_pBBHandler->SetEthTraffic((CBaseTraffic *)this);

    return SUCCESS;
}


