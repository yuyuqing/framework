

#include "dpdk_net_ipv4.h"
#include "dpdk_net_icmp.h"
#include "dpdk_net_udp.h"
#include "dpdk_net_sctp.h"
#include "dpdk_net_tcp.h"


CIPv4Stack::CIPv4Stack ()
{
    m_pIcmpStack = NULL;
    m_pUdpStack  = NULL;
    m_pSctpStack = NULL;
    m_pTcpStack  = NULL;
}


CIPv4Stack::~CIPv4Stack()
{
    if (NULL != m_pIcmpStack)
    {
        delete m_pIcmpStack;
        m_pMemInterface->Free((BYTE *)m_pIcmpStack);
    }

    if (NULL != m_pUdpStack)
    {
        delete m_pUdpStack;
        m_pMemInterface->Free((BYTE *)m_pUdpStack);
    }

    if (NULL != m_pSctpStack)
    {
        delete m_pSctpStack;
        m_pMemInterface->Free((BYTE *)m_pSctpStack);
    }

    if (NULL != m_pTcpStack)
    {
        delete m_pTcpStack;
        m_pMemInterface->Free((BYTE *)m_pTcpStack);
    }

    m_pIcmpStack = NULL;
    m_pUdpStack  = NULL;
    m_pSctpStack = NULL;
    m_pTcpStack  = NULL;
}


WORD32 CIPv4Stack::Initialize(CCentralMemPool *pMemInterface)
{
    CNetStack::Initialize(pMemInterface);

    BYTE *pIcmpMem = m_pMemInterface->Malloc(sizeof(CIcmpStack));
    BYTE *pUdpMem  = m_pMemInterface->Malloc(sizeof(CUdpStack));
    BYTE *pSctpMem = m_pMemInterface->Malloc(sizeof(CSctpStack));
    BYTE *pTcpMem  = m_pMemInterface->Malloc(sizeof(CTcpStack));

    if ( (NULL == pIcmpMem)
      || (NULL == pUdpMem)
      || (NULL == pSctpMem)
      || (NULL == pTcpMem))
    {
        assert(0);
    }

    m_pIcmpStack = new (pIcmpMem) CIcmpStack();
    m_pUdpStack  = new (pUdpMem)  CUdpStack();
    m_pSctpStack = new (pSctpMem) CSctpStack();
    m_pTcpStack  = new (pTcpMem)  CTcpStack();

    m_pIcmpStack->Initialize(pMemInterface);
    m_pUdpStack->Initialize(pMemInterface);
    m_pSctpStack->Initialize(pMemInterface);
    m_pTcpStack->Initialize(pMemInterface);

    return SUCCESS;
}


/* wProto : 低层协议栈类型(0 : EtherNet) */
WORD32 CIPv4Stack::RecvEthPacket(CAppInterface *pApp,
                                 WORD16         wProto,
                                 WORD32         dwDevID,
                                 WORD32         dwPortID,
                                 WORD32         dwQueueID,
                                 T_MBuf        *pMBuf,
                                 T_EthHead     *ptEthHead)
{
    T_Ipv4Head *ptIpv4Head = rte_pktmbuf_mtod_offset(pMBuf,
                                                     struct rte_ipv4_hdr *,
                                                     sizeof(struct rte_ether_hdr));
    switch (ptIpv4Head->next_proto_id)
    {
    case IPPROTO_ICMP :
        {
            return m_pIcmpStack->RecvEthPacket(pApp,
                                               RTE_ETHER_TYPE_IPV4,
                                               dwDevID,
                                               dwPortID,
                                               dwQueueID,
                                               pMBuf,
                                               ptEthHead);
        }
        break ;

    case IPPROTO_UDP :
        {
            return m_pUdpStack->RecvEthPacket(pApp,
                                              RTE_ETHER_TYPE_IPV4,
                                              dwDevID,
                                              dwPortID,
                                              dwQueueID,
                                              pMBuf,
                                              ptEthHead);
        }
        break ;

    case IPPROTO_SCTP :
        {
            return m_pSctpStack->RecvEthPacket(pApp,
                                               RTE_ETHER_TYPE_IPV4,
                                               dwDevID,
                                               dwPortID,
                                               dwQueueID,
                                               pMBuf,
                                               ptEthHead);
        }
        break ;

    case IPPROTO_TCP :
        {
            return m_pTcpStack->RecvEthPacket(pApp,
                                              RTE_ETHER_TYPE_IPV4,
                                              dwDevID,
                                              dwPortID,
                                              dwQueueID,
                                              pMBuf,
                                              ptEthHead);
        }
        break ;

    default :
        break ;
    }

    return SUCCESS;
}


