

#include "dpdk_net_ipv6.h"
#include "dpdk_net_icmpv6.h"
#include "dpdk_net_ipsec.h"
#include "dpdk_net_udp.h"
#include "dpdk_net_sctp.h"
#include "dpdk_net_tcp.h"
#include "dpdk_app_eth.h"
#include "dpdk_device_eth.h"


CIPv6Stack::CIPv6Stack ()
{
    m_pIcmpStack  = NULL;
    m_pIpSecStack = NULL;
    m_pUdpStack   = NULL;
    m_pSctpStack  = NULL;
    m_pTcpStack   = NULL;
}


CIPv6Stack::~CIPv6Stack()
{
    if (NULL != m_pIcmpStack)
    {
        delete m_pIcmpStack;
        m_pMemInterface->Free((BYTE *)m_pIcmpStack);
    }

    if (NULL != m_pIpSecStack)
    {
        delete m_pIpSecStack;
        m_pMemInterface->Free((BYTE *)m_pIpSecStack);
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

    m_pIcmpStack  = NULL;
    m_pIpSecStack = NULL;
    m_pUdpStack   = NULL;
    m_pSctpStack  = NULL;
    m_pTcpStack   = NULL;
}


WORD32 CIPv6Stack::Initialize(CCentralMemPool *pMemInterface)
{
    CNetStack::Initialize(pMemInterface);

    BYTE *pIcmpMem  = m_pMemInterface->Malloc(sizeof(CIcmpV6Stack));
    BYTE *pIpSecMem = m_pMemInterface->Malloc(sizeof(CIpSecStack));
    BYTE *pUdpMem   = m_pMemInterface->Malloc(sizeof(CUdpStack));
    BYTE *pSctpMem  = m_pMemInterface->Malloc(sizeof(CSctpStack));
    BYTE *pTcpMem   = m_pMemInterface->Malloc(sizeof(CTcpStack));

    if ( (NULL == pIcmpMem)
      || (NULL == pIpSecMem)
      || (NULL == pUdpMem)
      || (NULL == pSctpMem)
      || (NULL == pTcpMem))
    {
        assert(0);
    }

    m_pIcmpStack  = new (pIcmpMem) CIcmpV6Stack();
    m_pIpSecStack = new (pIpSecMem) CIpSecStack();
    m_pUdpStack   = new (pUdpMem)  CUdpStack();
    m_pSctpStack  = new (pSctpMem) CSctpStack();
    m_pTcpStack   = new (pTcpMem)  CTcpStack();

    m_pIcmpStack->Initialize(pMemInterface);
    m_pIpSecStack->Initialize(pMemInterface);
    m_pUdpStack->Initialize(pMemInterface);
    m_pSctpStack->Initialize(pMemInterface);
    m_pTcpStack->Initialize(pMemInterface);

    return SUCCESS;
}


/* 接收报文处理; pHead : IPv6头 */
WORD32 CIPv6Stack::RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead)
{
    T_IPAddr    tIpv6Addr;
    T_Ipv6Head *ptIpv6Head = (T_Ipv6Head *)pHead;
    CEthApp    *pEthApp    = (CEthApp *)pApp;
    CDevQueue  *pQueue     = pEthApp->GetQueue();
    CEthDevice *pDevice    = (CEthDevice *)(pQueue->GetDevice());

    tIpv6Addr.eType = E_IPV6_TYPE;
    memcpy(tIpv6Addr.tIPv6.aucIPAddr, ptIpv6Head->dst_addr, IPV6_ADDR_LEN);

    if (FALSE == pDevice->IsMatch(rtInfo.dwVlanID, tIpv6Addr))
    {
        CString<IPV6_STRING_LEN> cIPAddr;
        tIpv6Addr.toStr(cIPAddr);

        LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_WARN, TRUE,
                   "DstAddr is not match; DeviceID : %d, PortID : %d, "
                   "QueueID : %d, VlanID : %d, DstIPAddr : %s\n",
                   rtInfo.dwDeviceID,
                   rtInfo.dwPortID,
                   rtInfo.dwQueueID,
                   rtInfo.dwVlanID,
                   cIPAddr.toChar());

        return FAIL;
    }

    switch (rtInfo.ucL4Proto)
    {
    case IPPROTO_ICMPV6 :
        {
            return m_pIcmpStack->RecvPacket(pApp,
                                            rtInfo,
                                            pMBuf,
                                            (pHead + rtInfo.wL3Len));
        }
        break ;

    case IPPROTO_UDP :
        {
            return m_pUdpStack->RecvPacket(pApp,
                                            rtInfo,
                                            pMBuf,
                                            (pHead + rtInfo.wL3Len));
        }
        break ;

    case IPPROTO_SCTP :
        {
            return m_pSctpStack->RecvPacket(pApp,
                                            rtInfo,
                                            pMBuf,
                                            (pHead + rtInfo.wL3Len));
        }
        break ;

    case IPPROTO_TCP :
        {
            return m_pTcpStack->RecvPacket(pApp,
                                            rtInfo,
                                            pMBuf,
                                            (pHead + rtInfo.wL3Len));
        }
        break ;

    case IPPROTO_AH  :
    case IPPROTO_ESP :
        {
            return m_pIpSecStack->RecvPacket(pApp,
                                             rtInfo,
                                             pMBuf,
                                             (pHead + rtInfo.wL3Len));
        }
        break ;

    default :
        break ;
    }

    return SUCCESS;
}


