

#include "dpdk_net_vlan.h"
#include "dpdk_net_ipv4.h"
#include "dpdk_net_ipv6.h"


CVlanStack::CVlanStack ()
{
    m_pIPv4Stack = NULL;
    m_pIPv6Stack = NULL;
}


CVlanStack::~CVlanStack()
{
    if (NULL != m_pIPv4Stack)
    {
        delete m_pIPv4Stack;
        m_pMemInterface->Free((BYTE *)m_pIPv4Stack);
    }

    if (NULL != m_pIPv6Stack)
    {
        delete m_pIPv6Stack;
        m_pMemInterface->Free((BYTE *)m_pIPv6Stack);
    }

    m_pIPv4Stack = NULL;
    m_pIPv6Stack = NULL;
}


WORD32 CVlanStack::Initialize(CCentralMemPool *pMemInterface)
{
    CNetStack::Initialize(pMemInterface);

    BYTE *pIPv4Mem = m_pMemInterface->Malloc(sizeof(CIPv4Stack));
    BYTE *pIPv6Mem = m_pMemInterface->Malloc(sizeof(CIPv6Stack));

    if ( (NULL == pIPv4Mem)
      || (NULL == pIPv6Mem))
    {
        assert(0);
    }

    m_pIPv4Stack = new (pIPv4Mem) CIPv4Stack();
    m_pIPv6Stack = new (pIPv6Mem) CIPv6Stack();

    m_pIPv4Stack->Initialize(pMemInterface);
    m_pIPv6Stack->Initialize(pMemInterface);

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

    /* TBD : 将报文交给VLAN实例处理 */

    return SUCCESS;
}


