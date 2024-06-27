

#include <rte_bbdev.h>

#include "dpdk_mgr.h"
#include "base_log.h"
#include "base_init_component.h"
#include "base_sort.h"


CDpdkMgr *g_pDpdkMgr = NULL;


WORD32 ExitDpdk(VOID *pArg)
{
    T_InitFunc      *ptInitFunc      = (T_InitFunc *)pArg;
    CCentralMemPool *pCentralMemPool = ptInitFunc->pMemInterface;

    CDpdkMgr::Destroy();

    if (NULL != g_pDpdkMgr)
    {
        pCentralMemPool->Free((BYTE *)g_pDpdkMgr);
        g_pDpdkMgr = NULL;
    }

    CFactoryDevice::Destroy();

    return SUCCESS;
}


WORD32 InitDpdk(WORD32 dwProcID, VOID *pArg)
{
    TRACE_STACK("InitDpdk()");

    T_InitFunc *ptInitFunc = (T_InitFunc *)pArg;
    ptInitFunc->pExitFunc  = &ExitDpdk;

    T_DpdkJsonCfg &rJsonCfg = CBaseConfigFile::GetInstance()->GetDpdkJsonCfg();
    if (FALSE == rJsonCfg.bInitFlag)
    {
        return SUCCESS;
    }

    CCentralMemPool *pCentralMemPool = ptInitFunc->pMemInterface;

    BYTE *pMem = pCentralMemPool->Malloc(sizeof(CDpdkMgr));
    if (NULL == pMem)
    {
        assert(0);
    }

    g_pDpdkMgr = CDpdkMgr::GetInstance(pMem);
    if (NULL == g_pDpdkMgr)
    {
        assert(0);
    }

    WORD32 dwResult = g_pDpdkMgr->Initialize(pCentralMemPool,
                                             (const CHAR *)(s_aucFileName[dwProcID]),
                                             rJsonCfg);
    if (SUCCESS != dwResult)
    {
        assert(0);
    }

    return SUCCESS;
}
INIT_EXPORT(InitDpdk, 10);


CDpdkMgr::CDpdkMgr()
{
    m_pMemInterface = NULL;
    m_dwDevNum      = 0;
    memset((BYTE *)(&(m_atDevInfo[0])), 0x00, sizeof(m_atDevInfo));
}


CDpdkMgr::~CDpdkMgr()
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwDevNum; dwIndex++)
    {
        if (NULL != m_atDevInfo[dwIndex].pDevice)
        {
            (*(m_atDevInfo[dwIndex].pDestroyFunc)) (m_atDevInfo[dwIndex].pMem);
            m_pMemInterface->Free(m_atDevInfo[dwIndex].pMem);
        }
    }

    /* clean up the EAL */
    rte_eal_cleanup();

    m_pMemInterface = NULL;
    m_dwDevNum      = 0;
    memset((BYTE *)(&(m_atDevInfo[0])), 0x00, sizeof(m_atDevInfo));
}


WORD32 CDpdkMgr::Initialize(CCentralMemPool *pMemInterface,
                            const CHAR      *pArg0,
                            T_DpdkJsonCfg   &rtCfg)
{
    TRACE_STACK("CDpdkMgr::Initialize()");

    m_pMemInterface = pMemInterface;

    WORD32 dwResult   = INVALID_DWORD;
    WORD32 dwEthCount = 0;
    WORD32 dwBBCount  = 0;

    /* 启动DPDK */
    dwResult = InitDpdkEal(pArg0, rtCfg.aucCoreArg);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    dwEthCount = rte_eth_dev_count_avail();
    dwBBCount  = rte_bbdev_count();

    if ((0 != rtCfg.dwEthNum) && (rtCfg.dwEthNum > dwEthCount))
    {
        return FAIL;
    }

    if ((0 != rtCfg.dwBBNum) && (rtCfg.dwBBNum > dwBBCount))
    {
        return FAIL;
    }

    /* 根据json配置创建设备实例 */
    dwResult = InitDevice(rtCfg);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    return SUCCESS;
}


T_DeviceInfo * CDpdkMgr::FindDevInfo(WORD32 dwDeviceID)
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwDevNum; dwIndex++)
    {
        if (dwDeviceID == m_atDevInfo[dwIndex].dwDeviceID)
        {
            return &(m_atDevInfo[dwIndex]);
        }
    }

    return NULL;
}


CBaseDevice * CDpdkMgr::FindDevice(WORD32 dwDeviceID)
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwDevNum; dwIndex++)
    {
        if (dwDeviceID == m_atDevInfo[dwIndex].dwDeviceID)
        {
            return m_atDevInfo[dwIndex].pDevice;
        }
    }

    return NULL;
}


CBaseDevice * CDpdkMgr::FindDevice(E_DeviceType eType, WORD16 wPortID)
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwDevNum; dwIndex++)
    {
        if ( (wPortID == m_atDevInfo[dwIndex].dwPortID)
          && (eType == m_atDevInfo[dwIndex].pDevice->GetType()))
        {
            return m_atDevInfo[dwIndex].pDevice;
        }
    }

    return NULL;
}


WORD32 CDpdkMgr::GetDeviceNum()
{
    return m_dwDevNum;
}


VOID CDpdkMgr::Dump()
{
    TRACE_STACK("CDpdkMgr::Dump()");

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "m_dwDevNum : %d\n",
               m_dwDevNum);

    for (WORD32 dwIndex = 0; dwIndex < m_dwDevNum; dwIndex++)
    {
        m_atDevInfo[dwIndex].pDevice->Dump();
    }
}


WORD32 CDpdkMgr::InitDpdkEal(const CHAR *pArg0, CHAR *pArgCore)
{
    CHAR *pArgs[EAL_ARG_NUM] = {(CHAR *)pArg0, pArgCore};

    SWORD32 iResult = rte_eal_init((SWORD32)(EAL_ARG_NUM), pArgs);
    if (iResult < 0)
    {
        return FAIL;
    }

    return SUCCESS;
}


WORD32 CDpdkMgr::InitDevice(T_DpdkJsonCfg &rtCfg)
{
    WORD32          dwResult = INVALID_DWORD;
    CFactoryDevice *pFactory = CFactoryDevice::GetInstance();

    for (WORD32 dwIndex = 0; dwIndex < rtCfg.dwDevNum; dwIndex++)
    {
        T_DpdkDevJsonCfg &rtDevCfg  = rtCfg.atDevice[dwIndex];
        T_DeviceInfo     *ptDevInfo = FindDevInfo(rtDevCfg.dwDeviceID);
        if (NULL != ptDevInfo)
        {
            continue ;
        }

        T_ProductDefInfo *ptDefInfo = pFactory->FindDef(rtDevCfg.aucType);
        if (NULL == ptDefInfo)
        {
            /* 配置错误 */
            return FAIL;
        }

        ptDevInfo = CreateInfo(rtDevCfg.dwDeviceID,
                               rtDevCfg.dwPortID,
                               rtDevCfg.dwQueueNum,
                               ptDefInfo);
        if (NULL == ptDevInfo)
        {
            /* 设备创建失败 */
            return FAIL;
        }
    }

    /* 从小到大排序 */
    HeapSort<T_DeviceInfo, WORD32> (m_atDevInfo, m_dwDevNum,
                                    (&T_DeviceInfo::dwDeviceID),
                                    (PComparePFunc<T_DeviceInfo, WORD32>)(& GreaterV<T_DeviceInfo, WORD32>));

    for (WORD32 dwIndex = 0; dwIndex < m_dwDevNum; dwIndex++)
    {
        m_atDevInfo[dwIndex].pMem = m_pMemInterface->Malloc(m_atDevInfo[dwIndex].dwMemSize);

        T_DeviceParam tParam;
        tParam.dwDeviceID = m_atDevInfo[dwIndex].dwDeviceID;
        tParam.dwPortID   = m_atDevInfo[dwIndex].dwPortID;
        tParam.dwQueueNum = m_atDevInfo[dwIndex].dwQueueNum;
        tParam.pMemPool   = m_pMemInterface;

        m_atDevInfo[dwIndex].pDevice = (CBaseDevice *)((*(m_atDevInfo[dwIndex].pCreateFunc)) (m_atDevInfo[dwIndex].pMem, &tParam));
        dwResult = m_atDevInfo[dwIndex].pDevice->Initialize();
        if (SUCCESS != dwResult)
        {
            /* 初始化设备失败 */
            return FAIL;
        }
    }

    return SUCCESS;
}


T_DeviceInfo * CDpdkMgr::CreateInfo(WORD32            dwDeviceID,
                                    WORD32            dwPortID,
                                    WORD32            dwQueueNum,
                                    T_ProductDefInfo *ptDefInfo)
{
    if ((NULL == ptDefInfo) || (m_dwDevNum >= MAX_DEV_PORT_NUM))
    {
        return NULL;
    }

    T_DeviceInfo *ptDevInfo = &(m_atDevInfo[m_dwDevNum]);

    /* 设备类型名 */
    memcpy(ptDevInfo->aucName, ptDefInfo->aucName, DEV_NAME_LEN);

    ptDevInfo->dwDeviceID   = dwDeviceID;
    ptDevInfo->dwPortID     = dwPortID;
    ptDevInfo->dwQueueNum   = dwQueueNum;
    ptDevInfo->dwMemSize    = ptDefInfo->dwMemSize;
    ptDevInfo->pCreateFunc  = ptDefInfo->pCreateFunc;
    ptDevInfo->pDestroyFunc = ptDefInfo->pDestroyFunc;
    ptDevInfo->pMem         = NULL;
    ptDevInfo->pDevice      = NULL;

    m_dwDevNum++;

    return ptDevInfo;
}


