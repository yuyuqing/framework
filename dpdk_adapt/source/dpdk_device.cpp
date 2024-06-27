

#include "dpdk_device.h"
#include "base_log.h"


CDevQueue::CDevQueue(CBaseDevice *pDevice)
{
    m_pDev       = pDevice;
    m_dwDeviceID = pDevice->GetDeviceID();
    m_wPortID    = INVALID_WORD;
    m_wQueueID   = INVALID_WORD;
    m_pMbufPool  = NULL;
}


CDevQueue::~CDevQueue()
{
    m_pDev       = NULL;
    m_dwDeviceID = INVALID_DWORD;
    m_wPortID    = INVALID_WORD;
    m_wQueueID   = INVALID_WORD;
    m_pMbufPool  = NULL;
}


WORD32 CDevQueue::Initialize(WORD16 wPortID, WORD16 wQueueID)
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


CBaseDevice::CBaseDevice (E_DeviceType eType, const T_DeviceParam &rtParam)
    : m_rCentralMemPool(*(rtParam.pMemPool))
{
    m_dwDeviceID = rtParam.dwDeviceID;
    m_wPortID    = (WORD16)(rtParam.dwPortID);
    m_ucQueueNum = (BYTE)(rtParam.dwQueueNum);
    m_ucDevType  = (BYTE)(eType);
    m_wRxDescNum = RX_DESC_DEFAULT;
    m_wTxDescNum = TX_DESC_DEFAULT;

    memset(&m_tEthConf, 0x00, sizeof(m_tEthConf));
    memset(&m_tDevInfo, 0x00, sizeof(m_tDevInfo));
    memset(&m_tRxConf,  0x00, sizeof(m_tRxConf));
    memset(&m_tTxConf,  0x00, sizeof(m_tTxConf));

    assert(m_ucQueueNum < MAX_DEV_QUEUE_NUM);

    for (WORD32 dwIndex = 0; dwIndex < MAX_DEV_QUEUE_NUM; dwIndex++)
    {
        m_apQueue[dwIndex] = NULL;
    }
}


CBaseDevice::~CBaseDevice()
{
    for (WORD32 dwIndex = 0; dwIndex < MAX_DEV_QUEUE_NUM; dwIndex++)
    {
        if (NULL != m_apQueue[dwIndex])
        {
            delete m_apQueue[dwIndex];
            m_rCentralMemPool.Free((BYTE *)(m_apQueue[dwIndex]));

            m_apQueue[dwIndex] = NULL;
        }
    }

    m_dwDeviceID = INVALID_DWORD;
    m_wPortID    = INVALID_WORD;
    m_ucQueueNum = 0;
    m_ucDevType  = 0;

    memset(&m_tEthConf, 0x00, sizeof(m_tEthConf));
    memset(&m_tDevInfo, 0x00, sizeof(m_tDevInfo));
    memset(&m_tRxConf,  0x00, sizeof(m_tRxConf));
    memset(&m_tTxConf,  0x00, sizeof(m_tTxConf));
}


WORD32 CBaseDevice::Initialize(rte_eth_dev_cb_fn pFunc)
{
    TRACE_STACK("CBaseDevice::Initialize()");

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

    iSocketID = rte_eth_dev_socket_id(m_wPortID);
    if (SOCKET_ID_ANY == iSocketID)
    {
        iSocketID = 0;
    }

    dwResult = rte_eth_dev_configure(m_wPortID,
                                     m_ucQueueNum,
                                     m_ucQueueNum,
                                     &m_tEthConf);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    if (NULL != pFunc)
    {
        dwResult = rte_eth_dev_callback_register(m_wPortID,
                                                 RTE_ETH_EVENT_INTR_LSC,
                                                 pFunc,
                                                 NULL);
        if (SUCCESS != dwResult)
        {
            return FAIL;
        }
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

    /* ��������������RxConf���� */
    dwResult = SetRxConf();
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    /* ��������������TxConf���� */
    dwResult = SetTxConf();
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    for (WORD16 wIndex = 0; wIndex < m_ucQueueNum; wIndex++)
    {
        m_apQueue[wIndex] = CreateQueue(wIndex,
                                        iSocketID,
                                        m_wRxDescNum,
                                        m_wTxDescNum,
                                        m_tRxConf,
                                        m_tTxConf);
        if (NULL == m_apQueue[wIndex])
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

    return SUCCESS;
}


VOID CBaseDevice::Dump()
{
    TRACE_STACK("CBaseDevice::Dump()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "m_dwDeviceID : %d, m_wPortID : %d, m_ucQueueNum : %d, "
               "m_ucDevType : %d\n",
               m_dwDeviceID,
               m_wPortID,
               m_ucQueueNum,
               m_ucDevType);
}


CDevQueue * CBaseDevice::CreateQueue(WORD16  wQueueID,
                                     SWORD32 iSocketID,
                                     WORD16  wRxDescNum,
                                     WORD16  wTxDescNum,
                                     struct rte_eth_rxconf &rtRxConf,
                                     struct rte_eth_txconf &rtTxConf)
{
    TRACE_STACK("CBaseDevice::CreateQueue()");

    BYTE *pMem = m_rCentralMemPool.Malloc(sizeof(CDevQueue));
    if (NULL == pMem)
    {
        return NULL;
    }

    CDevQueue *pQueue = new (pMem) CDevQueue(this);

    WORD32 dwResult = pQueue->Initialize(m_wPortID, wQueueID);
    if (SUCCESS != dwResult)
    {
        delete pQueue;
        m_rCentralMemPool.Free(pMem);
        return NULL;
    }

    dwResult = rte_eth_rx_queue_setup(m_wPortID,
                                      wQueueID,
                                      wRxDescNum,
                                      iSocketID,
                                      &rtRxConf,
                                      pQueue->m_pMbufPool);
    if (SUCCESS != dwResult)
    {
        delete pQueue;
        m_rCentralMemPool.Free(pMem);
        return NULL;
    }

    dwResult = rte_eth_tx_queue_setup(m_wPortID,
                                      wQueueID,
                                      wTxDescNum,
                                      iSocketID,
                                      &rtTxConf);
    if (SUCCESS != dwResult)
    {
        delete pQueue;
        m_rCentralMemPool.Free(pMem);
        return NULL;
    }

    return pQueue;
}


VOID CFactoryDevice::Dump()
{
    TRACE_STACK("CFactoryDevice::Dump()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE, 
               "m_dwDefNum : %2d\n",
               m_dwDefNum);

    for (WORD32 dwIndex = 0; dwIndex < m_dwDefNum; dwIndex++)
    {
        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE, 
                   "pCreateFunc : %p, pResetFunc : %p, pDestroyFunc : %p, aucName : %s\n",
                   m_atDefInfo[dwIndex].pCreateFunc,
                   m_atDefInfo[dwIndex].pResetFunc,
                   m_atDefInfo[dwIndex].pDestroyFunc,
                   m_atDefInfo[dwIndex].aucName);
    }
}


