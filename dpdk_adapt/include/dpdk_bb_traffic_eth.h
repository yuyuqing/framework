

#ifndef _DPDK_BB_TRAFFIC_ETH_H_
#define _DPDK_BB_TRAFFIC_ETH_H_


#include "dpdk_bb_traffic.h"


class CEthTraffic : public CBaseTraffic
{
public :
    CEthTraffic (const T_TrafficParam &rtParam);
    virtual ~CEthTraffic();
};


#endif


