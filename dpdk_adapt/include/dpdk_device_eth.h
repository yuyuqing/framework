

#ifndef _DPDK_DEVICE_ETH_H_
#define _DPDK_DEVICE_ETH_H_


#include "dpdk_device.h"


typedef enum tagE_EthLinkType
{
    E_ETH_LINK_ACCESS = 0,    /* 上联到交换机Access端口(收发报文不带VLAN标签) */
    E_ETH_LINK_TRUNK,         /* 上联到交换机Trunk端口(收发报文必须带VLAN标签) */
    E_ETH_LINK_HYBRID,        /* 上联到交换机Hybrid端口(收发报文可以携带VLAN标签, 也可以不携带VLAN标签) */
}E_EthLinkType;


class CEthDevice : public CBaseDevice
{
public :
    CEthDevice (const T_DeviceParam &rtParam);
    virtual ~CEthDevice();

    /* 调用派生类设置RxConf属性 */
    virtual WORD32 SetRxConf();

    /* 调用派生类设置TxConf属性 */
    virtual WORD32 SetTxConf();

    virtual WORD32 Initialize();

protected :
    BYTE                   m_ucLinkType;
    BYTE                   m_ucVlanNum;
    BYTE                   m_ucIPNum;

    struct rte_ether_addr  m_tEthAddr;
};


#endif


