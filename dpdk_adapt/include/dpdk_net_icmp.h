

#ifndef _DPDK_NET_ICMP_H_
#define _DPDK_NET_ICMP_H_


#include "dpdk_net_interface.h"


class CIcmpStack : public CNetStack
{
public :
    CIcmpStack ();
    virtual ~CIcmpStack();

    virtual WORD32 Initialize(CCentralMemPool *pMemInterface);

    /* 接收报文处理 */
    virtual WORD32 RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead);

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


