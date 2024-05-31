

#include "base_mem_interface.h"
#include "base_log.h"


CMemInterface::CMemInterface ()
{
    m_pOriAddr    = NULL;
    m_lwAddr      = 0;
    m_lwSize      = 0;
    m_lwBegin     = 0;
    m_lwEnd       = 0;
    m_lwUsedSize  = 0;
    m_lwFreeSize  = 0;
    m_pFreeHeader = NULL;
}


CMemInterface::~CMemInterface()
{
    m_pOriAddr    = NULL;
    m_lwAddr      = 0;
    m_lwSize      = 0;
    m_lwBegin     = 0;
    m_lwEnd       = 0;
    m_lwUsedSize  = 0;
    m_lwFreeSize  = 0;
    m_pFreeHeader = NULL;
}


/* 在起始地址处需预留一个T_MemBufHeader头信息 */
WORD32 CMemInterface::Initialize(VOID *pOriAddr, WORD64 lwSize)
{
    if (NULL == pOriAddr)
    {
        return FAIL;
    }

    /* 备注 :
     * 1. 针对CCentralMemPool, 支持变长内存, 因此仅使用Header, 不使用Begin/End
     *    CCentralMemPool::Initialize维护内存池时需预留Header
     * 2. 针对CObjMemPoolInterface, 仅支持定长, 使用Begin/End, 不使用Header
     *    CObjMemPoolInterface::Initialize向CCentralMemPool申请内存块时需额外增加Header
     */

    m_pOriAddr    = pOriAddr;
    m_lwAddr      = (WORD64)pOriAddr;
    m_lwSize      = lwSize;
    m_lwBegin     = m_lwAddr + sizeof(T_MemBufHeader);
    m_lwEnd       = m_lwAddr + lwSize;
    m_lwUsedSize  = 0;
    m_lwFreeSize  = 0;
    m_pFreeHeader = (T_MemBufHeader *)m_pOriAddr;

    /* dwSize不包含MemBuf头, 因此在申请时需要在lwSize的基础上 + sizeof(T_MemBufHeader) */
    m_pFreeHeader->dwSize     = (WORD32)((lwSize - sizeof(T_MemBufHeader)) / CACHE_SIZE);
    m_pFreeHeader->dwRefCount = INVALID_DWORD;
    m_pFreeHeader->lwOffset   = 0;
    m_pFreeHeader->pNext      = NULL;

    return SUCCESS;
}


VOID CMemInterface::Dump()
{
    TRACE_STACK("CMemInterface::Dump()");

    WORD64 lwUsedSize = m_lwUsedSize.load(std::memory_order_relaxed);;
    WORD64 lwFreeSize = m_lwFreeSize.load(std::memory_order_relaxed);;

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "m_pOriAddr = %p, m_lwAddr = %lu, m_lwSize = %lu, "
               "m_lwBegin = %lu, m_lwEnd = %lu, "
               "m_lwUsedSize = %lu, m_lwFreeSize = %lu\n",
               m_pOriAddr,
               m_lwAddr,
               m_lwSize,
               m_lwBegin,
               m_lwEnd,
               lwUsedSize,
               lwFreeSize);
}


CCentralMemPool::CCentralMemPool ()
    : CMemInterface()
{
}


CCentralMemPool::~CCentralMemPool()
{
}


/* 分配dwSize大小内存块, 在分配的内存块前面额外申请一个T_MemBufHeader头
 * 在T_MemBufHeader头信息中记录所分配内存块信息
 */
BYTE * CCentralMemPool::Malloc(WORD32 dwSize,
                               WORD16 wThreshold,
                               WORD32 dwPoint)
{
    WORD32 dwRoundSize = ROUND_UP((dwSize + sizeof(T_MemBufHeader)), CACHE_SIZE) / CACHE_SIZE;
    WORD32 dwRealSize  = dwRoundSize * CACHE_SIZE;

    BYTE           *pValue = NULL;
    T_MemBufHeader *pCur   = NULL;
    T_MemBufHeader *pPrev  = NULL;
    T_MemBufHeader *pNext  = NULL;

    m_cLock.Lock();

    pPrev = m_pFreeHeader;
    pCur  = m_pFreeHeader;

    while (pCur)
    {
        pNext = pCur->pNext;

        if (dwRoundSize < pCur->dwSize)
        {
            if (pPrev == pCur)  /* 头结点 */
            {
                pPrev = (T_MemBufHeader *)((BYTE *)(pCur) + dwRealSize);

                /* 更新头指针 */
                m_pFreeHeader = pPrev;
            }
            else
            {
                pPrev->pNext = (T_MemBufHeader *)((BYTE *)(pCur) + dwRealSize);
                pPrev = pPrev->pNext;
            }

            pPrev->dwSize     = pCur->dwSize - dwRoundSize;
            pPrev->dwRefCount = INVALID_DWORD;
            pPrev->lwOffset   = pCur->lwOffset + dwRealSize;
            pPrev->pNext      = pCur->pNext;                

            m_cLock.UnLock();

            pCur->dwSize     = dwRoundSize;
            pCur->dwRefCount = 0;                
            pCur->pNext      = NULL;

            break ;
        }
        else if (dwRoundSize == pCur->dwSize)
        {
            if (pPrev == pCur)  /* 头结点 */
            {
                pPrev = pNext;

                /* 更新头指针 */
                m_pFreeHeader = pPrev;
            }
            else
            {
                pPrev->pNext = pNext;
            }

            m_cLock.UnLock();

            pCur->dwRefCount = 0;                
            pCur->pNext      = NULL;

            break ;
        }
        else
        {
            pPrev = pCur;
            pCur  = pCur->pNext;
        }
    }

    if (NULL != pCur)
    {
        m_lwUsedSize += dwRealSize;

        pValue = (BYTE *)((WORD64)(pCur) + sizeof(T_MemBufHeader));
    }
    else
    {
        m_cLock.UnLock();
    }

    return pValue;
}


WORD32 CCentralMemPool::Free(BYTE *pAddr)
{
    if (!IsValid(pAddr))
    {
        assert(0);
    }

    T_MemBufHeader *pBuf = (T_MemBufHeader *)((WORD64)(pAddr) - sizeof(T_MemBufHeader));

    if ( ((pBuf->lwOffset + ((WORD64)m_pOriAddr)) != (WORD64)pBuf)
      || (INVALID_DWORD == pBuf->dwRefCount))
    {
        assert(0);
    }

    if (0 != pBuf->dwRefCount)
    {
        pBuf->dwRefCount--;
        return FAIL;
    }

    T_MemBufHeader *pPrev  = NULL;
    T_MemBufHeader *pCur   = NULL;

    WORD64 lwBegin    = 0;
    WORD64 lwEnd      = 0;
    WORD64 lwRealSize = pBuf->dwSize * CACHE_SIZE;
    WORD64 lwBufBegin = pBuf->lwOffset;
    WORD64 lwBufEnd   = lwBufBegin + lwRealSize;

    m_lwFreeSize += lwRealSize;

    m_cLock.Lock();

    if (NULL == m_pFreeHeader)  /* 链表为空, 直接挂载 */
    {
        m_pFreeHeader             = pBuf;
        m_pFreeHeader->dwRefCount = INVALID_DWORD;

        m_cLock.UnLock();
        return SUCCESS;
    }

    pPrev = m_pFreeHeader;
    pCur  = m_pFreeHeader;

    while (pCur)
    {
        if (lwBufBegin < pCur->lwOffset)    /* pBuf应挂在pPrev和pCur之间 */
        {        
            lwBegin = pCur->lwOffset;
            if (lwBegin < lwBufEnd)
            {
                assert(0);
            }

            if (pPrev == pCur)  /* 头结点 */
            {
                pPrev = pBuf;

                /* 更新头指针 */
                m_pFreeHeader = pPrev;

                pBuf->dwRefCount = INVALID_DWORD;
                pBuf->pNext      = pCur;
            }
            else
            {
                lwEnd = pPrev->lwOffset + pPrev->dwSize * CACHE_SIZE;
                if (lwEnd > lwBufBegin)
                {
                    assert(0);
                }
                else if (lwEnd == lwBufBegin)  /* pBuf需要和其前的pPrev衔接 */
                {
                    pPrev->dwSize += pBuf->dwSize;

                    /* 前移pBuf指针 */
                    pBuf = pPrev;
                }
                else
                {
                    pPrev->pNext = pBuf;

                    pBuf->dwRefCount = INVALID_DWORD;
                    pBuf->pNext      = pCur;
                }
            }

            if (lwBegin == lwBufEnd)     /* pBuf需要和其后的pCur衔接 */
            {
                pBuf->dwSize += pCur->dwSize;
                pBuf->pNext   = pCur->pNext;
            }

            break ;
        }
        else if (lwBufBegin == pCur->lwOffset)
        {
            assert(0);
        }
        else    /* 继续寻找挂载点 */
        {
            pPrev = pCur;
            pCur  = pCur->pNext;

            if (NULL == pCur)  /* 挂在末尾 */
            {
                pPrev->pNext = pBuf;

                pBuf->dwRefCount = INVALID_DWORD;
                pBuf->pNext      = NULL;

                break ;
            }
        }
    }

    m_cLock.UnLock();

    return SUCCESS;
}


CObjMemPoolInterface::CObjMemPoolInterface (CCentralMemPool &rCentralMemPool)
    : CMemInterface(),
      m_rCentralMemPool(rCentralMemPool)
{
    m_dwPowerNum = 0;
    m_dwBufNum   = 0;
    m_dwBufSize  = 0;
}


CObjMemPoolInterface::~CObjMemPoolInterface()
{
    if (NULL != m_pOriAddr)
    {
        m_rCentralMemPool.Free((BYTE *)m_pOriAddr);
    }

    m_dwPowerNum = 0;
    m_dwBufNum   = 0;
    m_dwBufSize  = 0;
}


/* dwPowerNum : 内存块数量(2^N); dwBufSize : 每个内存块大小 */
WORD32 CObjMemPoolInterface::Initialize(WORD32 dwPowerNum, WORD32 dwBufSize)
{
    m_dwPowerNum = dwPowerNum;
    m_dwBufNum   = (1 << dwPowerNum);
    m_dwBufSize  = ROUND_UP((dwBufSize + sizeof(T_MemBufHeader)), CACHE_SIZE);

    WORD32 dwSize = (m_dwBufSize * m_dwBufNum) + sizeof(T_MemBufHeader);

    /* 需要预留一个T_MemBufHeader头部节点 */
    BYTE *pBuf = m_rCentralMemPool.Malloc(dwSize);
    if (NULL == pBuf)
    {
        return FAIL;
    }

    CMemInterface::Initialize(pBuf, (WORD64)(dwSize));

    return SUCCESS;
}


VOID CObjMemPoolInterface::Dump()
{
    TRACE_STACK("CObjMemPoolInterface::Dump()");

    WORD64 lwUsedSize = m_lwUsedSize.load(std::memory_order_relaxed);;
    WORD64 lwFreeSize = m_lwFreeSize.load(std::memory_order_relaxed);;

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "m_pOriAddr = %p, m_lwAddr = %lu, m_lwSize = %lu, "
               "m_lwBegin = %lu, m_lwEnd = %lu, "
               "m_lwUsedSize = %lu, m_lwFreeSize = %lu, "
               "m_dwPowerNum = %d, m_dwBufNum = %d, m_dwBufSize = %d\n",
               m_pOriAddr,
               m_lwAddr,
               m_lwSize,
               m_lwBegin,
               m_lwEnd,
               lwUsedSize,
               lwFreeSize,
               m_dwPowerNum,
               m_dwBufNum,
               m_dwBufSize);
}


