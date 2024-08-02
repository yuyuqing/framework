

#ifndef _BASE_SHM_HDL_H_
#define _BASE_SHM_HDL_H_


#include <sys/shm.h>
#include <string>
#include <assert.h>

#include "base_sem_ipc.h"
#include "base_singleton_tpl.h"
#include "base_util.h"
#include "base_data_container.h"
#include "base_lock.h"
#include "base_prefetch.h"
#include "base_measure.h"


#define SHM_HEAD_PAD_LEN             ((WORD32)(128))
#define SHM_POWER_NUM                ((WORD32)(12))
#define SHM_POWER_S_NUM              ((WORD32)(14))
#define SHM_POWER_R_NUM              ((WORD32)(12))
#define SHM_NODE_SIZE                ((WORD32)(2048))


typedef enum tagE_ShmMallocPoint
{
    E_SHM_MALLOC_POINT_00 = 0,
    E_SHM_MALLOC_POINT_01,
    E_SHM_MALLOC_POINT_02,
    E_SHM_MALLOC_POINT_03,
    E_SHM_MALLOC_POINT_04,
    E_SHM_MALLOC_POINT_05,
    E_SHM_MALLOC_POINT_06,
    E_SHM_MALLOC_POINT_07,
    E_SHM_MALLOC_POINT_08,
    E_SHM_MALLOC_POINT_09,
    E_SHM_MALLOC_POINT_10,
    E_SHM_MALLOC_POINT_11,
    E_SHM_MALLOC_POINT_12,
    E_SHM_MALLOC_POINT_13,
    E_SHM_MALLOC_POINT_14,
    E_SHM_MALLOC_POINT_15,

    E_SHM_MALLOC_POINT_NUM,
}E_ShmMallocPoint;


typedef struct tagT_ShmSnapshot
{
    volatile BOOL    bInitFlag;                    /* 初始化完成标志 */
    volatile SWORD32 iGlobalLock;                  /* 全局锁, 用于初始化及全局访问 */
    volatile SWORD32 iUserLock;                    /* 用户锁, 可用于业务 */
    volatile BOOL    bStatus;                      /* 信号量状态, 用于消息队列的收发通知 */

    volatile WORD32  dwProdHeadM;                  /* 内存池生产者游标:头指针 */
    volatile WORD32  dwProdTailM;                  /* 内存池生产者游标:尾指针 */
    volatile WORD32  dwConsHeadM;                  /* 内存池消费者游标:头指针 */
    volatile WORD32  dwConsTailM;                  /* 内存池消费者游标:尾指针 */
    
    volatile WORD32  dwProdHeadQ;                  /* 消息队列生产者游标:头指针 */
    volatile WORD32  dwProdTailQ;                  /* 消息队列生产者游标:尾指针 */
    volatile WORD32  dwConsHeadQ;                  /* 消息队列消费者游标:头指针 */
    volatile WORD32  dwConsTailQ;                  /* 消息队列消费者游标:尾指针 */
    
    volatile WORD64  lwMallocCount;                /* 内存分配次数统计 */
    volatile WORD64  lwFreeCount;                  /* 内存释放次数统计 */
    volatile WORD64  lwEnqueueCount;               /* 入消息队列统计 */
    volatile WORD64  lwDequeueCount;               /* 出消息队列统计 */
    
    volatile WORD64  alwStatM[BIT_NUM_OF_WORD32];  /* 内存分配后持续占用时间统计, 单位 : Cycle */
    volatile WORD64  alwStatQ[BIT_NUM_OF_WORD32];  /* 消息从入队到出队持续占用时间统计, 单位 : Cycle */

    /* 用于分析内存泄漏, 检查在指定Point申请的内存是否存在未释放 */
    volatile WORD64  alwMallocPoint[E_SHM_MALLOC_POINT_NUM];
    volatile WORD64  alwFreePoint[E_SHM_MALLOC_POINT_NUM];
}T_ShmSnapshot;


/* 建议NODE_SIZE取值为>64的2的整数次方, 如: 128/256/512/1024/2048/... */
template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
class CShmHandler
{
public :
    enum { SHM_SLAVE_INIT_WAIT = 128 };
    enum { SHM_BURST_NUM       =  32 };

    const static WORD64 s_lwAlignSize  = 0x200000UL;            /* 2M */
    const static WORD64 s_lwMaxSize    = 0x100000000UL;         /* 4G */
    const static WORD64 s_lwMagicValue = 0x0DE0B6B3A76409B1UL;  /* 大质数 */
    const static WORD32 s_dwVersion    = 1;                     /* 版本号 */
    const static WORD32 s_dwStartTag   = 0x05F5E0F5;            /* 质数 */
    const static WORD32 s_dwSizeM      = (1 << (POWER_NUM + 1));
    const static WORD32 s_dwMaskM      = (s_dwSizeM - 1);
    const static WORD32 s_dwSizeQ      = (1 << POWER_NUM);
    const static WORD32 s_dwMaskQ      = (s_dwSizeQ - 1);


    /* 共享内存大小必须小于4G */
    typedef struct tagT_ShmHead
    {
        volatile WORD64  lwMagic;                      /* 魔法数字 */
        volatile WORD32  dwVersion;                    /* 版本号 */
        volatile WORD32  dwHeadSize;                   /* 头信息size */
        
        volatile WORD32  dwPowerNumM;                  /* 指定内存池队列的指数次幂(N+1) */
        volatile WORD32  dwPowerNumQ;                  /* 指定消息队列的指数次幂(N) */
        volatile WORD32  dwNodeNumM;                   /* 必须是2^(N+1)次幂 */
        volatile WORD32  dwNodeNumQ;                   /* 必须是2^(N)次幂 */
        volatile WORD32  dwCapacityM;                  /* 必须是(2^(N+1) - 1) */
        volatile WORD32  dwCapacityQ;                  /* 必须是(2^(N) - 1) */
        volatile WORD32  dwStartPos;                   /* 节点起始地址偏移 */
        volatile WORD32  dwResved;

        volatile BOOL    bInitFlag;                    /* 初始化完成标志 */
        volatile SWORD32 iGlobalLock;                  /* 全局锁, 用于初始化及全局访问 */
        volatile SWORD32 iUserLock;                    /* 用户锁, 可用于业务 */
        volatile BOOL    bStatus;                      /* 信号量状态, 用于消息队列的收发通知 */

        volatile WORD32  dwProdHeadM;                  /* 内存池生产者游标:头指针 */
        volatile WORD32  dwProdTailM;                  /* 内存池生产者游标:尾指针 */
        volatile WORD32  dwConsHeadM;                  /* 内存池消费者游标:头指针 */
        volatile WORD32  dwConsTailM;                  /* 内存池消费者游标:尾指针 */

        volatile WORD32  dwProdHeadQ;                  /* 消息队列生产者游标:头指针 */
        volatile WORD32  dwProdTailQ;                  /* 消息队列生产者游标:尾指针 */
        volatile WORD32  dwConsHeadQ;                  /* 消息队列消费者游标:头指针 */
        volatile WORD32  dwConsTailQ;                  /* 消息队列消费者游标:尾指针 */

        volatile WORD64  lwMallocCount;                /* 内存分配次数统计 */
        volatile WORD64  lwFreeCount;                  /* 内存释放次数统计 */
        volatile WORD64  lwEnqueueCount;               /* 入消息队列统计 */
        volatile WORD64  lwDequeueCount;               /* 出消息队列统计 */

        volatile WORD64  alwStatM[BIT_NUM_OF_WORD32];  /* 内存分配后持续占用时间统计, 单位 : Cycle */
        volatile WORD64  alwStatQ[BIT_NUM_OF_WORD32];  /* 消息从入队到出队持续占用时间统计, 单位 : Cycle */

        /* 用于分析内存泄漏, 检查在指定Point申请的内存是否存在未释放 */
        volatile WORD64  alwMallocPoint[E_SHM_MALLOC_POINT_NUM];
        volatile WORD64  alwFreePoint[E_SHM_MALLOC_POINT_NUM];

        volatile CHAR    aucResved1[SHM_HEAD_PAD_LEN]; /* 保留字段, 防踩踏 */

        volatile WORD32  adwNodesM[s_dwSizeM];         /* 存放内存池队列的节点信息(地址偏移) */

        volatile CHAR    aucResved2[SHM_HEAD_PAD_LEN]; /* 隔离字段, 防踩踏 */

        volatile WORD32  adwNodesQ[s_dwSizeQ];         /* 存放消息队列的节点信息(地址偏移) */
    }T_ShmHead;


    typedef struct tagT_ShmNodeHead
    {
        volatile WORD32  dwStartTag;    /* 起始标志, 可用于检测踩踏 */
        volatile WORD32  dwPos;         /* 相对于T_ShmHead.lwStartPos的地址偏移 */
        volatile WORD32  dwRefCount;    /* 0 : 空闲未分配; 引用计数 */
        volatile WORD32  dwPoint;       /* 分配点(用于check内存) */
        volatile WORD32  dwLen;
        volatile WORD32  dwTimeOffset;  /* 时间偏移(从内存分配到入队的时间偏移) */
        volatile WORD64  lwClock;       /* 内存分配时间, 单位:Cycle */
        volatile CHAR    aucData[NODE_SIZE - 32];
    }T_ShmNodeHead;
    static_assert(sizeof(T_ShmNodeHead) == NODE_SIZE, "unexpected T_ShmNodeHead layout");

    const static WORD32 s_dwNodeOffset = offsetof(T_ShmNodeHead, aucData);

public :
    CShmHandler();
    virtual ~CShmHandler();

    /* Master作为消息队列接收方, Slave作为消息队列发送方 */
    WORD32 Initialize(WORD32 dwKey, BOOL bMaster);

    WORD32 Clean(WORD32 dwKey);

    BOOL IsInitialized();

    VOID LockGlobal(T_ShmHead &rtHead);
    VOID UnLockGlobal(T_ShmHead &rtHead);
    VOID LockUser(T_ShmHead &rtHead);
    VOID UnLockUser(T_ShmHead &rtHead);

    WORD32 Post();  /* 消息入队列后通知 */
    WORD32 Wait();  /* 等待收消息队列 */

    virtual BYTE * Malloc(WORD32 dwSize, WORD32 dwPoint);
    virtual WORD32 Free(BYTE *pAddr);
    virtual VOID IncRefCount(BYTE *pAddr);

    virtual WORD32 Enqueue(VOID *pObj);
    virtual WORD32 Dequeue(VOID **pObj);

    WORD32 EnqueueBurst(WORD32 dwNum, VOID **pObjs, WORD32 &rdwFreeSize);
    WORD32 DequeueBurst(WORD32 dwNum, VOID **pObjs, WORD32 &rdwAvailable);

    T_ShmHead * GetShmHead();

    VOID Snapshot(T_ShmSnapshot &rtSnapshot);

protected :
    WORD32 InitContext(BOOL bMaster, T_ShmHead *ptHead);
    WORD32 InitMemPool(T_ShmHead* ptHead);

    WORD32 Attach(WORD32 dwKey, BOOL bMaster, WORD64 lwSize);
    WORD32 Detach(VOID *ptAddr, SWORD32 iShmID);

    BOOL IsValid(BYTE *pAddr);

    WORD32 EnqueueM(T_ShmHead &rtHead, WORD32 dwPos);
    WORD32 DequeueM(T_ShmHead &rtHead, WORD32 &rdwPos);
    WORD32 EnqueueQ(T_ShmHead &rtHead, WORD32 dwPos);
    WORD32 DequeueQ(T_ShmHead &rtHead, WORD32 &rdwPos);

    WORD32 EnqueueBurstQ(T_ShmHead &rtHead, WORD32 dwNum, WORD32 *pdwPos, WORD32 &rdwFreeSize);
    WORD32 DequeueBurstQ(T_ShmHead &rtHead, WORD32 dwNum, WORD32 *pdwPos, WORD32 &rdwAvailable);

    /* 分配内存时, 检查内存节点是否异常 */
    WORD32 Check(T_ShmNodeHead &rtNode, WORD32 dwPoint);

protected :
    T_ShmHead   *m_ptShmHead;

    WORD32       m_dwKey;
    SWORD32      m_iShmID;
    BOOL         m_bMaster;    /* Master作为消息队列接收方, 负责完成消息队列的初始化 */
    WORD32       m_dwNodeNum;  /* MemPool的节点总数 */
    WORD64       m_lwMemSize;
    WORD64       m_lwMemBegin;
    WORD64       m_lwMemEnd;
};


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
CShmHandler<POWER_NUM, NODE_SIZE>::CShmHandler()
{
    m_ptShmHead  = NULL;
    m_dwKey      = INVALID_DWORD;
    m_iShmID     = -1;
    m_bMaster    = FALSE;
    m_dwNodeNum  = s_dwMaskQ;
    m_lwMemSize  = 0;
    m_lwMemBegin = 0;
    m_lwMemEnd   = 0;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
CShmHandler<POWER_NUM, NODE_SIZE>::~CShmHandler()
{
    if (m_bMaster)
    {
        Detach(m_ptShmHead, m_iShmID);
    }
    else
    {
        Detach(m_ptShmHead, -1);
    }

    m_ptShmHead  = NULL;
    m_dwKey      = INVALID_DWORD;
    m_iShmID     = -1;
    m_bMaster    = FALSE;
    m_dwNodeNum  = s_dwMaskQ;
    m_lwMemSize  = 0;
    m_lwMemBegin = 0;
    m_lwMemEnd   = 0;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::Initialize(WORD32 dwKey, BOOL bMaster)
{
    if (IsInitialized())
    {
        return FAIL;
    }

    WORD64 lwSize     = 0;
    WORD32 dwStartPos = ROUND_UP(sizeof(T_ShmHead), PAGE_SIZE);

    lwSize = dwStartPos + (m_dwNodeNum * NODE_SIZE);
    lwSize = ROUND_UP(lwSize, s_lwAlignSize);
    if (lwSize >= s_lwMaxSize)
    {
        return FAIL;
    }

    WORD32 dwResult = Attach(dwKey, bMaster, lwSize);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    return SUCCESS;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::Clean(WORD32 dwKey)
{
    key_t    tShmKey     = static_cast<key_t>(dwKey);
    SWORD32  iShmgetFlag = 0666;
    SWORD32  iShmID      = -1;

    iShmID = shmget(tShmKey, sizeof(T_ShmHead), iShmgetFlag);
    if (iShmID >= 0)
    {
        shmctl(iShmID, IPC_RMID, 0);
    }

    return SUCCESS;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline BOOL CShmHandler<POWER_NUM, NODE_SIZE>::IsInitialized()
{
    return (NULL != m_ptShmHead);
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline VOID CShmHandler<POWER_NUM, NODE_SIZE>::LockGlobal(
    typename CShmHandler<POWER_NUM, NODE_SIZE>::T_ShmHead &rtHead)
{
    SWORD32 iValue = 0;

    while (!__atomic_compare_exchange_n(&(rtHead.iGlobalLock), 
                                        &iValue, 
                                        1, 0,
                                        __ATOMIC_ACQUIRE, 
                                        __ATOMIC_RELAXED))
    {
        while (__atomic_load_n(&(rtHead.iGlobalLock), __ATOMIC_RELAXED))
        {
        #ifdef ARCH_ARM64    
            asm volatile("yield" ::: "memory");
        #else
            _mm_pause();
        #endif
        }

        iValue = 0;
    }
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline VOID CShmHandler<POWER_NUM, NODE_SIZE>::UnLockGlobal(
    typename CShmHandler<POWER_NUM, NODE_SIZE>::T_ShmHead &rtHead)
{
    __atomic_store_n(&(rtHead.iGlobalLock), 0, __ATOMIC_RELEASE);
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline VOID CShmHandler<POWER_NUM, NODE_SIZE>::LockUser(
    typename CShmHandler<POWER_NUM, NODE_SIZE>::T_ShmHead &rtHead)
{
    SWORD32 iValue = 0;

    while (!__atomic_compare_exchange_n(&(rtHead.iUserLock), 
                                        &iValue, 
                                        1, 0,
                                        __ATOMIC_ACQUIRE, 
                                        __ATOMIC_RELAXED))
    {
        while (__atomic_load_n(&(rtHead.iUserLock), __ATOMIC_RELAXED))
        {
        #ifdef ARCH_ARM64    
            asm volatile("yield" ::: "memory");
        #else
            _mm_pause();
        #endif
        }

        iValue = 0;
    }
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline VOID CShmHandler<POWER_NUM, NODE_SIZE>::UnLockUser(
    typename CShmHandler<POWER_NUM, NODE_SIZE>::T_ShmHead &rtHead)
{
    __atomic_store_n(&(rtHead.iUserLock), 0, __ATOMIC_RELEASE);
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::Post()
{
    __atomic_store_n(&(m_ptShmHead->bStatus), 1, __ATOMIC_RELEASE);

    return SUCCESS;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::Wait()
{
    SWORD32 iValue = 1;

    /* 如果bStatus==TRUE, 则将bStatus置位为FALSE */
    /* 如果bStatus已经为FALSE, 则等待bStatus被更新为TRUE */
    while (!__atomic_compare_exchange_n(&(m_ptShmHead->bStatus), 
                                        &iValue, 
                                        0, 0,
                                        __ATOMIC_ACQUIRE, 
                                        __ATOMIC_RELAXED))
    {
        while (!__atomic_load_n(&(m_ptShmHead->bStatus), __ATOMIC_RELAXED))
        {
        #ifdef ARCH_ARM64    
            asm volatile("yield" ::: "memory");
        #else
            _mm_pause();
        #endif
        }

        iValue = 1;
    }

    return SUCCESS;
}


/* 发送方进程调用 */
template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline BYTE * CShmHandler<POWER_NUM, NODE_SIZE>::Malloc(WORD32 dwSize, WORD32 dwPoint)
{
    if (unlikely((NULL == m_ptShmHead) || (dwSize > (NODE_SIZE - s_dwNodeOffset))))
    {
        return NULL;
    }

    assert(dwPoint < E_SHM_MALLOC_POINT_NUM);

    BYTE   *pValue = NULL;
    WORD32  dwPos  = 0;
    WORD32  dwNum  = DequeueM(*m_ptShmHead, dwPos);
    if (0 == dwNum)
    {
        return NULL;
    }

    pValue = (BYTE *)(((WORD64)m_ptShmHead) + dwPos);
    PrefetchW<DOUBLE_CACHE_SIZE, 2>(pValue);

    m_ptShmHead->lwMallocCount++;
    m_ptShmHead->alwMallocPoint[dwPoint]++;

    T_ShmNodeHead *ptNode = (T_ShmNodeHead *)pValue;

#ifdef MEM_CHECK
    if (SUCCESS != Check(*ptNode, dwPoint))
    {
        assert(0);
    }
#endif

    ptNode->dwRefCount   = 1;
    ptNode->dwPoint      = dwPoint;
    ptNode->dwLen        = dwSize;
    ptNode->dwTimeOffset = 0;
    ptNode->lwClock      = GetCycle();

    return pValue + s_dwNodeOffset;
}


/* 接收方进程调用 */
template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::Free(BYTE *pAddr)
{
    if (!IsValid(pAddr))
    {
        assert(0);
    }

    T_ShmNodeHead *ptNode = (T_ShmNodeHead *)(((WORD64)pAddr) - s_dwNodeOffset);

    ptNode->dwRefCount--;
    if (ptNode->dwRefCount > 0)
    {
        /* dwRefCount已经为0, 则存在重复释放 */
        assert(0xFFFFFFFF != ptNode->dwRefCount);

        /* 引用计数减为0时, 才会实际释放内存 */
        return SUCCESS;
    }

    WORD64  lwPos    = ((WORD64)ptNode) - ((WORD64)m_ptShmHead);
    WORD64  lwBegin  = ptNode->lwClock;
    WORD64  lwEnd    = 0;
    WORD32  dwPeriod = 0;
    WORD32  dwPoint  = ptNode->dwPoint;

    ptNode->dwPoint = INVALID_DWORD;

    EnqueueM(*m_ptShmHead, (WORD32)lwPos);

    m_ptShmHead->lwFreeCount++;
    m_ptShmHead->alwFreePoint[dwPoint]++;

    lwEnd    = GetCycle();
    dwPeriod = (WORD32) ((lwEnd > lwBegin) ? ((lwEnd - lwBegin) >> 10) : (0));

    m_ptShmHead->alwStatM[base_bsr_uint32(dwPeriod)]++;

    return SUCCESS;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline VOID CShmHandler<POWER_NUM, NODE_SIZE>::IncRefCount(BYTE *pAddr)
{
    if (!IsValid((BYTE *)pAddr))
    {
        assert(0);
    }

    T_ShmNodeHead *ptNode = (T_ShmNodeHead *)(((WORD64)pAddr) - s_dwNodeOffset);

    ptNode->dwRefCount++;
}


/* 发送方进程调用 */
template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::Enqueue(VOID *pObj)
{
    if (!IsValid((BYTE *)pObj))
    {
        assert(0);
    }

    T_ShmNodeHead *ptNode   = (T_ShmNodeHead *)(((WORD64)pObj) - s_dwNodeOffset);
    WORD64         lwOffset = ((WORD64)pObj) - ((WORD64)m_ptShmHead);
    WORD64         lwBegin  = ptNode->lwClock;
    WORD64         lwEnd    = GetCycle();
    WORD32         dwPeriod = (WORD32) ((lwEnd > lwBegin) ? (lwEnd - lwBegin) : (0));

    ptNode->dwTimeOffset = dwPeriod;

    EnqueueQ(*m_ptShmHead, (WORD32)lwOffset);

    m_ptShmHead->lwEnqueueCount++;

    return SUCCESS;
}


/* 接收方进程调用 */
template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::Dequeue(VOID **pObj)
{
    if (unlikely(NULL == m_ptShmHead))
    {
        return FAIL;
    }

    WORD64 lwAddr   = 0;
    WORD64 lwBegin  = 0;
    WORD64 lwEnd    = 0;
    WORD32 dwPeriod = 0;
    WORD32 dwPos    = 0;
    WORD32 dwNum    = DequeueQ(*m_ptShmHead, dwPos);
    if (0 == dwNum)
    {
        return FAIL;
    }

    lwAddr = (((WORD64)m_ptShmHead) + dwPos);

    T_ShmNodeHead *ptNode = (T_ShmNodeHead *)(lwAddr - s_dwNodeOffset);
    PrefetchW<DOUBLE_CACHE_SIZE, 2>((BYTE *)ptNode);

    m_ptShmHead->lwDequeueCount++;

    *pObj = (VOID *)lwAddr;

    lwBegin  = ptNode->lwClock;
    lwBegin += ptNode->dwTimeOffset;
    lwEnd    = GetCycle();
    dwPeriod = (WORD32) ((lwEnd > lwBegin) ? ((lwEnd - lwBegin) >> 10) : (0));

    m_ptShmHead->alwStatQ[base_bsr_uint32(dwPeriod)]++;

    return SUCCESS;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::EnqueueBurst(
    WORD32   dwNum,
    VOID   **pObjs,
    WORD32  &rdwFreeSize)
{
    if (unlikely(NULL == m_ptShmHead))
    {
        return 0;
    }

    T_ShmNodeHead *ptNode = NULL;

    WORD32  adwPos[SHM_BURST_NUM] = {0, };
    VOID   *pObj     = NULL;
    WORD64  lwBegin  = 0;
    WORD64  lwEnd    = GetCycle();
    WORD32  dwPeriod = 0;

    dwNum = MIN(dwNum, SHM_BURST_NUM);

    for (WORD32 dwIndex = 0; dwIndex < dwNum; dwIndex++)
    {
        pObj = pObjs[dwIndex];
        if (!IsValid((BYTE *)pObj))
        {
            assert(0);
        }

        ptNode   = (T_ShmNodeHead *)(((WORD64)pObj) - s_dwNodeOffset);
        lwBegin  = ptNode->lwClock;
        dwPeriod = (WORD32) ((lwEnd > lwBegin) ? (lwEnd - lwBegin) : (0));

        ptNode->dwTimeOffset = dwPeriod;

        adwPos[dwIndex] = (WORD32)(((WORD64)pObj) - ((WORD64)m_ptShmHead));
    }

    dwNum = EnqueueBurstQ(*m_ptShmHead, dwNum, adwPos, rdwFreeSize);

    m_ptShmHead->lwEnqueueCount += dwNum;

    return dwNum;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::DequeueBurst(
    WORD32   dwNum,
    VOID   **pObjs,
    WORD32  &rdwAvailable)
{
    if (unlikely(NULL == m_ptShmHead))
    {
        return 0;
    }

    T_ShmNodeHead *ptNode = NULL;

    WORD32  adwPos[SHM_BURST_NUM] = {0, };
    WORD64  lwAddr   = 0;
    WORD64  lwBegin  = 0;
    WORD64  lwEnd    = GetCycle();
    WORD32  dwPeriod = 0;

    dwNum = MIN(dwNum, SHM_BURST_NUM);

    dwNum = DequeueBurstQ(*m_ptShmHead, dwNum, adwPos, rdwAvailable);
    if (0 == dwNum)
    {
        return 0;
    }

    m_ptShmHead->lwDequeueCount += dwNum;

    for (WORD32 dwIndex = 0; dwIndex < dwNum; dwIndex++)
    {
        pObjs[dwIndex] = (VOID *)(((WORD64)m_ptShmHead) + adwPos[dwIndex]);

        lwAddr   = (((WORD64)(pObjs[dwIndex])) - s_dwNodeOffset);
        ptNode   = (T_ShmNodeHead *)lwAddr;
        lwBegin  = ptNode->lwClock;
        lwBegin += ptNode->dwTimeOffset;
        dwPeriod = (WORD32) ((lwEnd > lwBegin) ? ((lwEnd - lwBegin) >> 10) : (0));

        m_ptShmHead->alwStatQ[base_bsr_uint32(dwPeriod)]++;
    }

    return dwNum;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline typename CShmHandler<POWER_NUM, NODE_SIZE>::T_ShmHead * 
CShmHandler<POWER_NUM, NODE_SIZE>::GetShmHead()
{
    return m_ptShmHead;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline VOID CShmHandler<POWER_NUM, NODE_SIZE>::Snapshot(T_ShmSnapshot &rtSnapshot)
{
    rtSnapshot.bInitFlag      = __atomic_load_n(&(m_ptShmHead->bInitFlag), __ATOMIC_RELAXED);
    rtSnapshot.iGlobalLock    = __atomic_load_n(&(m_ptShmHead->iGlobalLock), __ATOMIC_RELAXED);
    rtSnapshot.iUserLock      = __atomic_load_n(&(m_ptShmHead->iUserLock), __ATOMIC_RELAXED);
    rtSnapshot.bStatus        = __atomic_load_n(&(m_ptShmHead->bStatus), __ATOMIC_RELAXED);
    rtSnapshot.dwProdHeadM    = __atomic_load_n(&(m_ptShmHead->dwProdHeadM), __ATOMIC_RELAXED);
    rtSnapshot.dwProdTailM    = __atomic_load_n(&(m_ptShmHead->dwProdTailM), __ATOMIC_RELAXED);
    rtSnapshot.dwConsHeadM    = __atomic_load_n(&(m_ptShmHead->dwConsHeadM), __ATOMIC_RELAXED);
    rtSnapshot.dwConsTailM    = __atomic_load_n(&(m_ptShmHead->dwConsTailM), __ATOMIC_RELAXED);
    rtSnapshot.dwProdHeadQ    = __atomic_load_n(&(m_ptShmHead->dwProdHeadQ), __ATOMIC_RELAXED);
    rtSnapshot.dwProdTailQ    = __atomic_load_n(&(m_ptShmHead->dwProdTailQ), __ATOMIC_RELAXED);
    rtSnapshot.dwConsHeadQ    = __atomic_load_n(&(m_ptShmHead->dwConsHeadQ), __ATOMIC_RELAXED);
    rtSnapshot.dwConsTailQ    = __atomic_load_n(&(m_ptShmHead->dwConsTailQ), __ATOMIC_RELAXED);
    rtSnapshot.lwMallocCount  = __atomic_load_n(&(m_ptShmHead->lwMallocCount), __ATOMIC_RELAXED);
    rtSnapshot.lwFreeCount    = __atomic_load_n(&(m_ptShmHead->lwFreeCount), __ATOMIC_RELAXED);
    rtSnapshot.lwEnqueueCount = __atomic_load_n(&(m_ptShmHead->lwEnqueueCount), __ATOMIC_RELAXED);
    rtSnapshot.lwDequeueCount = __atomic_load_n(&(m_ptShmHead->lwDequeueCount), __ATOMIC_RELAXED);

    memcpy((VOID *)(rtSnapshot.alwStatM),
           (VOID *)(m_ptShmHead->alwStatM),
           (BIT_NUM_OF_WORD32 * sizeof(WORD64)));
    memcpy((VOID *)(rtSnapshot.alwStatQ),
           (VOID *)(m_ptShmHead->alwStatQ),
           (BIT_NUM_OF_WORD32 * sizeof(WORD64)));

    memcpy((VOID *)(rtSnapshot.alwMallocPoint),
           (VOID *)(m_ptShmHead->alwMallocPoint),
           (E_SHM_MALLOC_POINT_NUM * sizeof(WORD64)));
    memcpy((VOID *)(rtSnapshot.alwFreePoint),
           (VOID *)(m_ptShmHead->alwFreePoint),
           (E_SHM_MALLOC_POINT_NUM * sizeof(WORD64)));
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::InitContext(BOOL bMaster, T_ShmHead *ptHead)
{
    WORD32 dwStartPos  = ROUND_UP(sizeof(T_ShmHead), PAGE_SIZE);
    WORD32 dwWaitCount = 0;

    if (bMaster)
    {
        ptHead->lwMagic     = s_lwMagicValue;
        ptHead->dwVersion   = s_dwVersion;
        ptHead->dwHeadSize  = sizeof(T_ShmHead);
        ptHead->dwPowerNumM = POWER_NUM + 1;
        ptHead->dwPowerNumQ = POWER_NUM;
        ptHead->dwNodeNumM  = s_dwSizeM;
        ptHead->dwNodeNumQ  = s_dwSizeQ;
        ptHead->dwCapacityM = s_dwMaskM;
        ptHead->dwCapacityQ = s_dwMaskQ;
        ptHead->dwStartPos  = dwStartPos;
        ptHead->bInitFlag   = FALSE;
        ptHead->iGlobalLock = 0;
        ptHead->iUserLock   = 0;
        ptHead->bStatus     = FALSE;

        LockGlobal(*ptHead);

        ptHead->dwProdHeadM    = 0;
        ptHead->dwProdTailM    = 0;
        ptHead->dwConsHeadM    = 0;
        ptHead->dwConsTailM    = 0;
        ptHead->dwProdHeadQ    = 0;
        ptHead->dwProdTailQ    = 0;
        ptHead->dwConsHeadQ    = 0;
        ptHead->dwConsTailQ    = 0;
        ptHead->lwMallocCount  = 0;
        ptHead->lwFreeCount    = 0;
        ptHead->lwEnqueueCount = 0;
        ptHead->lwDequeueCount = 0;

        memset((VOID *)(ptHead->alwStatM),       0x00, (BIT_NUM_OF_WORD32 * sizeof(WORD64)));
        memset((VOID *)(ptHead->alwStatQ),       0x00, (BIT_NUM_OF_WORD32 * sizeof(WORD64)));
        memset((VOID *)(ptHead->alwMallocPoint), 0x00, (E_SHM_MALLOC_POINT_NUM * sizeof(WORD64)));
        memset((VOID *)(ptHead->alwFreePoint),   0x00, (E_SHM_MALLOC_POINT_NUM * sizeof(WORD64)));
        memset((VOID *)(ptHead->aucResved1),     0x00, SHM_HEAD_PAD_LEN);
        memset((VOID *)(ptHead->adwNodesM),      0x00, (s_dwSizeM * sizeof(WORD32)));
        memset((VOID *)(ptHead->aucResved2),     0x00, SHM_HEAD_PAD_LEN);
        memset((VOID *)(ptHead->adwNodesQ),      0x00, (s_dwSizeQ * sizeof(WORD32)));

        InitMemPool(ptHead);

        ptHead->bInitFlag = TRUE;

        UnLockGlobal(*ptHead);
    }
    else
    {
        do
        {
            if ( (s_lwMagicValue    != ptHead->lwMagic)
              || (s_dwVersion       != ptHead->dwVersion)
              || (sizeof(T_ShmHead) != ptHead->dwHeadSize)
              || ((POWER_NUM + 1)   != ptHead->dwPowerNumM)
              || (POWER_NUM         != ptHead->dwPowerNumQ)
              || (s_dwSizeM         != ptHead->dwNodeNumM)
              || (s_dwSizeQ         != ptHead->dwNodeNumQ)
              || (s_dwMaskM         != ptHead->dwCapacityM)
              || (s_dwMaskQ         != ptHead->dwCapacityQ)
              || (dwStartPos        != ptHead->dwStartPos))
            {
                sleep(1);

                dwWaitCount++;
                if (dwWaitCount >= SHM_SLAVE_INIT_WAIT)
                {
                    return FAIL;
                }
            }
            else
            {
                LockGlobal(*ptHead);

                if (TRUE == ptHead->bInitFlag)
                {
                    UnLockGlobal(*ptHead);
                    break ;
                }
                else
                {
                    UnLockGlobal(*ptHead);
                    usleep(100);
                }
            }
        } while(TRUE);
    }

    return SUCCESS;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::InitMemPool(T_ShmHead *ptHead)
{
    if (NULL == ptHead)
    {
        return FAIL;
    }

    T_ShmNodeHead *ptNode = NULL;

    WORD32 dwBegin    = ptHead->dwStartPos;
    WORD32 dwOffset   = 0;
    WORD64 lwBaseAddr = (WORD64)ptHead;
    WORD64 lwAddr     = 0;

    for (WORD32 dwIndex = 0; dwIndex < m_dwNodeNum; dwIndex++)
    {
        dwOffset = dwBegin + (dwIndex * NODE_SIZE);
        lwAddr   = lwBaseAddr + dwOffset;
        ptNode   = (T_ShmNodeHead *)lwAddr;

        ptNode->dwStartTag   = s_dwStartTag;
        ptNode->dwPos        = dwOffset;
        ptNode->dwRefCount   = 0;
        ptNode->dwPoint      = INVALID_DWORD;
        ptNode->dwLen        = 0;
        ptNode->dwTimeOffset = 0;
        ptNode->lwClock      = 0;

        EnqueueM(*ptHead, dwOffset);
    }

    return SUCCESS;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::Attach(WORD32 dwKey, BOOL bMaster, WORD64 lwSize)
{
    key_t    tShmKey     = static_cast<key_t>(dwKey);
    SWORD32  iShmgetFlag = 0666;
    SWORD32  iShmID      = -1;
    WORD32   dwResult    = 0;
    VOID    *pAddr       = NULL;

    struct shmid_ds tShmDS;

    /* 创建共享内存 */
    iShmID = shmget(tShmKey, lwSize, iShmgetFlag);
    if ((iShmID < 0) && (ENOENT == errno))
    {
        iShmgetFlag = 0666 | IPC_CREAT | IPC_EXCL;
        //iShmgetFlag = 0666 | SHM_HUGETLB | IPC_CREAT | IPC_EXCL;

        iShmID = shmget(tShmKey, lwSize, iShmgetFlag);
        if (iShmID < 0)
        {
            return FAIL;
        }
    }

    pAddr = shmat(iShmID, NULL, 0);
    if (((VOID *)-1) == pAddr)
    {
        return FAIL;
    }

    if (shmctl(iShmID, IPC_STAT, &tShmDS) < 0)
    {
        Detach(pAddr, iShmID);
        return FAIL;
    }

    if (lwSize != tShmDS.shm_segsz)
    {
        Detach(pAddr, iShmID);
        return FAIL;
    }

    dwResult = InitContext(bMaster, (T_ShmHead *)pAddr);
    if (SUCCESS != dwResult)
    {
        Detach(pAddr, iShmID);
        return FAIL;
    }

    m_ptShmHead  = (T_ShmHead *)pAddr;
    m_dwKey      = dwKey;
    m_iShmID     = iShmID;
    m_bMaster    = bMaster;
    m_lwMemSize  = lwSize;
    m_lwMemBegin = ((WORD64)pAddr) + ROUND_UP(sizeof(T_ShmHead), PAGE_SIZE);
    m_lwMemEnd   = m_lwMemBegin + (m_dwNodeNum * NODE_SIZE);

    return SUCCESS;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::Detach(VOID *ptAddr, SWORD32 iShmID)
{
    if (NULL != ptAddr)
    {
        shmdt(ptAddr);
    }

    if (iShmID > 0)
    {
        shmctl(iShmID, IPC_RMID, 0);
    }

    return SUCCESS;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline BOOL CShmHandler<POWER_NUM, NODE_SIZE>::IsValid(BYTE *pAddr)
{
    if (unlikely(NULL == m_ptShmHead))
    {
        return FALSE;
    }

    WORD64 lwAddr = (WORD64)pAddr;
    if ((lwAddr < (m_lwMemBegin + s_dwNodeOffset)) || (lwAddr >= m_lwMemEnd))
    {
        return FALSE;
    }

    return (0 == ((lwAddr - s_dwNodeOffset - m_lwMemBegin) % NODE_SIZE));
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::EnqueueM(
    typename CShmHandler<POWER_NUM, NODE_SIZE>::T_ShmHead &rtHead,
    WORD32 dwPos)
{
    WORD32 dwProdHead    = 0;
    WORD32 dwProdNext    = 0;
    WORD32 dwConsTail    = 0;
    WORD32 dwFreeEntries = 0;
    WORD32 dwSuccess     = 0;

    dwProdHead = __atomic_load_n(&(rtHead.dwProdHeadM), __ATOMIC_RELAXED);
    
    do
    {
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
    
        dwConsTail    = __atomic_load_n(&(rtHead.dwConsTailM), __ATOMIC_ACQUIRE);
        dwFreeEntries = (s_dwMaskM + dwConsTail - dwProdHead);
        if (unlikely(0 == dwFreeEntries))
        {
            continue ;
        }
    
        dwProdNext = dwProdHead + 1;
    
        /* 若(m_ptShmHead->dwProdHeadM)取值与dwProdHead相等, 则将dwProdNext的值写入(m_ptShmHead->dwProdHeadM) */
        /* 若(m_ptShmHead->dwProdHeadM)取值与dwProdHead不等, 则将(m_ptShmHead->dwProdHeadM)的值写入dwProdHead */
        dwSuccess = __atomic_compare_exchange_n(&(rtHead.dwProdHeadM),
                                                &dwProdHead,
                                                dwProdNext,
                                                0,
                                                __ATOMIC_RELAXED,
                                                __ATOMIC_RELAXED);
    }while (unlikely(0 == dwSuccess));

    rtHead.adwNodesM[(dwProdHead & s_dwMaskM)] = dwPos;

    while (__atomic_load_n(&(rtHead.dwProdTailM), __ATOMIC_RELAXED) != dwProdHead)
    {
#ifdef ARCH_ARM64    
        asm volatile("yield" ::: "memory");
#else
        _mm_pause();
#endif
    }
    
    __atomic_store_n(&(rtHead.dwProdTailM), dwProdNext, __ATOMIC_RELEASE);

    return 1;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::DequeueM(
    typename CShmHandler<POWER_NUM, NODE_SIZE>::T_ShmHead &rtHead,
    WORD32 &rdwPos)
{
    WORD32 dwConsHead = 0;
    WORD32 dwConsNext = 0;
    WORD32 dwProdTail = 0;
    WORD32 dwEntries  = 0;
    WORD32 dwSuccess  = 0;

    dwConsHead = __atomic_load_n(&(rtHead.dwConsHeadM), __ATOMIC_RELAXED);

    do 
    {
        __atomic_thread_fence(__ATOMIC_ACQUIRE);

        dwProdTail = __atomic_load_n(&(rtHead.dwProdTailM), __ATOMIC_ACQUIRE);
        dwEntries  = (dwProdTail - dwConsHead);
        if (0 == dwEntries)
        {
            return 0;
        }

        dwConsNext = dwConsHead + 1;

        /* 若dwHead取值与rdwOldHead相等, 则将rdwNewHead的值写入dwHead */
        /* 若dwHead取值与rdwOldHead不等, 则将dwHead的值写入rdwOldHead */
        dwSuccess = __atomic_compare_exchange_n(&(rtHead.dwConsHeadM),
                                                &dwConsHead,
                                                dwConsNext,
                                                0,
                                                __ATOMIC_RELAXED,
                                                __ATOMIC_RELAXED);
    }while (unlikely(dwSuccess == 0));

    rdwPos = rtHead.adwNodesM[(dwConsHead & s_dwMaskM)];
    
    while (__atomic_load_n(&(rtHead.dwConsTailM), __ATOMIC_RELAXED) != dwConsHead)
    {
#ifdef ARCH_ARM64    
        asm volatile("yield" ::: "memory");
#else
        _mm_pause();
#endif
    }
    
    __atomic_store_n(&(rtHead.dwConsTailM), dwConsNext, __ATOMIC_RELEASE);

    return 1;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::EnqueueQ(
    typename CShmHandler<POWER_NUM, NODE_SIZE>::T_ShmHead &rtHead,
    WORD32 dwPos)
{
    WORD32 dwProdHead    = 0;
    WORD32 dwProdNext    = 0;
    WORD32 dwConsTail    = 0;
    WORD32 dwFreeEntries = 0;
    WORD32 dwSuccess     = 0;

    dwProdHead = __atomic_load_n(&(rtHead.dwProdHeadQ), __ATOMIC_RELAXED);
    
    do
    {
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
    
        dwConsTail    = __atomic_load_n(&(rtHead.dwConsTailQ), __ATOMIC_ACQUIRE);
        dwFreeEntries = (s_dwMaskM + dwConsTail - dwProdHead);
        if (unlikely(0 == dwFreeEntries))
        {
            continue ;
        }
    
        dwProdNext = dwProdHead + 1;
    
        /* 若(m_ptShmHead->dwProdHeadM)取值与dwProdHead相等, 则将dwProdNext的值写入(m_ptShmHead->dwProdHeadM) */
        /* 若(m_ptShmHead->dwProdHeadM)取值与dwProdHead不等, 则将(m_ptShmHead->dwProdHeadM)的值写入dwProdHead */
        dwSuccess = __atomic_compare_exchange_n(&(rtHead.dwProdHeadQ),
                                                &dwProdHead,
                                                dwProdNext,
                                                0,
                                                __ATOMIC_RELAXED,
                                                __ATOMIC_RELAXED);
    }while (unlikely(0 == dwSuccess));

    rtHead.adwNodesQ[(dwProdHead & s_dwMaskQ)] = dwPos;

    while (__atomic_load_n(&(rtHead.dwProdTailQ), __ATOMIC_RELAXED) != dwProdHead)
    {
#ifdef ARCH_ARM64    
        asm volatile("yield" ::: "memory");
#else
        _mm_pause();
#endif
    }
    
    __atomic_store_n(&(rtHead.dwProdTailQ), dwProdNext, __ATOMIC_RELEASE);

    return 1;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::DequeueQ(
    typename CShmHandler<POWER_NUM, NODE_SIZE>::T_ShmHead &rtHead,
    WORD32 &rdwPos)
{
    WORD32 dwConsHead = 0;
    WORD32 dwConsNext = 0;
    WORD32 dwProdTail = 0;
    WORD32 dwEntries  = 0;
    WORD32 dwSuccess  = 0;

    dwConsHead = __atomic_load_n(&(rtHead.dwConsHeadQ), __ATOMIC_RELAXED);

    do 
    {
        __atomic_thread_fence(__ATOMIC_ACQUIRE);

        dwProdTail = __atomic_load_n(&(rtHead.dwProdTailQ), __ATOMIC_ACQUIRE);
        dwEntries  = (dwProdTail - dwConsHead);
        if (0 == dwEntries)
        {
            return 0;
        }

        dwConsNext = dwConsHead + 1;

        /* 若dwHead取值与rdwOldHead相等, 则将rdwNewHead的值写入dwHead */
        /* 若dwHead取值与rdwOldHead不等, 则将dwHead的值写入rdwOldHead */
        dwSuccess = __atomic_compare_exchange_n(&(rtHead.dwConsHeadQ),
                                                &dwConsHead,
                                                dwConsNext,
                                                0,
                                                __ATOMIC_RELAXED,
                                                __ATOMIC_RELAXED);
    }while (unlikely(dwSuccess == 0));

    rdwPos = rtHead.adwNodesQ[(dwConsHead & s_dwMaskQ)];
    
    while (__atomic_load_n(&(rtHead.dwConsTailQ), __ATOMIC_RELAXED) != dwConsHead)
    {
#ifdef ARCH_ARM64    
        asm volatile("yield" ::: "memory");
#else
        _mm_pause();
#endif
    }
    
    __atomic_store_n(&(rtHead.dwConsTailQ), dwConsNext, __ATOMIC_RELEASE);

    return 1;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::EnqueueBurstQ(
    T_ShmHead &rtHead,
    WORD32     dwNum,
    WORD32    *pdwPos,
    WORD32    &rdwFreeSize)
{
    WORD32 dwProdHead    = 0;
    WORD32 dwProdNext    = 0;
    WORD32 dwConsTail    = 0;
    WORD32 dwFreeEntries = 0;
    WORD32 dwSuccess     = 0;

    dwProdHead = __atomic_load_n(&(rtHead.dwProdHeadQ), __ATOMIC_RELAXED);
    
    do
    {
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
    
        dwConsTail    = __atomic_load_n(&(rtHead.dwConsTailQ), __ATOMIC_ACQUIRE);
        dwFreeEntries = (s_dwMaskM + dwConsTail - dwProdHead);
        if (unlikely(dwNum > dwFreeEntries))
        {
            dwNum = dwFreeEntries;
        }

        if (0 == dwNum)
        {
            return 0;
        }
    
        dwProdNext = dwProdHead + dwNum;
    
        /* 若(m_ptShmHead->dwProdHeadM)取值与dwProdHead相等, 则将dwProdNext的值写入(m_ptShmHead->dwProdHeadM) */
        /* 若(m_ptShmHead->dwProdHeadM)取值与dwProdHead不等, 则将(m_ptShmHead->dwProdHeadM)的值写入dwProdHead */
        dwSuccess = __atomic_compare_exchange_n(&(rtHead.dwProdHeadQ),
                                                &dwProdHead,
                                                dwProdNext,
                                                0,
                                                __ATOMIC_RELAXED,
                                                __ATOMIC_RELAXED);
    }while (unlikely(0 == dwSuccess));

    WORD32   dwI      = 0;
    WORD32   dwSize   = s_dwSizeQ;
    WORD32   dwIndex  = (dwProdHead & s_dwMaskQ);

    if (likely(dwIndex + dwNum < dwSize))
    {
        for (dwI = 0; dwI < (dwNum & ((~(unsigned)0x3))); dwI += 4, dwIndex += 4)
        {
            rtHead.adwNodesQ[dwIndex]   = pdwPos[dwI];
            rtHead.adwNodesQ[dwIndex+1] = pdwPos[dwI+1];
            rtHead.adwNodesQ[dwIndex+2] = pdwPos[dwI+2];
            rtHead.adwNodesQ[dwIndex+3] = pdwPos[dwI+3];
        }

        switch (dwNum & 0x03)
        {
        case 3:
            rtHead.adwNodesQ[dwIndex++] = pdwPos[dwI++];
        case 2:
            rtHead.adwNodesQ[dwIndex++] = pdwPos[dwI++];
        case 1:
            rtHead.adwNodesQ[dwIndex++] = pdwPos[dwI++];
        }
    }
    else
    {
        for (dwI = 0; dwIndex < dwSize; dwI++, dwIndex++)
        {
            rtHead.adwNodesQ[dwIndex] = pdwPos[dwI];
        }

        for (dwIndex = 0; dwI < dwNum; dwI++, dwIndex++)
        {
            rtHead.adwNodesQ[dwIndex] = pdwPos[dwI];
        }
    }

    while (__atomic_load_n(&(rtHead.dwProdTailQ), __ATOMIC_RELAXED) != dwProdHead)
    {
#ifdef ARCH_ARM64    
        asm volatile("yield" ::: "memory");
#else
        _mm_pause();
#endif
    }
    
    __atomic_store_n(&(rtHead.dwProdTailQ), dwProdNext, __ATOMIC_RELEASE);

    rdwFreeSize = dwFreeEntries - dwNum;

    return dwNum;
}


template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::DequeueBurstQ(
    T_ShmHead &rtHead,
    WORD32     dwNum,
    WORD32    *pdwPos,
    WORD32    &rdwAvailable)
{
    WORD32 dwConsHead = 0;
    WORD32 dwConsNext = 0;
    WORD32 dwProdTail = 0;
    WORD32 dwEntries  = 0;
    WORD32 dwSuccess  = 0;

    dwConsHead = __atomic_load_n(&(rtHead.dwConsHeadQ), __ATOMIC_RELAXED);

    do 
    {
        __atomic_thread_fence(__ATOMIC_ACQUIRE);

        dwProdTail = __atomic_load_n(&(rtHead.dwProdTailQ), __ATOMIC_ACQUIRE);
        dwEntries  = (dwProdTail - dwConsHead);
        if (dwNum > dwEntries)
        {
            dwNum = dwEntries;
        }

        if (unlikely(0 == dwNum))
        {
            return 0;
        }

        dwConsNext = dwConsHead + dwNum;

        /* 若(rtHead.dwConsHeadQ)取值与dwConsHead相等, 则将dwConsNext的值写入(rtHead.dwConsHeadQ) */
        /* 若(rtHead.dwConsHeadQ)取值与dwConsHead不等, 则将(rtHead.dwConsHeadQ)的值写入dwConsHead */
        dwSuccess = __atomic_compare_exchange_n(&(rtHead.dwConsHeadQ),
                                                &dwConsHead,
                                                dwConsNext,
                                                0,
                                                __ATOMIC_RELAXED,
                                                __ATOMIC_RELAXED);
    }while (unlikely(dwSuccess == 0));

    WORD32 dwI     = 0;
    WORD32 dwSize  = s_dwSizeQ;
    WORD32 dwIndex = (dwConsHead & s_dwMaskQ);

    if (likely(dwIndex + dwNum < dwSize))
    {
        for (dwI = 0; dwI < (dwNum & ((~(unsigned)0x3))); dwI += 4, dwIndex += 4)
        {
            pdwPos[dwI]   = rtHead.adwNodesQ[dwIndex];
            pdwPos[dwI+1] = rtHead.adwNodesQ[dwIndex+1];
            pdwPos[dwI+2] = rtHead.adwNodesQ[dwIndex+2];
            pdwPos[dwI+3] = rtHead.adwNodesQ[dwIndex+3];
        }

        switch (dwNum & 0x03)
        {
        case 3:
            pdwPos[dwI++] = rtHead.adwNodesQ[dwIndex++];
        case 2:
            pdwPos[dwI++] = rtHead.adwNodesQ[dwIndex++];
        case 1:
            pdwPos[dwI++] = rtHead.adwNodesQ[dwIndex++];
        }
    }
    else
    {
        for (dwI = 0; dwIndex < dwSize; dwI++, dwIndex++)
        {
            pdwPos[dwI] = rtHead.adwNodesQ[dwIndex];
        }

        for (dwIndex = 0; dwI < dwNum; dwI++, dwIndex++)
        {
            pdwPos[dwI] = rtHead.adwNodesQ[dwIndex];
        }
    }

    while (__atomic_load_n(&(rtHead.dwConsTailQ), __ATOMIC_RELAXED) != dwConsHead)
    {
#ifdef ARCH_ARM64    
        asm volatile("yield" ::: "memory");
#else
        _mm_pause();
#endif
    }
    
    __atomic_store_n(&(rtHead.dwConsTailQ), dwConsNext, __ATOMIC_RELEASE);

    rdwAvailable = dwEntries - dwNum;

    return dwNum;
}


/* 分配内存时, 检查内存节点是否异常 */
template <WORD32 POWER_NUM, WORD32 NODE_SIZE>
inline WORD32 CShmHandler<POWER_NUM, NODE_SIZE>::Check(
    typename CShmHandler<POWER_NUM, NODE_SIZE>::T_ShmNodeHead &rtNode,
    WORD32 dwPoint)
{
    if (INVALID_DWORD == dwPoint)
    {
        return SUCCESS;
    }

    WORD32 dwOffset = ((WORD64)(&rtNode)) - ((WORD64)m_ptShmHead);

    if ( (rtNode.dwStartTag != s_dwStartTag)
      || (rtNode.dwPos      != dwOffset)
      || (rtNode.dwRefCount != 0))
    {
        return FAIL;
    }

    return SUCCESS;
}


#endif


