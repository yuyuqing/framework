

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


/* ���ձ��Ĵ���; pHead : SCTPͷ */
WORD32 CSctpStack::RecvPacket(CAppInterface *pApp,
                              T_OffloadInfo &rtInfo,
                              T_MBuf        *pMBuf,
                              CHAR          *pHead)
{
    return SUCCESS;
}


