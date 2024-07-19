

#include "dpdk_device_eth.h"
#include "dpdk_mgr.h"
#include "dpdk_net_ipv6.h"
#include "dpdk_net_icmpv6.h"

#include "base_log.h"


DEFINE_DEVICE(CEthDevice);


CEthDevice::CEthDevice (const T_DeviceParam &rtParam)
    : CBaseDevice(E_ETH_DEVICE, rtParam)
{
    m_ucLinkType  = E_ETH_LINK_ACCESS;
    m_ucVlanNum   = 0;
    m_ucIPNum     = 0;
    m_dwPrimaryIP = 0;

    memset(&m_tLinkLocalIP, 0x00, sizeof(m_tLinkLocalIP));
    memset(m_adwVlanID,     0x00, sizeof(m_adwVlanID));
    memset(m_atIPAddr,      0x00, sizeof(m_atIPAddr));
    memset(&m_tEthAddr,     0x00, sizeof(m_tEthAddr));

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

    InitIPConfig(*ptCfg);

    CIPTable   &rIPTalbe   = g_pNetIntfHandler->GetIPTable();
    CVlanTable &rVlanTable = g_pNetIntfHandler->GetVlanTable();

    WORD32 dwResult = CBaseDevice::Initialize(NULL);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    dwResult = DevStart();
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

    InitLinkLocalIP(tMacAddr, rIPTalbe);

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


WORD32 CEthDevice::Start(WORD16 wQueueID)
{
    TRACE_STACK("CEthDevice::Start()");

    WORD32 dwResult = ProcDAD(wQueueID);
    if (SUCCESS != dwResult)
    {
        assert(0);
    }

    return SUCCESS;
}


/* ִ�е�ַ�ظ���� */
WORD32 CEthDevice::ProcDAD(WORD16 wQueueID)
{
    TRACE_STACK("CEthDevice::ProcDAD()");

    if (wQueueID >= m_ucQueueNum)
    {
        return FAIL;
    }

    CDevQueue *pQueue = m_apQueue[wQueueID];
    if (NULL == pQueue)
    {
        return FAIL;
    }

    CIPv6Stack *pIPv6Stack = (CIPv6Stack *)(g_pNetIntfHandler->GetIPv6Stack());
    if (NULL == pIPv6Stack)
    {
        return FAIL;
    }

    CIcmpV6Stack *pIcmpV6Stack = (CIcmpV6Stack *)(pIPv6Stack->GetIcmpStack());
    if (NULL == pIcmpV6Stack)
    {
        return FAIL;
    }

    WORD64    lwOption = HTONL(0x0E01000000000000UL);
    T_IPAddr  tTargetIP;
    T_IPAddr  tSrcIP;
    T_IPAddr  tDstIP;
    T_MacAddr tDstMac;

    tTargetIP.eType = E_IPV6_TYPE;
    tSrcIP.eType    = E_IPV6_TYPE;
    tDstIP.eType    = E_IPV6_TYPE;

    /* link-local address��ΪTargetIP */
    memcpy(tTargetIP.tIPv6.aucIPAddr, m_tLinkLocalIP.aucIPAddr, IPV6_ADDR_LEN);

    /* Ŀ��IP�Ǳ�����ڵ��鲥IP */
    memset(tDstIP.tIPv6.aucIPAddr, 0x00, IPV6_ADDR_LEN);
    tDstIP.tIPv6.aucIPAddr[0]  = 0xFF;
    tDstIP.tIPv6.aucIPAddr[1]  = 0x02;
    tDstIP.tIPv6.aucIPAddr[11] = 0x01;
    tDstIP.tIPv6.aucIPAddr[12] = 0xFF;
    tDstIP.tIPv6.aucIPAddr[13] = tTargetIP.tIPv6.aucIPAddr[13];
    tDstIP.tIPv6.aucIPAddr[14] = tTargetIP.tIPv6.aucIPAddr[14];
    tDstIP.tIPv6.aucIPAddr[15] = tTargetIP.tIPv6.aucIPAddr[15];

    /* ԴIPΪ��Чֵ */
    memset(tSrcIP.tIPv6.aucIPAddr, 0x00, IPV6_ADDR_LEN);

    /* Ŀ��MACΪ�鲥��ַ */
    tDstMac.aucMacAddr[0] = 0x33;
    tDstMac.aucMacAddr[1] = 0x33;
    tDstMac.aucMacAddr[2] = tDstIP.tIPv6.aucIPAddr[12];
    tDstMac.aucMacAddr[3] = tDstIP.tIPv6.aucIPAddr[13];
    tDstMac.aucMacAddr[4] = tDstIP.tIPv6.aucIPAddr[14];
    tDstMac.aucMacAddr[5] = tDstIP.tIPv6.aucIPAddr[15];

    return pIcmpV6Stack->SendNeighborSolication(pQueue,
                                                lwOption,
                                                tTargetIP,
                                                tSrcIP,
                                                tDstIP,
                                                tDstMac);
}


VOID CEthDevice::Dump()
{
    TRACE_STACK("CEthDevice::Dump()");

    CString<IPV6_STRING_LEN> cIpv6;
    T_IPAddr tLinkLocalAddr;

    tLinkLocalAddr.eType = E_IPV6_TYPE;
    memcpy(&(tLinkLocalAddr.tIPv6), &m_tLinkLocalIP, sizeof(T_IPv6Addr));

    tLinkLocalAddr.toStr(cIpv6);

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "m_dwDeviceID : %d, m_wPortID : %d, m_ucQueueNum : %d, "
               "m_ucDevType : %d, m_ucLinkType : %d, m_ucVlanNum : %d, "
               "m_ucIPNum : %d, m_dwPrimaryIP : %u, m_tLinkLocalIP : %s \n",
               m_dwDeviceID,
               m_wPortID,
               m_ucQueueNum,
               m_ucDevType,
               m_ucLinkType,
               m_ucVlanNum,
               m_ucIPNum,
               m_dwPrimaryIP,
               cIpv6.toChar());

    for (WORD32 dwIndex = 0; dwIndex < m_ucIPNum; dwIndex++)
    {
        CString<IPV6_STRING_LEN> cIPAddr;
        T_IPAddr &rtIPAddr = m_atIPAddr[dwIndex];

        rtIPAddr.toStr(cIPAddr);

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
                   "VlanID : %d, IPAddr : %s\n",
                   m_adwVlanID[dwIndex],
                   cIPAddr.toChar());
    }
}


WORD32 CEthDevice::InitIPConfig(T_DpdkEthDevJsonCfg &rtCfg)
{
    WORD32 dwIPNum   = rtCfg.dwIpNum;
    WORD32 dwVlanNum = rtCfg.dwVlanNum;

    for (WORD32 dwIndex = 0; dwIndex < dwIPNum; dwIndex++)
    {
        T_DpdkEthIPJsonCfg &rtIPCfg = rtCfg.atIP[dwIndex];

        if (E_IPV4_TYPE == rtIPCfg.dwIPType)
        {
            m_atIPAddr[m_ucIPNum].SetIPv4(rtIPCfg.aucIpv4Addr);

            /* ȡ��һ������IPv4��ַ��Ϊ��IP */
            if (0 == m_dwPrimaryIP)
            {
                m_dwPrimaryIP = m_atIPAddr[m_ucIPNum].tIPv4.dwIPAddr;
            }
        }
        else
        {
            m_atIPAddr[m_ucIPNum].SetIPv6(rtIPCfg.aucIpv6Addr);
        }

        m_ucIPNum++;
    }

    for (WORD32 dwIndex = 0; dwIndex < dwVlanNum; dwIndex++)
    {
        T_DpdkEthVlanJsonCfg &rtVlanCfg = rtCfg.atVlan[dwIndex];

        m_adwVlanID[m_ucIPNum] = rtVlanCfg.dwVlanID;

        if (E_IPV4_TYPE == rtVlanCfg.tIP.dwIPType)
        {
            m_atIPAddr[m_ucIPNum].SetIPv4(rtVlanCfg.tIP.aucIpv4Addr);
        }
        else
        {
            m_atIPAddr[m_ucIPNum].SetIPv6(rtVlanCfg.tIP.aucIpv6Addr);
        }

        m_ucIPNum++;
    }

    return SUCCESS;
}


/* ����EUI-64������IPv6 Link-Local Address */
WORD32 CEthDevice::InitLinkLocalIP(T_MacAddr &rtAddr, CIPTable &rIPTalbe)
{
    m_tLinkLocalIP.aucIPAddr[0] = 0xFE;
    m_tLinkLocalIP.aucIPAddr[1] = 0x80;

    BYTE ucMask = 1 << 1;  /* ��7λ */

    m_tLinkLocalIP.aucIPAddr[8]  = rtAddr.aucMacAddr[0] ^ ucMask;
    m_tLinkLocalIP.aucIPAddr[9]  = rtAddr.aucMacAddr[1];
    m_tLinkLocalIP.aucIPAddr[10] = rtAddr.aucMacAddr[2];
    m_tLinkLocalIP.aucIPAddr[11] = 0xFF;
    m_tLinkLocalIP.aucIPAddr[12] = 0xFE;
    m_tLinkLocalIP.aucIPAddr[13] = rtAddr.aucMacAddr[3];
    m_tLinkLocalIP.aucIPAddr[14] = rtAddr.aucMacAddr[4];
    m_tLinkLocalIP.aucIPAddr[15] = rtAddr.aucMacAddr[5];

    m_atIPAddr[m_ucIPNum++].SetIPv6(m_tLinkLocalIP);

    rIPTalbe.RegistIP(&m_tLinkLocalIP, m_dwDeviceID, FALSE, INVALID_DWORD);

    /* ע��ಥ��ַFF02::1, FF02::1:FFxx:xxxx �����ಥ��ַ */
    T_IPv6Addr  tNodeMultiAddr;
    T_IPv6Addr  tLinkMultiAddr;

    tNodeMultiAddr.alwIPAddr[0] = 0;
    tNodeMultiAddr.alwIPAddr[1] = 0;
    tLinkMultiAddr.alwIPAddr[0] = 0;
    tLinkMultiAddr.alwIPAddr[1] = 0;

    tNodeMultiAddr.aucIPAddr[0]  = 0xFF;
    tNodeMultiAddr.aucIPAddr[1]  = 0x02;
    tNodeMultiAddr.aucIPAddr[15] = 0x01;

    tLinkMultiAddr.aucIPAddr[0]  = 0xFF;
    tLinkMultiAddr.aucIPAddr[1]  = 0x02;
    tLinkMultiAddr.aucIPAddr[11] = 0x01;
    tLinkMultiAddr.aucIPAddr[12] = 0xFF;
    tLinkMultiAddr.aucIPAddr[13] = m_tLinkLocalIP.aucIPAddr[13];
    tLinkMultiAddr.aucIPAddr[14] = m_tLinkLocalIP.aucIPAddr[14];
    tLinkMultiAddr.aucIPAddr[15] = m_tLinkLocalIP.aucIPAddr[15];

    m_atIPAddr[m_ucIPNum++].SetIPv6(tNodeMultiAddr);
    m_atIPAddr[m_ucIPNum++].SetIPv6(tLinkMultiAddr);

    rIPTalbe.RegistIP(&tNodeMultiAddr, m_dwDeviceID, FALSE, INVALID_DWORD);
    rIPTalbe.RegistIP(&tLinkMultiAddr, m_dwDeviceID, FALSE, INVALID_DWORD);

    return SUCCESS;
}


