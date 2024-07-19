

#ifndef _DPDK_NET_ICMPV6_H_
#define _DPDK_NET_ICMPV6_H_


#include "dpdk_net_interface.h"
#include "dpdk_net_arp_table.h"


typedef enum tagE_Icmpv6Type
{
    E_ICMP6_TYPE_DUR  = 1,    /* Ŀ�Ĳ��ɴ� */
    E_ICMP6_TYPE_PTB  = 2,    /* ����̫�� */
    E_ICMP6_TYPE_TE   = 3,    /* ��ʱ */
    E_ICMP6_TYPE_PP   = 4,    /* �������� */

    E_ICMP6_TYPE_EREQ = 128,  /* EchoRequest */
    E_ICMP6_TYPE_EREP = 129,  /* EchoReply */
    E_ICMP6_TYPE_MLQ  = 130,  /* Multicast Listener Query */
    E_ICMP6_TYPE_MLR  = 131,  /* Multicast Listener Report */
    E_ICMP6_TYPE_MLD  = 132,  /* Multicast Listener Done */
    E_ICMP6_TYPE_RS   = 133,  /* Router Solicitation */
    E_ICMP6_TYPE_RA   = 134,  /* Router Advertisement */
    E_ICMP6_TYPE_NS   = 135,  /* Neighbor Solicitation */
    E_ICMP6_TYPE_NA   = 136,  /* Neighbor Advertisement */
    E_ICMP6_TYPE_RD   = 137,  /* Redirect */
    E_ICMP6_TYPE_MRA  = 151,  /* Multicast Router Advertisement */
    E_ICMP6_TYPE_MRS  = 152,  /* Multicast Router Solicitation */
    E_ICMP6_TYPE_MRT  = 153,  /* Multicast Router Termination */
}E_Icmpv6Type;


class CIcmpV6Stack : public CNetStack
{
public :
    CIcmpV6Stack ();
    virtual ~CIcmpV6Stack();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

    /* ���ձ��Ĵ��� */
    virtual WORD32 RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead);

    /* ������Ŀ��IP��NS����, ���ڲ�ѯTargetIP��Ӧ��MAC��ַ */
    WORD32 SendNeighborSolication(CDevQueue *pQueue,
                                  WORD64     lwOption,
                                  T_IPAddr  &rtTargetIP,
                                  T_IPAddr  &rtSrcIP,
                                  T_IPAddr  &rtDstIP,
                                  T_MacAddr &rtDstMac,
                                  WORD32     dwVlanID = 0);

protected :
    T_MBuf * EncodeNeighborSolicitation(BYTE     *pSrcMacAddr,
                                        BYTE     *pDstMacAddr,
                                        WORD32    dwDeviceID,
                                        WORD32    dwVlanID,
                                        T_IPAddr &rtSrcIP,
                                        T_IPAddr &rtDstIP,
                                        T_IPAddr &rtTargetIP,
                                        WORD64    lwOption,
                                        struct rte_mempool *pMBufPool);

    /* ƥ�����ಥ��ַ */
    BOOL IsMultiCastAddr(BYTE *pDstMac);

    /* ƥ������ಥ��ַ */
    BOOL IsMultiCastIP(T_IPAddr &rtIPAddr);

    WORD32 ProcNeighborSolicatation(CAppInterface  *pApp,
                                    T_OffloadInfo  &rtInfo,
                                    T_EthHead      *ptEthHead,
                                    T_Ipv6Head     *ptIpv6Head,
                                    T_IcmpHead     *ptIcmpHead);

    T_MBuf * EncodeNeighborAdvertisement(BYTE      *pSrcMacAddr,
                                         BYTE      *pDstMacAddr,
                                         WORD32     dwDeviceID,
                                         WORD32     dwVlanID,
                                         T_IPAddr  &rtSrcIP,
                                         T_IPAddr  &rtDstIP,
                                         struct rte_mempool *pMBufPool);

    WORD32 ProcNeighborAdvertisement(T_EthHead  *ptEthHead,
                                     T_Ipv6Head *ptIpv6Head,
                                     WORD32      dwDevID);

    WORD32 UpdateNeighbor(WORD32 dwDevID, T_IPAddr &rtIP, BYTE *pMacAddr);

    WORD32 ProcEchoRequest(CAppInterface  *pApp,
                           T_OffloadInfo  &rtInfo,
                           T_EthHead      *ptEthHead,
                           T_Ipv6Head     *ptIpv6Head,
                           T_IcmpHead     *ptIcmpHead);

    T_MBuf * EncodeReply(BYTE      *pSrcMacAddr,
                         BYTE      *pDstMacAddr,
                         WORD32     dwDeviceID,
                         WORD32     dwVlanID,
                         T_IPAddr  &rtSrcIP,
                         T_IPAddr  &rtDstIP,
                         WORD16     wIdentify,
                         WORD16     wSeqNum,
                         BYTE      *pIcmpPayLoad,
                         WORD16     wPayLoadLen,
                         struct rte_mempool *pMBufPool);

protected :
    CSpinLock     m_cLock;      /* ��Ե����߳�(����CArpStack���߳�) */
    CArpTable    *m_pArpTable;
};


/* ƥ�����ಥ��ַ */
inline BOOL CIcmpV6Stack::IsMultiCastAddr(BYTE *pDstMac)
{
    return ((0x33 == pDstMac[0])
         && (0x33 == pDstMac[1])
         && (0xFF == pDstMac[2]));
}


/* ƥ������ಥ��ַ */
inline BOOL CIcmpV6Stack::IsMultiCastIP(T_IPAddr &rtIPAddr)
{
    return (0xFF == rtIPAddr.tIPv6.aucIPAddr[0]);
}


#endif


