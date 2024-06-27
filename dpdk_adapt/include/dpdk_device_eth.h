

#ifndef _DPDK_DEVICE_ETH_H_
#define _DPDK_DEVICE_ETH_H_


#include "dpdk_device.h"


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

protected :
    BYTE                   m_ucLinkType;
    BYTE                   m_ucVlanNum;
    BYTE                   m_ucIPNum;

    struct rte_ether_addr  m_tEthAddr;
};


#endif


