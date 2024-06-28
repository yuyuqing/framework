

#include "dpdk_app_eth.h"


DEFINE_APP(CEthApp);


/********************************************************************
 **************************** Ethernet ******************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                Destination Hardware Address                  | *
 * |                              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ *
 * |                              |                               | *
 * |                   Source Hardware Address                    | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |         Length/Type          |             Data              | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 **************************** Ethernet ******************************

 ****************************** VLAN ********************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                Destination Hardware Address                  | *
 * |                              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ *
 * |                              |                               | *
 * |                   Source Hardware Address                    | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |              TPID            | PRI |CFI|       VID           | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |         Length/Type          |             Data              | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 ****************************** VLAN ********************************

 ******************************* ARP ********************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |          Hardware Type       |        Protocol Type          | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * | Hard Addr Len | ProtAddrLen  |           Operation           | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                   Sender Hardware Address                    | *
 * |                              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ *
 * |                              |      Sender IP Address        | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                   Target Hardware Address                    | *
 * |                              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ *
 * |                              |      Target IP Address        | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |           Identifier         |        Sequence Number        | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 ******************************* ARP ********************************

 *******************************  IP ********************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |Version|  IHL |Type of Service|          Total Length         | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |         Identification       |Flags|      Fragment Offset    | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * | Time to Live |    Protocol   |         Header Checksum       | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                       Source Address                         | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                    Destination Address                       | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                    Options                    |    Padding   | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 *******************************  IP ********************************

 ****************************** ICMP ********************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 7 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |     Type     |     Code      |          Checksum             | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |          Identifier          |        Sequence Number        | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                            Data                              | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 ****************************** ICMP ********************************

 ******************************* UDP ********************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 7 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |          Source Port         |       Destination Port        | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |            Length            |           Checksum            | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                         data octets                          | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 ******************************* UDP ********************************

 ******************************* TCP ********************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 7 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |          Source Port         |       Destination Port        | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                       Sequence Number                        | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                   Acknowledgment Number                      | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |  Data |          |U|A|P|R|S|F|                               | *
 * | Offset| Reserved |R|C|S|S|Y|I|            Window             | *
 * |       |          |G|K|H|T|N|N|                               | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |           Checksum           |         Urgent Pointer        | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                   Options                    |    Padding    | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                            data                              | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 ******************************* TCP ********************************
 *******************************************************************/
WORD32 CEthApp::RecvPacket(VOID   *pArg,
                           WORD32  dwDevID,
                           WORD32  dwPortID,
                           WORD32  dwQueueID,
                           T_MBuf *pMBuf)
{
    CEthApp *pApp = (CEthApp *)pArg;

    WORD16     wEthType  = INVALID_WORD;
    T_EthHead *ptEthHead = NULL;

    ptEthHead = rte_pktmbuf_mtod(pMBuf, T_EthHead *);
    wEthType  = rte_be_to_cpu_16(ptEthHead->ether_type);

    switch (wEthType)
    {
    case RTE_ETHER_TYPE_VLAN :
        {
            return pApp->RecvVlanPacket(pMBuf, ptEthHead);
        }
        break ;

    case RTE_ETHER_TYPE_ARP :
        {
            return pApp->RecvArpPacket(pMBuf, ptEthHead);
        }
        break ;

    case RTE_ETHER_TYPE_IPV4 :
        {
            return pApp->RecvIpv4Packet(pMBuf, ptEthHead);
        }
        break ;

    case RTE_ETHER_TYPE_IPV6 :
        {
            return pApp->RecvIpv6Packet(pMBuf, ptEthHead);
        }
        break ;

    default :
        {
        }
        break ;
    }

    return SUCCESS;
}


CEthApp::CEthApp ()
    : CAppInterface(E_APP_DPDK_ETH)
{
    m_dwDeviceID = INVALID_DWORD;
    m_wPortID    = INVALID_WORD;
    m_wQueueID   = INVALID_WORD;
    m_pQueue     = NULL;
}


CEthApp::~CEthApp()
{
    m_dwDeviceID = INVALID_DWORD;
    m_wPortID    = INVALID_WORD;
    m_wQueueID   = INVALID_WORD;
    m_pQueue     = NULL;
}


WORD32 CEthApp::Init()
{
    TRACE_STACK("CEthApp::Exit()");

    if ((FALSE == m_bAssocFlag) || (1 != m_dwAssocNum) || (NULL == g_pDpdkMgr))
    {
        assert(0);
    }

    /* 如果配置为多队列, 则dwAssocID需要由DeviceID+QueueID合成 */
    WORD32 dwAssocID = m_adwAssocID[0];

    m_dwDeviceID = dwAssocID >> 2;
    m_wQueueID   = (WORD16)(dwAssocID & 0x00000003);

    CBaseDevice *pDevice = g_pDpdkMgr->FindDevice(m_dwDeviceID);
    if (NULL == pDevice)
    {
        assert(0);
    }

    m_wPortID = pDevice->GetPortID();
    m_pQueue  = pDevice->GetQueue(m_wQueueID);
    if (NULL == m_pQueue)
    {
        assert(0);
    }

    return SUCCESS;
}


WORD32 CEthApp::DeInit()
{
    TRACE_STACK("CEthApp::DeInit()");

    return SUCCESS;
}


WORD32 CEthApp::Exit(WORD32 dwMsgID, VOID *pIn, WORD16 wMsgLen)
{
    TRACE_STACK("CEthApp::Exit()");

    m_dwDeviceID = INVALID_DWORD;
    m_wPortID    = INVALID_WORD;
    m_wQueueID   = INVALID_WORD;
    m_pQueue     = NULL;

    return SUCCESS;
}


WORD32 CEthApp::Polling()
{
    return m_pQueue->RecvPacket(MBUF_BURST_NUM,
                                this,
                                (PMBufCallBack)(&CEthApp::RecvPacket));
}


/********************************************************************
 ****************************** VLAN ********************************
 * 0               1               2               3                *
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7  *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |                Destination Hardware Address                  | *
 * |                              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ *
 * |                              |                               | *
 * |                   Source Hardware Address                    | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |          TPID(0x8100)        | PRI |CFI|       VID           | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 * |         Length/Type          |             Data              | *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- *
 ****************************** VLAN ********************************
*******************************************************************/
WORD32 CEthApp::RecvVlanPacket(T_MBuf *pMBuf, T_EthHead *ptEthHead)
{
    WORD32 dwVlanID = INVALID_DWORD;
    WORD16 wVlanID  = INVALID_WORD;
    WORD16 wEthType = INVALID_WORD;

    T_VlanHead *ptVlanHead = (T_VlanHead *)(ptEthHead + 1);

    wVlanID  = (rte_be_to_cpu_16(ptVlanHead->vlan_tci)) & 0x0FFF;
    dwVlanID = wVlanID;
    wEthType = rte_be_to_cpu_16(ptVlanHead->eth_proto);

    if (RTE_ETHER_TYPE_VLAN == wEthType)
    {
        /* QinQ类型报文, 继续跳过Vlan头 */
        ptVlanHead = ptVlanHead + 1;
        dwVlanID   = dwVlanID << 12;
        wVlanID    = (rte_be_to_cpu_16(ptVlanHead->vlan_tci)) & 0x0FFF;
        dwVlanID  += wVlanID;
        wEthType   = rte_be_to_cpu_16(ptVlanHead->eth_proto);
    }

    /* 将报文交给VLAN实例处理 */

    return SUCCESS;
}


WORD32 CEthApp::RecvArpPacket(T_MBuf *pMBuf, T_EthHead *ptEthHead)
{
    return SUCCESS;
}


WORD32 CEthApp::RecvIpv4Packet(T_MBuf *pMBuf, T_EthHead *ptEthHead)
{
    return SUCCESS;
}


WORD32 CEthApp::RecvIpv6Packet(T_MBuf *pMBuf, T_EthHead *ptEthHead)
{
    return SUCCESS;
}


