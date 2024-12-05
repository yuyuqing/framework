

#include "base_shm_hdl.h"


CShmHandler::CShmHandler ()
{
    m_eRole       = E_SHM_OBSERVER;
    m_dwNodeNum   = 0;
    m_dwNodeSize  = 0;
    m_ptHdlHead   = NULL;
    m_pOriMemAddr = NULL;
    m_lwMemSize   = 0;
    m_lwMemBegin  = 0;
    m_lwMemEnd    = 0;
}


CShmHandler::~CShmHandler()
{
    m_eRole       = E_SHM_OBSERVER;
    m_dwNodeNum   = 0;
    m_dwNodeSize  = 0;
    m_ptHdlHead   = NULL;
    m_pOriMemAddr = NULL;
    m_lwMemSize   = 0;
    m_lwMemBegin  = 0;
    m_lwMemEnd    = 0;
}


WORD32 CShmHandler::Initialize(E_ShmRole     eRole,
                               T_ShmHdlHead *ptHead,
                               BYTE         *pOriMemAddr,
                               WORD64        lwMemSize,
                               WORD32        dwNodeNum,
                               WORD32        dwNodeSize)
{
    assert(dwNodeNum < SHM_MEM_NUM);

    m_eRole       = eRole;
    m_dwNodeNum   = dwNodeNum;
    m_dwNodeSize  = dwNodeSize;
    m_ptHdlHead   = ptHead;
    m_pOriMemAddr = pOriMemAddr;
    m_lwMemSize   = lwMemSize;
    m_lwMemBegin  = (WORD64)pOriMemAddr;
    m_lwMemEnd    = m_lwMemBegin + lwMemSize;

    if (E_SHM_MASTER == eRole)
    {
        ptHead->bInitFlag      = FALSE;
        __atomic_thread_fence(__ATOMIC_ACQUIRE);

        ptHead->bStatus        = FALSE;
        ptHead->lwShmAddr      = m_lwMemBegin;
        ptHead->lwShmSize      = lwMemSize;
        ptHead->dwMemNum       = dwNodeNum;
        ptHead->dwMemSize      = dwNodeSize;
        ptHead->dwPowerNumM    = SHM_POWER_NUM_M;
        ptHead->dwPowerNumQ    = SHM_POWER_NUM_Q;
        ptHead->dwNodeNumM     = SHM_MEM_NUM;
        ptHead->dwNodeNumQ     = SHM_QUEUE_NUM;
        ptHead->dwCapacityM    = SHM_MEM_NUM - 1;
        ptHead->dwCapacityQ    = SHM_QUEUE_NUM - 1;
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
        ptHead->lwResved       = 0;

        memset((VOID *)(ptHead->alwStatM),       0x00, (BIT_NUM_OF_WORD32 * sizeof(WORD64)));
        memset((VOID *)(ptHead->alwStatQ),       0x00, (BIT_NUM_OF_WORD32 * sizeof(WORD64)));
        memset((VOID *)(ptHead->alwMallocPoint), 0x00, (E_SHM_MALLOC_POINT_NUM * sizeof(WORD64)));
        memset((VOID *)(ptHead->alwFreePoint),   0x00, (E_SHM_MALLOC_POINT_NUM * sizeof(WORD64)));
        memset((VOID *)(ptHead->alwResved),      0x00, (16 * sizeof(WORD64)));
        memset((VOID *)(ptHead->alwNodesM),      0x00, (SHM_MEM_NUM * sizeof(WORD64)));
        memset((VOID *)(ptHead->aucResved),      0x00, QUARTER_PAGE_SIZE);
        memset((VOID *)(ptHead->alwNodesQ),      0x00, (SHM_QUEUE_NUM * sizeof(WORD64)));

        T_ShmNodeHead *ptNode     = NULL;
        WORD64         lwNodeAddr = m_lwMemBegin;

        for (WORD32 dwIndex = 0; dwIndex < dwNodeNum; dwIndex++)
        {
            ptNode = (T_ShmNodeHead *)lwNodeAddr;

            ptNode->dwStartTag   = s_dwStartTag;
            ptNode->dwIndex      = dwIndex;
            ptNode->dwRefCount   = 0;
            ptNode->dwPoint      = INVALID_DWORD;
            ptNode->dwLen        = 0;
            ptNode->dwTimeOffset = 0;
            ptNode->lwClock      = 0;

            EnqueueM(*ptHead, lwNodeAddr);

            lwNodeAddr += dwNodeSize;
        }

        __atomic_thread_fence(__ATOMIC_ACQUIRE);
        ptHead->bInitFlag = TRUE;
    }
    else
    {
        if (FALSE == ptHead->bInitFlag)
        {
            return FAIL;
        }
    }

    return SUCCESS;
}


