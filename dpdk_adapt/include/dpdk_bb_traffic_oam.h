

#ifndef _DPDK_BB_TRAFFIC_OAM_H_
#define _DPDK_BB_TRAFFIC_OAM_H_


#include "dpdk_bb_traffic.h"


class COamTraffic : public CBaseTraffic
{
public :
    COamTraffic (const T_TrafficParam &rtParam);
    virtual ~COamTraffic();
};


#endif


