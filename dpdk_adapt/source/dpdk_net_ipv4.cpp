

#include "dpdk_net_ipv4.h"
#include "dpdk_net_icmp.h"


CIPv4Stack::CIPv4Stack ()
{
    m_pIcmpStack  = NULL;
    m_pIpSecStack = NULL;
    m_pUdpStack   = NULL;
    m_pSctpStack  = NULL;
    m_pTcpStack   = NULL;
}


CIPv4Stack::~CIPv4Stack()
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


WORD32 CIPv4Stack::Initialize(CCentralMemPool *pMemInterface,
                              CNetStack       *pIpSecStack,
                              CNetStack       *pUdpStack,
                              CNetStack       *pSctpStack,
                              CNetStack       *pTcpStack)
{
    CNetStack::Initialize(pMemInterface);

    BYTE *pIcmpMem = m_pMemInterface->Malloc(sizeof(CIcmpStack));
    if ((NULL == pIcmpMem))
    {
        assert(0);
    }

    m_pIcmpStack  = new (pIcmpMem) CIcmpStack();
    m_pIpSecStack = pIpSecStack;
    m_pUdpStack   = pUdpStack;
    m_pSctpStack  = pSctpStack;
    m_pTcpStack   = pTcpStack;

    m_pIcmpStack->Initialize(pMemInterface);

    return SUCCESS;
}


/* 接收报文处理; pHead : IPv4头 */
WORD32 CIPv4Stack::RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead)
{
    switch (rtInfo.ucL4Proto)
    {
    case IPPROTO_ICMP :
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


