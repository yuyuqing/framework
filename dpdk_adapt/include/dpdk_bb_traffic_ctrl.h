

#ifndef _DPDK_BB_TRAFFIC_CTRL_H_
#define _DPDK_BB_TRAFFIC_CTRL_H_


#include "dpdk_bb_traffic.h"

#ifdef PICOCOM_FAPI
#include "pcxx_ipc.h"
#endif


class CCtrlTraffic : public CBaseTraffic
{
public :
    static WORD32 RecvPacket(const CHAR *pBuf, WORD32 dwPayloadSize, WORD16 wDevID, WORD16 wFAPICellID);
    static WORD32 SendPacket(const CHAR *pBuf, WORD32 dwPayloadSize, WORD16 wDevID, WORD16 wFAPICellID);

public :
    CCtrlTraffic (const T_TrafficParam &rtParam);
    virtual ~CCtrlTraffic();

    virtual WORD32 Initialize();

protected :
#ifdef PICOCOM_FAPI
    pcxxInfo_s    m_tCallBackInfo;
#endif
};


#endif


