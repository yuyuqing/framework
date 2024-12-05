

#ifndef _BASE_SHM_MGR_H_
#define _BASE_SHM_MGR_H_


#include "base_config_file.h"
#include "base_shm_channel.h"


typedef struct tagT_ChannelSnapshot
{
    T_ShmSnapshot  tRecv;
    T_ShmSnapshot  tSend;
}T_ChannelSnapshot;


typedef struct tagT_ShmMetaHead
{
    volatile WORD64  lwMagic;            /* ħ������ */
    volatile WORD32  dwVersion;          /* �汾�� */
    volatile WORD32  dwHeadSize;         /* ͷ��Ϣsize */
    volatile WORD32  dwShmKey;           /* ���������ڴ�Keyֵ */
    volatile SWORD32 iShmID;             /* Masterά���Ĺ����ڴ�ID */

    volatile WORD64  lwMasterLock;       /* ��������(ֻ����1��Master���̴򿪱������ڴ�) */
    volatile SWORD32 iGlobalLock;        /* ȫ����, ���ڳ�ʼ����ȫ�ַ��� */
    volatile BOOL    bInitFlag;          /* ��ʼ����ɱ�־ */

    volatile WORD64  lwMetaAddr;         /* �����̹����ڴ�Ԫ���������ַ */
    volatile WORD64  lwMetaSize;         /* �����ڴ�Ԫ�����ܴ�С */

    volatile SWORD32 iMLock;             /* �ڴ������, ���� */
    volatile WORD32  dwChannelNum;       /* ͨ������(�û���) */

    volatile BYTE    aucResved0[960];

    /* F1-U�û���ͨ�� */
    T_ShmChannel     atChannel[MAX_CHANNEL_NUM];

    T_ShmChannel     tCtrlChannel;       /* F1-C������ͨ�� */
    T_ShmChannel     tOamChannel;        /* F1-O������ͨ�� */

    volatile BYTE    aucResved1[QUARTER_PAGE_SIZE];

    volatile BYTE    aucMaster[OCTA_PAGE_SIZE];
    volatile BYTE    aucSlave[OCTA_PAGE_SIZE];
    volatile BYTE    aucObserver[OCTA_PAGE_SIZE];

    volatile BYTE    aucResved2[19 * QUARTER_PAGE_SIZE];
    volatile BYTE    aucResved3[768];

    /* ������ʵ����CCentralMemPool��������(128 + 64) */
    volatile BYTE    aucMemPool[TRIPLE_CACHE_SIZE];
    volatile BYTE    aucOriAddr[CACHE_SIZE];
}T_ShmMetaHead;
static_assert(sizeof(T_ShmMetaHead) == (32 * PAGE_SIZE), "unexpected T_ShmMetaHead layout");


class CShmMgr : public CBaseData
{
public :
    enum { SHM_SLAVE_INIT_WAIT = 128 };

    const static WORD64  s_lwMagicValue  = 0x0DE0B6B3A76409B1UL;  /* ������ */
    const static WORD32  s_dwVersion     = 1;                     /* �汾�� */
    const static WORD32  s_dwShmKey      = 0x05F5E505;
    const static WORD64  s_lwVirBassAddr = 0x4400000000UL;
    const static WORD64  s_lwGranularity = 0x40000000UL;

    static CShmMgr * CreateShmMgr(WORD32 dwProcID, T_ShmJsonCfg &rtParam);

    static CShmMgr * GetInstance();
    static VOID Destroy();

    static VOID LockGlobal(T_ShmMetaHead &rtHead);
    static VOID UnLockGlobal(T_ShmMetaHead &rtHead);

private :
    static T_ShmMetaHead * Attach(E_ShmRole eRole, T_ShmJsonCfg &rtParam);
    static WORD32 Detach(VOID *ptAddr, SWORD32 iShmID);

    static WORD32 InitMetaHead(E_ShmRole      eRole,
                               SWORD32        iShmID,
                               T_ShmMetaHead *ptHead,
                               T_ShmJsonCfg  &rtParam);

    static CShmMgr * InitMaster(T_ShmMetaHead *ptHead, T_ShmJsonCfg &rtParam);
    static CShmMgr * InitSlave(T_ShmMetaHead *ptHead, T_ShmJsonCfg &rtParam);
    static CShmMgr * InitObserver(T_ShmMetaHead *ptHead, T_ShmJsonCfg &rtParam);

public : 
    CShmMgr();
    virtual ~CShmMgr();

    WORD32 Initiaize(T_ShmMetaHead *ptHead, T_ShmJsonCfg &rtParam);

    CShmChannel * GetOamChannel();
    CShmChannel * GetCtrlChannel();
    CShmChannel * GetDataChannel(WORD32 dwChannelID);

    VOID Snapshot();

    VOID Dump();
    VOID Printf();

protected :
    CShmChannel * InitOamChannel(T_ShmJsonCfg &rtCfg);
    CShmChannel * InitCtrlChannel(T_ShmJsonCfg &rtCfg);
    CShmChannel * InitDataChannel(WORD32 dwChannelID, T_ShmJsonCfg &rtCfg);

protected :
    T_ShmMetaHead    *m_pMetaHead;     /* �����ڴ�Ԫ������ */

    E_ShmRole         m_eRole;
    WORD32            m_dwChannelNum;  /* F1-U������ͨ������ */

    CCentralMemPool  *m_pMemPool;      /* Master���̸����ʼ����ʹ�� */

    CShmChannel      *m_pOamChannel;
    CShmChannel      *m_pCtrlChannel;
    CShmChannel      *m_apDataChannel[MAX_CHANNEL_NUM];

    T_ChannelSnapshot  m_tOamChannel;
    T_ChannelSnapshot  m_tCtrlChannel;
    T_ChannelSnapshot  m_atDataChannel[MAX_CHANNEL_NUM];

private :
    static CShmMgr   *s_pInstance;
};
static_assert(sizeof(CShmMgr) < OCTA_PAGE_SIZE, "unexpected CShmMgr layout");


inline CShmChannel * CShmMgr::GetOamChannel()
{
    return m_pOamChannel;
}


inline CShmChannel * CShmMgr::GetCtrlChannel()
{
    return m_pCtrlChannel;
}


inline CShmChannel * CShmMgr::GetDataChannel(WORD32 dwChannelID)
{
    if (unlikely(dwChannelID >= MAX_CHANNEL_NUM))
    {
        return NULL;
    }

    return m_apDataChannel[dwChannelID];
}


inline VOID CShmMgr::Snapshot()
{
    //m_pOamChannel->Snapshot(m_tOamChannel.tRecv, m_tOamChannel.tSend);
    m_pCtrlChannel->Snapshot(m_tCtrlChannel.tRecv, m_tCtrlChannel.tSend);

    for (WORD32 dwIndex = 0; dwIndex < m_dwChannelNum; dwIndex++)
    {
        T_ChannelSnapshot &rDataChannel = m_atDataChannel[dwIndex];
        m_apDataChannel[dwIndex]->Snapshot(rDataChannel.tRecv,
                                           rDataChannel.tSend);
    }
}


#endif


