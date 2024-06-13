

#ifndef _BASE_TIME_H_
#define _BASE_TIME_H_


#ifdef ARCH_ARM64    
#else
#include <emmintrin.h>
#endif


#include <sys/time.h>
#include <chrono>
#include <atomic>

#include "base_singleton_tpl.h"
#include "base_util.h"
#include "base_data_container.h"


/* 年-月-日 时:分:秒:微秒 */
typedef struct tagT_BaseSystemTime
{
    WORD16   wYear;    /* 年 */
    BYTE     ucMon;    /* 月 */
    BYTE     ucMDay;   /* 日(按月) */
    BYTE     ucWDay;   /* 日(按周) */
    BYTE     ucHour;   /* 时 */
    BYTE     ucMin;    /* 分 */
    BYTE     ucSec;    /* 秒 */
    WORD32   dwMicro;  /* 微秒 */
}T_BaseSystemTime;


typedef enum tagE_WeekDayType
{
    E_WEEK_SUNDAY  = 0,  /* 周日 */
    E_WEEK_MONDAY,       /* 周一 */
    E_WEEK_TUEDAY,       /* 周二 */
    E_WEEK_WEDDAY,       /* 周三 */
    E_WEEK_THUDAY,       /* 周四 */
    E_WEEK_FRIDAY,       /* 周五 */
    E_WEEK_SATDAY,       /* 周六 */
    E_WEEK_MAX,
}E_WeekDayType;


typedef enum tagE_MonthType
{
    E_MONTH_JAN = 0,   /*  1月 */
    E_MONTH_FEB,       /*  2月 */
    E_MONTH_MARCH,     /*  3月 */
    E_MONTH_APRIL,     /*  4月 */
    E_MONTH_MAY,       /*  5月 */
    E_MONTH_JUNE,      /*  6月 */
    E_MONTH_JULY,      /*  7月 */
    E_MONTH_AUGUST,    /*  8月 */
    E_MONTH_SEP,       /*  9月 */
    E_MONTH_OCT,       /* 10月 */
    E_MONTH_NOV,       /* 11月 */
    E_MONTH_DEC,       /* 12月 */
    E_MONTH_MAX = 12,
}E_MonthType;


#define BaseTimeYear   ((WORD16)(1900))


class CHighClock
{
public :
    CHighClock ();
    virtual ~CHighClock ();

    WORD64 Second();

    WORD64 MillSecond();

    WORD64 MicroSecond();

    WORD64 NanoSecond();

    VOID Reset();

    SWORD64 ElapseHour();
    SWORD64 ElapseMinute();
    SWORD64 ElapseSecond();
    SWORD64 ElapseMilli();
    SWORD64 ElapseMicro();
    SWORD64 ElapseNano();

    std::chrono::time_point<std::chrono::high_resolution_clock> m_tTime;;
};


class CBaseSystemTime
{
public :
    CBaseSystemTime ();
    CBaseSystemTime (CHighClock &rCurClock);
    
    virtual ~CBaseSystemTime ();

    CBaseSystemTime & operator=(WORD64 lwCurMicroSec);

    T_BaseSystemTime  m_tTime;
};


class CGlobalClock : public CSingleton<CGlobalClock>, public CBaseData
{
public :
    enum { MIN_CLOCK_CALC_WINDOW    = 0x00100000 };
    enum { MAX_CLOCK_CALC_WINDOW    = 0x10000000 };
    
public :
    CGlobalClock ();
    virtual ~CGlobalClock();

    /* 强制时钟同步(lwSysClock : 当前系统绝对时间, 单位us) */
    VOID SyncTime(WORD64 lwSysClock);

    /* 获取当前秒部+微秒部+Cycle, 本接口仅日志线程使用 */
    VOID GetTime(WORD64 &lwSeconds, WORD64 &lwMicroSec, WORD64 &lwCycle);

    /* 获取当前系统时间(年/月/日/时/分/秒/微秒) */
    VOID GetTime2(T_BaseSystemTime &rtTime);

    /* 获取当前系统时间(单位:微秒) */
    VOID GetTime3(WORD64 &lwMicroSec, WORD64 &lwCycle);

    /* 获取当前系统CPU主频 */
    WORD32 GetCpuFreq();

    /* 获取当前系统CPU 0.1us时段的Cycle数 */
    WORD32 GetCyclePer100NS();

    /* 设置当前系统帧号&时隙号 */
    VOID SetSFN(WORD16 wSFN, BYTE ucSlot);

    /* 获取当前系统帧号&时隙号 */
    VOID GetSFN(WORD16 &rwSFN, BYTE &rucSlot);

    /* 获取当前TTI技术&当前系统帧号&时隙号 */
    VOID GetTickAndSFN(WORD64 &rlwTick, WORD16 &rwSFN, BYTE &rucSlot);

    /* 获取当前TTI计数器 */
    WORD64 GetCurTick();

protected :
    VOID CalcCpuFreq(WORD64 lwLastCycle,
                     WORD64 lwLastMicroSec,
                     WORD32 dwLastCpuFreq);

protected :
    volatile WORD64        m_lwStartCycle;      /* 进程启动时的Cycle */
    volatile WORD64        m_lwStartMicroSec;   /* 进程启动时的绝对时间(单位 : 微秒) */

    volatile WORD32        m_dwCalcWindow;      /* 计算CPU主频的时间窗口(单位:us) */
    volatile WORD32        m_dwCyclePer100ns;   /* 计算0.1us的CPU Cycle数(10的倍数) */

    std::atomic<BOOL>      m_bFlag;             /* 标志下面原子变量是否整处于更新中 */
    std::atomic<WORD32>    m_dwCpuFreq;         /* CPU主频(Unit : CycleNum per 1us) */
    std::atomic<WORD64>    m_lwLastCycle;       /* 进程最近一次更新的Cycle */
    std::atomic<WORD64>    m_lwLastMicroSec;    /* 进程最近一次更新的绝对时间(单位 : 微秒) */

    std::atomic<WORD64>    m_lwCurTick;         /* TTI计数器 */
    std::atomic<WORD16>    m_wSFN;
    std::atomic<BYTE>      m_ucSlot;
};


/* 获取当前系统时间(单位:微秒) */
inline VOID CGlobalClock::GetTime3(WORD64 &lwMicroSec, WORD64 &lwCycle)
{
    while (FALSE == m_bFlag.load(std::memory_order_relaxed))
    {
#ifdef ARCH_ARM64    
        asm volatile("yield" ::: "memory");
#else
        _mm_pause();
#endif
    }

    WORD64 lwLastCycle    = m_lwLastCycle.load(std::memory_order_relaxed);
    WORD64 lwLastMicroSec = m_lwLastMicroSec.load(std::memory_order_relaxed);
    WORD32 dwCpuFreq      = m_dwCpuFreq.load(std::memory_order_relaxed);

    lwCycle = GetCycle();

    WORD64 lwDiffCycle = lwCycle - lwLastCycle;
    WORD64 lwDiffUs    = TRANSFER_CYCLE_TO_US(lwDiffCycle, dwCpuFreq);

    lwMicroSec = lwDiffUs + lwLastMicroSec;
}


/* 获取当前系统CPU主频 */
inline WORD32 CGlobalClock::GetCpuFreq()
{
    return m_dwCpuFreq.load(std::memory_order_relaxed);
}


/* 获取当前系统CPU 0.1us时段的Cycle数 */
inline WORD32 CGlobalClock::GetCyclePer100NS()
{
    return m_dwCyclePer100ns;
}


/* 设置当前系统帧号&时隙号 */
inline VOID CGlobalClock::SetSFN(WORD16 wSFN, BYTE ucSlot)
{
    m_wSFN.store(wSFN, std::memory_order_relaxed);
    m_ucSlot.store(ucSlot, std::memory_order_relaxed);
    m_lwCurTick++;
}


/* 获取当前系统帧号&时隙号 */
inline VOID CGlobalClock::GetSFN(WORD16 &rwSFN, BYTE &rucSlot)
{
    rwSFN   = m_wSFN.load(std::memory_order_relaxed);
    rucSlot = m_ucSlot.load(std::memory_order_relaxed);
}


/* 获取当前TTI技术&当前系统帧号&时隙号 */
inline VOID CGlobalClock::GetTickAndSFN(WORD64 &rlwTick,
                                        WORD16 &rwSFN,
                                        BYTE   &rucSlot)
{
    rlwTick = m_lwCurTick.load(std::memory_order_relaxed);
    rwSFN   = m_wSFN.load(std::memory_order_relaxed);
    rucSlot = m_ucSlot.load(std::memory_order_relaxed);
}


/* 获取当前TTI计数器 */
inline WORD64 CGlobalClock::GetCurTick()
{
    return m_lwCurTick.load(std::memory_order_relaxed);
}


#endif


