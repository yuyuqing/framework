

#include "dpdk_net_ipsec.h"


CIpSecStack::CIpSecStack ()
{
}


CIpSecStack::~CIpSecStack()
{
}


WORD32 CIpSecStack::Initialize(CCentralMemPool *pMemInterface)
{
    return SUCCESS;
}


/* ���ձ��Ĵ��� */
WORD32 CIpSecStack::RecvPacket(CAppInterface *pApp,
                               T_OffloadInfo &rtInfo,
                               T_MBuf        *pMBuf,
                               CHAR          *pHead)
{
    return SUCCESS;
}


