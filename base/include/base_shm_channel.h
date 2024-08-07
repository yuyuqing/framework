

#ifndef _BASE_SHM_CHANNEL_H_
#define _BASE_SHM_CHANNEL_H_


#include "base_shm_hdl.h"


using PSyncRecvMsg = WORD32 (*)(VOID *pArg, const VOID *ptMsg, WORD32 dwLen);


class CChannelTpl : public CBaseData
{
public :
    CChannelTpl();
    virtual ~CChannelTpl();

    virtual WORD32 Initialize(WORD32  dwKeyS,
                              WORD32  dwKeyR,
                              VOID   *pVirtAddrS,
                              VOID   *pVirtAddrR);

    /* 发送方申请内存 */
    virtual BYTE * Malloc(WORD32 dwSize, WORD32 dwPoint = 0);

    /* 接收方释放内存 */
    virtual WORD32 Free(BYTE *pAddr);

    /* 发送方释放内存, 针对异常流程 */
    virtual WORD32 LocalFree(BYTE *pAddr);

    /* 接收方引用计数++操作 */
    virtual VOID IncRefCount(BYTE *pAddr);

    virtual WORD32 SendMessage(VOID *ptMsg);                     /* 单条发送 */
    virtual WORD32 SendMessage(WORD32 dwNum, VOID **ptMsg);      /* 批量发送 */
    virtual WORD32 RecvMessage(VOID *pObj, PSyncRecvMsg pFunc);  /* 批量接收, 逐消息回调 */

    virtual WORD32 Post();
    virtual WORD32 Wait();

    virtual VOID Snapshot(T_ShmSnapshot &rtRecv, T_ShmSnapshot &rtSend) = 0;
};


template <BOOL MASTER, WORD32 POWER_NUM_S, WORD32 POWER_NUM_R, WORD32 NODE_SIZE>
class CChannelEnable : public CChannelTpl
{
};


template <WORD32 POWER_NUM_S, WORD32 POWER_NUM_R, WORD32 NODE_SIZE>
class CChannelEnable<TRUE, POWER_NUM_S, POWER_NUM_R, NODE_SIZE> : public CChannelTpl
{
public :
    using CChannelSendBuf = CShmHandler<POWER_NUM_S, NODE_SIZE>;
    using CChannelRecvBuf = CShmHandler<POWER_NUM_R, NODE_SIZE>;
};


template <WORD32 POWER_NUM_S, WORD32 POWER_NUM_R, WORD32 NODE_SIZE>
class CChannelEnable<FALSE, POWER_NUM_S, POWER_NUM_R, NODE_SIZE> : public CChannelTpl
{
public :
    using CChannelSendBuf = CShmHandler<POWER_NUM_R, NODE_SIZE>;
    using CChannelRecvBuf = CShmHandler<POWER_NUM_S, NODE_SIZE>;
};


/* POWER_NUM_S : 发送方内存节点数量(2^P_S)
 * POWER_NUM_R : 接收方内存节点数量(2^P_R)
 */
template <BOOL MASTER, WORD32 POWER_NUM_S, WORD32 POWER_NUM_R, WORD32 NODE_SIZE>
class CShmChannel : public CChannelEnable<MASTER, POWER_NUM_S, POWER_NUM_R, NODE_SIZE>
{
public :
    typedef typename CChannelEnable<MASTER, POWER_NUM_S, POWER_NUM_R, NODE_SIZE>::CChannelSendBuf    CShmSendBuf;
    typedef typename CChannelEnable<MASTER, POWER_NUM_S, POWER_NUM_R, NODE_SIZE>::CChannelRecvBuf    CShmRecvBuf;

    typedef typename CShmRecvBuf::T_ShmNodeHead    T_RecvBufHead;

public :
    CShmChannel();
    virtual ~CShmChannel();

    virtual WORD32 Initialize(WORD32  dwKeyS,
                              WORD32  dwKeyR,
                              VOID   *pVirtAddrS,
                              VOID   *pVirtAddrR);

    /* 发送方申请内存 */
    virtual BYTE * Malloc(WORD32 dwSize, WORD32 dwPoint = 0);

    /* 接收方释放内存 */
    virtual WORD32 Free(BYTE *pAddr);

    /* 发送方释放内存, 针对异常流程 */
    virtual WORD32 LocalFree(BYTE *pAddr);

    /* 接收方引用计数++操作 */
    virtual VOID IncRefCount(BYTE *pAddr);

    virtual WORD32 SendMessage(VOID *ptMsg);                     /* 单条发送 */
    virtual WORD32 SendMessage(WORD32 dwNum, VOID **ptMsg);      /* 批量发送 */
    virtual WORD32 RecvMessage(VOID *pObj, PSyncRecvMsg pFunc);  /* 批量接收, 逐消息回调 */

    virtual WORD32 Post();
    virtual WORD32 Wait();

    virtual VOID Snapshot(T_ShmSnapshot &rtRecv, T_ShmSnapshot &rtSend);

protected :
    CShmRecvBuf    m_cSyncBufRecver;
    CShmSendBuf    m_cSyncBufSender;
};


template <BOOL MASTER, WORD32 POWER_NUM_S, WORD32 POWER_NUM_R, WORD32 NODE_SIZE>
CShmChannel<MASTER, POWER_NUM_S, POWER_NUM_R, NODE_SIZE>::CShmChannel()
{
}


template <BOOL MASTER, WORD32 POWER_NUM_S, WORD32 POWER_NUM_R, WORD32 NODE_SIZE>
CShmChannel<MASTER, POWER_NUM_S, POWER_NUM_R, NODE_SIZE>::~CShmChannel()
{
}


template <BOOL MASTER, WORD32 POWER_NUM_S, WORD32 POWER_NUM_R, WORD32 NODE_SIZE>
WORD32 CShmChannel<MASTER, POWER_NUM_S, POWER_NUM_R, NODE_SIZE>::Initialize(
    WORD32  dwKeyS,
    WORD32  dwKeyR,
    VOID   *pVirtAddrS,
    VOID   *pVirtAddrR)
{
    WORD32 dwResult = 0;

    if (MASTER)
    {
        m_cSyncBufRecver.Clean(dwKeyR);
        m_cSyncBufSender.Clean(dwKeyS);

        dwResult = m_cSyncBufRecver.Initialize(dwKeyR, TRUE, pVirtAddrR);
        if (SUCCESS != dwResult)
        {
            return FAIL;
        }

        dwResult = m_cSyncBufSender.Initialize(dwKeyS, TRUE, pVirtAddrS);
        if (SUCCESS != dwResult)
        {
            return FAIL;
        }
    }
    else
    {
        dwResult = m_cSyncBufSender.Initialize(dwKeyR, FALSE, pVirtAddrR);
        if (SUCCESS != dwResult)
        {
            return FAIL;
        }

        dwResult = m_cSyncBufRecver.Initialize(dwKeyS, FALSE, pVirtAddrS);
        if (SUCCESS != dwResult)
        {
            return FAIL;
        }
    }

    return SUCCESS;
}


/* 发送方申请内存 */
template <BOOL MASTER, WORD32 POWER_NUM_S, WORD32 POWER_NUM_R, WORD32 NODE_SIZE>
inline BYTE * CShmChannel<MASTER, POWER_NUM_S, POWER_NUM_R, NODE_SIZE>::Malloc(
    WORD32 dwSize,
    WORD32 dwPoint)
{
    return m_cSyncBufSender.Malloc(dwSize, dwPoint);
}


/* 接收方释放内存 */
template <BOOL MASTER, WORD32 POWER_NUM_S, WORD32 POWER_NUM_R, WORD32 NODE_SIZE>
inline WORD32 CShmChannel<MASTER, POWER_NUM_S, POWER_NUM_R, NODE_SIZE>::Free(
    BYTE *pAddr)
{
    return m_cSyncBufRecver.Free(pAddr);
}


/* 发送方释放内存, 针对异常流程 */
template <BOOL MASTER, WORD32 POWER_NUM_S, WORD32 POWER_NUM_R, WORD32 NODE_SIZE>
inline WORD32 CShmChannel<MASTER, POWER_NUM_S, POWER_NUM_R, NODE_SIZE>::LocalFree(BYTE *pAddr)
{
    return m_cSyncBufSender.Free(pAddr);
}


/* 接收方引用计数++操作 */
template <BOOL MASTER, WORD32 POWER_NUM_S, WORD32 POWER_NUM_R, WORD32 NODE_SIZE>
inline VOID CShmChannel<MASTER, POWER_NUM_S, POWER_NUM_R, NODE_SIZE>::IncRefCount(BYTE *pAddr)
{
    m_cSyncBufRecver.IncRefCount(pAddr);
}


/* 单条发送, 在发送方分配好的内存上填写消息, 调用发送方Enqueue接口发送消息 */
template <BOOL MASTER, WORD32 POWER_NUM_S, WORD32 POWER_NUM_R, WORD32 NODE_SIZE>
inline WORD32 CShmChannel<MASTER, POWER_NUM_S, POWER_NUM_R, NODE_SIZE>::SendMessage(
    VOID *ptMsg)
{
    return m_cSyncBufSender.Enqueue((VOID *)ptMsg);
}


/* 批量发送, 返回发送成功条数 */
template <BOOL MASTER, WORD32 POWER_NUM_S, WORD32 POWER_NUM_R, WORD32 NODE_SIZE>
inline WORD32 CShmChannel<MASTER, POWER_NUM_S, POWER_NUM_R, NODE_SIZE>::SendMessage(
    WORD32   dwNum,
    VOID   **ptMsg)
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


/* 批量接收, 逐消息回调; 有业务控制内存的释放回收 */
template <BOOL MASTER, WORD32 POWER_NUM_S, WORD32 POWER_NUM_R, WORD32 NODE_SIZE>
inline WORD32 CShmChannel<MASTER, POWER_NUM_S, POWER_NUM_R, NODE_SIZE>::RecvMessage(
    VOID         *pObj,
    PSyncRecvMsg  pFunc)
{
    VOID          *apBuf[CShmRecvBuf::SHM_BURST_NUM] = {NULL, };
    WORD32         dwNum       = 0;
    WORD32         dwTotalNum  = 0;
    WORD32         dwRemainNum = 0;
    T_RecvBufHead *ptHead      = NULL;

    do
    {
        dwNum = m_cSyncBufRecver.DequeueBurst(CShmRecvBuf::SHM_BURST_NUM,
                                              apBuf,
                                              dwRemainNum);
        for (WORD32 dwIndex = 0; dwIndex < dwNum; dwIndex++)
        {
            ptHead = (T_RecvBufHead *)(((WORD64)(apBuf[dwIndex])) - (CShmRecvBuf::s_dwNodeOffset));
            (*pFunc) (pObj, apBuf[dwIndex], ptHead->dwLen);
            //m_cSyncBufRecver.Free((BYTE *)(apBuf[dwIndex]));
        }

        dwTotalNum += dwNum;
    } while(dwRemainNum > 0);

    return dwTotalNum;
}


template <BOOL MASTER, WORD32 POWER_NUM_S, WORD32 POWER_NUM_R, WORD32 NODE_SIZE>
inline WORD32 CShmChannel<MASTER, POWER_NUM_S, POWER_NUM_R, NODE_SIZE>::Post()
{    
    return m_cSyncBufSender.Post();
}


template <BOOL MASTER, WORD32 POWER_NUM_S, WORD32 POWER_NUM_R, WORD32 NODE_SIZE>
inline WORD32 CShmChannel<MASTER, POWER_NUM_S, POWER_NUM_R, NODE_SIZE>::Wait()
{
    return m_cSyncBufRecver.Wait();
}


template <BOOL MASTER, WORD32 POWER_NUM_S, WORD32 POWER_NUM_R, WORD32 NODE_SIZE>
inline VOID CShmChannel<MASTER, POWER_NUM_S, POWER_NUM_R, NODE_SIZE>::Snapshot(
    T_ShmSnapshot &rtRecv,
    T_ShmSnapshot &rtSend)
{
    m_cSyncBufRecver.Snapshot(rtRecv);
    m_cSyncBufSender.Snapshot(rtSend);
}


#endif


