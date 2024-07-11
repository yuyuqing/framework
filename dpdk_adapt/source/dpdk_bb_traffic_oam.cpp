

#include "dpdk_bb_traffic_oam.h"


DEFINE_TRAFFIC(COamTraffic);


COamTraffic::COamTraffic (const T_TrafficParam &rtParam)
    : CBaseTraffic(E_OAM_TRAFFIC, rtParam)
{
}


COamTraffic::~COamTraffic()
{
}


