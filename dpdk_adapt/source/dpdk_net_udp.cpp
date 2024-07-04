

#include "dpdk_net_udp.h"


CUdpStack::CUdpStack ()
{
}


CUdpStack::~CUdpStack()
{
}


WORD32 CUdpStack::Initialize(CCentralMemPool *pMemInterface)
{
    return SUCCESS;
}


/* 接收报文处理; pHead : UDP头 */
WORD32 CUdpStack::RecvPacket(CAppInterface *pApp,
                             T_OffloadInfo &rtInfo,
                             T_MBuf        *pMBuf,
                             CHAR          *pHead)
{
    return SUCCESS;
}


