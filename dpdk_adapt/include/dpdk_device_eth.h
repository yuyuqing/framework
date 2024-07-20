

#ifndef _DPDK_DEVICE_ETH_H_
#define _DPDK_DEVICE_ETH_H_


#include "dpdk_net_interface.h"


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

    virtual WORD32 Start(WORD16 wQueueID);

    WORD32 GetPrimaryIPv4();

    BYTE * GetMacAddr();

    /* ���Ŀ��MAC��ַ�Ƿ��뱾�豸MACƥ�� */
    BOOL IsMatch(BYTE *pDstMac);

    /* ���Ŀ��IP�Ƿ��뱾�豸IPƥ��; dwIPv4(�����ֽ���) */
    BOOL IsMatch(WORD32 dwVlanID, WORD32 dwIPv4);

    /* ���Ŀ��IP�Ƿ��뱾�豸IPƥ�� */
    BOOL IsMatch(WORD32 dwVlanID, T_IPAddr &rtIPAddr);

    /* ִ�е�ַ�ظ���� */
    WORD32 ProcDAD(CDevQueue *pQueue, CNetStack *pIcmpV6Stack);

    /* ��ѯ·������Ϣ */
    WORD32 ProcRS(CDevQueue *pQueue, CNetStack *pIcmpV6Stack);

    virtual VOID Dump();

protected :
    WORD32 InitIPConfig(T_DpdkEthDevJsonCfg &rtCfg);
    WORD32 InitLinkLocalIP(T_MacAddr &rtAddr, CIPTable &rIPTalbe);

protected :
    BYTE                   m_ucLinkType;
    BYTE                   m_ucVlanNum;
    BYTE                   m_ucIPNum;

    WORD32                 m_dwPrimaryIP;   /* ȡ��һ��IPv4������Ϊ��IPv4��ַ */
    T_IPv6Addr             m_tLinkLocalIP;  /* ��·����IP��Ϊ��IPv6��ַ */

    WORD32                 m_adwVlanID[MAX_DEV_IP_NUM];  /* ÿ��IP��Ӧ��VLANID */
    T_IPAddr               m_atIPAddr[MAX_DEV_IP_NUM];   /* ��ǰ�ӿ��ϵ�IP��ַ */

    struct rte_ether_addr  m_tEthAddr;

    CDevNdpTable          *m_pDevNdpTable;  /* NDP�ھӱ� */
};


inline WORD32 CEthDevice::GetPrimaryIPv4()
{
    return m_dwPrimaryIP;
}


inline BYTE * CEthDevice::GetMacAddr()
{
    return (BYTE *)(m_tEthAddr.addr_bytes);
}


/* ���Ŀ��MAC��ַ�Ƿ��뱾�豸MACƥ�� */
inline BOOL CEthDevice::IsMatch(BYTE *pDstMac)
{
    return ((pDstMac[0] == m_tEthAddr.addr_bytes[0])
         && (pDstMac[1] == m_tEthAddr.addr_bytes[1])
         && (pDstMac[2] == m_tEthAddr.addr_bytes[2])
         && (pDstMac[3] == m_tEthAddr.addr_bytes[3])
         && (pDstMac[4] == m_tEthAddr.addr_bytes[4])
         && (pDstMac[5] == m_tEthAddr.addr_bytes[5]));
}


/* ���Ŀ��IP�Ƿ��뱾�豸IPƥ��; dwIPv4(�����ֽ���) */
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


/* ���Ŀ��IP�Ƿ��뱾�豸IPƥ�� */
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


