

#include "dpdk_net_tcp.h"


CTcpStack::CTcpStack ()
{
}


CTcpStack::~CTcpStack()
{
}


WORD32 CTcpStack::Initialize(CCentralMemPool *pMemInterface)
{
    return SUCCESS;
}


/* ���ձ��Ĵ���; pHead : TCPͷ */
WORD32 CTcpStack::RecvPacket(CAppInterface *pApp,
                             T_OffloadInfo &rtInfo,
                             T_MBuf        *pMBuf,
                             CHAR          *pHead)
{
    return SUCCESS;
}


