

#ifndef _BASE_MEM_MGR_H_
#define _BASE_MEM_MGR_H_


#include "base_mem_pool.h"
#include "base_mem_trunk.h"


#define HUGE_MOUNTS_DIR           "/proc/mounts"
#define MEM_META_ZONE_PATH        "mem_meta.data"


#define DATA_ZONE_NUM             ((WORD32)(128))

/* �̳߳�ʼ��ʱ�ص�, ������DataZoneע���߳�ר������ */
typedef WORD32 (*CB_RegistZone) (T_DataZone &rtThreadZone, VOID *pThread);
typedef WORD32 (*CB_RemoveZone) (T_DataZone &rtThreadZone, VOID *pThread);


class CDataZone : public CBaseData
{
public :
    typedef CBaseDataContainer<T_DataZone, DATA_ZONE_NUM>    CZoneList;

public :
    CDataZone ();
    virtual ~CDataZone();

    WORD32 Initialize();

    /* ע���߳�ר��������(֧�ֱ��ظ�����, ��ʼ����־�̺߳Ͷ�ʱ���̺߳����ص�) */
    WORD32 RegistZone(WORD32         dwThreadType, 
                      CB_RegistZone  pFunc, 
                      VOID          *pThread = NULL);

    /* ȥע���߳�ר�������� */
    WORD32 RemoveZone(WORD32         dwThreadIdx, 
                      CB_RemoveZone  pFunc,
                      VOID          *pThread = NULL);

    T_DataZone * FindZone(WORD32 dwThreadIdx);

    VOID Dump();

    VOID Printf();

protected :
    /* ���߳�ר����������Ӧ���±� */
    WORD32                 m_dwMainZoneIdx;

    std::atomic<WORD32>    m_dwZoneNum;
    CZoneList              m_cList;
};


inline T_DataZone * CDataZone::FindZone(WORD32 dwThreadIdx)
{
    if (unlikely(dwThreadIdx >= DATA_ZONE_NUM))
    {
        return NULL;
    }

    T_DataZone *ptData = (T_DataZone *)(m_cList[dwThreadIdx]);
    if (unlikely(NULL == ptData))
    {
        return NULL;
    }

    if (INVALID_DWORD == ptData->dwThreadIdx)
    {
        return NULL;
    }

    return ptData;
}


#define HUGE_PATH_LEN        ((WORD32)(80))


typedef struct tagT_MemMetaHuge
{
    volatile SWORD32  iPrimaryFileID;
    volatile SWORD32  iSecondaryFileID;
    volatile WORD64   lwHugeAddr;
    CHAR              aucHugePath[HUGE_PATH_LEN];
}T_MemMetaHuge;


typedef struct tagT_MemMetaHead
{
    volatile WORD64  lwMagic;            /* ħ������ */
    volatile WORD32  dwVersion;          /* �汾�� */
    volatile WORD32  dwHeadSize;         /* ͷ��Ϣsize */

    volatile WORD64  lwMasterLock;       /* ��������(ֻ����1��Master���̴򿪱������ڴ�) */

    volatile SWORD32 iGlobalLock;        /* ȫ����, ���ڳ�ʼ����ȫ�ַ��� */
    volatile BOOL    bInitFlag;          /* ��ʼ����ɱ�־ */

    volatile SWORD32 iMLock;             /* �ڴ������, ���ڷ���/�����ڴ�� */
    volatile WORD32  dwHugeNum;          /* ��ҳ���� */

    T_MemMetaHuge    atHugeInfo[PAGE_NUM];

    volatile WORD64  lwMetaAddr;         /* �����̹����ڴ�Ԫ���������ַ */
    volatile WORD64  lwMetaSize;         /* �����ڴ�Ԫ�����ܴ�С */
    volatile WORD64  lwHugeAddr;         /* ��ҳ�ڴ���ʼ��ַ */
    volatile WORD64  lwShareMemSize;     /* ���̼乲���ڴ��ܴ�С */

    volatile WORD64  lwAppCntrlAddr;     /* CAppCntrlʵ����ַ */
    volatile WORD64  lwThreadCntrlAddr;  /* CThreadCntrlʵ����ַ */
}T_MemMetaHead;


/*********************************************************************************
 ---------------------------------------------------------------------------------
 |     64 BYTE    |    24704 BYTE |    128 BYTE     |   64 BYTE    |  128 BYTE   |
 |     CMemMgr    |    CDataZone  | CCentralMemPool |T_MemBufHeader|CGlobalClock |
 ---------------------------------------------------------------------------------
 ---------------------------------------------------------------------------------
 |     64 BYTE    |    Flexible    |    64 BYTE     |          Flexible          |
 | T_MemBufHeader |    CMemPools   | T_MemBufHeader |         g_pLogThread       |
 ---------------------------------------------------------------------------------
 ---------------------------------------------------------------------------------
 |     64 BYTE    |   14720 BYTE   |    64 BYTE     |  16504 BYTE + ObjMemPool   |
 | T_MemBufHeader |    g_pLogger   | T_MemBufHeader | pLogMemPool | xxxx(Buffer) | 
 ---------------------------------------------------------------------------------
 ---------------------------------------------------------------------------------
 * �ڻ���ַ��(m_pCentralMemPool)ƫ��320�ֽ�, ��Ϊ���߳�ר����T_DataZone������    *
 * �����߳�ר����T_DataZone������ƫ��128�ֽ�, ��Ϊg_pLogThread���߳�ר��������   *
 * ��g_pLogThread���߳�ר��������ƫ��128�ֽ�, ��Ϊ��һ��ҵ���̵߳�ר��������     *
 * ����ƫ��128�ֽ�, ��Ϊ����ҵ���̵߳�ר��������                                 *
 * ���û��������ʱ���߳�, ��g_pLogThread֮���������ҵ���߳�ר��������          *
 ---------------------------------------------------------------------------------
 *********************************************************************************/
class CMemMgr : public CBaseData
{
public :
    enum { MEM_META_SLAVE_INIT_WAIT = 128 };
    enum { HUGE_MOUNTS_LINE_LEN     = 256 };

    const static WORD64  s_lwVirBassAddr  = 0x2000000000UL;
    const static WORD64  s_lwAlign2M      = 0x200000UL;            /* 2M */
    const static WORD64  s_lwAlign1G      = 0x40000000UL;          /* 1G */
    const static WORD64  s_lwMaxSize      = 0x100000000UL;         /* 4G */
    const static WORD64  s_lwMagicValue   = 0x0DE0B6B3A76408A9UL;  /* ������ */
    const static WORD32  s_dwVersion      = 1;                     /* �汾�� */
    const static WORD32  s_dwKey          = 0x3B9ACB21;
    static const WORD32  s_dwDataZoneSize = ROUND_UP(sizeof(CDataZone), CACHE_SIZE);
    static const WORD32  s_dwMemPoolSize  = ROUND_UP(sizeof(CCentralMemPool), CACHE_SIZE);

    static CMemMgr * CreateMemMgr(WORD32      dwProcID,
                                  BYTE        ucMemType,
                                  BYTE        ucPageNum,
                                  WORD32      dwMemSize,
                                  const CHAR *pDir,
                                  BOOL        bMaster = TRUE);
    static CMemMgr * GetInstance();
    static VOID Destroy();

    static VOID LockGlobal(T_MemMetaHead &rtHead);
    static VOID UnLockGlobal(T_MemMetaHead &rtHead);

private :
    static WORD32 Clean(T_MemMetaHead *ptHead);

    static VOID * GetVirtualArea(VOID *pAddr, WORD32 dwSize);

    static T_MemMetaHead * CreateMetaMemory(WORD32      dwProcID,
                                            BYTE        ucMemType,
                                            BYTE        ucPageNum,
                                            WORD32      dwMemSize,
                                            const CHAR *pDir,
                                            BOOL        bMaster);

    static T_MemMetaHead * Create(WORD32 dwSize, SWORD32 &rdwFileID);
    static T_MemMetaHead * Attach(WORD32 dwSize, SWORD32 &rdwFileID);

    static T_MemMetaHead * ReAttach(T_MemMetaHead *ptHead,
                                    WORD32         dwSize,
                                    SWORD32        iFileID);

    static WORD32 InitContext(BOOL           bMaster,
                              WORD32         dwMetaSize,
                              WORD64         lwMemSize,
                              BYTE           ucMemType,
                              BYTE           ucPageNum,
                              WORD32         dwProcID,
                              const CHAR    *pDir,
                              T_MemMetaHead *ptHead);

    static WORD32 CheckHugePageDir(const CHAR *pDir);

    static WORD32 GetHugePageDir(WORD32      dwProcID,
                                 WORD32      dwPos,
                                 const CHAR *pBaseDir,
                                 CHAR       *pDstDir,
                                 WORD32      dwLen);

    static WORD32 InitHugePage(T_MemMetaHead *ptHead,
                               const CHAR    *pDir,
                               WORD32         dwProcID,
                               BYTE           ucPageNum);

    static WORD32 AttachHugePage(T_MemMetaHead *ptHead, WORD16 wPageNum);

    static WORD32 CreateHugePage(T_MemMetaHead *ptHead,
                                 const CHAR    *pDir,
                                 WORD32         dwProcID,
                                 WORD32         dwPos);

    static WORD32 LinkHugePage(T_MemMetaHead *ptHead, WORD32 dwPos);

    static WORD32 CleanHuge(T_MemMetaHead &rtHead);

public : 
    CMemMgr ();
    virtual ~CMemMgr();

    /* ��eTypeȡֵΪE_MEM_SHARE_TYPEʱ, ��ָ�������ļ��� */
    WORD32 Initialize(T_MemMetaHead *ptHead,
                      WORD64         lwBaseAddr,
                      WORD64         lwMemSize);

    T_MemMetaHead * GetMetaHead();
    CCentralMemPool * GetCentralMemPool();
    CDataZone * GetDataZone();

    /* ��ȡʹ������Ϣ */
    VOID GetUsage(WORD64 &lwSize, WORD64 &lwUsedSize, WORD64 &lwFreeSize);

    VOID Dump();

private :
    WORD32 InitDataZone();
    WORD32 InitCentralMemPool();

protected :
    T_MemMetaHead    *m_pMetaHead;       /* �����ڴ�Ԫ������ */

    WORD64            m_lwBaseAddr;      /* �����ڴ����ַ */
    WORD64            m_lwMemSize;       /* �����ڴ��ܴ�С */
    WORD64            m_lwDataZoneAddr;  /* CDataZone��ַ */
    WORD64            m_lwCentralAddr;   /* CCentralMemPool��ַ */

    CDataZone        *m_pDataZone;       /* �߳�ר�������� */

    /* �ڹ����ڴ�������ǰ�洴��CCentralMemPoolʵ��, ���ڿ���̼��ڴ湲�������� */
    CCentralMemPool  *m_pCentralMemPool;

private :
    static CMemMgr   *s_pInstance;
    static BOOL       s_bMaster;
};


inline T_MemMetaHead * CMemMgr::GetMetaHead()
{
    return m_pMetaHead;
}


inline CCentralMemPool * CMemMgr::GetCentralMemPool()
{
    return m_pCentralMemPool;
}


inline CDataZone * CMemMgr::GetDataZone()
{
    return m_pDataZone;
}


/* ��ȡʹ������Ϣ */
inline VOID CMemMgr::GetUsage(WORD64 &lwSize, WORD64 &lwUsedSize, WORD64 &lwFreeSize)
{
    m_pCentralMemPool->GetUsage(lwSize, lwUsedSize, lwFreeSize);
}


#endif


