

#ifndef _DPDK_NET_ICMP_H_
#define _DPDK_NET_ICMP_H_


#include "dpdk_net_interface.h"


class CIcmpStack : public CNetStack
{
public :
    CIcmpStack ();
    virtual ~CIcmpStack();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

    /* wProto : 低层协议栈类型(0 : EtherNet) */
    virtual WORD32 RecvEthPacket(CAppInterface *pApp,
                                 WORD16         wProto,
                                 WORD32         dwDevID,
                                 WORD32         dwPortID,
                                 WORD32         dwQueueID,
                                 T_MBuf        *pMBuf,
                                 T_EthHead     *ptEthHead);

protected :
    WORD32 ProcIcmpRequest(BYTE          *pSrcMacAddr,
                           WORD32         dwSrcIP,
                           WORD32         dwDstIP,
                           T_Ipv4Head    *ptIpv4Head,
                           T_IcmpHead    *ptIcmpHead,
                           CAppInterface *pApp);

    T_MBuf * EncodeIcmpReply(BYTE               *pSrcMacAddr,
                             BYTE               *pDstMacAddr,
                             WORD32              dwSrcIP,
                             WORD32              dwDstIP,
                             WORD16              wIdentify,
                             WORD16              wSeqNum,
                             BYTE               *pIcmpPayLoad,
                             WORD16              wPayLoadLen,
                             struct rte_mempool *pMBufPool);

    WORD16 CalcIcmpCheckSum(WORD16 *pwAddr, WORD32 dwCount);
};


#endif


