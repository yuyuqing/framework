

#include "dpdk_device_bb.h"
#include "base_log.h"


DEFINE_DEVICE(CBBDevice);


SWORD32 CBBDevice::LsiEventCallBack(WORD16          wPortID,
                                    E_EthEventType  eType,
                                    VOID           *pParam,
                                    VOID           *pRetParam)
{
    struct rte_eth_link tLink;

    SWORD32 iResult = rte_eth_link_get(wPortID, &tLink);
    if (iResult < 0)
    {
        return iResult;
    }

    return 0;
}


CBBDevice::CBBDevice (const T_DeviceParam &rtParam)
    : CBaseDevice(E_BB_DEVICE, rtParam)
{
    m_tEthConf.rxmode.mq_mode        = RTE_ETH_MQ_RX_NONE;
#if RTE_VERSION >= RTE_VERSION_NUM(21, 11, 1, 0)
    m_tEthConf.rxmode.mtu            = RTE_ETHER_MAX_LEN;
#else
    m_tEthConf.rxmode.max_rx_pkt_len = RTE_ETHER_MAX_LEN;
#endif
    m_tEthConf.txmode.mq_mode        = RTE_ETH_MQ_TX_NONE;
    m_tEthConf.txmode.offloads       = RTE_ETH_TX_OFFLOAD_MULTI_SEGS;

    m_tEthConf.rx_adv_conf.vmdq_dcb_conf.nb_queue_pools      = RTE_ETH_16_POOLS;
    m_tEthConf.rx_adv_conf.vmdq_dcb_conf.enable_default_pool = 1;
    m_tEthConf.rx_adv_conf.vmdq_dcb_conf.default_pool        = 0;
    m_tEthConf.rx_adv_conf.vmdq_dcb_conf.nb_pool_maps        = 0;
}


CBBDevice::~CBBDevice()
{
}


/* 调用派生类设置RxConf属性 */
WORD32 CBBDevice::SetRxConf()
{
    return SUCCESS;
}


/* 调用派生类设置TxConf属性 */
WORD32 CBBDevice::SetTxConf()
{
    return SUCCESS;
}


WORD32 CBBDevice::Initialize()
{
    TRACE_STACK("CBBDevice::Initialize()");

    T_DpdkBBDevJsonCfg *ptCfg = CBaseConfigFile::GetInstance()->GetBBDevJsonCfg(m_dwDeviceID);
    if (NULL == ptCfg)
    {
        /* 配置错误 */
        return FAIL;
    }

    WORD32 dwResult = CBaseDevice::Initialize(CBBDevice::LsiEventCallBack);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    return SUCCESS;
}


