

#ifndef _DPDK_APP_ETH_H_
#define _DPDK_APP_ETH_H_


#include "dpdk_mgr.h"

#include "base_app_cntrl.h"


typedef struct rte_ether_hdr  T_EthHead;
typedef struct rte_vlan_hdr   T_VlanHead;


class CEthApp : public CAppInterface
{
public :
    static WORD32 RecvPacket(VOID   *pArg,
                             WORD32  dwDevID,
                             WORD32  dwPortID,
                             WORD32  dwQueueID,
                             T_MBuf *pMBuf);

public :
    CEthApp ();
    virtual ~CEthApp();

    WORD32 Init();
    WORD32 DeInit();
    WORD32 Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen);

    WORD32 Polling();

protected :
    WORD32 RecvVlanPacket(T_MBuf *pMBuf, T_EthHead *ptEthHead);
    WORD32 RecvArpPacket(T_MBuf *pMBuf, T_EthHead *ptEthHead);
    WORD32 RecvIpv4Packet(T_MBuf *pMBuf, T_EthHead *ptEthHead);
    WORD32 RecvIpv6Packet(T_MBuf *pMBuf, T_EthHead *ptEthHead);

protected :
    WORD32      m_dwDeviceID;
    WORD16      m_wPortID;
    WORD16      m_wQueueID;

    CDevQueue  *m_pQueue;
};


#endif


