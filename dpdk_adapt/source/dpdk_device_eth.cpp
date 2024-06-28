

#include "dpdk_device_eth.h"
#include "dpdk_mgr.h"
#include "base_log.h"


DEFINE_DEVICE(CEthDevice);


CEthDevice::CEthDevice (const T_DeviceParam &rtParam)
    : CBaseDevice(E_ETH_DEVICE, rtParam)
{
    m_ucLinkType = E_ETH_LINK_ACCESS;
    m_ucVlanNum  = 0;
    m_ucIPNum    = 0;

    memset(&m_tEthAddr, 0x00, sizeof(m_tEthAddr));

    m_tEthConf.rxmode.mq_mode               = RTE_ETH_MQ_RX_RSS;
#if RTE_VERSION >= RTE_VERSION_NUM(23, 07, 0, 0)
    m_tEthConf.rxmode.mtu                   = RTE_ETHER_MAX_LEN;
#else
    m_tEthConf.rxmode.max_rx_pkt_len        = RTE_ETHER_MAX_LEN;
#endif
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
}


CEthDevice::~CEthDevice()
{
}


/* 调用派生类设置RxConf属性 */
WORD32 CEthDevice::SetRxConf()
{
    return SUCCESS;
}


/* 调用派生类设置TxConf属性 */
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
        /* 配置错误 */
        return FAIL;
    }

    m_ucLinkType = (BYTE)(ptCfg->dwLinkType);
    m_ucVlanNum  = (BYTE)(ptCfg->dwVlanNum);
    m_ucIPNum    = (BYTE)(ptCfg->dwIpNum);

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

    /* TBD : 初始化ARP表项 */

    /* 注册本地IP表项 */
    dwResult = rIPTalbe.RegistIP(*ptCfg);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    /* 注册本地VLAN表项 */
    dwResult = rVlanTable.RegistVlan(*ptCfg);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    return SUCCESS;
}


