

#include "base_shm_channel.h"


CChannelTpl::CChannelTpl()
{
}


CChannelTpl::~CChannelTpl()
{
}


WORD32 CChannelTpl::Initialize(WORD32  dwKeyS,
                               WORD32  dwKeyR,
                               VOID   *pVirtAddrS,
                               VOID   *pVirtAddrR)
{
    return SUCCESS;
}


/* ���ͷ������ڴ� */
BYTE * CChannelTpl::Malloc(WORD32 dwSize, WORD32 dwPoint)
{
    return NULL;
}


/* ���շ��ͷ��ڴ� */
WORD32 CChannelTpl::Free(BYTE *pAddr)
{
    return SUCCESS;
}


/* ���ͷ��ͷ��ڴ�, ����쳣���� */
WORD32 CChannelTpl::LocalFree(BYTE *pAddr)
{
    return SUCCESS;
}


/* ���շ����ü���++���� */
VOID CChannelTpl::IncRefCount(BYTE *pAddr)
{
}


WORD32 CChannelTpl::SendMessage(VOID *ptMsg)
{
    return SUCCESS;
}


WORD32 CChannelTpl::SendMessage(WORD32 dwNum, VOID **ptMsg)
{
    return SUCCESS;
}


WORD32 CChannelTpl::RecvMessage(VOID *pObj, PSyncRecvMsg pFunc)
{
    return SUCCESS;
}


WORD32 CChannelTpl::Post()
{
    return SUCCESS;
}


WORD32 CChannelTpl::Wait()
{
    return SUCCESS;
}


