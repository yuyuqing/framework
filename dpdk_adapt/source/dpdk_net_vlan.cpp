

#include "dpdk_mgr.h"
#include "dpdk_net_vlan.h"
#include "dpdk_net_ipv4.h"
#include "dpdk_net_ipv6.h"


CVlanStack::CVlanStack ()
{
    m_pVlanTable = NULL;
    m_pArpStack  = NULL;
    m_pIPv4Stack = NULL;
    m_pIPv6Stack = NULL;
}


CVlanStack::~CVlanStack()
{
    m_pVlanTable = NULL;
    m_pArpStack  = NULL;
    m_pIPv4Stack = NULL;
    m_pIPv6Stack = NULL;
}


WORD32 CVlanStack::Initialize(CCentralMemPool *pMemInterface,
                              CNetStack       *pArpStack,
                              CNetStack       *pIPv4Stack,
                              CNetStack       *pIPv6Stack)
{
    CNetStack::Initialize(pMemInterface);

    m_pVlanTable = &(g_pDpdkMgr->GetVlanTable());

    m_pArpStack  = pArpStack;
    m_pIPv4Stack = pIPv4Stack;
    m_pIPv6Stack = pIPv6Stack;

    return SUCCESS;
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
WORD32 CVlanStack::RecvEthPacket(CAppInterface *pApp,
                                 WORD16         wProto,
                                 WORD32         dwDevID,
                                 WORD32         dwPortID,
                                 WORD32         dwQueueID,
                                 T_MBuf        *pMBuf,
                                 T_EthHead     *ptEthHead)
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

    CVlanInst *pVlanInst = m_pVlanTable->FindVlan(dwDevID, dwVlanID);
    if (NULL == pVlanInst)
    {
        return FAIL;
    }

    switch (wEthType)
    {
    case RTE_ETHER_TYPE_ARP :
        {
            return m_pArpStack->RecvVlanPacket(pApp,
                                               pVlanInst,
                                               dwDevID,
                                               dwPortID,
                                               dwQueueID,
                                               pMBuf,
                                               (CHAR *)(ptVlanHead + 1));
        }
        break ;

    case RTE_ETHER_TYPE_IPV4 :
        {
            return m_pIPv4Stack->RecvVlanPacket(pApp,
                                                pVlanInst,
                                                dwDevID,
                                                dwPortID,
                                                dwQueueID,
                                                pMBuf,
                                                (CHAR *)(ptVlanHead + 1));
        }
        break ;

    case RTE_ETHER_TYPE_IPV6 :
        {
            return m_pIPv6Stack->RecvVlanPacket(pApp,
                                                pVlanInst,
                                                dwDevID,
                                                dwPortID,
                                                dwQueueID,
                                                pMBuf,
                                                (CHAR *)(ptVlanHead + 1));
        }
        break ;

    default :
        {
        }
        break ;
    }

    return SUCCESS;
}


