

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
    volatile WORD64  lwMagic;            /* 魔法数字 */
    volatile WORD32  dwVersion;          /* 版本号 */
    volatile WORD32  dwHeadSize;         /* 头信息size */
    volatile WORD32  dwShmKey;           /* 创建共享内存Key值 */
    volatile SWORD32 iShmID;             /* Master维护的共享内存ID */

    volatile WORD64  lwMasterLock;       /* 主进程锁(只能有1个Master进程打开本共享内存) */
    volatile SWORD32 iGlobalLock;        /* 全局锁, 用于初始化及全局访问 */
    volatile BOOL    bInitFlag;          /* 初始化完成标志 */

    volatile WORD64  lwMetaAddr;         /* 主进程共享内存元数据虚拟地址 */
    volatile WORD64  lwMetaSize;         /* 共享内存元数据总大小 */

    volatile SWORD32 iMLock;             /* 内存管理锁, 保留 */
    volatile WORD32  dwChannelNum;       /* 通道数量(用户面) */

    volatile BYTE    aucResved0[960];

    /* F1-U用户面通道 */
    T_ShmChannel     atChannel[MAX_CHANNEL_NUM];

    T_ShmChannel     tCtrlChannel;       /* F1-C控制面通道 */
    T_ShmChannel     tOamChannel;        /* F1-O管理面通道 */

    volatile BYTE    aucResved1[QUARTER_PAGE_SIZE];

    volatile BYTE    aucMaster[OCTA_PAGE_SIZE];
    volatile BYTE    aucSlave[OCTA_PAGE_SIZE];
    volatile BYTE    aucObserver[OCTA_PAGE_SIZE];

    volatile BYTE    aucResved2[19 * QUARTER_PAGE_SIZE];
    volatile BYTE    aucResved3[768];

    /* 主进程实例化CCentralMemPool的数据区(128 + 64) */
    volatile BYTE    aucMemPool[TRIPLE_CACHE_SIZE];
    volatile BYTE    aucOriAddr[CACHE_SIZE];
}T_ShmMetaHead;
static_assert(sizeof(T_ShmMetaHead) == (32 * PAGE_SIZE), "unexpected T_ShmMetaHead layout");


class CShmMgr : public CBaseData
{
public :
    enum { SHM_SLAVE_INIT_WAIT = 128 };

    const static WORD64  s_lwMagicValue  = 0x0DE0B6B3A76409B1UL;  /* 大质数 */
    const static WORD32  s_dwVersion     = 1;                     /* 版本号 */
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
    T_ShmMetaHead    *m_pMetaHead;     /* 共享内存元数据区 */

    E_ShmRole         m_eRole;
    WORD32            m_dwChannelNum;  /* F1-U数据面通道数量 */

    CCentralMemPool  *m_pMemPool;      /* Master进程负责初始化并使用 */

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


