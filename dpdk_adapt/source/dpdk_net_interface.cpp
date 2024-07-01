

#include "dpdk_app_eth.h"
#include "dpdk_net_arp.h"
#include "dpdk_net_vlan.h"
#include "dpdk_net_ipv4.h"
#include "dpdk_net_ipv6.h"


CNetIntfHandler *g_pNetIntfHandler = NULL;


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
WORD32 CNetStack::RecvPacket(VOID   *pArg,
                             WORD32  dwDevID,
                             WORD32  dwPortID,
                             WORD32  dwQueueID,
                             T_MBuf *pMBuf)
{
    CEthApp   *pApp      = (CEthApp *)pArg;
    T_EthHead *ptEthHead = rte_pktmbuf_mtod(pMBuf, T_EthHead *);

    return g_pNetIntfHandler->RecvEthPacket(pApp,
                                            0,
                                            dwDevID,
                                            dwPortID,
                                            dwQueueID,
                                            pMBuf,
                                            ptEthHead);
}


CNetStack::CNetStack ()
{
    m_pMemInterface = NULL;
}


CNetStack::~CNetStack()
{
    m_pMemInterface = NULL;
}


WORD32 CNetStack::Initialize(CCentralMemPool *pMemInterface)
{
    m_pMemInterface = pMemInterface;

    return SUCCESS;
}


/* wProto : 低层协议栈类型(0 : EtherNet) */
WORD32 CNetStack::RecvEthPacket(CAppInterface *pApp,
                                WORD16         wProto,
                                WORD32         dwDevID,
                                WORD32         dwPortID,
                                WORD32         dwQueueID,
                                T_MBuf        *pMBuf,
                                T_EthHead     *ptEthHead)
{
    return SUCCESS;
}


CNetIntfHandler::CNetIntfHandler ()
{
    m_pArpStack  = NULL;
    m_pVlanStack = NULL;
    m_pIPv4Stack = NULL;
    m_pIPv6Stack = NULL;

    g_pNetIntfHandler = this;
}


CNetIntfHandler::~CNetIntfHandler()
{
    if (NULL != m_pArpStack)
    {
        delete m_pArpStack;
        m_pMemInterface->Free((BYTE *)m_pArpStack);
    }

    if (NULL != m_pVlanStack)
    {
        delete m_pVlanStack;
        m_pMemInterface->Free((BYTE *)m_pVlanStack);
    }

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

    m_pArpStack  = NULL;
    m_pVlanStack = NULL;
    m_pIPv4Stack = NULL;
    m_pIPv6Stack = NULL;

    g_pNetIntfHandler = NULL;
}


WORD32 CNetIntfHandler::Initialize(CCentralMemPool *pMemInterface)
{
    CNetStack::Initialize(pMemInterface);

    BYTE *pArpMem  = m_pMemInterface->Malloc(sizeof(CArpStack));
    BYTE *pVlanMem = m_pMemInterface->Malloc(sizeof(CVlanStack));
    BYTE *pIPv4Mem = m_pMemInterface->Malloc(sizeof(CIPv4Stack));
    BYTE *pIPv6Mem = m_pMemInterface->Malloc(sizeof(CIPv6Stack));

    if ( (NULL == pArpMem)
      || (NULL == pVlanMem)
      || (NULL == pIPv4Mem)
      || (NULL == pIPv6Mem))
    {
        assert(0);
    }

    m_pArpStack  = new (pArpMem) CArpStack();
    m_pVlanStack = new (pVlanMem) CVlanStack();
    m_pIPv4Stack = new (pIPv4Mem) CIPv4Stack();
    m_pIPv6Stack = new (pIPv6Mem) CIPv6Stack();

    m_pArpStack->Initialize(pMemInterface);
    m_pVlanStack->Initialize(pMemInterface);
    m_pIPv4Stack->Initialize(pMemInterface);
    m_pIPv6Stack->Initialize(pMemInterface);

    return SUCCESS;
}


/* wProto : 低层协议栈类型(0 : EtherNet) */
WORD32 CNetIntfHandler::RecvEthPacket(CAppInterface *pApp,
                                      WORD16         wProto,
                                      WORD32         dwDevID,
                                      WORD32         dwPortID,
                                      WORD32         dwQueueID,
                                      T_MBuf        *pMBuf,
                                      T_EthHead     *ptEthHead)
{
    WORD16 wEthType = rte_be_to_cpu_16(ptEthHead->ether_type);
    switch (wEthType)
    {
    case RTE_ETHER_TYPE_VLAN :
        {
            return m_pVlanStack->RecvEthPacket(pApp,
                                               0,
                                               dwDevID,
                                               dwPortID,
                                               dwQueueID,
                                               pMBuf,
                                               ptEthHead);
        }
        break ;

    case RTE_ETHER_TYPE_ARP :
        {
            return m_pArpStack->RecvEthPacket(pApp,
                                              0,
                                              dwDevID,
                                              dwPortID,
                                              dwQueueID,
                                              pMBuf,
                                              ptEthHead);
        }
        break ;

    case RTE_ETHER_TYPE_IPV4 :
        {
            return m_pIPv4Stack->RecvEthPacket(pApp,
                                               0,
                                               dwDevID,
                                               dwPortID,
                                               dwQueueID,
                                               pMBuf,
                                               ptEthHead);
        }
        break ;

    case RTE_ETHER_TYPE_IPV6 :
        {
            return m_pIPv6Stack->RecvEthPacket(pApp,
                                               0,
                                               dwDevID,
                                               dwPortID,
                                               dwQueueID,
                                               pMBuf,
                                               ptEthHead);
        }
        break ;

    default :
        {
        }
        break ;
    }

    return SUCCESS;
}


