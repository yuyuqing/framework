

#ifndef _DPDK_BB_TRAFFIC_DATA_H_
#define _DPDK_BB_TRAFFIC_DATA_H_


#include "dpdk_bb_traffic.h"


class CDataTraffic : CBaseTraffic
{
public :
    static WORD32 SendPacket(const CHAR *pBuf, WORD32 dwPayloadSize, WORD16 wDevID, WORD16 wFAPICellID);

public :
    CDataTraffic (const T_TrafficParam &rtParam);
    virtual ~CDataTraffic();

    virtual WORD32 Initialize();

protected :
#ifdef PICOCOM_FAPI
    pcxxInfo_s    m_tCallBackInfo;
#endif
};


#endif


