

#ifndef _DPDK_DEVICE_ETH_H_
#define _DPDK_DEVICE_ETH_H_


#include "dpdk_device.h"
#include "dpdk_net_ip_table.h"


typedef enum tagE_EthLinkType
{
    E_ETH_LINK_ACCESS = 0,    /* ������������Access�˿�(�շ����Ĳ���VLAN��ǩ) */
    E_ETH_LINK_TRUNK,         /* ������������Trunk�˿�(�շ����ı����VLAN��ǩ) */
    E_ETH_LINK_HYBRID,        /* ������������Hybrid�˿�(�շ����Ŀ���Я��VLAN��ǩ, Ҳ���Բ�Я��VLAN��ǩ) */
}E_EthLinkType;


class CEthDevice : public CBaseDevice
{
public :
    CEthDevice (const T_DeviceParam &rtParam);
    virtual ~CEthDevice();

    /* ��������������RxConf���� */
    virtual WORD32 SetRxConf();

    /* ��������������TxConf���� */
    virtual WORD32 SetTxConf();

    virtual WORD32 Initialize();

    WORD32 GetPrimaryIPv4();

    BYTE * GetMacAddr();

    /* dwIPv4�Ƿ�Ϊ���豸��IP��ַ(dwIPv4 : �������ֽ���) */
    BOOL IsMatch(WORD32 dwIPv4);

protected :
    WORD32 InitIPConfig(T_DpdkEthDevJsonCfg &rtCfg);
    WORD32 InitLinkLocalIP(T_MacAddr &rtAddr);

protected :
    BYTE                   m_ucLinkType;
    BYTE                   m_ucVlanNum;
    BYTE                   m_ucIPNum;

    WORD32                 m_dwPrimaryIP;   /* ȡ��һ��IPv4������Ϊ��IPv4��ַ */
    T_IPv6Addr             m_tLinkLocalIP;  /* ��·����IP��Ϊ��IPv6��ַ */

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


/* dwIPv4�Ƿ�Ϊ���豸��IP��ַ(dwIPv4 : �������ֽ���) */
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


