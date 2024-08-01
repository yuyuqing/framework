

#include "dpdk_device_bb.h"
#include "dpdk_mgr.h"

#include "base_log.h"
#include "base_sort.h"

#ifdef PICOCOM_FAPI
#include "rte_pmd_pc802.h"
#endif


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
    m_tEthConf.rxmode.mtu            = 1500;
    m_tEthConf.txmode.mq_mode        = RTE_ETH_MQ_TX_NONE;
    m_tEthConf.txmode.offloads       = 0;
    //m_tEthConf.rx_adv_conf.vmdq_dcb_conf.nb_queue_pools = RTE_ETH_16_POOLS;
#else
    m_tEthConf.rxmode.mq_mode        = ETH_MQ_RX_NONE;
    m_tEthConf.rxmode.max_rx_pkt_len = 1500;
    m_tEthConf.txmode.mq_mode        = ETH_MQ_TX_NONE;
    m_tEthConf.txmode.offloads       = 0;
    //m_tEthConf.rx_adv_conf.vmdq_dcb_conf.nb_queue_pools = ETH_16_POOLS;
#endif

    //m_tEthConf.rx_adv_conf.vmdq_dcb_conf.enable_default_pool = 1;
    m_tEthConf.rx_adv_conf.vmdq_dcb_conf.default_pool        = 0;
    m_tEthConf.rx_adv_conf.vmdq_dcb_conf.nb_pool_maps        = 0;

    m_dwTrafficNum = 0;
    memset((BYTE *)(&(m_atTrafficInfo[0])), 0x00, sizeof(m_atTrafficInfo));

#ifdef PICOCOM_FAPI
    m_tCallBackInfo.readHandle  = CBBDevice::RecvPacket;
    m_tCallBackInfo.writeHandle = CBBDevice::SendPacket;
#endif
}


CBBDevice::~CBBDevice()
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwTrafficNum; dwIndex++)
    {
        if (NULL != m_atTrafficInfo[dwIndex].pTraffic)
        {
            (*(m_atTrafficInfo[dwIndex].pDestroyFunc)) (m_atTrafficInfo[dwIndex].pMem);
            m_rCentralMemPool.Free(m_atTrafficInfo[dwIndex].pMem);
        }
    }

    m_dwTrafficNum = 0;
    memset((BYTE *)(&(m_atTrafficInfo[0])), 0x00, sizeof(m_atTrafficInfo));
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
                            LEGACY_CELL_INDEX);
    assert(0 == dwResult);
#endif

    dwResult = DevStart();
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    return SUCCESS;
}


VOID CBBDevice::Dump()
{
    TRACE_STACK("CBBDevice::Dump()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "m_dwDeviceID : %d, m_wPortID : %d, m_ucQueueNum : %d, "
               "m_ucDevType : %d, m_dwTrafficNum : %d\n",
               m_dwDeviceID,
               m_wPortID,
               m_ucQueueNum,
               m_ucDevType,
               m_dwTrafficNum);
    for (WORD32 dwIndex = 0; dwIndex < m_dwTrafficNum; dwIndex++)
    {
        T_TrafficInfo &rtTraffic = m_atTrafficInfo[dwIndex];

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                   "Type : %s, DeviceID : %d, PortID : %d, QueueID : %d, "
                   "TrafficID : %d, FAPICellID : %d, BindCellID : %d, "
                   "pTraffic : %lu\n",
                   rtTraffic.aucName,
                   rtTraffic.dwDeviceID,
                   rtTraffic.dwPortID,
                   rtTraffic.dwQueueID,
                   rtTraffic.dwTrafficID,
                   rtTraffic.dwFAPICellID,
                   rtTraffic.dwBindCellID,
                   (WORD64)(rtTraffic.pTraffic));
    }
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


