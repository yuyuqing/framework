

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


/* 发送方申请内存 */
BYTE * CChannelTpl::Malloc(WORD32 dwSize, WORD32 dwPoint)
{
    return NULL;
}


/* 接收方释放内存 */
WORD32 CChannelTpl::Free(BYTE *pAddr)
{
    return SUCCESS;
}


/* 发送方释放内存, 针对异常流程 */
WORD32 CChannelTpl::LocalFree(BYTE *pAddr)
{
    return SUCCESS;
}


/* 接收方引用计数++操作 */
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


