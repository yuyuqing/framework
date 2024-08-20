

#ifndef _BASE_SHM_MGR_H_
#define _BASE_SHM_MGR_H_


#include "base_shm_channel.h"
#include "base_mem_interface.h"


typedef enum tagE_ShmNodeSize
{
    E_ShmNodeSize_002K = 2048,
    E_ShmNodeSize_004K = 4096,
    E_ShmNodeSize_128K = 131072,
}E_ShmNodeSize;


typedef enum tagE_ShmPowerNum
{
    E_ShmPowerNum_10 = 10,
    E_ShmPowerNum_14 = 14,
    E_ShmPowerNum_15 = 15,
    E_ShmPowerNum_16 = 16,
    E_ShmPowerNum_17 = 17,
    E_ShmPowerNum_18 = 18,
    E_ShmPowerNum_19 = 19,
    E_ShmPowerNum_20 = 20,
}E_ShmPowerNum;


typedef CShmChannel<TRUE,  E_ShmPowerNum_20, E_ShmPowerNum_18, E_ShmNodeSize_002K>    CShmChannelM20;
typedef CShmChannel<TRUE,  E_ShmPowerNum_19, E_ShmPowerNum_17, E_ShmNodeSize_002K>    CShmChannelM19;
typedef CShmChannel<TRUE,  E_ShmPowerNum_18, E_ShmPowerNum_16, E_ShmNodeSize_002K>    CShmChannelM18;
typedef CShmChannel<TRUE,  E_ShmPowerNum_17, E_ShmPowerNum_15, E_ShmNodeSize_002K>    CShmChannelM17;
typedef CShmChannel<TRUE,  E_ShmPowerNum_16, E_ShmPowerNum_14, E_ShmNodeSize_002K>    CShmChannelM16;

typedef CShmChannel<FALSE, E_ShmPowerNum_20, E_ShmPowerNum_18, E_ShmNodeSize_002K>    CShmChannelS20;
typedef CShmChannel<FALSE, E_ShmPowerNum_19, E_ShmPowerNum_17, E_ShmNodeSize_002K>    CShmChannelS19;
typedef CShmChannel<FALSE, E_ShmPowerNum_18, E_ShmPowerNum_16, E_ShmNodeSize_002K>    CShmChannelS18;
typedef CShmChannel<FALSE, E_ShmPowerNum_17, E_ShmPowerNum_15, E_ShmNodeSize_002K>    CShmChannelS17;
typedef CShmChannel<FALSE, E_ShmPowerNum_16, E_ShmPowerNum_14, E_ShmNodeSize_002K>    CShmChannelS16;

/* 用于控制面SCTP消息的共享内存 */
typedef CShmChannel<TRUE,  E_ShmPowerNum_10, E_ShmPowerNum_10, E_ShmNodeSize_128K>    CShmCHannelMCtrl;
typedef CShmChannel<FALSE, E_ShmPowerNum_10, E_ShmPowerNum_10, E_ShmNodeSize_128K>    CShmCHannelSCtrl;


#define SHM_CHANNEL_SIZE    MAX(MAX(MAX(sizeof(CShmChannelM20), sizeof(CShmChannelS20)),       \
                                    MAX(sizeof(CShmChannelM19), sizeof(CShmChannelS19))),      \
                                MAX(MAX(sizeof(CShmChannelM18), sizeof(CShmChannelS18)),       \
                                    MAX(sizeof(CShmCHannelMCtrl), sizeof(CShmCHannelSCtrl))))


typedef struct tagT_ChannelSnapshot
{
    T_ShmSnapshot  tRecv;
    T_ShmSnapshot  tSend;
}T_ChannelSnapshot;


class CShmMgr : public CSingleton<CShmMgr>, public CBaseData
{
public :
    enum { MAX_CHANNEL_NUM = 16 };

    const static WORD64  s_lwVirBassAddr = 0x4400000000UL;
    const static WORD64  s_lwGranularity = 0x100000000UL;
    const static WORD32  s_dwMasterKey   = 0x05F5E505;
    const static WORD32  s_dwSlaveKey    = 0x05F5B665;

public : 
    CShmMgr();
    virtual ~CShmMgr();

    WORD32 Initialize(BOOL             bMaster,
                      WORD32           dwChannelNum,
                      WORD32           dwPowerNum,
                      CCentralMemPool *pCentralMemPool);

    WORD32 GetChannelNum();

    CChannelTpl * GetChannel(WORD32 dwIndex);
    CChannelTpl * GetCtrlChannel();

    VOID Snapshot();

    VOID Dump();
    VOID Printf();

protected :
    CChannelTpl * CreateChannel(BOOL bMaster, WORD32 dwPowerNum, BYTE *pBuf, WORD32 dwKeyS, WORD32 dwKeyR);
    CChannelTpl * CreateMaster(WORD32 dwPowerNum, BYTE *pBuf, WORD32 dwKeyS, WORD32 dwKeyR);
    CChannelTpl * CreateSlave(WORD32 dwPowerNum, BYTE *pBuf, WORD32 dwKeyS, WORD32 dwKeyR);

    CChannelTpl * CreateCtrlChannel(BOOL bMaster, BYTE *pBuf, WORD32 dwKeyS, WORD32 dwKeyR);
    CChannelTpl * CreateCtrlMaster(BYTE *pBuf, WORD32 dwKeyS, WORD32 dwKeyR);
    CChannelTpl * CreateCtrlSlave(BYTE *pBuf, WORD32 dwKeyS, WORD32 dwKeyR);

private :
    VOID * GetNextVirtAddr();

protected :
    WORD64               m_lwNextVirAddr;
    BOOL                 m_bMaster;
    WORD32               m_dwCtrlChannelID;
    WORD32               m_dwChannelNum;
    WORD32               m_dwPowerNum;

    CCentralMemPool     *m_pMemPool;
    CChannelTpl         *m_apChannel[MAX_CHANNEL_NUM];

    T_ChannelSnapshot    m_atChannel[MAX_CHANNEL_NUM];
};


inline WORD32 CShmMgr::GetChannelNum()
{
    return m_dwChannelNum;
}


inline CChannelTpl * CShmMgr::GetChannel(WORD32 dwIndex)
{
    if (dwIndex >= MAX_CHANNEL_NUM)
    {
        return NULL;
    }

    return m_apChannel[dwIndex];
}


inline CChannelTpl * CShmMgr::GetCtrlChannel()
{
    if (m_dwCtrlChannelID >= MAX_CHANNEL_NUM)
    {
        return NULL;
    }

    return m_apChannel[m_dwCtrlChannelID];
}


inline VOID CShmMgr::Snapshot()
{
    for (WORD32 dwIndex = 0; dwIndex < m_dwChannelNum; dwIndex++)
    {
        m_apChannel[dwIndex]->Snapshot(m_atChannel[dwIndex].tRecv,
                                       m_atChannel[dwIndex].tSend);
    }
}


#endif


