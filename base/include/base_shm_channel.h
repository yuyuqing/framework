

#ifndef _BASE_SHM_CHANNEL_H_
#define _BASE_SHM_CHANNEL_H_


#include "base_shm_hdl.h"


using PSyncRecvMsg = WORD32 (*)(VOID *pArg, const VOID *ptMsg, WORD32 dwLen);


typedef struct tagT_ShmChannel
{
    volatile WORD32  dwHdlSNodeNum;                   /* ����Handler�ڴ������ */
    volatile WORD32  dwHdlSNodeSize;                  /* ����Handler�ڴ��size */
    volatile WORD32  dwHdlRNodeNum;                   /* ����Handler�ڴ������ */
    volatile WORD32  dwHdlRNodeSize;                  /* ����Handler�ڴ��size */
    volatile WORD64  lwHdlSAddr;                      /* ����Handler��ַ */
    volatile WORD64  lwHdlRAddr;                      /* ����Handler��ַ */
    volatile WORD64  lwOriSAddr;                      /* ����Handler�ڴ�ص�ַ */
    volatile WORD64  lwOriRAddr;                      /* ����Handler�ڴ�ص�ַ */
    volatile BYTE    aucResved[208];                  /* �����ֶ�, ��֤256�ֽڶ��� */
    volatile BYTE    aucMaster[QUATRA_CACHE_SIZE];    /* ���ڴ���MasterChannelʵ�� */
    volatile BYTE    aucSlave[QUATRA_CACHE_SIZE];     /* ���ڴ���SlaveChannelʵ�� */
    volatile BYTE    aucObserver[QUATRA_CACHE_SIZE];  /* ���ڴ���ObserverChannelʵ�� */
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

    /* ���ͷ������ڴ� */
    BYTE * Malloc(WORD32 dwSize, WORD32 dwPoint = 0);

    /* ���շ��ͷ��ڴ� */
    WORD32 Free(BYTE *pAddr);

    /* ���ͷ��ͷ��ڴ�, ����쳣���� */
    WORD32 LocalFree(BYTE *pAddr);

    /* ���շ����ü���++���� */
    VOID IncRefCount(BYTE *pAddr);

    WORD32 SendMessage(VOID *ptMsg);                     /* �������� */
    WORD32 SendMessage(WORD32 dwNum, VOID **ptMsg);      /* �������� */
    WORD32 RecvMessage(VOID *pObj, PSyncRecvMsg pFunc);  /* ��������, ����Ϣ�ص� */

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


