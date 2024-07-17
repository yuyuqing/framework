

#ifndef _DPDK_DEVICE_ETH_H_
#define _DPDK_DEVICE_ETH_H_


#include "dpdk_device.h"
#include "dpdk_net_ip_table.h"


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

    WORD32 GetPrimaryIPv4();

    BYTE * GetMacAddr();

    /* dwIPv4是否为本设备的IP地址(dwIPv4 : 按网络字节序) */
    BOOL IsMatch(WORD32 dwIPv4);

protected :
    WORD32 InitIPConfig(T_DpdkEthDevJsonCfg &rtCfg);
    WORD32 InitLinkLocalIP(T_MacAddr &rtAddr);

protected :
    BYTE                   m_ucLinkType;
    BYTE                   m_ucVlanNum;
    BYTE                   m_ucIPNum;

    WORD32                 m_dwPrimaryIP;   /* 取第一个IPv4配置作为主IPv4地址 */
    T_IPv6Addr             m_tLinkLocalIP;  /* 链路本地IP作为主IPv6地址 */

    T_IPAddr               m_atIPAddr[MAX_DEV_IP_NUM];

    struct rte_ether_addr  m_tEthAddr;
};


inline WORD32 CEthDevice::GetPrimaryIPv4()
{
    return m_dwPrimaryIP;
}


inline BYTE * CEthDevice::GetMacAddr()
{
    return (BYTE *)(m_tEthAddr.addr_bytes);
}


/* dwIPv4是否为本设备的IP地址(dwIPv4 : 按网络字节序) */
inline BOOL CEthDevice::IsMatch(WORD32 dwIPv4)
{
    for (WORD32 dwIndex = 0; dwIndex < m_ucIPNum; dwIndex++)
    {
        if ( (E_IPV4_TYPE == m_atIPAddr[dwIndex].eType)
          && (dwIPv4 == m_atIPAddr[dwIndex].tIPv4.dwIPAddr))
        {
            return TRUE;
        }
    }

    return FALSE;
}


#endif


