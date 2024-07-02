

#include "base_json_parse.h"
#include "base_config_file.h"


const BYTE s_aucModule[E_LOG_MODULE_NUM][LOG_MODULE_LEN] =
{
    "BASE              ",
        
    /* OSS模块 */
    "OSS               ",

    /* SCS模块 */
    "SCS               ",
    "NETCONF_CLI       ",
    "NETCONF_SERVER    ",

    /* OAM模块 */
    "OAM               ",
    "OAM_DELAY         ",
    "OAM_ES            ",
    "OAM_KPI           ",
    "OAM_UPGRADE       ",
    "OAM_ALARM         ",
    "OAM_AUTOSTART     ",

    /* DHCP模块 */
    "DHCP              ",

    /* TR069模块 */
    "TR069             ",

    /* MONITOR模块 */
    "MONITOR           ",

    /* CU模块 15 ~ 89 */
    "CU_APP            ",
    "CU_COMMON         ",
    "CU_OAM_AGENT      ",
    "CU_GNB_MGR        ",
    "CU_UP_MGR         ",
    "CU_RM             ",
    "CU_UE_CONN_MGR    ",
    "CU_BEARER_MGR     ",
    "CU_CODEC_COMMON   ",
    "CU_X2AP_CODEC     ",
    "CU_F1AP_CODEC     ",
    "CU_RRC_CODEC      ",
    "CU_NGAP_CODEC     ",
    "CU_XNAP_CODEC     ",
    "CU_E1AP_CODEC     ",
    "CU_SCTP_COMMON    ",
    "CU_SCTP_CNTRL     ",
    "CU_SCTP_TX        ",
    "CU_GTPU_COMMON    ",
    "CU_GTPU_U_TX_CNTRL",
    "CU_GTPU_U_RX_CNTRL",
    "CU_GTPU_U_TX      ",
    "CU_GTPU_U_RX      ",
    "CU_GTPU_L_TX_CNTRL",
    "CU_GTPU_L_RX_CNTRL",
    "CU_GTPU_L_TX      ",
    "CU_GTPU_L_RX      ",
    "CU_PDCP_COMMON    ",
    "CU_PDCP_TX_CNTRL  ",
    "CU_PDCP_RX_CNTRL  ",
    "CU_PDCP_TX        ",
    "CU_PDCP_RX        ",
    "CU_UDP_CNTRL      ",
    "CU_UDP_TX         ",
    "CU_UDP_RX         ",
    "CU_NRUP_CODEC     ",
    "CU_SDAP_COMMON    ",
    "CU_SDAP_TX_CNTRL  ",
    "CU_SDAP_RX_CNTRL  ",
    "CU_SDAP_TX        ",
    "CU_SDAP_RX        ",
    "CU_SDAP_CODEC     ",
    "CU_TIMER          ",
    "CU_EGTPU_TIMER    ",
    "CU_CRYPTO_RX      ",
    "CU_PM             ",
    "CU_NGP_MEM        ",
    "CU_NGP_BUF        ",
    "CU_NGP_STATS      ",
    "CU_NGP_TIMER      ",
    "CU_NGP_STHREAD    ",
    "CU_NGP_CTHREAD    ",
    "CU_NGP_SYS        ",
    "CU_NGP_EXCP       ",
    "CU_NGP_COMM       ",
    "CU_NGP_SCTP       ",
    "CU_NGP_UDP        ",
    "CU_NGP_TCP        ",
    "CU_NGP_MSGQ       ",
    "CU_NGP_PRIOQ      ",
    "CU_NGP_WORKQ      ",
    "CU_NGP_PERF       ",
    "CU_USERP_EXTRA    ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "DU_MGR2           ",
    "DU_APP_UE2        ",

    /* DU模块 90 ~ 169 */
    "DU_OAM_AGENT      ",
    "DU_MGR            ",
    "DU_RRM            ",
    "DU_APP_UE         ",
    "DU_CODEC          ",
    "DU_EVENT          ",
    "DU_EGTPU          ",
    "DU_UDP            ",
    "DU_CMN            ",
    "DU_MAC            ",
    "DU_SCHL1          ",
    "DU_SCHL2          ",
    "DU_DUCL           ",
    "DU_NS             ",
    "DU_FSPKT          ",
    "DU_NRUP           ",
    "DU_F1AP           ",
    "DU_SCTP           ",

    /* mac start */
    "DU_CELL_CMN       ",
    "DU_UE_CMN         ",
    "DU_CFG_CMN        ",
    "DU_CL_UL          ",
    "DU_CL_DL          ",
    "DU_CL_UCI         ",
    "DU_CL_CMN         ",
    "DU_CL_UL_TO_L1    ",
    "DU_CL_DL_TO_L1    ",
    "DU_LVL2_UL        ",
    "DU_LVL2_DL        ",
    "DU_CMN_SCH        ",
    "DU_LVL1_UL        ",
    "DU_LVL1_DL        ",
    "DU_LVL1_MEAS      ",
    "DU_SCH            ",
    "DU_RACH           ",
    "DU_TA             ",
    "DU_LA             ",
    "DU_DRX            ",
    "DU_MAC_UL         ",
    "DU_MAC_DL         ",
    "DU_MAC_COMMON     ",
    /* mac end */

    "DU_COMMON_UMN     ",
    "DU_RLC_UL         ",
    "DU_RLC_DL         ",
    "DU_RLC_COMMON     ",
    "DU_NGP_MEM        ",
    "DU_NGP_BUF        ",
    "DU_NGP_STATS      ",
    "DU_NGP_TIMER      ",
    "DU_NGP_STHREAD    ",
    "DU_NGP_CTHREAD    ",
    "DU_NGP_SYS        ",
    "DU_NGP_EXCP       ",
    "DU_NGP_COMM       ",
    "DU_NGP_SCTP       ",
    "DU_NGP_UDP        ",
    "DU_NGP_TCP        ",
    "DU_NGP_MSGQ       ",
    "DU_NGP_PRIOQ      ",
    "DU_NGP_WORKQ      ",
    "DU_NGP_PERF       ",
    "DU_QOS            ",
    "DU_MAC1           ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",
    "                  ",

    /* L1模块 170- */
    "L1                ",
    "L1_CPA            ",
    "L1_BBUPOOL        ",
    "L1_API            ",
    "L1_PDSCH          ",
    "L1_PDCCH          ",
    "L1_PUSCH          ",
    "L1_PUCCH          ",
    "L1_PRACH          ",
    "L1_SRS            ",
};


WORD32 GetModuleID(CString<LOG_MODULE_LEN> &rName)
{
    for (WORD32 dwIndex = 0; dwIndex < E_LOG_MODULE_NUM; dwIndex++)
    {
        if (rName.isMatch((CHAR *)(s_aucModule[dwIndex])))
        {
            return dwIndex;
        }
    }

    return INVALID_DWORD;
}


WORD32 ParseMem(T_MemJsonCfg &tMemConfig, CJsonValue &rRoot)
{
    tMemConfig.ucMemType = (BYTE)(rRoot["mem_type"].AsDWORD());
    tMemConfig.ucPageNum = (BYTE)(rRoot["page_num"].AsDWORD());
    tMemConfig.ucPoolNum = (BYTE)(rRoot["pools"].size());
    tMemConfig.dwMemSize = (WORD32)(rRoot["mem_size"].AsDWORD());

    CString<MAX_HUGE_DIR_LEN> cDir(rRoot["huge_dir"].AsString());
    memcpy(tMemConfig.aucHugeDir, cDir.toChar(), cDir.Length());

    for (WORD32 dwIndex = 0; dwIndex < tMemConfig.ucPoolNum; dwIndex++)
    {
        CJsonValue &rPool = rRoot["pools"][dwIndex];

        tMemConfig.atPool[dwIndex].dwPoolID   = (WORD32)(rPool["pool_id"].AsDWORD());
        tMemConfig.atPool[dwIndex].dwBlockNum = (WORD32)(rPool["blocks"].size());

        for (WORD32 dwIndex1 = 0; dwIndex1 < tMemConfig.atPool[dwIndex].dwBlockNum; dwIndex1++)
        {
            CJsonValue &rBlock = rPool["blocks"][dwIndex1];

            tMemConfig.atPool[dwIndex].atBlock[dwIndex1].dwTrunkSize = (WORD32)(rBlock["trunk_size"].AsDWORD());
            tMemConfig.atPool[dwIndex].atBlock[dwIndex1].dwPowerNum  = (WORD32)(rBlock["power_num"].AsDWORD());
        }
    }

    return SUCCESS;
}


WORD32 ParseLog(T_LogJsonCfg &tLogConfig, CJsonValue &rRoot)
{
    tLogConfig.dwCoreID          = (WORD32)(rRoot["thread_core"].AsDWORD());
    tLogConfig.dwPolicy          = (WORD32)(rRoot["policy"].AsDWORD());
    tLogConfig.dwPriority        = (WORD32)(rRoot["priority"].AsDWORD());
    tLogConfig.dwStackSize       = (WORD32)(rRoot["stack_size"].AsDWORD());
    tLogConfig.dwCBNum           = (WORD32)(rRoot["cb_num"].AsDWORD());
    tLogConfig.dwPacketCBNum     = (WORD32)(rRoot["packet_num"].AsDWORD());
    tLogConfig.dwMultiCBNum      = (WORD32)(rRoot["multi_cb_num"].AsDWORD());
    tLogConfig.dwTimerThresh     = (WORD32)(rRoot["timer_cb_num"].AsDWORD());
    tLogConfig.bMeasSwitch       = (BOOL)(rRoot["measure_switch"].AsBOOL());
    tLogConfig.bGlobalSwitch     = (BOOL)(rRoot["global_switch"].AsBOOL());
    tLogConfig.dwSyncFlag        = (WORD32)(rRoot["enable_sync"].AsDWORD());
    tLogConfig.dwLogMeasure      = (WORD32)(rRoot["log_measure"].AsDWORD());
    tLogConfig.dwWriteFilePeriod = (WORD32)(rRoot["write_file_period"].AsDWORD());
    tLogConfig.wThresholdWait    = (WORD16)(rRoot["wait_threshold"].AsDWORD());
    tLogConfig.wThresholdLock    = (WORD16)(rRoot["lock_threshold"].AsDWORD());
    tLogConfig.wThresholdLoop    = (WORD16)(rRoot["loop_threshold"].AsDWORD());
    tLogConfig.lwMaxFileSize     = (WORD32)(rRoot["file_size"].AsDWORD());
    tLogConfig.dwModuleNum       = rRoot["modules"].size();
    tLogConfig.dwAppNum          = rRoot["apps"].size();

    tLogConfig.lwMaxFileSize     = tLogConfig.lwMaxFileSize * BYTE_NUM_PER_MB;

    CString<LOG_FILE_NAME_LEN> cPath(rRoot["path"].AsString());
    memcpy(tLogConfig.aucPath, cPath.toChar(), cPath.Length());

    for (WORD32 dwIndex = 0; dwIndex < tLogConfig.dwAppNum; dwIndex++)
    {
        CJsonValue   &rApp  = rRoot["apps"][dwIndex];
        T_AppJsonCfg &rtCfg = tLogConfig.atApp[dwIndex];

        CString<WORKER_NAME_LEN> cName(rApp["name"].AsString());
        memcpy(rtCfg.aucName, 
               cName.toChar(),
               cName.Length());

        rtCfg.dwAppID      = (WORD32)(rApp["app_id"].AsDWORD());
        rtCfg.dwEventBegin = (WORD32)(rApp["event_begin"].AsDWORD());
        rtCfg.dwAssocNum   = (WORD32)(rApp["assoc_cell"].size());
        rtCfg.bAssocFlag   = (0 == rtCfg.dwAssocNum) ? FALSE : TRUE;

        if (rtCfg.dwAssocNum > MAX_ASSOCIATE_NUM_PER_APP)
        {
            rtCfg.dwAssocNum = MAX_ASSOCIATE_NUM_PER_APP;
        }

        for (WORD32 dwIndex1 = 0; 
             dwIndex1 < rtCfg.dwAssocNum; 
             dwIndex1++)
        {
            rtCfg.adwAssocID[dwIndex1] = (WORD32)(rApp["assoc_cell"][dwIndex1].AsDWORD());
        }
    }

    for (WORD32 dwIndex = 0; dwIndex < tLogConfig.dwModuleNum; dwIndex++)
    {
        CJsonValue &rModule = rRoot["modules"][dwIndex];

        CString<LOG_MODULE_LEN> cName(rModule["name"].AsString());
        memcpy(tLogConfig.atModule[dwIndex].aucName, 
               cName.toChar(),
               cName.Length());

        tLogConfig.atModule[dwIndex].dwModuleID = GetModuleID(cName);

        tLogConfig.atModule[dwIndex].bSwitch = (BOOL)(rModule["switch"].AsBOOL());

        tLogConfig.atModule[dwIndex].ucCellNum  = rModule["cell"].size();
        tLogConfig.atModule[dwIndex].ucLevelNum = rModule["level"].size();

        for (WORD32 dwIndex1 = 0; 
             dwIndex1 < tLogConfig.atModule[dwIndex].ucCellNum; 
             dwIndex1++)
        {
            tLogConfig.atModule[dwIndex].adwCellID[dwIndex1] = (WORD32)(rModule["cell"][dwIndex1].AsDWORD());
        }

        for (WORD32 dwIndex1 = 0; 
             dwIndex1 < tLogConfig.atModule[dwIndex].ucLevelNum; 
             dwIndex1++)
        {
             tLogConfig.atModule[dwIndex].adwLevelID[dwIndex1] = (WORD32)(rModule["level"][dwIndex1].AsDWORD());
        }
    }

    return SUCCESS;
}


WORD32 ParseTimer(T_TimerJsonCfg &tTimerConfig, CJsonValue &rRoot)
{
    tTimerConfig.bCreateFlag   = (BOOL)(rRoot["create_flag"].AsBOOL());
    tTimerConfig.dwCoreID      = (WORD32)(rRoot["logical_id"].AsDWORD());
    tTimerConfig.dwPolicy      = (WORD32)(rRoot["policy"].AsDWORD());
    tTimerConfig.dwPriority    = (WORD32)(rRoot["priority"].AsDWORD());
    tTimerConfig.dwStackSize   = (WORD32)(rRoot["stack_size"].AsDWORD());
    tTimerConfig.dwServTimeLen = (WORD32)(rRoot["service_measure"].AsDWORD());
    tTimerConfig.dwAppTimeLen  = (WORD32)(rRoot["app_measure"].AsDWORD());
    tTimerConfig.bAloneLog     = (BOOL)(rRoot["alone_log"].AsBOOL());

    return SUCCESS;
}


WORD32 ParseShm(T_ShmJsonCfg &tShmConfig, CJsonValue &rRoot)
{
    tShmConfig.bCreateFlag  = (BOOL)(rRoot["create_flag"].AsBOOL());
    tShmConfig.bMaster      = (BOOL)(rRoot["master_flag"].AsBOOL());
    tShmConfig.dwChannelNum = (WORD32)(rRoot["channel_num"].AsDWORD());
    tShmConfig.dwPowerNum   = (WORD32)(rRoot["power_num"].AsDWORD());

    return SUCCESS;
}


WORD32 ParseApp(T_ThreadJsonCfg &tConfig, CJsonValue &rRoot)
{
    tConfig.dwAppNum = MIN(rRoot.size(), MAX_APP_NUM_PER_THREAD);

    for (WORD32 dwIndex = 0; dwIndex < tConfig.dwAppNum; dwIndex++)
    {
        CJsonValue &rApp = rRoot[dwIndex];

        CString<WORKER_NAME_LEN> cName(rApp["name"].AsString());
        memcpy(tConfig.atApp[dwIndex].aucName, 
               cName.toChar(),
               cName.Length());

        tConfig.atApp[dwIndex].dwAppID      = (WORD32)(rApp["app_id"].AsDWORD());
        tConfig.atApp[dwIndex].dwEventBegin = (WORD32)(rApp["event_begin"].AsDWORD());
        tConfig.atApp[dwIndex].dwAssocNum   = (WORD32)(rApp["assoc_cell"].size());
        tConfig.atApp[dwIndex].bAssocFlag   = (0 == tConfig.atApp[dwIndex].dwAssocNum) ? FALSE : TRUE;

        if (tConfig.atApp[dwIndex].dwAssocNum > MAX_ASSOCIATE_NUM_PER_APP)
        {
            tConfig.atApp[dwIndex].dwAssocNum = MAX_ASSOCIATE_NUM_PER_APP;
        }

        for (WORD32 dwIndex1 = 0; 
             dwIndex1 < tConfig.atApp[dwIndex].dwAssocNum; 
             dwIndex1++)
        {
            tConfig.atApp[dwIndex].adwAssocID[dwIndex1] = (WORD32)(rApp["assoc_cell"][dwIndex1].AsDWORD());
        }
    }
    
    return SUCCESS;
}


WORD32 ParseWorker(T_ThreadPoolJsonCfg &tConfig, CJsonValue &rRoot)
{
    tConfig.dwWorkerNum = rRoot.size();

    for (WORD32 dwIndex = 0; dwIndex < tConfig.dwWorkerNum; dwIndex++)
    {
        CJsonValue &rWorker = rRoot[dwIndex];

        CString<WORKER_NAME_LEN> cType(rWorker["type"].AsString());
        memcpy(tConfig.atWorker[dwIndex].aucType, 
               cType.toChar(),
               cType.Length());

        CString<WORKER_NAME_LEN> cName(rWorker["name"].AsString());
        memcpy(tConfig.atWorker[dwIndex].aucName, 
               cName.toChar(),
               cName.Length());

        tConfig.atWorker[dwIndex].dwThreadID    = (WORD32)(rWorker["thread_id"].AsDWORD());
        tConfig.atWorker[dwIndex].dwLogicalID   = (WORD32)(rWorker["logical_id"].AsDWORD());
        tConfig.atWorker[dwIndex].dwPolicy      = (WORD32)(rWorker["policy"].AsDWORD());
        tConfig.atWorker[dwIndex].dwPriority    = (WORD32)(rWorker["priority"].AsDWORD());
        tConfig.atWorker[dwIndex].dwStackSize   = (WORD32)(rWorker["stack_size"].AsDWORD());
        tConfig.atWorker[dwIndex].dwCBNum       = (WORD32)(rWorker["cb_num"].AsDWORD());
        tConfig.atWorker[dwIndex].dwPacketCBNum = (WORD32)(rWorker["packet_num"].AsDWORD());
        tConfig.atWorker[dwIndex].dwMultiCBNum  = (WORD32)(rWorker["multi_cb_num"].AsDWORD());
        tConfig.atWorker[dwIndex].dwTimerThresh = (WORD32)(rWorker["timer_cb_num"].AsDWORD());
        tConfig.atWorker[dwIndex].bAloneLog     = (BOOL)(rWorker["alone_log"].AsBOOL());

        ParseApp(tConfig.atWorker[dwIndex], rWorker["apps"]);
    }

    return SUCCESS;
}


WORD32 ParseDpdk(T_DpdkJsonCfg &tConfig, CJsonValue &rRoot)
{
    tConfig.bInitFlag = (BOOL)(rRoot["init_flag"].AsBOOL());

    CJsonValue &rArgs   = rRoot["init_args"];
    CJsonValue &rIntf   = rRoot["interface"];
    CJsonValue &rBBDev  = rRoot["bbdev"];
    CJsonValue &rEthDev = rRoot["ethdev"];

    tConfig.dwArgNum = rArgs.size();
    tConfig.dwDevNum = rIntf.size();
    tConfig.dwBBNum  = rBBDev.size();
    tConfig.dwEthNum = rEthDev.size();

    tConfig.dwArgNum = MIN(tConfig.dwArgNum, EAL_ARG_NUM);
    tConfig.dwDevNum = MIN(tConfig.dwDevNum, MAX_DEV_PORT_NUM);
    tConfig.dwBBNum  = MIN(tConfig.dwBBNum,  MAX_DEV_PORT_NUM);
    tConfig.dwEthNum = MIN(tConfig.dwEthNum, MAX_DEV_PORT_NUM);

    for (WORD32 dwIndex = 0; dwIndex < tConfig.dwArgNum; dwIndex++)
    {
        CJsonValue &rArg = rArgs[dwIndex];
        CHAR       *pArg = tConfig.aucArgs[dwIndex];

        CString<EAL_ARG_LEN> cCmdArg(rArg["cmd_line"].AsString());
        memcpy(pArg, cCmdArg.toChar(), cCmdArg.Length());
    }

    for (WORD32 dwIndex = 0; dwIndex < tConfig.dwDevNum; dwIndex++)
    {
        CJsonValue        &rDevice  = rIntf[dwIndex];
        T_DpdkDevJsonCfg  &rtDevCfg = tConfig.atDevice[dwIndex];

        CString<DEV_NAME_LEN> cType(rDevice["type"].AsString());
        memcpy(rtDevCfg.aucType, cType.toChar(), cType.Length());

        CString<DEV_NAME_LEN> cAddr(rDevice["pci_addr"].AsString());
        memcpy(rtDevCfg.aucAddr, cAddr.toChar(), cAddr.Length());

        rtDevCfg.dwDeviceID = rDevice["dev_id"].AsDWORD();
        rtDevCfg.dwPortID   = INVALID_DWORD;
        rtDevCfg.dwQueueNum = rDevice["queue_num"].AsDWORD();
    }

    for (WORD32 dwIndex = 0; dwIndex < tConfig.dwBBNum; dwIndex++)
    {
        CJsonValue &rBBItem = rBBDev[dwIndex];

        T_DpdkBBDevJsonCfg &rtBBCfg = tConfig.atBBDev[dwIndex];

        rtBBCfg.dwDeviceID = rBBItem["dev_id"].AsDWORD();
    }

    for (WORD32 dwIndex = 0; dwIndex < tConfig.dwEthNum; dwIndex++)
    {
        CJsonValue &rEthItem = rEthDev[dwIndex];
        CJsonValue &rIPs     = rEthItem["ip_cfg"];
        CJsonValue &rVlans   = rEthItem["vlan_cfg"];

        T_DpdkEthDevJsonCfg &rtEthCfg = tConfig.atEthDev[dwIndex];

        rtEthCfg.dwDeviceID = rEthItem["dev_id"].AsDWORD();
        rtEthCfg.dwLinkType = rEthItem["link_type"].AsDWORD();
        rtEthCfg.dwIpNum    = rIPs.size();
        rtEthCfg.dwVlanNum  = rVlans.size();

        for (WORD32 dwIndex1 = 0; dwIndex1 < rtEthCfg.dwIpNum; dwIndex1++)
        {
            CJsonValue          &rIP     = rIPs[dwIndex1];
            T_DpdkEthIPJsonCfg  &rtIPCfg = rtEthCfg.atIP[dwIndex1];

            rtIPCfg.dwIPType = (WORD32)(rIP["addr_type"].AsDWORD());

            CString<IPV4_STRING_LEN> cIpv4Addr(rIP["ipv4_addr"].AsString());
            CString<IPV4_STRING_LEN> cIpv4GW(rIP["ipv4_gate_way"].AsString());
            CString<IPV6_STRING_LEN> cIpv6Addr(rIP["ipv6_addr"].AsString());
            CString<IPV6_STRING_LEN> cIpv6GW(rIP["ipv6_gate_way"].AsString());

            memcpy(rtIPCfg.aucIpv4Addr,
                   cIpv4Addr.toChar(),
                   cIpv4Addr.Length());
            memcpy(rtIPCfg.aucIpv4GW,
                   cIpv4GW.toChar(),
                   cIpv4GW.Length());
            memcpy(rtIPCfg.aucIpv6Addr,
                   cIpv6Addr.toChar(),
                   cIpv6Addr.Length());
            memcpy(rtIPCfg.aucIpv6GW,
                   cIpv6GW.toChar(),
                   cIpv6GW.Length());
        }

        for (WORD32 dwIndex1 = 0; dwIndex1 < rtEthCfg.dwVlanNum; dwIndex1++)
        {
            CJsonValue           &rVlan     = rVlans[dwIndex1];
            CJsonValue           &rVlanIP   = rVlan["ip_cfg"];
            T_DpdkEthVlanJsonCfg &rtVlanCfg = rtEthCfg.atVlan[dwIndex1];

            rtVlanCfg.dwVlanID     = rVlan["vlan_id"].AsDWORD();
            rtVlanCfg.dwPriority   = rVlan["priority"].AsDWORD();
            rtVlanCfg.tIP.dwIPType = rVlanIP["addr_type"].AsDWORD();

            CString<IPV4_STRING_LEN> cIpv4Addr(rVlanIP["ipv4_addr"].AsString());
            CString<IPV4_STRING_LEN> cIpv4GW(rVlanIP["ipv4_gate_way"].AsString());
            CString<IPV6_STRING_LEN> cIpv6Addr(rVlanIP["ipv6_addr"].AsString());
            CString<IPV6_STRING_LEN> cIpv6GW(rVlanIP["ipv6_gate_way"].AsString());

            memcpy(rtVlanCfg.tIP.aucIpv4Addr,
                   cIpv4Addr.toChar(),
                   cIpv4Addr.Length());
            memcpy(rtVlanCfg.tIP.aucIpv4GW,
                   cIpv4GW.toChar(),
                   cIpv4GW.Length());
            memcpy(rtVlanCfg.tIP.aucIpv6Addr,
                   cIpv6Addr.toChar(),
                   cIpv6Addr.Length());
            memcpy(rtVlanCfg.tIP.aucIpv6GW,
                   cIpv6GW.toChar(),
                   cIpv6GW.Length());
        }
    }

    return SUCCESS;
}


CBaseConfigFile::CBaseConfigFile ()
{
    memset(&m_tRootConfig, 0x00, sizeof(m_tRootConfig));
}


CBaseConfigFile::~CBaseConfigFile()
{
    memset(&m_tRootConfig, 0x00, sizeof(m_tRootConfig));
}


WORD32 CBaseConfigFile::ParseFile()
{
    std::ifstream  cFile;

    cFile.open("base.json");

    CJsonReader *pReader = new CJsonReader();
    CJsonValue   cRoot;

    /* 将Log.json中所有元素解析到cRoot */
    if (!pReader->Parse(cFile, cRoot))
    {
        delete pReader;
        cFile.close();
        return FAIL;
    }

    ParseMem(m_tRootConfig.tMemConfig, cRoot["memory"]);
    ParseLog(m_tRootConfig.tLogConfig, cRoot["log"]);
    ParseTimer(m_tRootConfig.tTimerConfig, cRoot["timer"]);
    ParseShm(m_tRootConfig.tShmConfig, cRoot["shm"]);
    ParseWorker(m_tRootConfig.tWorkerConfig, cRoot["workers"]);
    ParseDpdk(m_tRootConfig.tDpdkConfig, cRoot["dpdk"]);

    delete pReader;
    cFile.close();

    return SUCCESS;
}


T_MemJsonCfg & CBaseConfigFile::GetMemJsonCfg()
{
    return m_tRootConfig.tMemConfig;
}


T_LogJsonCfg  & CBaseConfigFile::GetLogJsonCfg()
{
    return m_tRootConfig.tLogConfig;
}


T_TimerJsonCfg & CBaseConfigFile::GetTimerJsonCfg()
{
    return m_tRootConfig.tTimerConfig;
}


T_ShmJsonCfg & CBaseConfigFile::GetShmJsonCfg()
{
    return m_tRootConfig.tShmConfig;
}


T_ThreadPoolJsonCfg & CBaseConfigFile::GetWorkerJsonCfg()
{
    return m_tRootConfig.tWorkerConfig;
}


T_DpdkJsonCfg & CBaseConfigFile::GetDpdkJsonCfg()
{
    return m_tRootConfig.tDpdkConfig;
}


T_RootJsonCfg & CBaseConfigFile::GetRootJsonCfg()
{
    return m_tRootConfig;
}


T_DpdkBBDevJsonCfg  * CBaseConfigFile::GetBBDevJsonCfg(WORD32 dwDeviceID)
{
    T_DpdkJsonCfg &rtDpdkCfg = m_tRootConfig.tDpdkConfig;

    for (WORD32 dwIndex = 0; dwIndex < rtDpdkCfg.dwBBNum; dwIndex++)
    {
        T_DpdkBBDevJsonCfg &rtBBCfg = rtDpdkCfg.atBBDev[dwIndex];
        if (dwDeviceID == rtBBCfg.dwDeviceID)
        {
            return &rtBBCfg;
        }
    }

    return NULL;
}


T_DpdkEthDevJsonCfg * CBaseConfigFile::GetEthDevJsonCfg(WORD32 dwDeviceID)
{
    T_DpdkJsonCfg &rtDpdkCfg = m_tRootConfig.tDpdkConfig;

    for (WORD32 dwIndex = 0; dwIndex < rtDpdkCfg.dwEthNum; dwIndex++)
    {
        T_DpdkEthDevJsonCfg &rtEthCfg = rtDpdkCfg.atEthDev[dwIndex];
        if (dwDeviceID == rtEthCfg.dwDeviceID)
        {
            return &rtEthCfg;
        }
    }

    return NULL;
}


