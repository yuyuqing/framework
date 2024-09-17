

#ifndef _BASE_UTIL_H_
#define _BASE_UTIL_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>

#include "pub_typedef.h"


#ifndef PATH_MAX
#define PATH_MAX        ((WORD32)(256))
#else
#undef  PATH_MAX
#define PATH_MAX        ((WORD32)(256))
#endif


#define SYS_CPU_DIR        "/sys/devices/system/cpu/cpu%u"
#define CORE_ID_FILE       "topology/core_id"
#define NUMA_NODE_PATH     "/sys/devices/system/node"


#define gettid()  syscall(SYS_gettid)


#define MAX_CPU_CORE_NUM                        ((WORD32)(64))


/* CPU主频2.5GHz : 1us=2500 Cycles */
#define CYCLE_NUM_PER_1US                       ((WORD32)(1000))
#define CYCLE_NUM_PER_100NS                     ((WORD32)(100))

#define TRANSFER_CYCLE_TO_100NS2(Cycle, Freq)   ((WORD32)((Cycle) / (Freq)))
#define TRANSFER_CYCLE_TO_US(Cycle, Freq)       ((WORD64)((Cycle) / (Freq)))


extern WORD32 Rand(WORD32 dwMod);
extern WORD32 SocketID(WORD32 dwCoreIndex);
extern WORD32 CoreID(WORD32 dwCoreIndex);

extern BOOL CpuDetected(WORD32 dwCoreIndex);

extern BOOL IsRoot();

extern BYTE GetHostEncoding();

extern WORD32 Hash(const BYTE *pName);

extern WORD64 GetCycle();

extern WORD32 LocalTime(time_t *ptSrcTime, tm &rDstTime);

/* 计算对数log(N) */
extern BYTE CalcLog32BitNBase2Ceil(WORD32 dwNumber);


/* 字节序大小端转换 */
class CEndianConvertor
{
public :
    CEndianConvertor ();
    ~CEndianConvertor();

    VOID Setup(BYTE ucElfFileEncode);

    WORD64 operator() (WORD64 lwValue) const;

    SWORD64 operator() (SWORD64 slwValue) const;

    WORD32 operator() (WORD32 dwValue) const;

    SWORD32 operator() (SWORD32 sdwValue) const;

    WORD16 operator() (WORD16 wValue) const;

    SWORD16 operator() (SWORD16 swValue) const;

    BYTE operator() (BYTE ucValue) const;

    CHAR operator() (CHAR cValue) const;

protected :
    BOOL  m_bNeedConvert;
};


inline WORD64 GetCycle()
{
    WORD64 lwTsc = 0;

#ifdef ARCH_ARM64
    asm volatile("isb" : : : "memory");

    /* cntvct_el0  可以在用户态下直接调用, 但精度可能不够 */
    /* pmccntr_el0 需要在内核中使能PMU用户态访问开关 */
    asm volatile("mrs %0, cntvct_el0" : "=r" (lwTsc));
    //asm volatile("mrs %0, pmccntr_el0" : "=r"(lwTsc));
#else
    WORD32 dwLower  = 0;
    WORD32 dwHigher = 0;

    asm volatile("rdtsc" :
                 "=a" (dwLower),
                 "=d" (dwHigher));

    lwTsc = (((WORD64)dwHigher) << 32) | dwLower;
#endif

    return lwTsc;
}


inline WORD32 base_bsr_uint32(WORD32 dwNum)
{
#ifdef ARCH_ARM64
    return (0 == dwNum) ? (0) : ((sizeof(dwNum) << 3) - 1 - __builtin_clz(dwNum));
#else
    WORD32 dwCount;

    __asm__("bsrl %1, %0\n\t"
            "jnz 1f\n\t"
            "movl $0, %0\n\t"
            "1:"
            :"=r"(dwCount):"r"(dwNum));

    return dwCount;
#endif
}


#endif


