

#include "dpdk_device_eth.h"
#include "dpdk_mgr.h"
#include "base_log.h"


DEFINE_DEVICE(CEthDevice);


CEthDevice::CEthDevice (const T_DeviceParam &rtParam)
    : CBaseDevice(E_ETH_DEVICE, rtParam)
{
    m_ucLinkType  = E_ETH_LINK_ACCESS;
    m_ucVlanNum   = 0;
    m_ucIPNum     = 0;
    m_dwPrimaryIP = 0;

    memset(m_aucIPType, 0x00, sizeof(m_aucIPType));
    memset(m_atIPAddr,  0x00, sizeof(m_atIPAddr));
    memset(&m_tEthAddr, 0x00, sizeof(m_tEthAddr));

#if RTE_VERSION >= RTE_VERSION_NUM(21, 11, 1, 0)
    m_tEthConf.rxmode.mq_mode               = RTE_ETH_MQ_RX_RSS;
    m_tEthConf.rxmode.mtu                   = RTE_ETHER_MAX_LEN;
    m_tEthConf.rxmode.offloads              = RTE_ETH_RX_OFFLOAD_UDP_CKSUM
                                            | RTE_ETH_RX_OFFLOAD_TCP_CKSUM;
    m_tEthConf.txmode.mq_mode               = RTE_ETH_MQ_TX_NONE;
    m_tEthConf.txmode.offloads              = RTE_ETH_TX_OFFLOAD_UDP_CKSUM
                                            | RTE_ETH_TX_OFFLOAD_TCP_CKSUM;
    m_tEthConf.rx_adv_conf.rss_conf.rss_key = NULL;
    m_tEthConf.rx_adv_conf.rss_conf.rss_hf  = RTE_ETH_RSS_IPV4
                                            | RTE_ETH_RSS_NONFRAG_IPV4_TCP
                                            | RTE_ETH_RSS_IPV6
                                            | RTE_ETH_RSS_NONFRAG_IPV6_TCP;
#else
    m_tEthConf.rxmode.mq_mode               = ETH_MQ_RX_RSS;
    m_tEthConf.rxmode.max_rx_pkt_len        = RTE_ETHER_MAX_LEN;
    m_tEthConf.rxmode.offloads              = DEV_RX_OFFLOAD_UDP_CKSUM
                                            | DEV_RX_OFFLOAD_TCP_CKSUM;
    m_tEthConf.txmode.mq_mode               = ETH_MQ_TX_NONE;
    m_tEthConf.txmode.offloads              = DEV_TX_OFFLOAD_UDP_CKSUM
                                            | DEV_TX_OFFLOAD_TCP_CKSUM;
    m_tEthConf.rx_adv_conf.rss_conf.rss_key = NULL;
    m_tEthConf.rx_adv_conf.rss_conf.rss_hf  = ETH_RSS_IPV4
                                            | ETH_RSS_NONFRAG_IPV4_TCP
                                            | ETH_RSS_IPV6
                                            | ETH_RSS_NONFRAG_IPV6_TCP;
#endif
}


CEthDevice::~CEthDevice()
{
}


/* ��������������RxConf���� */
WORD32 CEthDevice::SetRxConf()
{
    return SUCCESS;
}


/* ��������������TxConf���� */
WORD32 CEthDevice::SetTxConf()
{
    return SUCCESS;
}


WORD32 CEthDevice::Initialize()
{
    TRACE_STACK("CEthDevice::Initialize()");

    T_DpdkEthDevJsonCfg *ptCfg = CBaseConfigFile::GetInstance()->GetEthDevJsonCfg(m_dwDeviceID);
    if (NULL == ptCfg)
    {
        /* ���ô��� */
        return FAIL;
    }

    m_ucLinkType  = (BYTE)(ptCfg->dwLinkType);
    m_ucVlanNum   = (BYTE)(ptCfg->dwVlanNum);
    m_ucIPNum     = (BYTE)(ptCfg->dwIpNum);
    m_dwPrimaryIP = FetchPrimaryIP(*ptCfg);

    CIPTable   &rIPTalbe   = g_pDpdkMgr->GetIPTable();
    CVlanTable &rVlanTable = g_pDpdkMgr->GetVlanTable();

    WORD32 dwResult = CBaseDevice::Initialize(NULL);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    dwResult = rte_eth_macaddr_get(m_wPortID, &m_tEthAddr);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    T_MacAddr tMacAddr;
    memcpy(tMacAddr.aucMacAddr, m_tEthAddr.addr_bytes, ARP_MAC_ADDR_LEN);

    /* ע�᱾��IP���� */
    dwResult = rIPTalbe.RegistIP(*ptCfg);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    /* ע�᱾��VLAN���� */
    dwResult = rVlanTable.RegistVlan(*ptCfg);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    return SUCCESS;
}


WORD32 CEthDevice::FetchPrimaryIP(T_DpdkEthDevJsonCfg &rtCfg)
{
    WORD32   dwIPNum     = rtCfg.dwIpNum;
    WORD32   dwPrimaryIP = 0;

    for (WORD32 dwIndex = 0; dwIndex < dwIPNum; dwIndex++)
    {
        T_DpdkEthIPJsonCfg &rtIPCfg = rtCfg.atIP[dwIndex];

        if (E_IPV4_TYPE == rtIPCfg.dwIPType)
        {
            m_aucIPType[dwIndex] = (BYTE)(E_IPV4_TYPE);
            m_atIPAddr[dwIndex].SetIPv4(rtIPCfg.aucIpv4Addr);

            /* ȡ��һ������IPv4��ַ��Ϊ��IP */
            if (0 == dwPrimaryIP)
            {
                dwPrimaryIP = m_atIPAddr[dwIndex].dwIPv4;
            }
        }
        else
        {
            m_aucIPType[dwIndex] = (BYTE)(E_IPV6_TYPE);
            m_atIPAddr[dwIndex].SetIPv6(rtIPCfg.aucIpv6Addr);
        }
    }

    return dwPrimaryIP;
}


