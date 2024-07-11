

#include "dpdk_device_bb.h"
#include "dpdk_mgr.h"

#include "base_log.h"
#include "base_sort.h"


DEFINE_DEVICE(CBBDevice);


CBaseTraffic * GetBBTraffic(E_TrafficType eType, WORD32 dwBindCellID)
{
    if (NULL == g_pDpdkMgr)
    {
        return NULL;
    }

    WORD32       dwBBNum = 0;
    CBaseDevice *apDevice[MAX_DEV_PORT_NUM] = {NULL, };

    dwBBNum = g_pDpdkMgr->GetDevices(E_BB_DEVICE, apDevice);
    if (0 == dwBBNum)
    {
        return NULL;
    }

    for (WORD32 dwIndex = 0; dwIndex < dwBBNum; dwIndex++)
    {
        CBBDevice *pBBDev = (CBBDevice *)(apDevice[dwIndex]);
        if (NULL == pBBDev)
        {
            continue ;
        }

        return pBBDev->FindTraffic(eType, dwBindCellID);
    }

    return NULL;
}


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


WORD32 CBBDevice::RecvPacket(const CHAR *pBuf,
                             WORD32      dwPayloadSize,
                             WORD16      wDevID,
                             WORD16      wFAPICellID)
{
    TRACE_STACK("CBBDevice::RecvPacket()");
    return SUCCESS;
}


WORD32 CBBDevice::SendPacket(const CHAR *pBuf,
                             WORD32      dwPayloadSize,
                             WORD16      wDevID,
                             WORD16      wFAPICellID)
{
    TRACE_STACK("CBBDevice::SendPacket()");
    return SUCCESS;
}


CBBDevice::CBBDevice (const T_DeviceParam &rtParam)
    : CBaseDevice(E_BB_DEVICE, rtParam)
{
#if RTE_VERSION >= RTE_VERSION_NUM(21, 11, 1, 0)
    m_tEthConf.rxmode.mq_mode        = RTE_ETH_MQ_RX_NONE;
    m_tEthConf.rxmode.mtu            = RTE_ETHER_MAX_LEN;
    m_tEthConf.txmode.mq_mode        = RTE_ETH_MQ_TX_NONE;
    m_tEthConf.txmode.offloads       = RTE_ETH_TX_OFFLOAD_MULTI_SEGS;
    m_tEthConf.rx_adv_conf.vmdq_dcb_conf.nb_queue_pools = RTE_ETH_16_POOLS;
#else
    m_tEthConf.rxmode.mq_mode        = ETH_MQ_RX_NONE;
    m_tEthConf.rxmode.max_rx_pkt_len = RTE_ETHER_MAX_LEN;
    m_tEthConf.txmode.mq_mode        = ETH_MQ_TX_NONE;
    m_tEthConf.txmode.offloads       = DEV_TX_OFFLOAD_MULTI_SEGS;
    m_tEthConf.rx_adv_conf.vmdq_dcb_conf.nb_queue_pools = ETH_16_POOLS;
#endif

    m_tEthConf.rx_adv_conf.vmdq_dcb_conf.enable_default_pool = 1;
    m_tEthConf.rx_adv_conf.vmdq_dcb_conf.default_pool        = 0;
    m_tEthConf.rx_adv_conf.vmdq_dcb_conf.nb_pool_maps        = 0;

#ifdef PICOCOM_FAPI
    m_tCallBackInfo.readHandle  = CBBDevice::RecvPacket;
    m_tCallBackInfo.writeHandle = CBBDevice::SendPacket;
#endif
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

    /* 初始化DPDK设备及队列 */
    WORD32 dwResult = CBaseDevice::Initialize(CBBDevice::LsiEventCallBack);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

#ifdef PICOCOM_FAPI
    WORD32 dwPortID = pc802_get_port_id(m_dwDeviceID);
    assert(dwPortID == m_wPortID);
#endif

    /* 初始化FAPI_Traffic */
    dwResult = InitTraffic(*ptCfg);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

#ifdef PICOCOM_FAPI
    dwResult = pcxxCtrlOpen(&m_tCallBackInfo,
                            (WORD16)m_dwDeviceID,
                            MAC_NMM_LEGACY_CELL_INDEX);
    assert(0 == dwResult);
#endif

    dwResult = DevStart();
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    return SUCCESS;
}


WORD32 CBBDevice::InitTraffic(T_DpdkBBDevJsonCfg &rtCfg)
{
    TRACE_STACK("CBBDevice::InitTraffic()");

    WORD32 dwResult    = INVALID_DWORD;
    WORD32 dwQueueID   = rtCfg.dwQueueID;
    WORD32 dwTrafficID = INVALID_DWORD;

    CFactoryTraffic *pFactory = CFactoryTraffic::GetInstance();

    for (WORD32 dwIndex = 0; dwIndex < rtCfg.dwFapiTrafficNum; dwIndex++)
    {
        T_TrafficInfo              *ptTrafficInfo = NULL;
        T_ProductDefInfo           *ptDefInfo     = NULL;
        T_DpdkBBFapiTrafficJsonCfg &rtTrafficCfg  = rtCfg.atFAPITraffic[dwIndex];

        dwTrafficID   = rtTrafficCfg.dwTrafficID;
        ptTrafficInfo = FindTrafficInfo(dwTrafficID);
        if (NULL != ptTrafficInfo)
        {
            continue ;
        }

        ptDefInfo = pFactory->FindDef(rtTrafficCfg.aucType);
        if (NULL == ptDefInfo)
        {
            /* 配置错误 */
            return FAIL;
        }

        ptTrafficInfo = CreateInfo(dwQueueID, rtTrafficCfg, ptDefInfo);
        if (NULL == ptTrafficInfo)
        {
            /* 创建Traffic失败 */
            return FAIL;
        }
    }

    /* 从小到大排序 */
    HeapSort<T_TrafficInfo, WORD32> (m_atTrafficInfo, m_dwTrafficNum,
                                     (&T_TrafficInfo::dwTrafficID),
                                     (PComparePFunc<T_TrafficInfo, WORD32>)(& GreaterV<T_TrafficInfo, WORD32>));

    for (WORD32 dwIndex = 0; dwIndex < m_dwTrafficNum; dwIndex++)
    {
        m_atTrafficInfo[dwIndex].pMem = m_rCentralMemPool.Malloc(m_atTrafficInfo[dwIndex].dwMemSize);

        T_TrafficParam tParam;
        tParam.dwDeviceID   = m_atTrafficInfo[dwIndex].dwDeviceID;
        tParam.dwPortID     = m_atTrafficInfo[dwIndex].dwPortID;
        tParam.dwQueueID    = m_atTrafficInfo[dwIndex].dwQueueID;
        tParam.dwTrafficID  = m_atTrafficInfo[dwIndex].dwTrafficID;
        tParam.dwFAPICellID = m_atTrafficInfo[dwIndex].dwFAPICellID;
        tParam.dwBindCellID = m_atTrafficInfo[dwIndex].dwBindCellID;

        m_atTrafficInfo[dwIndex].pTraffic = (CBaseTraffic *)((*(m_atTrafficInfo[dwIndex].pCreateFunc)) (m_atTrafficInfo[dwIndex].pMem, &tParam));
        dwResult = m_atTrafficInfo[dwIndex].pTraffic->Initialize();
        if (SUCCESS != dwResult)
        {
            /* 初始化Traffic失败 */
            return FAIL;
        }
    }

    return SUCCESS;
}


T_TrafficInfo * CBBDevice::CreateInfo(WORD32                      dwQueueID,
                                      T_DpdkBBFapiTrafficJsonCfg &rtCfg,
                                      T_ProductDefInfo           *ptDefInfo)
{
    TRACE_STACK("CBBDevice::CreateInfo()");

    if ((NULL == ptDefInfo) || (m_dwTrafficNum >= MAX_BB_TRAFFIC_NUM))
    {
        return NULL;
    }

    T_TrafficInfo *ptTrafficInfo = &(m_atTrafficInfo[m_dwTrafficNum]);

    memcpy(ptTrafficInfo->aucName, ptDefInfo->aucName, BB_TRAFFIC_NAME_LEN);

    ptTrafficInfo->dwDeviceID   = m_dwDeviceID;
    ptTrafficInfo->dwPortID     = m_wPortID;
    ptTrafficInfo->dwQueueID    = dwQueueID;
    ptTrafficInfo->dwTrafficID  = rtCfg.dwTrafficID;
    ptTrafficInfo->dwFAPICellID = rtCfg.dwFAPICell;
    ptTrafficInfo->dwBindCellID = rtCfg.dwBindCell;
    ptTrafficInfo->dwMemSize    = ptDefInfo->dwMemSize;
    ptTrafficInfo->pCreateFunc  = ptDefInfo->pCreateFunc;
    ptTrafficInfo->pDestroyFunc = ptDefInfo->pDestroyFunc;
    ptTrafficInfo->pMem         = NULL;
    ptTrafficInfo->pTraffic     = NULL;

    m_dwTrafficNum++;

    return ptTrafficInfo;
}


