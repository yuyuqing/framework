

#include "dpdk_bb_traffic_eth.h"


DEFINE_TRAFFIC(CEthTraffic);


CEthTraffic::CEthTraffic (const T_TrafficParam &rtParam)
    : CBaseTraffic(E_ETH_TRAFFIC, rtParam)
{
}


CEthTraffic::~CEthTraffic()
{
}


