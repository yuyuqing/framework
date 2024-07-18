

#include "dpdk_net_icmpv6.h"


CIcmpV6Stack::CIcmpV6Stack ()
{
}


CIcmpV6Stack::~CIcmpV6Stack()
{
}


WORD32 CIcmpV6Stack::Initialize(CCentralMemPool *pMemInterface)
{
    return SUCCESS;
}


/* 接收报文处理 */
WORD32 CIcmpV6Stack::RecvPacket(CAppInterface *pApp,
                                T_OffloadInfo &rtInfo,
                                T_MBuf        *pMBuf,
                                CHAR          *pHead)
{
    return SUCCESS;
}


