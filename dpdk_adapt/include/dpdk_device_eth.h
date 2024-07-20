

#ifndef _DPDK_DEVICE_ETH_H_
#define _DPDK_DEVICE_ETH_H_


#include "dpdk_net_interface.h"


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

    virtual WORD32 Start(WORD16 wQueueID);

    WORD32 GetPrimaryIPv4();

    BYTE * GetMacAddr();

    /* 检查目标MAC地址是否与本设备MAC匹配 */
    BOOL IsMatch(BYTE *pDstMac);

    /* 检查目标IP是否与本设备IP匹配; dwIPv4(网络字节序) */
    BOOL IsMatch(WORD32 dwVlanID, WORD32 dwIPv4);

    /* 检查目标IP是否与本设备IP匹配 */
    BOOL IsMatch(WORD32 dwVlanID, T_IPAddr &rtIPAddr);

    /* 执行地址重复检测 */
    WORD32 ProcDAD(CDevQueue *pQueue, CNetStack *pIcmpV6Stack);

    /* 查询路由器信息 */
    WORD32 ProcRS(CDevQueue *pQueue, CNetStack *pIcmpV6Stack);

    virtual VOID Dump();

protected :
    WORD32 InitIPConfig(T_DpdkEthDevJsonCfg &rtCfg);
    WORD32 InitLinkLocalIP(T_MacAddr &rtAddr, CIPTable &rIPTalbe);

protected :
    BYTE                   m_ucLinkType;
    BYTE                   m_ucVlanNum;
    BYTE                   m_ucIPNum;

    WORD32                 m_dwPrimaryIP;   /* 取第一个IPv4配置作为主IPv4地址 */
    T_IPv6Addr             m_tLinkLocalIP;  /* 链路本地IP作为主IPv6地址 */

    WORD32                 m_adwVlanID[MAX_DEV_IP_NUM];  /* 每个IP对应的VLANID */
    T_IPAddr               m_atIPAddr[MAX_DEV_IP_NUM];   /* 当前接口上的IP地址 */

    struct rte_ether_addr  m_tEthAddr;

    CDevNdpTable          *m_pDevNdpTable;  /* NDP邻居表 */
};


inline WORD32 CEthDevice::GetPrimaryIPv4()
{
    return m_dwPrimaryIP;
}


inline BYTE * CEthDevice::GetMacAddr()
{
    return (BYTE *)(m_tEthAddr.addr_bytes);
}


/* 检查目标MAC地址是否与本设备MAC匹配 */
inline BOOL CEthDevice::IsMatch(BYTE *pDstMac)
{
    return ((pDstMac[0] == m_tEthAddr.addr_bytes[0])
         && (pDstMac[1] == m_tEthAddr.addr_bytes[1])
         && (pDstMac[2] == m_tEthAddr.addr_bytes[2])
         && (pDstMac[3] == m_tEthAddr.addr_bytes[3])
         && (pDstMac[4] == m_tEthAddr.addr_bytes[4])
         && (pDstMac[5] == m_tEthAddr.addr_bytes[5]));
}


/* 检查目标IP是否与本设备IP匹配; dwIPv4(网络字节序) */
inline BOOL CEthDevice::IsMatch(WORD32 dwVlanID, WORD32 dwIPv4)
{
    for (WORD32 dwIndex = 0; dwIndex < m_ucIPNum; dwIndex++)
    {
        if ( (dwVlanID    == m_adwVlanID[dwIndex])
          && (E_IPV4_TYPE == m_atIPAddr[dwIndex].eType)
          && (dwIPv4      == m_atIPAddr[dwIndex].tIPv4.dwIPAddr))
        {
            return TRUE;
        }
    }

    return FALSE;
}


/* 检查目标IP是否与本设备IP匹配 */
inline BOOL CEthDevice::IsMatch(WORD32 dwVlanID, T_IPAddr &rtIPAddr)
{
    for (WORD32 dwIndex = 0; dwIndex < m_ucIPNum; dwIndex++)
    {
        if ( (dwVlanID == m_adwVlanID[dwIndex])
          && (rtIPAddr == m_atIPAddr[dwIndex]))
        {
            return TRUE;
        }
    }

    return FALSE;
}


#endif


