

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


/* ��-��-�� ʱ:��:��:΢�� */
typedef struct tagT_BaseSystemTime
{
    WORD16   wYear;    /* �� */
    BYTE     ucMon;    /* �� */
    BYTE     ucMDay;   /* ��(����) */
    BYTE     ucWDay;   /* ��(����) */
    BYTE     ucHour;   /* ʱ */
    BYTE     ucMin;    /* �� */
    BYTE     ucSec;    /* �� */
    WORD32   dwMicro;  /* ΢�� */
}T_BaseSystemTime;


typedef enum tagE_WeekDayType
{
    E_WEEK_SUNDAY  = 0,  /* ���� */
    E_WEEK_MONDAY,       /* ��һ */
    E_WEEK_TUEDAY,       /* �ܶ� */
    E_WEEK_WEDDAY,       /* ���� */
    E_WEEK_THUDAY,       /* ���� */
    E_WEEK_FRIDAY,       /* ���� */
    E_WEEK_SATDAY,       /* ���� */
    E_WEEK_MAX,
}E_WeekDayType;


typedef enum tagE_MonthType
{
    E_MONTH_JAN = 0,   /*  1�� */
    E_MONTH_FEB,       /*  2�� */
    E_MONTH_MARCH,     /*  3�� */
    E_MONTH_APRIL,     /*  4�� */
    E_MONTH_MAY,       /*  5�� */
    E_MONTH_JUNE,      /*  6�� */
    E_MONTH_JULY,      /*  7�� */
    E_MONTH_AUGUST,    /*  8�� */
    E_MONTH_SEP,       /*  9�� */
    E_MONTH_OCT,       /* 10�� */
    E_MONTH_NOV,       /* 11�� */
    E_MONTH_DEC,       /* 12�� */
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

    /* ǿ��ʱ��ͬ��(lwSysClock : ��ǰϵͳ����ʱ��, ��λus) */
    VOID SyncTime(WORD64 lwSysClock);

    /* ��ȡ��ǰ�벿+΢�벿+Cycle, ���ӿڽ���־�߳�ʹ�� */
    VOID GetTime(WORD64 &lwSeconds, WORD64 &lwMicroSec, WORD64 &lwCycle);

    /* ��ȡ��ǰϵͳʱ��(��/��/��/ʱ/��/��/΢��) */
    VOID GetTime2(T_BaseSystemTime &rtTime);

    /* ��ȡ��ǰϵͳʱ��(��λ:΢��) */
    VOID GetTime3(WORD64 &lwMicroSec, WORD64 &lwCycle);

    /* ��ȡ��ǰϵͳCPU��Ƶ */
    WORD32 GetCpuFreq();

    /* ��ȡ��ǰϵͳCPU 0.1usʱ�ε�Cycle�� */
    WORD32 GetCyclePer100NS();

    /* ���õ�ǰϵͳ֡��&ʱ϶�� */
    VOID SetSFN(WORD16 wSFN, BYTE ucSlot);

    /* ��ȡ��ǰϵͳ֡��&ʱ϶�� */
    VOID GetSFN(WORD16 &rwSFN, BYTE &rucSlot);

    /* ��ȡ��ǰTTI����&��ǰϵͳ֡��&ʱ϶�� */
    VOID GetTickAndSFN(WORD64 &rlwTick, WORD16 &rwSFN, BYTE &rucSlot);

    /* ��ȡ��ǰTTI������ */
    WORD64 GetCurTick();

protected :
    VOID CalcCpuFreq(WORD64 lwLastCycle,
                     WORD64 lwLastMicroSec,
                     WORD32 dwLastCpuFreq);

protected :
    volatile WORD64        m_lwStartCycle;      /* ��������ʱ��Cycle */
    volatile WORD64        m_lwStartMicroSec;   /* ��������ʱ�ľ���ʱ��(��λ : ΢��) */

    volatile WORD32        m_dwCalcWindow;      /* ����CPU��Ƶ��ʱ�䴰��(��λ:us) */
    volatile WORD32        m_dwCyclePer100ns;   /* ����0.1us��CPU Cycle��(10�ı���) */

    std::atomic<BOOL>      m_bFlag;             /* ��־����ԭ�ӱ����Ƿ������ڸ����� */
    std::atomic<WORD32>    m_dwCpuFreq;         /* CPU��Ƶ(Unit : CycleNum per 1us) */
    std::atomic<WORD64>    m_lwLastCycle;       /* �������һ�θ��µ�Cycle */
    std::atomic<WORD64>    m_lwLastMicroSec;    /* �������һ�θ��µľ���ʱ��(��λ : ΢��) */

    std::atomic<WORD64>    m_lwCurTick;         /* TTI������ */
    std::atomic<WORD16>    m_wSFN;
    std::atomic<BYTE>      m_ucSlot;
};


/* ��ȡ��ǰϵͳʱ��(��λ:΢��) */
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


/* ��ȡ��ǰϵͳCPU��Ƶ */
inline WORD32 CGlobalClock::GetCpuFreq()
{
    return m_dwCpuFreq.load(std::memory_order_relaxed);
}


/* ��ȡ��ǰϵͳCPU 0.1usʱ�ε�Cycle�� */
inline WORD32 CGlobalClock::GetCyclePer100NS()
{
    return m_dwCyclePer100ns;
}


/* ���õ�ǰϵͳ֡��&ʱ϶�� */
inline VOID CGlobalClock::SetSFN(WORD16 wSFN, BYTE ucSlot)
{
    m_wSFN.store(wSFN, std::memory_order_relaxed);
    m_ucSlot.store(ucSlot, std::memory_order_relaxed);
    m_lwCurTick++;
}


/* ��ȡ��ǰϵͳ֡��&ʱ϶�� */
inline VOID CGlobalClock::GetSFN(WORD16 &rwSFN, BYTE &rucSlot)
{
    rwSFN   = m_wSFN.load(std::memory_order_relaxed);
    rucSlot = m_ucSlot.load(std::memory_order_relaxed);
}


/* ��ȡ��ǰTTI����&��ǰϵͳ֡��&ʱ϶�� */
inline VOID CGlobalClock::GetTickAndSFN(WORD64 &rlwTick,
                                        WORD16 &rwSFN,
                                        BYTE   &rucSlot)
{
    rlwTick = m_lwCurTick.load(std::memory_order_relaxed);
    rwSFN   = m_wSFN.load(std::memory_order_relaxed);
    rucSlot = m_ucSlot.load(std::memory_order_relaxed);
}


/* ��ȡ��ǰTTI������ */
inline WORD64 CGlobalClock::GetCurTick()
{
    return m_lwCurTick.load(std::memory_order_relaxed);
}


#endif


