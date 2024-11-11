

#ifndef _BASE_MEASURE_H_
#define _BASE_MEASURE_H_


#include <atomic>

#include "pub_typedef.h"
#include "pub_global_def.h"

#include "base_ring.h"

#define MSG_MEM_TYPE_NUM         ((WORD32)(8))

#define BIT_NUM_OF_WORD32        ((WORD32)(32))


#define WLS_POOL_MAX_NUM         ((WORD32)(16))
#define WLS_BLOCK_MAX_NUM        ((WORD32)(160))


#define MEASURE_RING_NUM         ((WORD32)(8))


typedef struct tagT_AtomicMemMeasure
{
    std::atomic<WORD64>  alwMallocStat[E_MP_MALLOC_POINT_NUM];
    std::atomic<WORD64>  alwFreeStat[E_MP_MALLOC_POINT_NUM];    /* ��������� */
}T_AtomicMemMeasure;


typedef struct tagT_MemMeasure
{
    WORD64  alwMallocStat[E_MP_MALLOC_POINT_NUM];
    WORD64  alwFreeStat[E_MP_MALLOC_POINT_NUM];
}T_MemMeasure;


typedef struct tagT_AtomicMsgMemMeasure
{
    std::atomic<WORD32>   dwTypeNum;
    std::atomic<WORD64>   alwTotalSize[MSG_MEM_TYPE_NUM];
    std::atomic<WORD64>   alwUsedCount[MSG_MEM_TYPE_NUM];
    std::atomic<WORD64>   alwFreeCount[MSG_MEM_TYPE_NUM];
}T_AtomicMsgMemMeasure;


typedef struct tagT_MsgMemMeasure
{
    WORD32   dwTypeNum;
    WORD64   alwTotalSize[MSG_MEM_TYPE_NUM];
    WORD64   alwUsedCount[MSG_MEM_TYPE_NUM];
    WORD64   alwFreeCount[MSG_MEM_TYPE_NUM];
}T_MsgMemMeasure;


typedef struct tagT_AtomicMsgMeasure
{
    std::atomic<WORD64>  lwMsgCount;                  /* ��Ϣ���� */
    std::atomic<WORD32>  dwQMaxMsgID;                 /* ��Ϣ��������ӳٺ�ʱ����ϢID */
    std::atomic<WORD32>  dwMsgQMaxUsed;               /* ��Ϣ��������ӳٺ�ʱ(��λ:0.1us) */
    std::atomic<WORD64>  lwMsgQTotalTime;             /* ��Ϣ�����ܺ�ʱ(��λ:0.1us) */
    std::atomic<WORD64>  alwStat[BIT_NUM_OF_WORD32];  /* ��Ϣ�����ʱ�ֲ�(��λ : 0.1us) */
}T_AtomicMsgMeasure;


typedef struct tagT_MsgMeasure
{
    WORD64  lwMsgCount;                  /* ��Ϣ���� */
    WORD32  dwQMaxMsgID;                 /* ��Ϣ��������ӳٺ�ʱ����ϢID */
    WORD32  dwMsgQMaxUsed;               /* ��Ϣ��������ӳٺ�ʱ(��λ : 0.1us) */
    WORD64  lwMsgQTotalTime;             /* ��Ϣ�����ܺ�ʱ(��λ : 0.1us) */
    WORD64  alwStat[BIT_NUM_OF_WORD32];  /* ��Ϣ�����ʱ�ֲ�(��λ : 0.1us) */
}T_MsgMeasure;


typedef struct tagT_AtomicAppMeasure
{
    WORD32               dwAppID;
    WORD32               dwAppClass;

    std::atomic<WORD64>  lwProcNum;                   /* ����ҵ����Ϣ���� */
    std::atomic<WORD32>  dwMsgID;                     /* ����ʱ��������Ӧ����ϢID */
    std::atomic<WORD64>  dwMaxTimeUsed;               /* ��������Ϣ�������ʱ(��λ : 0.1us) */

    std::atomic<WORD64>  lwTimeUsedTotal;             /* ������Ϣ����ʱ  (��λ : 0.1us) */
    std::atomic<WORD64>  alwStat[BIT_NUM_OF_WORD32];  /* ��Ϣ�����ʱ�ֲ�(��λ : 0.1us) */

    T_AtomicMsgMeasure   tMsgQStat;
}T_AtomicAppMeasure;


typedef struct tagT_AppMeasure
{
    WORD32        dwAppID;
    WORD32        dwAppClass;

    WORD64        lwProcNum;                   /* ����ҵ����Ϣ���� */
    WORD32        dwMsgID;                     /* ����ʱ��������Ӧ����ϢID */
    WORD64        dwMaxTimeUsed;               /* ��������Ϣ�������ʱ(��λ : 0.1us) */

    WORD64        lwTimeUsedTotal;             /* ������Ϣ����ʱ  (��λ : 0.1us) */
    WORD64        alwStat[BIT_NUM_OF_WORD32];  /* ��Ϣ�����ʱ�ֲ�(��λ : 0.1us) */

    T_MsgMeasure  tMsgQStat;
}T_AppMeasure;


typedef struct tagT_AtomicThreadMeasure
{
    WORD32                 dwThreadID;       /* OS�̱߳�ʶ */
    WORD16                 wThreadID;        /* �ڲ��̱߳�ʶ */
    BYTE                   ucThreadClass;    /* Polling / RT / Worker */
    BYTE                   ucLogicalID;      /* ���߼���ID */
    
    std::atomic<WORD64>    lwLoop;            /* Loop���� */
    std::atomic<WORD64>    lwProcHNum;        /* �ۼƴ�������ȼ�������Ϣ���� */
    std::atomic<WORD64>    lwProcLNum;        /* �ۼƴ�������ȼ�������Ϣ���� */
    std::atomic<WORD64>    lwProcDNum;        /* �ۼƴ������ݰ�������Ϣ���� */
    std::atomic<WORD32>    dwMaxRemainLNum;   /* �����ȼ�������Ϣ���ʣ������ */
    std::atomic<WORD32>    dwMaxRemainDNum;   /* ���ݰ�������Ϣ���ʣ������ */
    
    std::atomic<WORD64>    lwTimeUsedTotalH;  /* �����ȼ���Ϣ��������ʱ(��λ : 0.1us) */
    std::atomic<WORD64>    lwTimeUsedTotalL;  /* �����ȼ���Ϣ��������ʱ(��λ : 0.1us) */
    std::atomic<WORD64>    lwTimeUsedTotalD;  /* ���ݰ���������ʱ      (��λ : 0.1us) */
    std::atomic<WORD64>    lwTimeUsedTotalT;  /* ��ʱ����������ʱ      (��λ : 0.1us) */
    std::atomic<WORD64>    lwTimeUsedTotalA;  /* �߳�Runִ��һ������ʱ (��λ : 0.1us) */

    std::atomic<WORD64>    alwStatH[BIT_NUM_OF_WORD32];  /* �����ȼ���Ϣ���к�ʱ�ֲ�(��λ0.1us) */
    std::atomic<WORD64>    alwStatL[BIT_NUM_OF_WORD32];  /* �����ȼ���Ϣ���к�ʱ�ֲ�(��λ0.1us) */
    std::atomic<WORD64>    alwStatD[BIT_NUM_OF_WORD32];  /* ���ݰ����к�ʱ�ֲ�      (��λ0.1us) */
    std::atomic<WORD64>    alwStatT[BIT_NUM_OF_WORD32];  /* ��ʱ�����к�ʱ�ֲ�      (��λ0.1us) */
    std::atomic<WORD64>    alwStatA[BIT_NUM_OF_WORD32];  /* �߳�Runִ��һ�ֺ�ʱ�ֲ� (��λ0.1us) */
    
    T_AtomicMsgMeasure     tHPMsgQStat;       /* �����ȼ�������Ϣ�ڶ����еĵȴ�ʱ��ͳ�� */
    T_AtomicMsgMeasure     tLPMsgQStat;       /* �����ȼ�������Ϣ�ڶ����еĵȴ�ʱ��ͳ�� */
    T_AtomicMsgMeasure     tMultiMsgQStat;    /* Multi������Ϣ�ڶ����еĵȴ�ʱ��ͳ�� */

    T_AtomicMsgMemMeasure  tMsgMemStat;       /* �̵߳���Ϣ�ڴ��ͳ�� */
}T_AtomicThreadMeasure;


typedef struct tagT_ThreadMeasure
{
    WORD32           dwThreadID;       /* OS�̱߳�ʶ */
    WORD16           wThreadID;        /* �ڲ��̱߳�ʶ */
    BYTE             ucThreadClass;    /* Polling / RT / Worker */
    BYTE             ucLogicalID;      /* ���߼���ID */
    
    WORD64           lwLoop;            /* Loop���� */
    WORD64           lwProcHNum;        /* �ۼƴ�������ȼ�������Ϣ���� */
    WORD64           lwProcLNum;        /* �ۼƴ�������ȼ�������Ϣ���� */
    WORD64           lwProcDNum;        /* �ۼƴ������ݰ�������Ϣ���� */
    WORD32           dwMaxRemainLNum;   /* �����ȼ�������Ϣ���ʣ������ */
    WORD32           dwMaxRemainDNum;   /* ���ݰ�������Ϣ���ʣ������ */
    
    WORD64           lwTimeUsedTotalH;  /* �����ȼ���Ϣ��������ʱ(��λ : 0.1us) */
    WORD64           lwTimeUsedTotalL;  /* �����ȼ���Ϣ��������ʱ(��λ : 0.1us) */
    WORD64           lwTimeUsedTotalD;  /* ���ݰ���������ʱ      (��λ : 0.1us) */
    WORD64           lwTimeUsedTotalT;  /* ��ʱ����������ʱ      (��λ : 0.1us) */
    WORD64           lwTimeUsedTotalA;  /* �߳�Runִ��һ������ʱ (��λ : 0.1us) */

    WORD64           alwStatH[BIT_NUM_OF_WORD32];  /* �����ȼ���Ϣ���к�ʱ�ֲ�(��λ0.1us) */
    WORD64           alwStatL[BIT_NUM_OF_WORD32];  /* �����ȼ���Ϣ���к�ʱ�ֲ�(��λ0.1us) */
    WORD64           alwStatD[BIT_NUM_OF_WORD32];  /* ���ݰ����к�ʱ�ֲ�      (��λ0.1us) */
    WORD64           alwStatT[BIT_NUM_OF_WORD32];  /* ��ʱ�����к�ʱ�ֲ�      (��λ0.1us) */
    WORD64           alwStatA[BIT_NUM_OF_WORD32];  /* �߳�Runִ��һ�ֺ�ʱ�ֲ� (��λ0.1us) */

    T_MsgMeasure     tHPMsgQStat;       /* �����ȼ�������Ϣ�ڶ����еĵȴ�ʱ��ͳ�� */
    T_MsgMeasure     tLPMsgQStat;       /* �����ȼ�������Ϣ�ڶ����еĵȴ�ʱ��ͳ�� */
    T_MsgMeasure     tMultiMsgQStat;    /* Multi������Ϣ�ڶ����еĵȴ�ʱ��ͳ�� */

    T_MsgMemMeasure  tMsgMemStat;       /* �̵߳���Ϣ�ڴ��ͳ�� */
}T_ThreadMeasure;


typedef struct tagT_ThreadMsgRingMeasure
{
    T_RingHeadTail  tProd;
    T_RingHeadTail  tCons;
}T_ThreadMsgRingMeasure;


typedef struct tagT_ThreadPacketRingMeasure
{
    WORD32          dwRingNum;
    T_RingHeadTail  atProd[MEASURE_RING_NUM];
    T_RingHeadTail  atCons[MEASURE_RING_NUM];
}T_ThreadPacketRingMeasure;


typedef struct tagT_ThreadRingMeasure
{
    T_ThreadMsgRingMeasure     tRingH;  /* �����ȼ���Ϣ���п��� */
    T_ThreadMsgRingMeasure     tRingL;  /* �����ȼ���Ϣ���п��� */
    T_ThreadPacketRingMeasure  tRingD;  /* ���ݰ����п��� */
}T_ThreadRingMeasure;


#define SLOT_NUM_PER_SFN    ((WORD32)(20))

#define MAX_DIFF_SLOT_SCH2RLC        ((BYTE)(4))
#define SLOT_NUM_PER_HALF_SFN        ((BYTE)(10))


typedef struct tagT_ClUciMeasureItem
{
    WORD64  lwUciNum;
    WORD64  lwHQNum;

    WORD32  adwRecvUci8Stat[SLOT_NUM_PER_SFN];  /* �ڿտڶ�Ӧ��ĳ��slot���յ�8��UCI */
    WORD32  adwRecvUci9Stat[SLOT_NUM_PER_SFN];  /* �ڿտڶ�Ӧ��ĳ��slot���յ�9��UCI */
    WORD32  adwRecvUci18Stat[SLOT_NUM_PER_SFN]; /* �ڿտڶ�Ӧ��ĳ��slot���յ�18��UCI */
    WORD32  adwRecvUci19Stat[SLOT_NUM_PER_SFN]; /* �ڿտڶ�Ӧ��ĳ��slot���յ�19��UCI */

    WORD32  adwProcUci8Stat[SLOT_NUM_PER_SFN];  /* �ڿտڶ�Ӧ��ĳ��slot�ϴ���8��UCI */
    WORD32  adwProcUci9Stat[SLOT_NUM_PER_SFN];  /* �ڿտڶ�Ӧ��ĳ��slot�ϴ���9��UCI */
    WORD32  adwProcUci18Stat[SLOT_NUM_PER_SFN]; /* �ڿտڶ�Ӧ��ĳ��slot�ϴ���18��UCI */
    WORD32  adwProcUci19Stat[SLOT_NUM_PER_SFN]; /* �ڿտڶ�Ӧ��ĳ��slot�ϴ���19��UCI */

    WORD64  lwTimeUsed8Total;    /* ����slot-8��UCI����ʱͳ��(��λ : 0.1us) */
    WORD64  lwTimeUsed9Total;    /* ����slot-9��UCI����ʱͳ��(��λ : 0.1us) */
    WORD64  lwTimeUsed18Total;   /* ����slot-18��UCI����ʱͳ��(��λ : 0.1us) */
    WORD64  lwTimeUsed19Total;   /* ����slot-19��UCI����ʱͳ��(��λ : 0.1us) */

    WORD32  adwStatSlot8[BIT_NUM_OF_WORD32];    /* slot8��UCI��ʱ�ֲ�(��λ : 0.1us) */
    WORD32  adwStatSlot9[BIT_NUM_OF_WORD32];    /* slot9��UCI��ʱ�ֲ�(��λ : 0.1us) */
    WORD32  adwStatSlot18[BIT_NUM_OF_WORD32];   /* slot18��UCI��ʱ�ֲ�(��λ : 0.1us) */
    WORD32  adwStatSlot19[BIT_NUM_OF_WORD32];   /* slot19��UCI��ʱ�ֲ�(��λ : 0.1us) */
}T_ClUciMeasureItem;


typedef struct tagT_ClUciAtomicMeasureItem
{
    std::atomic<WORD64>  lwUciNum;
    std::atomic<WORD64>  lwHQNum;

    std::atomic<WORD32>  adwRecvUci8Stat[SLOT_NUM_PER_SFN];  /* �ڿտڶ�Ӧ��ĳ��slot���յ�8��UCI */
    std::atomic<WORD32>  adwRecvUci9Stat[SLOT_NUM_PER_SFN];  /* �ڿտڶ�Ӧ��ĳ��slot���յ�9��UCI */
    std::atomic<WORD32>  adwRecvUci18Stat[SLOT_NUM_PER_SFN]; /* �ڿտڶ�Ӧ��ĳ��slot���յ�18��UCI */
    std::atomic<WORD32>  adwRecvUci19Stat[SLOT_NUM_PER_SFN]; /* �ڿտڶ�Ӧ��ĳ��slot���յ�19��UCI */

    std::atomic<WORD32>  adwProcUci8Stat[SLOT_NUM_PER_SFN];  /* �ڿտڶ�Ӧ��ĳ��slot�ϴ���8��UCI */
    std::atomic<WORD32>  adwProcUci9Stat[SLOT_NUM_PER_SFN];  /* �ڿտڶ�Ӧ��ĳ��slot�ϴ���9��UCI */
    std::atomic<WORD32>  adwProcUci18Stat[SLOT_NUM_PER_SFN]; /* �ڿտڶ�Ӧ��ĳ��slot�ϴ���18��UCI */
    std::atomic<WORD32>  adwProcUci19Stat[SLOT_NUM_PER_SFN]; /* �ڿտڶ�Ӧ��ĳ��slot�ϴ���19��UCI */

    std::atomic<WORD64>  lwTimeUsed8Total;    /* ����slot-8��UCI����ʱͳ��(��λ : 0.1us) */
    std::atomic<WORD64>  lwTimeUsed9Total;    /* ����slot-9��UCI����ʱͳ��(��λ : 0.1us) */
    std::atomic<WORD64>  lwTimeUsed18Total;   /* ����slot-18��UCI����ʱͳ��(��λ : 0.1us) */
    std::atomic<WORD64>  lwTimeUsed19Total;   /* ����slot-19��UCI����ʱͳ��(��λ : 0.1us) */

    std::atomic<WORD32>  adwStatSlot8[BIT_NUM_OF_WORD32];    /* slot8��UCI��ʱ�ֲ�(��λ : 0.1us) */
    std::atomic<WORD32>  adwStatSlot9[BIT_NUM_OF_WORD32];    /* slot9��UCI��ʱ�ֲ�(��λ : 0.1us) */
    std::atomic<WORD32>  adwStatSlot18[BIT_NUM_OF_WORD32];   /* slot18��UCI��ʱ�ֲ�(��λ : 0.1us) */
    std::atomic<WORD32>  adwStatSlot19[BIT_NUM_OF_WORD32];   /* slot19��UCI��ʱ�ֲ�(��λ : 0.1us) */
}T_ClUciAtomicMeasureItem;


typedef struct tagT_ClUciAtomicMeasure
{
    T_ClUciAtomicMeasureItem  atCellMeasure[MAX_CELL_PER_GNB];
}T_ClUciAtomicMeasure;


typedef struct tagT_ClDlPhyMeasureItem
{
    WORD32  adwSchedPhyTmUsed[BIT_NUM_OF_WORD32];  /* �ڿտڶ�Ӧ��ĳ��slot�ϴ���slot-x��API��ʱ�ֲ� */
}T_ClDlPhyMeasureItem;


typedef struct tagT_ClDlPhyAtomicMeasureItem
{
    std::atomic<WORD32>  adwSchedPhyTmUsed[BIT_NUM_OF_WORD32];  /* �ڿտڶ�Ӧ��ĳ��slot�ϴ���slot-x��API��ʱ�ֲ� */
}T_ClDlPhyAtomicMeasureItem;


typedef struct tagT_ClDlPhyCellMeasure
{
    T_ClDlPhyMeasureItem  atPhyMeasure[SLOT_NUM_PER_SFN];
}T_ClDlPhyCellMeasure;


typedef struct tagT_ClDlPhyCellAtomicMeasure
{
    T_ClDlPhyAtomicMeasureItem  atPhyMeasure[SLOT_NUM_PER_SFN];
}T_ClDlPhyCellAtomicMeasure;


typedef struct tagT_ClDlMeasure
{
    WORD64  lwDlNum;

    /* CL process(triggered by Slot Ind) = fill Dl PHY API + Lvl2 process */
    WORD64  lwTimeUsedTotal;  /* CL���д�������ʱ(���� : ����Lv2 + DLAPI + DL���; ��λ:0.1us)*/
    WORD64  lwTimeUsedLv2;    /* CL���е���Lv2����ʱ */
    WORD64  lwTimeUsedPhy;    /* CL������API+�������ʱ */

    WORD32  adwStatTotal[BIT_NUM_OF_WORD32];  /* CL���д����ʱ�ֲ�(��λ:0.1us) */
    WORD32  adwStatLv2[BIT_NUM_OF_WORD32];    /* CL���е���Lv2��ʱ�ֲ�(��λ:0.1us) */
    WORD32  adwStatPhy[BIT_NUM_OF_WORD32];    /* CL������API+�����ʱ�ֲ�(��λ:0.1us) */

    T_ClDlPhyCellMeasure  atCellMeasure[MAX_CELL_PER_GNB];
}T_ClDlMeasure;


typedef struct tagT_ClDlAtomicMeasure
{
    std::atomic<WORD64>  lwDlNum;

    std::atomic<WORD64>  lwTimeUsedTotal;  /* CL���д�������ʱ(���� : ����Lv2 + DLAPI + DL���; ��λ:0.1us)*/
    std::atomic<WORD64>  lwTimeUsedLv2;    /* CL���е���Lv2����ʱ */
    std::atomic<WORD64>  lwTimeUsedPhy;    /* CL������API+�������ʱ */

    std::atomic<WORD32>  adwStatTotal[BIT_NUM_OF_WORD32];  /* CL���д����ʱ�ֲ�(��λ:0.1us) */
    std::atomic<WORD32>  adwStatLv2[BIT_NUM_OF_WORD32];    /* CL���е���Lv2��ʱ�ֲ�(��λ:0.1us) */
    std::atomic<WORD32>  adwStatPhy[BIT_NUM_OF_WORD32];    /* CL������API+�����ʱ�ֲ�(��λ:0.1us) */

    T_ClDlPhyCellAtomicMeasure  atCellMeasure[MAX_CELL_PER_GNB];
}T_ClDlAtomicMeasure;


typedef struct tagT_Lvl1CellMeasureItem
{
    WORD32  adwStartIndStat[BIT_NUM_OF_WORD32];
    WORD32  adwLvl1ProcStat[BIT_NUM_OF_WORD32];
}T_Lvl1CellMeasureItem;


typedef struct tagT_Lvl1CellAtomicMeasureItem
{
    std::atomic<WORD32>  adwStartIndStat[BIT_NUM_OF_WORD32];
    std::atomic<WORD32>  adwLvl1ProcStat[BIT_NUM_OF_WORD32];
}T_Lvl1CellAtomicMeasureItem;


typedef struct tagT_Lvl1CellMeasure
{
    WORD64  lwStartIndNum;
    WORD64  lwStartIndWaitTotal;
    WORD64  lwLvl1ProcTotal;

    T_Lvl1CellMeasureItem  atLvl1MeasureItem[SLOT_NUM_PER_SFN];
}T_Lvl1CellMeasure;


typedef struct tagT_Lvl1CellAtomicMeasure
{
    std::atomic<WORD64>  lwStartIndNum;
    std::atomic<WORD64>  lwStartIndWaitTotal;
    std::atomic<WORD64>  lwLvl1ProcTotal;

    T_Lvl1CellAtomicMeasureItem  atLvl1MeasureItem[SLOT_NUM_PER_SFN];
}T_Lvl1CellAtomicMeasure;


typedef struct tagT_Lvl1Measure
{
    T_Lvl1CellMeasure  atCellMeasure[MAX_CELL_PER_GNB];
}T_Lvl1Measure;


typedef struct tagT_Lvl1AtomicMeasure
{
    T_Lvl1CellAtomicMeasure  atCellMeasure[MAX_CELL_PER_GNB];
}T_Lvl1AtomicMeasure;


typedef struct tagT_ClMsgWaitCellMeasure
{
    WORD64  lwUlSchNum;
    WORD64  lwCrcIndNum;
    WORD64  lwRachIndNum;
    WORD64  lwSrsIndNum;

    WORD64  lwUlSchWaitTotal;
    WORD64  lwCrcIndWaitTotal;
    WORD64  lwRachIndWaitTotal;
    WORD64  lwSrsIndWaitTotal;

    WORD32  adwUlSchStat[BIT_NUM_OF_WORD32];
    WORD32  adwCrcIndStat[BIT_NUM_OF_WORD32];
    WORD32  adwRachIndStat[BIT_NUM_OF_WORD32];
    WORD32  adwSrsIndStat[BIT_NUM_OF_WORD32];
}T_ClMsgWaitCellMeasure;


typedef struct tagT_ClMsgWaitCellAtomicMeasure
{
    std::atomic<WORD64>  lwUlSchNum;
    std::atomic<WORD64>  lwCrcIndNum;
    std::atomic<WORD64>  lwRachIndNum;
    std::atomic<WORD64>  lwSrsIndNum;

    std::atomic<WORD64>  lwUlSchWaitTotal;
    std::atomic<WORD64>  lwCrcIndWaitTotal;
    std::atomic<WORD64>  lwRachIndWaitTotal;
    std::atomic<WORD64>  lwSrsIndWaitTotal;

    std::atomic<WORD32>  adwUlSchStat[BIT_NUM_OF_WORD32];
    std::atomic<WORD32>  adwCrcIndStat[BIT_NUM_OF_WORD32];
    std::atomic<WORD32>  adwRachIndStat[BIT_NUM_OF_WORD32];
    std::atomic<WORD32>  adwSrsIndStat[BIT_NUM_OF_WORD32];
}T_ClMsgWaitCellAtomicMeasure;


typedef struct tagT_ClMsgWaitMeasure
{
    T_ClMsgWaitCellMeasure  atCellMeasure[MAX_CELL_PER_GNB];
}T_ClMsgWaitMeasure;


typedef struct tagT_ClMsgWaitAtomicMeasure
{
    T_ClMsgWaitCellAtomicMeasure  atCellMeasure[MAX_CELL_PER_GNB];
}T_ClMsgWaitAtomicMeasure;


typedef struct tagT_PhyRecvCellMeasItem
{
    /* SCH���Ƚ����Чͳ�� */
    WORD64  lwSchInvalidPdcchNum;
    WORD64  lwSchInvalidPdschNum;

    /* PDCCHͳ�� */
    WORD64  lwDlActvUeNum;
    WORD64  lwUlPdcchNum;
    WORD64  lwDlPdcchNum;
    WORD64  lwCsiRsNum;

    /* PDSCHͳ�� */
    WORD64  lwBchNum;
    WORD64  lwPduNum;
    WORD64  lwSibNum;
    WORD64  lwPagingNum;
    WORD64  lwMsg2Num;
    WORD64  lwMsg4Num;
    WORD64  lwTBNum;
    WORD64  lwTBSize;

    /* PUSCHͳ�� */
    WORD64  lwPuschNum;
    WORD64  lwPucchFrmt1Num;
    WORD64  lwHqCsiSrNum;
    WORD64  lwCsiSrNum;
    WORD64  lwHqCsiNum;
    WORD64  lwHqSrNum;
    WORD64  lwHarqNum;
    WORD64  lwCsiNum;
    WORD64  lwSrNum;
    WORD64  lwSrsNum;
    WORD64  lwRachNum;
}T_PhyRecvCellMeasItem;


typedef struct tagT_PhyRecvCellAtomicMeasItem
{
    /* SCH���Ƚ����Чͳ�� */
    std::atomic<WORD64>  lwSchInvalidPdcchNum;
    std::atomic<WORD64>  lwSchInvalidPdschNum;

    /* PDCCHͳ�� */
    std::atomic<WORD64>  lwDlActvUeNum;
    std::atomic<WORD64>  lwUlPdcchNum;
    std::atomic<WORD64>  lwDlPdcchNum;
    std::atomic<WORD64>  lwCsiRsNum;

    /* PDSCHͳ�� */
    std::atomic<WORD64>  lwBchNum;
    std::atomic<WORD64>  lwPduNum;
    std::atomic<WORD64>  lwSibNum;
    std::atomic<WORD64>  lwPagingNum;
    std::atomic<WORD64>  lwMsg2Num;
    std::atomic<WORD64>  lwMsg4Num;
    std::atomic<WORD64>  lwTBNum;
    std::atomic<WORD64>  lwTBSize;

    /* PUSCHͳ�� */
    std::atomic<WORD64>  lwPuschNum;
    std::atomic<WORD64>  lwPucchFrmt1Num;
    std::atomic<WORD64>  lwHqCsiSrNum;
    std::atomic<WORD64>  lwCsiSrNum;
    std::atomic<WORD64>  lwHqCsiNum;
    std::atomic<WORD64>  lwHqSrNum;
    std::atomic<WORD64>  lwHarqNum;
    std::atomic<WORD64>  lwCsiNum;
    std::atomic<WORD64>  lwSrNum;
    std::atomic<WORD64>  lwSrsNum;
    std::atomic<WORD64>  lwRachNum;
}T_PhyRecvCellAtomicMeasItem;


typedef struct tagT_PhyRecvCellMeasure
{
    WORD64                 lwSlotIndNum;
    WORD64                 lwTotalTimeUsed;                      /* ���յ���һ��TTI��ʼ��ʱ(��λ:0.1us) */
    WORD64                 alwJitter[BIT_NUM_OF_WORD32];         /* Slot����ͳ��(��λ:0.1us) */
    T_PhyRecvCellMeasItem  atMeas[SLOT_NUM_PER_HALF_SFN];        /* ��API���ά�� */
}T_PhyRecvCellMeasure;


typedef struct tagT_PhyRecvCellAtomicMeasure
{
    std::atomic<WORD64>          lwSlotIndNum;
    std::atomic<WORD64>          lwTotalTimeUsed;                /* ���յ���һ��TTI��ʼ��ʱ(��λ:0.1us) */
    std::atomic<WORD64>          alwJitter[BIT_NUM_OF_WORD32];   /* Slot����ͳ��(��λ:0.1us) */
    T_PhyRecvCellAtomicMeasItem  atMeas[SLOT_NUM_PER_HALF_SFN];  /* ��API���ά�� */
}T_PhyRecvCellAtomicMeasure;


typedef struct tagT_PhyRecvMeasure
{
    T_PhyRecvCellMeasure  atCell[MAX_CELL_PER_GNB];
}T_PhyRecvMeasure;


typedef struct tagT_PhyRecvAtomicMeasure
{
    T_PhyRecvCellAtomicMeasure  atCell[MAX_CELL_PER_GNB];
}T_PhyRecvAtomicMeasure;


typedef struct tagT_UlRecvCellMeasItem
{
    WORD64  lwUciRcvNum;
    WORD64  lwCrcRcvNum;
    WORD64  lwCrcSuccNum;
    WORD64  lwCrcFailNum;
    WORD64  lwSrsRcvNum;
    WORD64  lwRachRcvNum;
    WORD64  alwUeUciSlot[SLOT_NUM_PER_HALF_SFN];
    WORD64  alwUeCrcSlot[SLOT_NUM_PER_HALF_SFN];
    WORD64  alwUeSrsSlot[SLOT_NUM_PER_HALF_SFN];
    WORD64  alwUeRachSlot[SLOT_NUM_PER_HALF_SFN];
}T_UlRecvCellMeasItem;


typedef struct tagT_UlRecvCellAtomicMeasItem
{
    std::atomic<WORD64>  lwUciRcvNum;
    std::atomic<WORD64>  lwCrcRcvNum;
    std::atomic<WORD64>  lwCrcSuccNum;
    std::atomic<WORD64>  lwCrcFailNum;
    std::atomic<WORD64>  lwSrsRcvNum;
    std::atomic<WORD64>  lwRachRcvNum;
    std::atomic<WORD64>  alwUeUciSlot[SLOT_NUM_PER_HALF_SFN];
    std::atomic<WORD64>  alwUeCrcSlot[SLOT_NUM_PER_HALF_SFN];
    std::atomic<WORD64>  alwUeSrsSlot[SLOT_NUM_PER_HALF_SFN];
    std::atomic<WORD64>  alwUeRachSlot[SLOT_NUM_PER_HALF_SFN];
}T_UlRecvCellAtomicMeasItem;


typedef struct tagT_UlRecvCellMeas
{
    T_UlRecvCellMeasItem  atMeas[SLOT_NUM_PER_HALF_SFN];
}T_UlRecvCellMeas;


typedef struct tagT_UlRecvCellAtomicMeas
{
    T_UlRecvCellAtomicMeasItem  atMeas[SLOT_NUM_PER_HALF_SFN];
}T_UlRecvCellAtomicMeas;


typedef struct tagT_UlRecvMeasure
{
    T_UlRecvCellMeas  atCell[MAX_CELL_PER_GNB];
}T_UlRecvMeasure;


typedef struct tagT_UlRecvAtomicMeasure
{
    T_UlRecvCellAtomicMeas  atCell[MAX_CELL_PER_GNB];
}T_UlRecvAtomicMeasure;


typedef struct tagT_SchAppCellMeasItem
{
    WORD64  lwSlotIndNum;
    WORD64  lwInvalidNum;
    WORD64  lwUciIndNum;
    WORD64  lwCrcIndNum;
    WORD64  lwSrsIndNum;
    WORD64  alwDiffTtiNum[MAX_DIFF_SLOT_SCH2RLC];  /* SCH����TTI��Ϣ��ʱͳ�� */
    WORD64  alwDiffUciNum[SLOT_NUM_PER_HALF_SFN];  /* SCH����UCI��Ϣ��ʱͳ�� */
    WORD64  alwDiffCrcNum[SLOT_NUM_PER_HALF_SFN];  /* SCH����CRC��Ϣ��ʱͳ�� */
    WORD64  alwDiffSrsNum[SLOT_NUM_PER_HALF_SFN];  /* SCH����SRS��Ϣ��ʱͳ�� */
}T_SchAppCellMeasItem;


typedef struct tagT_SchAppCellAtomicMeasItem
{
    std::atomic<WORD64>  lwSlotIndNum;
    std::atomic<WORD64>  lwInvalidNum;
    std::atomic<WORD64>  lwUciIndNum;
    std::atomic<WORD64>  lwCrcIndNum;
    std::atomic<WORD64>  lwSrsIndNum;
    std::atomic<WORD64>  alwDiffTtiNum[MAX_DIFF_SLOT_SCH2RLC];  /* SCH����TTI��Ϣ��ʱͳ�� */
    std::atomic<WORD64>  alwDiffUciNum[SLOT_NUM_PER_HALF_SFN];  /* SCH����UCI��Ϣ��ʱͳ�� */
    std::atomic<WORD64>  alwDiffCrcNum[SLOT_NUM_PER_HALF_SFN];  /* SCH����CRC��Ϣ��ʱͳ�� */
    std::atomic<WORD64>  alwDiffSrsNum[SLOT_NUM_PER_HALF_SFN];  /* SCH����SRS��Ϣ��ʱͳ�� */
}T_SchAppCellAtomicMeasItem;


typedef struct tagT_SchAppCellMeasure
{
    WORD64                lwCcchReqNum;
    T_SchAppCellMeasItem  atMeas[SLOT_NUM_PER_HALF_SFN];
}T_SchAppCellMeasure;


typedef struct tagT_SchAppCellAtomicMeasure
{
    std::atomic<WORD64>         lwCcchReqNum;
    T_SchAppCellAtomicMeasItem  atMeas[SLOT_NUM_PER_HALF_SFN];
}T_SchAppCellAtomicMeasure;


typedef struct tagT_SchAppMeasure
{
    T_SchAppCellMeasure  atCell[MAX_CELL_PER_GNB];
}T_SchAppMeasure;


typedef struct tagT_SchAppAtomicMeasure
{
    T_SchAppCellAtomicMeasure  atCell[MAX_CELL_PER_GNB];
}T_SchAppAtomicMeasure;


typedef struct tagT_ClAppMeasItem
{
    WORD32  dwDlCfgMsgLen;
    WORD32  dwTxReqMsgLen;
    WORD32  dwUlDciMsgLen;
    WORD32  dwUlCfgMsgLen;

    WORD64  lwBchNum;
    WORD64  lwDlSchNum;
    WORD64  lwDlDciNum;
    WORD64  lwCsiRsNum;

    WORD64  lwTxPduNum;

    WORD64  lwUlDciPduNum;

    WORD64  lwPucchNum;
    WORD64  lwPuschNum;
    WORD64  lwSrsNum;
    WORD64  lwRachNum;
}T_ClAppMeasItem;


typedef struct tagT_ClAppAtomicMeasItem
{
    std::atomic<WORD32>  dwDlCfgMsgLen;
    std::atomic<WORD32>  dwTxReqMsgLen;
    std::atomic<WORD32>  dwUlDciMsgLen;
    std::atomic<WORD32>  dwUlCfgMsgLen;

    std::atomic<WORD64>  lwBchNum;
    std::atomic<WORD64>  lwDlSchNum;
    std::atomic<WORD64>  lwDlDciNum;
    std::atomic<WORD64>  lwCsiRsNum;

    std::atomic<WORD64>  lwTxPduNum;

    std::atomic<WORD64>  lwUlDciPduNum;

    std::atomic<WORD64>  lwPucchNum;
    std::atomic<WORD64>  lwPuschNum;
    std::atomic<WORD64>  lwSrsNum;
    std::atomic<WORD64>  lwRachNum;
}T_ClAppAtomicMeasItem;


typedef struct tagT_ClAppDlFapiCellMeasure
{
    WORD64           lwSlotIndNum;
    WORD64           lwDiscontinueNum;
    WORD64           lwTotalTimeUsed;               /* ���յ���һ��TTI��ʼ��ʱ(��λ:0.1us) */
    WORD64           alwJitter[BIT_NUM_OF_WORD32];  /* Slot����ͳ��(��λ:0.1us) */

    T_ClAppMeasItem  atMeas[SLOT_NUM_PER_HALF_SFN];
}T_ClAppDlFapiCellMeasure;


typedef struct tagT_ClAppDlFapiCellAtomicMeasure
{
    std::atomic<WORD64>    lwSlotIndNum;
    std::atomic<WORD64>    lwDiscontinueNum;
    std::atomic<WORD64>    lwTotalTimeUsed;               /* ���յ���һ��TTI��ʼ��ʱ(��λ:0.1us) */
    std::atomic<WORD64>    alwJitter[BIT_NUM_OF_WORD32];  /* Slot����ͳ��(��λ:0.1us) */

    T_ClAppAtomicMeasItem  atMeas[SLOT_NUM_PER_HALF_SFN];
}T_ClAppDlFapiCellAtomicMeasure;


typedef struct tagT_ClAppDlFapiMeasure
{
    T_ClAppDlFapiCellMeasure  atCell[MAX_CELL_PER_GNB];
}T_ClAppDlFapiMeasure;


typedef struct tagT_ClAppDlFapiAtomicMeasure
{
    T_ClAppDlFapiCellAtomicMeasure  atCell[MAX_CELL_PER_GNB];
}T_ClAppDlFapiAtomicMeasure;


#endif


