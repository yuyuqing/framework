

#include "dpdk_net_ipv6.h"


CIPv6Stack::CIPv6Stack ()
{
}


CIPv6Stack::~CIPv6Stack()
{
}


WORD32 CIPv6Stack::Initialize(CCentralMemPool *pMemInterface)
{
    return SUCCESS;
}


/* 接收报文处理; pHead : IPv6头 */
WORD32 CIPv6Stack::RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead)
{
    return SUCCESS;
}


