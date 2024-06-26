

#include "dpdk_device_eth.h"
#include "base_log.h"


DEFINE_DEVICE(CEthDevice);


CEthDevQueue::CEthDevQueue(CEthDevice &rEthDev)
    : m_rEthDev(rEthDev)
{
    m_dwDeviceID = m_rEthDev.GetDeviceID();
    m_wPortID    = INVALID_WORD;
    m_wQueueID   = INVALID_WORD;
    m_pMbufPool  = NULL;
}


CEthDevQueue::~CEthDevQueue()
{
    m_dwDeviceID = INVALID_DWORD;
    m_wPortID    = INVALID_WORD;
    m_wQueueID   = INVALID_WORD;
    m_pMbufPool  = NULL;
}


WORD32 CEthDevQueue::Initialize(WORD16 wPortID, WORD16 wQueueID)
{
    CHAR cPort  = ((CHAR)(wPortID))  + '0';
    CHAR cQueue = ((CHAR)(wQueueID)) + '0';
    CString<MBUF_NAME_LEN>  cMBufName("MBUF_POOL");

    cMBufName += '_';
    cMBufName += cPort;
    cMBufName += '_';
    cMBufName += cQueue;

    m_wPortID  = wPortID;
    m_wQueueID = wQueueID;

    m_pMbufPool = rte_pktmbuf_pool_create(
                    cMBufName.toChar(),
                    MBUF_NUM_PER_PORT,
                    MBUF_CACHE_SIZE,
                    MBUF_PRIV_SIZE,
                    MBUF_DATA_ROOM_SIZE,
                    rte_socket_id());
    if (NULL == m_pMbufPool)
    {
        return FAIL;
    }

    return SUCCESS;
}


CEthDevice::CEthDevice (const T_DeviceParam &rtParam)
    : CBaseDevice(E_ETH_DEVICE, rtParam)
{
    assert(m_ucQueueNum <= MAX_ETH_QUEUE_NUM);

    m_ucLinkType = E_ETH_LINK_ACCESS;
    m_ucVlanNum  = 0;
    m_ucIPNum    = 0;
    m_wRxDescNum = E_RX_DESC_DEFAULT;
    m_wTxDescNum = E_TX_DESC_DEFAULT;

    memset(&m_tEthConf, 0x00, sizeof(m_tEthConf));
    memset(&m_tDevInfo, 0x00, sizeof(m_tDevInfo));
    memset(&m_tEthAddr, 0x00, sizeof(m_tEthAddr));
    memset(&m_tRxConf,  0x00, sizeof(m_tRxConf));
    memset(&m_tTxConf,  0x00, sizeof(m_tTxConf));

    m_tEthConf.rxmode.mq_mode               = RTE_ETH_MQ_RX_RSS;
    m_tEthConf.rxmode.mtu                   = E_MAX_MTU;
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

    for (WORD32 dwIndex = 0; dwIndex < MAX_ETH_QUEUE_NUM; dwIndex++)
    {
        m_apEthQueue[dwIndex] = NULL;
    }
}


CEthDevice::~CEthDevice()
{
    for (WORD32 dwIndex = 0; dwIndex < MAX_ETH_QUEUE_NUM; dwIndex++)
    {
        if (NULL != m_apEthQueue[dwIndex])
        {
            delete m_apEthQueue[dwIndex];
            m_rCentralMemPool.Free((BYTE *)(m_apEthQueue[dwIndex]));

            m_apEthQueue[dwIndex] = NULL;
        }
    }
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

    WORD32   dwResult  = INVALID_DWORD;
    SWORD32  iSocketID = 0;

    if (!rte_eth_dev_is_valid_port(m_wPortID))
    {
        return FAIL;
    }

    dwResult = rte_eth_dev_info_get(m_wPortID, &m_tDevInfo);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    if ( (m_ucQueueNum > m_tDevInfo.max_rx_queues)
      || (m_ucQueueNum > m_tDevInfo.max_tx_queues))
    {
        return FAIL;
    }

    dwResult = rte_eth_macaddr_get(m_wPortID, &m_tEthAddr);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    iSocketID = rte_eth_dev_socket_id(m_wPortID);
    if (SOCKET_ID_ANY == iSocketID)
    {
        iSocketID = 0;
    }

    /* 设置配置参数... */
    //m_tEthConf.rxmode.mtu      = m_atDevInfo[m_wPortNum].max_mtu;
    //m_tEthConf.rxmode.offloads = m_atDevInfo[m_wPortNum].rx_offload_capa;
    //m_tEthConf.rx_adv_conf.rss_conf.rss_hf &= m_atDevInfo[m_wPortNum].flow_type_rss_offloads;

    dwResult = rte_eth_dev_configure(m_wPortID,
                                     m_ucQueueNum,
                                     m_ucQueueNum,
                                     &m_tEthConf);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    dwResult = rte_eth_dev_adjust_nb_rx_tx_desc(m_wPortID,
                                                &m_wRxDescNum,
                                                &m_wTxDescNum);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    m_tRxConf          = m_tDevInfo.default_rxconf;
    m_tTxConf          = m_tDevInfo.default_txconf;
    m_tRxConf.offloads = m_tEthConf.rxmode.offloads;
    m_tTxConf.offloads = m_tEthConf.txmode.offloads;

    for (WORD16 wIndex = 0; wIndex < m_ucQueueNum; wIndex++)
    {
        m_apEthQueue[wIndex] = CreateEthQueue(wIndex, iSocketID);
        if (NULL == m_apEthQueue[wIndex])
        {
            return FAIL;
        }
    }

    dwResult = rte_eth_dev_start(m_wPortID);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    dwResult = rte_eth_promiscuous_enable(m_wPortID);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    /* TBD : 初始化ARP/VLAN/IP表项 */

    return SUCCESS;
}


CEthDevQueue * CEthDevice::CreateEthQueue(WORD16 wQueueID, SWORD32 iSocketID)
{
    TRACE_STACK("CEthDevice::CreateEthQueue()");

    BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CEthDevQueue));
    if (NULL == pMem)
    {
        return NULL;
    }

    CEthDevQueue *pQueue = new (pMem) CEthDevQueue(*this);

    WORD32 dwResult = pQueue->Initialize(m_wPortID, wQueueID);
    if (SUCCESS != dwResult)
    {
        delete pQueue;
        m_rCentralMemPool.Free(pMem);
        return NULL;
    }

    dwResult = rte_eth_rx_queue_setup(m_wPortID,
                                      wQueueID,
                                      m_wRxDescNum,
                                      iSocketID,
                                      &m_tRxConf,
                                      pQueue->m_pMbufPool);
    if (SUCCESS != dwResult)
    {
        delete pQueue;
        m_rCentralMemPool.Free(pMem);
        return NULL;
    }

    dwResult = rte_eth_tx_queue_setup(m_wPortID,
                                      wQueueID,
                                      m_wTxDescNum,
                                      iSocketID,
                                      &m_tTxConf);
    if (SUCCESS != dwResult)
    {
        delete pQueue;
        m_rCentralMemPool.Free(pMem);
        return NULL;
    }

    return pQueue;
}


