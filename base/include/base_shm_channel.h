

#ifndef _BASE_SHM_CHANNEL_H_
#define _BASE_SHM_CHANNEL_H_


#include "base_shm_hdl.h"


using PSyncRecvMsg = WORD32 (*)(VOID *pArg, const VOID *ptMsg, WORD32 dwLen);


typedef struct tagT_ShmChannel
{
    volatile WORD32  dwHdlSNodeNum;                   /* 发送Handler内存块数量 */
    volatile WORD32  dwHdlSNodeSize;                  /* 发送Handler内存块size */
    volatile WORD32  dwHdlRNodeNum;                   /* 接收Handler内存块数量 */
    volatile WORD32  dwHdlRNodeSize;                  /* 接收Handler内存块size */
    volatile WORD64  lwHdlSAddr;                      /* 发送Handler地址 */
    volatile WORD64  lwHdlRAddr;                      /* 接收Handler地址 */
    volatile WORD64  lwOriSAddr;                      /* 发送Handler内存池地址 */
    volatile WORD64  lwOriRAddr;                      /* 接收Handler内存池地址 */
    volatile BYTE    aucResved[208];                  /* 保留字段, 保证256字节对齐 */
    volatile BYTE    aucMaster[QUATRA_CACHE_SIZE];    /* 用于创建MasterChannel实例 */
    volatile BYTE    aucSlave[QUATRA_CACHE_SIZE];     /* 用于创建SlaveChannel实例 */
    volatile BYTE    aucObserver[QUATRA_CACHE_SIZE];  /* 用于创建ObserverChannel实例 */
}T_ShmChannel;
static_assert(sizeof(T_ShmChannel) == QUARTER_PAGE_SIZE, "unexpected T_ShmMetaHead layout");


class CShmChannel : public CBaseData
{
public :
    const static WORD64 s_lwAlignSize  = 0x200000UL;            /* 2M */

public :
    CShmChannel();
    virtual ~CShmChannel();

    WORD32 Initialize(E_ShmRole        eRole,
                      T_ShmChannel    *ptChannel,
                      CCentralMemPool *pMemPool,
                      WORD32           dwHdlSNodeNum,
                      WORD32           dwHdlSNodeSize,
                      WORD32           dwHdlRNodeNum,
                      WORD32           dwHdlRNodeSize);

    /* 发送方申请内存 */
    BYTE * Malloc(WORD32 dwSize, WORD32 dwPoint = 0);

    /* 接收方释放内存 */
    WORD32 Free(BYTE *pAddr);

    /* 发送方释放内存, 针对异常流程 */
    WORD32 LocalFree(BYTE *pAddr);

    /* 接收方引用计数++操作 */
    VOID IncRefCount(BYTE *pAddr);

    WORD32 SendMessage(VOID *ptMsg);                     /* 单条发送 */
    WORD32 SendMessage(WORD32 dwNum, VOID **ptMsg);      /* 批量发送 */
    WORD32 RecvMessage(VOID *pObj, PSyncRecvMsg pFunc);  /* 批量接收, 逐消息回调 */

    WORD32 Post();
    WORD32 Wait();

    VOID Snapshot(T_ShmSnapshot &rtSend, T_ShmSnapshot &rtRecv);

protected :
    VOID CleanUp();

    WORD32 Create(WORD32 dwHdlSNodeNum,
                  WORD32 dwHdlSNodeSize,
                  WORD32 dwHdlRNodeNum,
                  WORD32 dwHdlRNodeSize);

    WORD32 Attach(WORD32 dwHdlSNodeNum,
                  WORD32 dwHdlSNodeSize,
                  WORD32 dwHdlRNodeNum,
                  WORD32 dwHdlRNodeSize);

protected :
    T_ShmChannel     *m_ptChannel;
    CCentralMemPool  *m_pMemPool;

    E_ShmRole         m_eRole;
    CShmHandler       m_cSyncBufSender;
    CShmHandler       m_cSyncBufRecver;
};
static_assert(sizeof(CShmChannel) < QUATRA_CACHE_SIZE, "unexpected CShmChannel layout");


#endif


