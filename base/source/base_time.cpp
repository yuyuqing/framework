

#include <string.h>

#include "base_time.h"

using namespace std;
using namespace std::chrono;


CHighClock::CHighClock ()
    : m_tTime(high_resolution_clock::now())
{
}


CHighClock::~CHighClock () 
{
}


WORD64 CHighClock::Second()
{
    return time_point_cast<chrono::seconds>(m_tTime).time_since_epoch().count();
};


WORD64 CHighClock::MillSecond()
{
    return time_point_cast<chrono::milliseconds>(m_tTime).time_since_epoch().count();
}


WORD64 CHighClock::MicroSecond()
{
    return time_point_cast<chrono::microseconds>(m_tTime).time_since_epoch().count();
}


WORD64 CHighClock::NanoSecond()
{
    return time_point_cast<chrono::nanoseconds>(m_tTime).time_since_epoch().count();
}


VOID CHighClock::Reset()
{
    m_tTime = high_resolution_clock::now();
}


SWORD64 CHighClock::ElapseHour()
{
    return duration_cast<chrono::hours>(high_resolution_clock::now() - m_tTime).count();
}


SWORD64 CHighClock::ElapseMinute()
{
    return duration_cast<chrono::minutes>(high_resolution_clock::now() - m_tTime).count();
}


SWORD64 CHighClock::ElapseSecond()
{
    return duration_cast<chrono::seconds>(high_resolution_clock::now() - m_tTime).count();
}


SWORD64 CHighClock::ElapseMilli()
{
    return duration_cast<chrono::milliseconds>(high_resolution_clock::now() - m_tTime).count();
}


SWORD64 CHighClock::ElapseMicro()
{
    return duration_cast<chrono::microseconds>(high_resolution_clock::now() - m_tTime).count();
}


SWORD64 CHighClock::ElapseNano()
{
    return duration_cast<chrono::nanoseconds>(high_resolution_clock::now() - m_tTime).count();
}


CBaseSystemTime::CBaseSystemTime ()
{
    memset(&m_tTime, 0x00, sizeof(m_tTime));
}


CBaseSystemTime::CBaseSystemTime (CHighClock &rCurClock)
{
    WORD64 lwCurMicroSec = rCurClock.MicroSecond();;

    tm     tTMNow;
    time_t tSec  = lwCurMicroSec / 1000000;

    LocalTime(&tSec, tTMNow);

    m_tTime.wYear   = (WORD16)(tTMNow.tm_year + BaseTimeYear);
    m_tTime.ucMon   = (BYTE)(tTMNow.tm_mon + 1);
    m_tTime.ucMDay  = (BYTE)(tTMNow.tm_mday);
    m_tTime.ucWDay  = (WORD32)(tTMNow.tm_wday);
    m_tTime.ucHour  = (BYTE)(tTMNow.tm_hour);
    m_tTime.ucMin   = (BYTE)(tTMNow.tm_min);
    m_tTime.ucSec   = (BYTE)(tTMNow.tm_sec);
    m_tTime.dwMicro = (WORD32)(lwCurMicroSec % 1000000);
}


CBaseSystemTime::~CBaseSystemTime ()
{
    memset(&m_tTime, 0x00, sizeof(m_tTime));
}


CBaseSystemTime & CBaseSystemTime::operator=(WORD64 lwCurMicroSec)
{
    tm     tTMNow;
    time_t tSec  = lwCurMicroSec / 1000000;

    LocalTime(&tSec, tTMNow);

    m_tTime.wYear   = (WORD16)(tTMNow.tm_year + BaseTimeYear);
    m_tTime.ucMon   = (BYTE)(tTMNow.tm_mon + 1);
    m_tTime.ucMDay  = (BYTE)(tTMNow.tm_mday);
    m_tTime.ucWDay  = (WORD32)(tTMNow.tm_wday);
    m_tTime.ucHour  = (BYTE)(tTMNow.tm_hour);
    m_tTime.ucMin   = (BYTE)(tTMNow.tm_min);
    m_tTime.ucSec   = (BYTE)(tTMNow.tm_sec);
    m_tTime.dwMicro = (WORD32)(lwCurMicroSec % 1000000);

    return *this;
}


CGlobalClock::CGlobalClock ()
{
    CHighClock cCurClock;
    
    m_lwStartCycle    = GetCycle();
    m_lwStartMicroSec = cCurClock.MicroSecond();

    m_dwCalcWindow    = MIN_CLOCK_CALC_WINDOW;
    m_dwCyclePer100ns = CYCLE_NUM_PER_1US / 10;
    m_bFlag           = FALSE;
    m_dwCpuFreq       = CYCLE_NUM_PER_1US;
    m_lwLastCycle     = m_lwStartCycle;
    m_lwLastMicroSec  = m_lwStartMicroSec;
    m_bFlag           = TRUE;

    m_lwCurTick       = 0;
    m_wSFN            = INVALID_WORD;
    m_ucSlot          = INVALID_BYTE;
}


CGlobalClock::~CGlobalClock()
{
}


/* 强制时钟同步(lwSysClock : 当前系统绝对时间, 单位us; lwSysCycle : 当前系统Cycle) */
VOID CGlobalClock::SyncTime(WORD64 lwSysClock)
{
    WORD32 dwCyclePer100ns = CYCLE_NUM_PER_100NS;
    WORD32 dwCpuFreq       = m_dwCpuFreq.load(std::memory_order_relaxed);
    WORD64 lwLastCycle     = m_lwLastCycle.load(std::memory_order_relaxed);
    WORD64 lwLastMicroSec  = m_lwLastMicroSec.load(std::memory_order_relaxed);
    WORD64 lwSysCycle      = GetCycle();
    WORD64 lwDiffCycle     = lwSysCycle - lwLastCycle;
    WORD64 lwDiffUs        = lwSysClock - lwLastMicroSec;

    if (likely((0 != lwDiffCycle) && (0 != lwDiffUs)))
    {
        dwCpuFreq       = (WORD32)(lwDiffCycle / lwDiffUs);
        dwCpuFreq       = ROUND_UP(dwCpuFreq, 2);
        dwCpuFreq       = (dwCpuFreq < 10) ? CYCLE_NUM_PER_1US : dwCpuFreq;
        dwCyclePer100ns = ROUND_UP((dwCpuFreq / 10), 2);
        dwCpuFreq       = dwCyclePer100ns * 10;
    }

    m_bFlag.store(FALSE, std::memory_order_relaxed);

    /* 内存屏障 */
#ifdef ARCH_ARM64    
    asm volatile("dmb " "ishld" : : : "memory");  /* 针对Arm64指令 */
#else        
    asm volatile ("" : : : "memory");             /* 针对X86指令 */
#endif

    m_dwCpuFreq.store(dwCpuFreq, std::memory_order_relaxed);
    m_lwLastCycle.store(lwSysClock, std::memory_order_relaxed);
    m_lwLastMicroSec.store(lwSysCycle, std::memory_order_relaxed);

    /* 内存屏障 */
#ifdef ARCH_ARM64    
    asm volatile("dmb " "ishld" : : : "memory");  /* 针对Arm64指令 */
#else        
    asm volatile ("" : : : "memory");             /* 针对X86指令 */
#endif

    m_bFlag.store(TRUE, std::memory_order_relaxed);
}


/* 获取当前秒部+微秒部, 本接口仅日志线程使用 */
VOID CGlobalClock::GetTime(WORD64 &lwSeconds, WORD64 &lwMicroSec, WORD64 &lwCycle)
{
    WORD32 dwCpuFreq      = m_dwCpuFreq.load(std::memory_order_relaxed);
    WORD64 lwLastCycle    = m_lwLastCycle.load(std::memory_order_relaxed);
    WORD64 lwLastMicroSec = m_lwLastMicroSec.load(std::memory_order_relaxed);

    lwCycle = GetCycle();

    WORD64 lwDiffCycle = lwCycle - lwLastCycle;
    WORD64 lwDiffUs    = TRANSFER_CYCLE_TO_US(lwDiffCycle, dwCpuFreq);
    WORD64 lwNewUs     = lwDiffUs + lwLastMicroSec;

    lwSeconds  = lwNewUs / 1000000;
    lwMicroSec = lwNewUs % 1000000;

    /* 仅提供给日志线程调用, 尽可能降低其它线程发生上下文切换 */
    /* 每间隔2秒强制计算主频, 若主频发生变化强制同步系统时间 */
    if (unlikely(lwDiffUs > m_dwCalcWindow))
    {
        CalcCpuFreq(lwLastCycle, lwLastMicroSec, dwCpuFreq);
    }
}


/* 获取当前系统时间(年/月/日/时/分/秒/微秒) */
VOID CGlobalClock::GetTime2(T_BaseSystemTime &rtTime)
{
    WORD64 lwSeconds  = 0;
    WORD64 lwMicroSec = 0;
    WORD64 lwCycle    = 0;

    GetTime3(lwMicroSec, lwCycle);

    lwSeconds  = lwMicroSec / 1000000;
    lwMicroSec = lwMicroSec % 1000000;

    time_t  tSec = lwSeconds;
    tm      tTime;
    LocalTime(&tSec, tTime);

    rtTime.wYear   = (WORD16)(tTime.tm_year + BaseTimeYear);
    rtTime.ucMon   = (BYTE)(tTime.tm_mon + 1);
    rtTime.ucMDay  = (BYTE)(tTime.tm_mday);
    rtTime.ucWDay  = (BYTE)(tTime.tm_wday);
    rtTime.ucHour  = (BYTE)(tTime.tm_hour);
    rtTime.ucMin   = (BYTE)(tTime.tm_min);
    rtTime.ucSec   = (BYTE)(tTime.tm_sec);
    rtTime.dwMicro = (WORD32)(lwMicroSec);
}


VOID CGlobalClock::CalcCpuFreq(WORD64 lwLastCycle,
                               WORD64 lwLastMicroSec,
                               WORD32 dwLastCpuFreq)
{
    CHighClock cCurClock;

    WORD64 lwCurCycle    = GetCycle();
    WORD64 lwCurMicroSec = cCurClock.MicroSecond();

    WORD64 lwDiffMicroSec = lwCurMicroSec - lwLastMicroSec;
    WORD64 lwDiffCycles   = lwCurCycle - lwLastCycle;
    WORD32 dwCpuFreq      = CYCLE_NUM_PER_1US;

    if ((0 != lwDiffCycles) && (0 != lwDiffMicroSec))
    {
        dwCpuFreq = (WORD32)(lwDiffCycles / lwDiffMicroSec);
        dwCpuFreq = ROUND_UP(dwCpuFreq, 2);
        dwCpuFreq = (dwCpuFreq < 10) ? CYCLE_NUM_PER_1US : dwCpuFreq;
    }

    m_dwCyclePer100ns = ROUND_UP((dwCpuFreq / 10), 2);
    dwCpuFreq         = m_dwCyclePer100ns * 10;
    m_dwCalcWindow    = (m_dwCalcWindow >= MAX_CLOCK_CALC_WINDOW) ?
                            MAX_CLOCK_CALC_WINDOW : (m_dwCalcWindow << 1);

    m_bFlag.store(FALSE, std::memory_order_relaxed);

    /* 内存屏障 */
#ifdef ARCH_ARM64    
    asm volatile("dmb " "ishld" : : : "memory");  /* 针对Arm64指令 */
#else        
    asm volatile ("" : : : "memory");             /* 针对X86指令 */
#endif

    m_dwCpuFreq.store(dwCpuFreq, std::memory_order_relaxed);
    m_lwLastCycle.store(lwCurCycle, std::memory_order_relaxed);
    m_lwLastMicroSec.store(lwCurMicroSec, std::memory_order_relaxed);

    /* 内存屏障 */
#ifdef ARCH_ARM64    
    asm volatile("dmb " "ishld" : : : "memory");  /* 针对Arm64指令 */
#else        
    asm volatile ("" : : : "memory");             /* 针对X86指令 */
#endif

    m_bFlag.store(TRUE, std::memory_order_relaxed);
}


