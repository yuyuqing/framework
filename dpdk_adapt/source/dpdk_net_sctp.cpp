

#include "dpdk_net_sctp.h"


CSctpStack::CSctpStack ()
{
}


CSctpStack::~CSctpStack()
{
}


WORD32 CSctpStack::Initialize(CCentralMemPool *pMemInterface)
{
    return SUCCESS;
}


/* 接收报文处理; pHead : SCTP头 */
WORD32 CSctpStack::RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead)
{
    return SUCCESS;
}


