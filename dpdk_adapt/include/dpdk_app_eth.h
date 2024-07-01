

#ifndef _DPDK_APP_ETH_H_
#define _DPDK_APP_ETH_H_


#include "dpdk_mgr.h"

#include "base_app_cntrl.h"


class CEthApp : public CAppInterface
{
public :
    CEthApp ();
    virtual ~CEthApp();

    WORD32 Init();
    WORD32 DeInit();
    WORD32 Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen);

    WORD32 Polling();

    CDevQueue * GetQueue();

protected :
    WORD32      m_dwDeviceID;
    WORD16      m_wPortID;
    WORD16      m_wQueueID;

    CDevQueue  *m_pQueue;
};


inline CDevQueue * CEthApp::GetQueue()
{
    return m_pQueue;
}


#endif


