

#ifndef _BASE_SHM_HDL_H_
#define _BASE_SHM_HDL_H_


#include <sys/shm.h>
#include <string>
#include <assert.h>

#include "base_prefetch.h"
#include "base_measure.h"
#include "base_mem_interface.h"


#define SHM_POWER_NUM_Q                ((WORD32)(16))
#define SHM_POWER_NUM_M                ((WORD32)(SHM_POWER_NUM_Q + 1))
#define SHM_QUEUE_NUM                  ((WORD32)(1 << SHM_POWER_NUM_Q))
#define SHM_MEM_NUM                    ((WORD32)(1 << SHM_POWER_NUM_M))

#define SHM_BURST_NUM                  ((WORD32)(32))


typedef enum tagE_ShmRole
{
    E_SHM_MASTER = 0,
    E_SHM_SLAVE,
    E_SHM_OBSERVER,
}E_ShmRole;


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


typedef struct tagT_ShmHdlHead
{
    volatile BOOL    bInitFlag;                    /* 初始化完成标志 */
    volatile BOOL    bStatus;                      /* 信号量状态, 用于消息队列的收发通知 */
    volatile WORD64  lwShmAddr;                    /* 共享内存起始地址(由主进程负责创建) */
    volatile WORD64  lwShmSize;                    /* 共享内存总大小(dwMemNum * dwMemSize) */
    volatile WORD32  dwMemNum;                     /* 内存块数量 */
    volatile WORD32  dwMemSize;                    /* 内存块size */

    volatile WORD32  dwPowerNumM;                  /* 指定内存池队列的指数次幂(N+1) */
    volatile WORD32  dwPowerNumQ;                  /* 指定消息队列的指数次幂(N) */
    volatile WORD32  dwNodeNumM;                   /* 必须是2^(N+1)次幂 */
    volatile WORD32  dwNodeNumQ;                   /* 必须是2^(N)次幂 */
    volatile WORD32  dwCapacityM;                  /* 必须是(2^(N+1) - 1) */
    volatile WORD32  dwCapacityQ;                  /* 必须是(2^(N) - 1) */

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

    volatile WORD64  lwResved;                     /* 保留字段, 字节对齐 */

    volatile WORD64  alwStatM[BIT_NUM_OF_WORD32];  /* 内存分配后持续占用时间统计, 单位 : Cycle */
    volatile WORD64  alwStatQ[BIT_NUM_OF_WORD32];  /* 消息从入队到出队持续占用时间统计, 单位 : Cycle */

    /* 用于分析内存泄漏, 检查在指定Point申请的内存是否存在未释放 */
    volatile WORD64  alwMallocPoint[E_SHM_MALLOC_POINT_NUM];
    volatile WORD64  alwFreePoint[E_SHM_MALLOC_POINT_NUM];

    volatile WORD64  alwResved[16];                /* 保留字段, 防止踩踏 */

    volatile WORD64  alwNodesM[SHM_MEM_NUM];       /* 存放内存池队列的节点信息(地址) */

    volatile BYTE    aucResved[QUARTER_PAGE_SIZE]; /* 隔离字段, 防踩踏 */

    volatile WORD64  alwNodesQ[SHM_QUEUE_NUM];     /* 存放消息队列的节点信息(地址) */
}T_ShmHdlHead;
static_assert((0 == (offsetof(T_ShmHdlHead, alwNodesM) % QUARTER_PAGE_SIZE)), "unexpected layout");
static_assert((0 == (offsetof(T_ShmHdlHead, alwNodesQ) % QUARTER_PAGE_SIZE)), "unexpected layout");


typedef struct tagT_ShmNodeHead
{
    volatile WORD32  dwStartTag;    /* 起始标志, 可用于检测踩踏 */
    volatile WORD32  dwIndex;       /* 偏移索引 */
    volatile WORD32  dwRefCount;    /* 0 : 空闲未分配; 引用计数 */
    volatile WORD32  dwPoint;       /* 分配点(用于check内存) */
    volatile WORD32  dwLen;
    volatile WORD32  dwTimeOffset;  /* 时间偏移(从内存分配到入队的时间偏移) */
    volatile WORD64  lwClock;       /* 内存分配时间, 单位:Cycle */
    volatile CHAR    aucData[];
}T_ShmNodeHead;
static_assert(sizeof(T_ShmNodeHead) == 32, "unexpected T_ShmNodeHead layout");


class CShmHandler
{
public :
    const static WORD32 s_dwStartTag   = 0x05F5E0F5;    /* 质数 */
    const static WORD32 s_dwNodeOffset = offsetof(T_ShmNodeHead, aucData);
    const static WORD32 s_dwSizeM      = SHM_MEM_NUM;
    const static WORD32 s_dwMaskM      = SHM_MEM_NUM - 1;
    const static WORD32 s_dwSizeQ      = SHM_QUEUE_NUM;
    const static WORD32 s_dwMaskQ      = SHM_QUEUE_NUM - 1;

public :
    CShmHandler ();
    virtual ~CShmHandler();

    WORD32 Initialize(E_ShmRole     eRole,
                      T_ShmHdlHead *ptHead,
                      BYTE         *pOriMemAddr,
                      WORD64        lwMemSize,
                      WORD32        dwNodeNum,
                      WORD32        dwNodeSize);

    WORD32 Post();  /* 消息入队列后通知 */
    WORD32 Wait();  /* 等待收消息队列 */

    BYTE * Malloc(WORD32 dwSize, WORD32 dwPoint);
    WORD32 Free(BYTE *pAddr);
    VOID IncRefCount(BYTE *pAddr);

    WORD32 Enqueue(VOID *pObj);
    WORD32 Dequeue(VOID **pObj);

    WORD32 EnqueueBurst(WORD32 dwNum, VOID **pObjs, WORD32 &rdwFreeSize);
    WORD32 DequeueBurst(WORD32 dwNum, VOID **pObjs, WORD32 &rdwAvailable);

    VOID Snapshot(T_ShmSnapshot &rtSnapshot);

protected :
    WORD32 EnqueueM(T_ShmHdlHead &rtHead, WORD64 lwAddr);
    WORD32 DequeueM(T_ShmHdlHead &rtHead, WORD64 &rlwAddr);
    WORD32 EnqueueQ(T_ShmHdlHead &rtHead, WORD64 lwAddr);
    WORD32 DequeueQ(T_ShmHdlHead &rtHead, WORD64 &rlwAddr);

    WORD32 EnqueueBurstQ(T_ShmHdlHead &rtHead, WORD32 dwNum, WORD64 *plwAddr, WORD32 &rdwFreeSize);
    WORD32 DequeueBurstQ(T_ShmHdlHead &rtHead, WORD32 dwNum, WORD64 *plwAddr, WORD32 &rdwAvailable);

    BOOL IsValid(BYTE *pAddr);

    /* 分配内存时, 检查内存节点是否异常 */
    WORD32 Check(T_ShmNodeHead &rtNode);

protected :
    E_ShmRole      m_eRole;
    WORD32         m_dwNodeNum;
    WORD32         m_dwNodeSize;

    T_ShmHdlHead  *m_ptHdlHead;
    BYTE          *m_pOriMemAddr;
    WORD64         m_lwMemSize;

    WORD64         m_lwMemBegin;
    WORD64         m_lwMemEnd;
};


/* 消息入队列后通知 */
inline WORD32 CShmHandler::Post()
{
    __atomic_store_n(&(m_ptHdlHead->bStatus), 1, __ATOMIC_RELEASE);

    return SUCCESS;
}


/* 等待收消息队列 */
inline WORD32 CShmHandler::Wait()
{
    SWORD32 iValue = 1;

    /* 如果bStatus==TRUE, 则将bStatus置位为FALSE */
    /* 如果bStatus已经为FALSE, 则等待bStatus被更新为TRUE */
    while (!__atomic_compare_exchange_n(&(m_ptHdlHead->bStatus), 
                                        &iValue, 
                                        0, 0,
                                        __ATOMIC_ACQUIRE, 
                                        __ATOMIC_RELAXED))
    {
        while (!__atomic_load_n(&(m_ptHdlHead->bStatus), __ATOMIC_RELAXED))
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


inline BYTE * CShmHandler::Malloc(WORD32 dwSize, WORD32 dwPoint)
{
    if (unlikely((NULL == m_ptHdlHead)
              || (dwSize > (m_dwNodeSize - s_dwNodeOffset))))
    {
        return NULL;
    }

    assert(dwPoint < E_SHM_MALLOC_POINT_NUM);

    BYTE   *pValue = NULL;
    WORD64  lwAddr = 0;
    WORD32  dwNum  = DequeueM(*m_ptHdlHead, lwAddr);
    if (0 == dwNum)
    {
        return NULL;
    }

    pValue = (BYTE *)lwAddr;
    PrefetchW<DOUBLE_CACHE_SIZE, 2>(pValue);

    m_ptHdlHead->lwMallocCount++;
    m_ptHdlHead->alwMallocPoint[dwPoint]++;

    T_ShmNodeHead *ptNode = (T_ShmNodeHead *)pValue;

#ifdef MEM_CHECK
    if (SUCCESS != Check(*ptNode))
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


inline WORD32 CShmHandler::Free(BYTE *pAddr)
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

    WORD64  lwBegin  = ptNode->lwClock;
    WORD64  lwEnd    = 0;
    WORD32  dwPeriod = 0;
    WORD32  dwPoint  = ptNode->dwPoint;

    ptNode->dwPoint = INVALID_DWORD;

    EnqueueM(*m_ptHdlHead, (WORD64)ptNode);

    m_ptHdlHead->lwFreeCount++;
    m_ptHdlHead->alwFreePoint[dwPoint]++;

    lwEnd    = GetCycle();
    dwPeriod = (WORD32) ((lwEnd > lwBegin) ? ((lwEnd - lwBegin) >> 10) : (0));

    m_ptHdlHead->alwStatM[base_bsr_uint32(dwPeriod)]++;

    return SUCCESS;
}


inline VOID CShmHandler::IncRefCount(BYTE *pAddr)
{
    if (!IsValid((BYTE *)pAddr))
    {
        assert(0);
    }

    T_ShmNodeHead *ptNode = (T_ShmNodeHead *)(((WORD64)pAddr) - s_dwNodeOffset);

    ptNode->dwRefCount++;
}


inline WORD32 CShmHandler::Enqueue(VOID *pObj)
{
    if (!IsValid((BYTE *)pObj))
    {
        assert(0);
    }

    T_ShmNodeHead *ptNode   = (T_ShmNodeHead *)(((WORD64)pObj) - s_dwNodeOffset);
    WORD64         lwBegin  = ptNode->lwClock;
    WORD64         lwEnd    = GetCycle();
    WORD32         dwPeriod = (WORD32) ((lwEnd > lwBegin) ? (lwEnd - lwBegin) : (0));

    ptNode->dwTimeOffset = dwPeriod;

    EnqueueQ(*m_ptHdlHead, (WORD64)pObj);

    m_ptHdlHead->lwEnqueueCount++;

    return SUCCESS;
}


inline WORD32 CShmHandler::Dequeue(VOID **pObj)
{
    WORD64 lwAddr   = 0;
    WORD64 lwBegin  = 0;
    WORD64 lwEnd    = 0;
    WORD32 dwPeriod = 0;
    WORD32 dwNum    = DequeueQ(*m_ptHdlHead, lwAddr);
    if (0 == dwNum)
    {
        return FAIL;
    }

    T_ShmNodeHead *ptNode = (T_ShmNodeHead *)(lwAddr - s_dwNodeOffset);
    PrefetchW<DOUBLE_CACHE_SIZE, 2>((BYTE *)ptNode);

    m_ptHdlHead->lwDequeueCount++;

    *pObj = (VOID *)lwAddr;

    lwBegin  = ptNode->lwClock;
    lwBegin += ptNode->dwTimeOffset;
    lwEnd    = GetCycle();
    dwPeriod = (WORD32) ((lwEnd > lwBegin) ? ((lwEnd - lwBegin) >> 10) : (0));

    m_ptHdlHead->alwStatQ[base_bsr_uint32(dwPeriod)]++;

    return SUCCESS;
}


inline WORD32 CShmHandler::EnqueueBurst(WORD32   dwNum,
                                        VOID   **pObjs,
                                        WORD32  &rdwFreeSize)
{
    T_ShmNodeHead *ptNode = NULL;

    WORD64  alwAddr[SHM_BURST_NUM] = {0, };
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

        alwAddr[dwIndex] = (WORD64)pObj;
    }

    dwNum = EnqueueBurstQ(*m_ptHdlHead, dwNum, alwAddr, rdwFreeSize);

    m_ptHdlHead->lwEnqueueCount += dwNum;

    return dwNum;
}


inline WORD32 CShmHandler::DequeueBurst(WORD32   dwNum,
                                        VOID   **pObjs,
                                        WORD32  &rdwAvailable)
{
    T_ShmNodeHead *ptNode = NULL;

    WORD64  alwAddr[SHM_BURST_NUM] = {0, };
    WORD64  lwBegin  = 0;
    WORD64  lwEnd    = GetCycle();
    WORD32  dwPeriod = 0;

    dwNum = MIN(dwNum, SHM_BURST_NUM);

    dwNum = DequeueBurstQ(*m_ptHdlHead, dwNum, alwAddr, rdwAvailable);
    if (0 == dwNum)
    {
        return 0;
    }

    m_ptHdlHead->lwDequeueCount += dwNum;

    for (WORD32 dwIndex = 0; dwIndex < dwNum; dwIndex++)
    {
        pObjs[dwIndex] = (VOID *)(alwAddr[dwIndex]);

        ptNode   = (T_ShmNodeHead *)(alwAddr[dwIndex] - s_dwNodeOffset);
        lwBegin  = ptNode->lwClock;
        lwBegin += ptNode->dwTimeOffset;
        dwPeriod = (WORD32) ((lwEnd > lwBegin) ? ((lwEnd - lwBegin) >> 10) : (0));

        m_ptHdlHead->alwStatQ[base_bsr_uint32(dwPeriod)]++;
    }

    return dwNum;
}


inline VOID CShmHandler::Snapshot(T_ShmSnapshot &rtSnapshot)
{
    rtSnapshot.bInitFlag      = __atomic_load_n(&(m_ptHdlHead->bInitFlag), __ATOMIC_RELAXED);
    rtSnapshot.bStatus        = __atomic_load_n(&(m_ptHdlHead->bStatus), __ATOMIC_RELAXED);
    rtSnapshot.dwProdHeadM    = __atomic_load_n(&(m_ptHdlHead->dwProdHeadM), __ATOMIC_RELAXED);
    rtSnapshot.dwProdTailM    = __atomic_load_n(&(m_ptHdlHead->dwProdTailM), __ATOMIC_RELAXED);
    rtSnapshot.dwConsHeadM    = __atomic_load_n(&(m_ptHdlHead->dwConsHeadM), __ATOMIC_RELAXED);
    rtSnapshot.dwConsTailM    = __atomic_load_n(&(m_ptHdlHead->dwConsTailM), __ATOMIC_RELAXED);
    rtSnapshot.dwProdHeadQ    = __atomic_load_n(&(m_ptHdlHead->dwProdHeadQ), __ATOMIC_RELAXED);
    rtSnapshot.dwProdTailQ    = __atomic_load_n(&(m_ptHdlHead->dwProdTailQ), __ATOMIC_RELAXED);
    rtSnapshot.dwConsHeadQ    = __atomic_load_n(&(m_ptHdlHead->dwConsHeadQ), __ATOMIC_RELAXED);
    rtSnapshot.dwConsTailQ    = __atomic_load_n(&(m_ptHdlHead->dwConsTailQ), __ATOMIC_RELAXED);
    rtSnapshot.lwMallocCount  = __atomic_load_n(&(m_ptHdlHead->lwMallocCount), __ATOMIC_RELAXED);
    rtSnapshot.lwFreeCount    = __atomic_load_n(&(m_ptHdlHead->lwFreeCount), __ATOMIC_RELAXED);
    rtSnapshot.lwEnqueueCount = __atomic_load_n(&(m_ptHdlHead->lwEnqueueCount), __ATOMIC_RELAXED);
    rtSnapshot.lwDequeueCount = __atomic_load_n(&(m_ptHdlHead->lwDequeueCount), __ATOMIC_RELAXED);

    rtSnapshot.alwStatM[0]  = __atomic_load_n(&(m_ptHdlHead->alwStatM[0]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[1]  = __atomic_load_n(&(m_ptHdlHead->alwStatM[1]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[2]  = __atomic_load_n(&(m_ptHdlHead->alwStatM[2]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[3]  = __atomic_load_n(&(m_ptHdlHead->alwStatM[3]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[4]  = __atomic_load_n(&(m_ptHdlHead->alwStatM[4]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[5]  = __atomic_load_n(&(m_ptHdlHead->alwStatM[5]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[6]  = __atomic_load_n(&(m_ptHdlHead->alwStatM[6]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[7]  = __atomic_load_n(&(m_ptHdlHead->alwStatM[7]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[8]  = __atomic_load_n(&(m_ptHdlHead->alwStatM[8]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[9]  = __atomic_load_n(&(m_ptHdlHead->alwStatM[9]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[10] = __atomic_load_n(&(m_ptHdlHead->alwStatM[10]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[11] = __atomic_load_n(&(m_ptHdlHead->alwStatM[11]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[12] = __atomic_load_n(&(m_ptHdlHead->alwStatM[12]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[13] = __atomic_load_n(&(m_ptHdlHead->alwStatM[13]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[14] = __atomic_load_n(&(m_ptHdlHead->alwStatM[14]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[15] = __atomic_load_n(&(m_ptHdlHead->alwStatM[15]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[16] = __atomic_load_n(&(m_ptHdlHead->alwStatM[16]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[17] = __atomic_load_n(&(m_ptHdlHead->alwStatM[17]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[18] = __atomic_load_n(&(m_ptHdlHead->alwStatM[18]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[19] = __atomic_load_n(&(m_ptHdlHead->alwStatM[19]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[20] = __atomic_load_n(&(m_ptHdlHead->alwStatM[20]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[21] = __atomic_load_n(&(m_ptHdlHead->alwStatM[21]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[22] = __atomic_load_n(&(m_ptHdlHead->alwStatM[22]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[23] = __atomic_load_n(&(m_ptHdlHead->alwStatM[23]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[24] = __atomic_load_n(&(m_ptHdlHead->alwStatM[24]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[25] = __atomic_load_n(&(m_ptHdlHead->alwStatM[25]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[26] = __atomic_load_n(&(m_ptHdlHead->alwStatM[26]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[27] = __atomic_load_n(&(m_ptHdlHead->alwStatM[27]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[28] = __atomic_load_n(&(m_ptHdlHead->alwStatM[28]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[29] = __atomic_load_n(&(m_ptHdlHead->alwStatM[29]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[30] = __atomic_load_n(&(m_ptHdlHead->alwStatM[30]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatM[31] = __atomic_load_n(&(m_ptHdlHead->alwStatM[31]), __ATOMIC_RELAXED);

    rtSnapshot.alwStatQ[0]  = __atomic_load_n(&(m_ptHdlHead->alwStatQ[0]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[1]  = __atomic_load_n(&(m_ptHdlHead->alwStatQ[1]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[2]  = __atomic_load_n(&(m_ptHdlHead->alwStatQ[2]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[3]  = __atomic_load_n(&(m_ptHdlHead->alwStatQ[3]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[4]  = __atomic_load_n(&(m_ptHdlHead->alwStatQ[4]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[5]  = __atomic_load_n(&(m_ptHdlHead->alwStatQ[5]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[6]  = __atomic_load_n(&(m_ptHdlHead->alwStatQ[6]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[7]  = __atomic_load_n(&(m_ptHdlHead->alwStatQ[7]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[8]  = __atomic_load_n(&(m_ptHdlHead->alwStatQ[8]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[9]  = __atomic_load_n(&(m_ptHdlHead->alwStatQ[9]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[10] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[10]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[11] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[11]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[12] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[12]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[13] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[13]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[14] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[14]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[15] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[15]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[16] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[16]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[17] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[17]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[18] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[18]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[19] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[19]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[20] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[20]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[21] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[21]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[22] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[22]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[23] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[23]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[24] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[24]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[25] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[25]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[26] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[26]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[27] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[27]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[28] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[28]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[29] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[29]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[30] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[30]), __ATOMIC_RELAXED);
    rtSnapshot.alwStatQ[31] = __atomic_load_n(&(m_ptHdlHead->alwStatQ[31]), __ATOMIC_RELAXED);

    rtSnapshot.alwMallocPoint[0]  = __atomic_load_n(&(m_ptHdlHead->alwMallocPoint[0]), __ATOMIC_RELAXED);
    rtSnapshot.alwMallocPoint[1]  = __atomic_load_n(&(m_ptHdlHead->alwMallocPoint[1]), __ATOMIC_RELAXED);
    rtSnapshot.alwMallocPoint[2]  = __atomic_load_n(&(m_ptHdlHead->alwMallocPoint[2]), __ATOMIC_RELAXED);
    rtSnapshot.alwMallocPoint[3]  = __atomic_load_n(&(m_ptHdlHead->alwMallocPoint[3]), __ATOMIC_RELAXED);
    rtSnapshot.alwMallocPoint[4]  = __atomic_load_n(&(m_ptHdlHead->alwMallocPoint[4]), __ATOMIC_RELAXED);
    rtSnapshot.alwMallocPoint[5]  = __atomic_load_n(&(m_ptHdlHead->alwMallocPoint[5]), __ATOMIC_RELAXED);
    rtSnapshot.alwMallocPoint[6]  = __atomic_load_n(&(m_ptHdlHead->alwMallocPoint[6]), __ATOMIC_RELAXED);
    rtSnapshot.alwMallocPoint[7]  = __atomic_load_n(&(m_ptHdlHead->alwMallocPoint[7]), __ATOMIC_RELAXED);
    rtSnapshot.alwMallocPoint[8]  = __atomic_load_n(&(m_ptHdlHead->alwMallocPoint[8]), __ATOMIC_RELAXED);
    rtSnapshot.alwMallocPoint[9]  = __atomic_load_n(&(m_ptHdlHead->alwMallocPoint[9]), __ATOMIC_RELAXED);
    rtSnapshot.alwMallocPoint[10] = __atomic_load_n(&(m_ptHdlHead->alwMallocPoint[10]), __ATOMIC_RELAXED);
    rtSnapshot.alwMallocPoint[11] = __atomic_load_n(&(m_ptHdlHead->alwMallocPoint[11]), __ATOMIC_RELAXED);
    rtSnapshot.alwMallocPoint[12] = __atomic_load_n(&(m_ptHdlHead->alwMallocPoint[12]), __ATOMIC_RELAXED);
    rtSnapshot.alwMallocPoint[13] = __atomic_load_n(&(m_ptHdlHead->alwMallocPoint[13]), __ATOMIC_RELAXED);
    rtSnapshot.alwMallocPoint[14] = __atomic_load_n(&(m_ptHdlHead->alwMallocPoint[14]), __ATOMIC_RELAXED);
    rtSnapshot.alwMallocPoint[15] = __atomic_load_n(&(m_ptHdlHead->alwMallocPoint[15]), __ATOMIC_RELAXED);

    rtSnapshot.alwFreePoint[0]  = __atomic_load_n(&(m_ptHdlHead->alwFreePoint[0]), __ATOMIC_RELAXED);
    rtSnapshot.alwFreePoint[1]  = __atomic_load_n(&(m_ptHdlHead->alwFreePoint[1]), __ATOMIC_RELAXED);
    rtSnapshot.alwFreePoint[2]  = __atomic_load_n(&(m_ptHdlHead->alwFreePoint[2]), __ATOMIC_RELAXED);
    rtSnapshot.alwFreePoint[3]  = __atomic_load_n(&(m_ptHdlHead->alwFreePoint[3]), __ATOMIC_RELAXED);
    rtSnapshot.alwFreePoint[4]  = __atomic_load_n(&(m_ptHdlHead->alwFreePoint[4]), __ATOMIC_RELAXED);
    rtSnapshot.alwFreePoint[5]  = __atomic_load_n(&(m_ptHdlHead->alwFreePoint[5]), __ATOMIC_RELAXED);
    rtSnapshot.alwFreePoint[6]  = __atomic_load_n(&(m_ptHdlHead->alwFreePoint[6]), __ATOMIC_RELAXED);
    rtSnapshot.alwFreePoint[7]  = __atomic_load_n(&(m_ptHdlHead->alwFreePoint[7]), __ATOMIC_RELAXED);
    rtSnapshot.alwFreePoint[8]  = __atomic_load_n(&(m_ptHdlHead->alwFreePoint[8]), __ATOMIC_RELAXED);
    rtSnapshot.alwFreePoint[9]  = __atomic_load_n(&(m_ptHdlHead->alwFreePoint[9]), __ATOMIC_RELAXED);
    rtSnapshot.alwFreePoint[10] = __atomic_load_n(&(m_ptHdlHead->alwFreePoint[10]), __ATOMIC_RELAXED);
    rtSnapshot.alwFreePoint[11] = __atomic_load_n(&(m_ptHdlHead->alwFreePoint[11]), __ATOMIC_RELAXED);
    rtSnapshot.alwFreePoint[12] = __atomic_load_n(&(m_ptHdlHead->alwFreePoint[12]), __ATOMIC_RELAXED);
    rtSnapshot.alwFreePoint[13] = __atomic_load_n(&(m_ptHdlHead->alwFreePoint[13]), __ATOMIC_RELAXED);
    rtSnapshot.alwFreePoint[14] = __atomic_load_n(&(m_ptHdlHead->alwFreePoint[14]), __ATOMIC_RELAXED);
    rtSnapshot.alwFreePoint[15] = __atomic_load_n(&(m_ptHdlHead->alwFreePoint[15]), __ATOMIC_RELAXED);
}


inline WORD32 CShmHandler::EnqueueM(T_ShmHdlHead &rtHead, WORD64 lwAddr)
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

    rtHead.alwNodesM[(dwProdHead & s_dwMaskM)] = lwAddr;

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


inline WORD32 CShmHandler::DequeueM(T_ShmHdlHead &rtHead, WORD64 &rlwAddr)
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

    rlwAddr = rtHead.alwNodesM[(dwConsHead & s_dwMaskM)];

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


inline WORD32 CShmHandler::EnqueueQ(T_ShmHdlHead &rtHead, WORD64 lwAddr)
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

    rtHead.alwNodesQ[(dwProdHead & s_dwMaskQ)] = lwAddr;

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


inline WORD32 CShmHandler::DequeueQ(T_ShmHdlHead &rtHead, WORD64 &rlwAddr)
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

    rlwAddr = rtHead.alwNodesQ[(dwConsHead & s_dwMaskQ)];

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


inline WORD32 CShmHandler::EnqueueBurstQ(T_ShmHdlHead &rtHead,
                                         WORD32        dwNum,
                                         WORD64       *plwAddr,
                                         WORD32       &rdwFreeSize)
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
            rtHead.alwNodesQ[dwIndex]   = plwAddr[dwI];
            rtHead.alwNodesQ[dwIndex+1] = plwAddr[dwI+1];
            rtHead.alwNodesQ[dwIndex+2] = plwAddr[dwI+2];
            rtHead.alwNodesQ[dwIndex+3] = plwAddr[dwI+3];
        }

        switch (dwNum & 0x03)
        {
        case 3:
            rtHead.alwNodesQ[dwIndex++] = plwAddr[dwI++];
        case 2:
            rtHead.alwNodesQ[dwIndex++] = plwAddr[dwI++];
        case 1:
            rtHead.alwNodesQ[dwIndex++] = plwAddr[dwI++];
        }
    }
    else
    {
        for (dwI = 0; dwIndex < dwSize; dwI++, dwIndex++)
        {
            rtHead.alwNodesQ[dwIndex] = plwAddr[dwI];
        }

        for (dwIndex = 0; dwI < dwNum; dwI++, dwIndex++)
        {
            rtHead.alwNodesQ[dwIndex] = plwAddr[dwI];
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


inline WORD32 CShmHandler::DequeueBurstQ(T_ShmHdlHead &rtHead,
                                         WORD32        dwNum,
                                         WORD64       *plwAddr,
                                         WORD32       &rdwAvailable)
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
            plwAddr[dwI]   = rtHead.alwNodesQ[dwIndex];
            plwAddr[dwI+1] = rtHead.alwNodesQ[dwIndex+1];
            plwAddr[dwI+2] = rtHead.alwNodesQ[dwIndex+2];
            plwAddr[dwI+3] = rtHead.alwNodesQ[dwIndex+3];
        }

        switch (dwNum & 0x03)
        {
        case 3:
            plwAddr[dwI++] = rtHead.alwNodesQ[dwIndex++];
        case 2:
            plwAddr[dwI++] = rtHead.alwNodesQ[dwIndex++];
        case 1:
            plwAddr[dwI++] = rtHead.alwNodesQ[dwIndex++];
        }
    }
    else
    {
        for (dwI = 0; dwIndex < dwSize; dwI++, dwIndex++)
        {
            plwAddr[dwI] = rtHead.alwNodesQ[dwIndex];
        }

        for (dwIndex = 0; dwI < dwNum; dwI++, dwIndex++)
        {
            plwAddr[dwI] = rtHead.alwNodesQ[dwIndex];
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


inline BOOL CShmHandler::IsValid(BYTE *pAddr)
{
    WORD64 lwAddr = (WORD64)pAddr;
    if ((lwAddr < (m_lwMemBegin + s_dwNodeOffset)) || (lwAddr >= m_lwMemEnd))
    {
        return FALSE;
    }

    return (0 == ((lwAddr - s_dwNodeOffset - m_lwMemBegin) % m_dwNodeSize));
}


/* 分配内存时, 检查内存节点是否异常 */
inline WORD32 CShmHandler::Check(T_ShmNodeHead &rtNode)
{
    WORD64 lwIndex = rtNode.dwIndex;
    WORD64 lwAddrR = m_lwMemBegin + (lwIndex * m_dwNodeSize);

    if ( (rtNode.dwStartTag != s_dwStartTag)
      || ((WORD64)(&rtNode) != lwAddrR)
      || (rtNode.dwRefCount != 0))
    {
        return FAIL;
    }

    return SUCCESS;
}


#endif


