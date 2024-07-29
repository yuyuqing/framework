

#include "dpdk_net_ipv6.h"
#include "dpdk_net_icmpv6.h"
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

    m_pIcmpStack  = NULL;
    m_pIpSecStack = NULL;
    m_pUdpStack   = NULL;
    m_pSctpStack  = NULL;
    m_pTcpStack   = NULL;
}


WORD32 CIPv6Stack::Initialize(CCentralMemPool *pMemInterface,
                              CNetStack       *pIpSecStack,
                              CNetStack       *pUdpStack,
                              CNetStack       *pSctpStack,
                              CNetStack       *pTcpStack)
{
    CNetStack::Initialize(pMemInterface);

    BYTE *pIcmpMem = m_pMemInterface->Malloc(sizeof(CIcmpV6Stack));
    if ((NULL == pIcmpMem))
    {
        assert(0);
    }

    m_pIcmpStack  = new (pIcmpMem) CIcmpV6Stack();
    m_pIpSecStack = pIpSecStack;
    m_pUdpStack   = pUdpStack;
    m_pSctpStack  = pSctpStack;
    m_pTcpStack   = pTcpStack;

    m_pIcmpStack->Initialize(pMemInterface);

    return SUCCESS;
}


/* ���ձ��Ĵ���; pHead : IPv6ͷ */
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
        {
            /* ���� Hop-By-Hop / Destination / Routing / Fragment ��ͷ��ѡ�� */

            CString<IPV6_STRING_LEN> cIPAddr;
            tIpv6Addr.toStr(cIPAddr);

            LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_WARN, TRUE,
                       "DstAddr is not match; DeviceID : %d, PortID : %d, "
                       "QueueID : %d, VlanID : %d, NextHeader : %d, "
                       "DstIPAddr : %s\n",
                       rtInfo.dwDeviceID,
                       rtInfo.dwPortID,
                       rtInfo.dwQueueID,
                       rtInfo.dwVlanID,
                       rtInfo.ucL4Proto,
                       cIPAddr.toChar());
        }
        break ;
    }

    return SUCCESS;
}


