

#include "base_shm_channel.h"


CShmChannel::CShmChannel()
{
    m_ptChannel = NULL;
    m_pMemPool  = NULL;
    m_eRole     = E_SHM_OBSERVER;
}


CShmChannel::~CShmChannel()
{
    if (E_SHM_MASTER == m_eRole)
    {
        CleanUp();
    }

    m_ptChannel = NULL;
    m_pMemPool  = NULL;
    m_eRole     = E_SHM_OBSERVER;
}


WORD32 CShmChannel::Initialize(E_ShmRole        eRole,
                               T_ShmChannel    *ptChannel,
                               CCentralMemPool *pMemPool,
                               WORD32           dwHdlSNodeNum,
                               WORD32           dwHdlSNodeSize,
                               WORD32           dwHdlRNodeNum,
                               WORD32           dwHdlRNodeSize)
{
    m_ptChannel = ptChannel;
    m_pMemPool  = pMemPool;
    m_eRole     = eRole;

    WORD32 dwResult = INVALID_DWORD;

    switch (eRole)
    {
    case E_SHM_MASTER :
        {
            dwResult = Create(dwHdlSNodeNum,
                              dwHdlSNodeSize,
                              dwHdlRNodeNum,
                              dwHdlRNodeSize);
        }
        break ;

    case E_SHM_SLAVE :
        {
            dwResult = Attach(dwHdlSNodeNum,
                              dwHdlSNodeSize,
                              dwHdlRNodeNum,
                              dwHdlRNodeSize);
        }
        break ;

    case E_SHM_OBSERVER :
        {
            dwResult = Attach(dwHdlSNodeNum,
                              dwHdlSNodeSize,
                              dwHdlRNodeNum,
                              dwHdlRNodeSize);
        }
        break ;

    default :
        break ;
    }

    return dwResult;
}


/* 发送方申请内存 */
BYTE * CShmChannel::Malloc(WORD32 dwSize, WORD32 dwPoint)
{
    return m_cSyncBufSender.Malloc(dwSize, dwPoint);
}


/* 接收方释放内存 */
WORD32 CShmChannel::Free(BYTE *pAddr)
{
    return m_cSyncBufRecver.Free(pAddr);
}


/* 发送方释放内存, 针对异常流程 */
WORD32 CShmChannel::LocalFree(BYTE *pAddr)
{
    return m_cSyncBufSender.Free(pAddr);
}


/* 接收方引用计数++操作 */
VOID CShmChannel::IncRefCount(BYTE *pAddr)
{
    return m_cSyncBufRecver.IncRefCount(pAddr);
}


/* 单条发送 */
WORD32 CShmChannel::SendMessage(VOID *ptMsg)
{
    return m_cSyncBufSender.Enqueue((VOID *)ptMsg);
}


/* 批量发送 */
WORD32 CShmChannel::SendMessage(WORD32 dwNum, VOID **ptMsg)
{
    WORD32 dwSuccNum  = 0;
    WORD32 dwTotalNum = 0;
    WORD32 dwFreeSize = 0;

    while (dwNum > 0)
    {
        dwSuccNum = m_cSyncBufSender.EnqueueBurst(dwNum, &(ptMsg[dwTotalNum]), dwFreeSize);

        dwNum      -= dwSuccNum;
        dwTotalNum += dwSuccNum;

        if (0 == dwFreeSize)
        {
            break ;
        }
    }

    return dwTotalNum;
}


/* 批量接收, 逐条消息回调 */
WORD32 CShmChannel::RecvMessage(VOID *pObj, PSyncRecvMsg pFunc)
{
    VOID          *apBuf[SHM_BURST_NUM] = {NULL, };
    WORD32         dwNum       = 0;
    WORD32         dwTotalNum  = 0;
    WORD32         dwRemainNum = 0;
    T_ShmNodeHead *ptHead      = NULL;

    do
    {
        dwNum = m_cSyncBufRecver.DequeueBurst(SHM_BURST_NUM,
                                              apBuf,
                                              dwRemainNum);
        for (WORD32 dwIndex = 0; dwIndex < dwNum; dwIndex++)
        {
            ptHead = (T_ShmNodeHead *)(((WORD64)(apBuf[dwIndex])) - sizeof(T_ShmNodeHead));
            (*pFunc) (pObj, apBuf[dwIndex], ptHead->dwLen);
            //m_cSyncBufRecver.Free((BYTE *)(apBuf[dwIndex]));
        }

        dwTotalNum += dwNum;
    } while(dwRemainNum > 0);

    return dwTotalNum;
}


WORD32 CShmChannel::Post()
{
    return m_cSyncBufSender.Post();
}


WORD32 CShmChannel::Wait()
{
    return m_cSyncBufRecver.Wait();
}


VOID CShmChannel::Snapshot(T_ShmSnapshot &rtSend, T_ShmSnapshot &rtRecv)
{
    m_cSyncBufSender.Snapshot(rtSend);
    m_cSyncBufRecver.Snapshot(rtRecv);
}


VOID CShmChannel::CleanUp()
{
    if ( (NULL == m_ptChannel)
      || (NULL == m_pMemPool)
      || (E_SHM_MASTER != m_eRole))
    {
        return ;
    }

    VOID *ptHeadS  = (VOID *)(m_ptChannel->lwHdlSAddr);
    VOID *ptHeadR  = (VOID *)(m_ptChannel->lwHdlRAddr);
    VOID *pOriMemS = (VOID *)(m_ptChannel->lwOriSAddr);
    VOID *pOriMemR = (VOID *)(m_ptChannel->lwOriRAddr);

    m_pMemPool->Free(pOriMemR);
    m_pMemPool->Free(pOriMemS);
    m_pMemPool->Free(ptHeadR);
    m_pMemPool->Free(ptHeadS);
}


WORD32 CShmChannel::Create(WORD32 dwHdlSNodeNum,
                           WORD32 dwHdlSNodeSize,
                           WORD32 dwHdlRNodeNum,
                           WORD32 dwHdlRNodeSize)
{
    WORD32        dwResult     = INVALID_DWORD;
    T_ShmHdlHead *ptHeadS      = NULL;
    T_ShmHdlHead *ptHeadR      = NULL;
    BYTE         *pOriMemAddrS = NULL;
    BYTE         *pOriMemAddrR = NULL;
    WORD32        dwMemSizeS   = dwHdlSNodeNum * dwHdlSNodeSize;
    WORD32        dwMemSizeR   = dwHdlRNodeNum * dwHdlRNodeSize;

    ptHeadS = (T_ShmHdlHead *)(m_pMemPool->Malloc(ROUND_UP(sizeof(T_ShmHdlHead), s_lwAlignSize)));
    if (NULL == ptHeadS)
    {
        return FAIL;
    }

    ptHeadR = (T_ShmHdlHead *)(m_pMemPool->Malloc(ROUND_UP(sizeof(T_ShmHdlHead), s_lwAlignSize)));
    if (NULL == ptHeadR)
    {
        return FAIL;
    }

    pOriMemAddrS = m_pMemPool->Malloc(ROUND_UP(dwMemSizeS, s_lwAlignSize));
    if (NULL == pOriMemAddrS)
    {
        return FAIL;
    }

    pOriMemAddrR = m_pMemPool->Malloc(ROUND_UP(dwMemSizeR, s_lwAlignSize));
    if (NULL == pOriMemAddrR)
    {
        return FAIL;
    }

    dwResult = m_cSyncBufSender.Initialize(m_eRole,
                                           ptHeadS,
                                           pOriMemAddrS,
                                           (WORD64)dwMemSizeS,
                                           dwHdlSNodeNum,
                                           dwHdlSNodeSize);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    dwResult = m_cSyncBufRecver.Initialize(m_eRole,
                                           ptHeadR,
                                           pOriMemAddrR,
                                           (WORD64)dwMemSizeR,
                                           dwHdlRNodeNum,
                                           dwHdlRNodeSize);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    m_ptChannel->dwHdlSNodeNum  = dwHdlSNodeNum;
    m_ptChannel->dwHdlSNodeSize = dwHdlSNodeSize;
    m_ptChannel->dwHdlRNodeNum  = dwHdlRNodeNum;
    m_ptChannel->dwHdlRNodeSize = dwHdlRNodeSize;
    m_ptChannel->lwHdlSAddr     = (WORD64)ptHeadS;
    m_ptChannel->lwHdlRAddr     = (WORD64)ptHeadR;
    m_ptChannel->lwOriSAddr     = (WORD64)pOriMemAddrS;
    m_ptChannel->lwOriRAddr     = (WORD64)pOriMemAddrR;

    return dwResult;
}


WORD32 CShmChannel::Attach(WORD32 dwHdlSNodeNum,
                           WORD32 dwHdlSNodeSize,
                           WORD32 dwHdlRNodeNum,
                           WORD32 dwHdlRNodeSize)
{
    WORD32        dwResult     = INVALID_DWORD;
    T_ShmHdlHead *ptHeadS      = (T_ShmHdlHead *)(m_ptChannel->lwHdlSAddr);
    T_ShmHdlHead *ptHeadR      = (T_ShmHdlHead *)(m_ptChannel->lwHdlRAddr);
    BYTE         *pOriMemAddrS = (BYTE *)(m_ptChannel->lwOriSAddr);
    BYTE         *pOriMemAddrR = (BYTE *)(m_ptChannel->lwOriRAddr);
    WORD32        dwMemSizeS   = dwHdlSNodeNum * dwHdlSNodeSize;
    WORD32        dwMemSizeR   = dwHdlRNodeNum * dwHdlRNodeSize;

    assert((dwHdlSNodeNum  == m_ptChannel->dwHdlSNodeNum)
        && (dwHdlSNodeSize == m_ptChannel->dwHdlSNodeSize)
        && (dwHdlRNodeNum  == m_ptChannel->dwHdlRNodeNum)
        && (dwHdlRNodeSize == m_ptChannel->dwHdlRNodeSize));

    dwResult = m_cSyncBufRecver.Initialize(m_eRole,
                                           ptHeadS,
                                           pOriMemAddrS,
                                           (WORD64)dwMemSizeS,
                                           dwHdlSNodeNum,
                                           dwHdlSNodeSize);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    dwResult = m_cSyncBufSender.Initialize(m_eRole,
                                           ptHeadR,
                                           pOriMemAddrR,
                                           (WORD64)dwMemSizeR,
                                           dwHdlRNodeNum,
                                           dwHdlRNodeSize);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    return SUCCESS;
}


