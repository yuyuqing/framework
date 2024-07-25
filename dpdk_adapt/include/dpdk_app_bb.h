

#ifndef _DPDK_APP_BB_H_
#define _DPDK_APP_BB_H_


#include "dpdk_mgr.h"
#include "dpdk_bb_traffic_ctrl.h"
#include "dpdk_bb_traffic_data.h"

#include "base_app_cntrl.h"


class CBBApp : public CAppInterface
{
public :
    CBBApp ();
    virtual ~CBBApp();

    WORD32 Init();
    WORD32 DeInit();
    WORD32 Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen);

    WORD32 Polling();

    CCtrlTraffic * GetCtrlTraffic();
    CDataTraffic * GetDataTraffic();

protected :
    WORD16         m_wCellID;      /* 高层逻辑小区ID */
    WORD16         m_wDeviceID;    /* 设备ID */
    WORD16         m_wFapiCellID;  /* 设备内物理小区ID */

    CCtrlTraffic  *m_pCtrlTraffic;
    CDataTraffic  *m_pDataTraffic;
};


inline CCtrlTraffic * CBBApp::GetCtrlTraffic()
{
    return m_pCtrlTraffic;
}


inline CDataTraffic * CBBApp::GetDataTraffic()
{
    return m_pDataTraffic;
}


#endif


