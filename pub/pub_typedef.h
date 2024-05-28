

#ifndef _PUB_TYPEDEF_H_
#define _PUB_TYPEDEF_H_


/* 与C兼容 */
#ifdef __cplusplus
extern "C"{
#endif


#include <stdint.h>


#ifndef BYTE
typedef unsigned char   BYTE;
#endif

#ifndef UCHAR
typedef unsigned char   UCHAR;
#endif

#ifndef CHAR
typedef char            CHAR;
#endif

#ifndef WORD16
typedef unsigned short  WORD16;
#endif

#ifndef SWORD16
typedef short           SWORD16;
#endif

#ifndef WORD32
typedef unsigned int    WORD32;
#endif

#ifndef SWORD32
typedef int             SWORD32;
#endif

#ifndef WORD64
typedef uint64_t        WORD64;
#endif

#ifndef SWORD64
typedef int64_t         SWORD64;
#endif


#ifndef FLOAT
typedef float           FLOAT;
#endif

#ifndef DOUBLE
typedef double          DOUBLE;
#endif


#ifndef S8
typedef char            S8;
#endif


#ifndef U8
typedef unsigned char   U8;
#endif


#ifndef S16
typedef short           S16;
#endif


#ifndef U16
typedef unsigned short  U16;
#endif


#ifndef S32
typedef int             S32;
#endif


#ifndef U32
typedef unsigned int    U32;
#endif


#ifndef S64
typedef long long       S64;
#endif


#ifndef U64
typedef unsigned long long  U64;
#endif


#ifndef uint8_t
typedef unsigned char uint8_t;
#endif


#ifndef uint16_t
typedef unsigned short uint16_t;
#endif


#ifndef uint32_t
typedef unsigned int uint32_t;
#endif


#define PTR             WORD64


#ifndef _EFTCYGWIN
#ifndef VOID
typedef void   VOID;
#endif
#else
#ifndef VOID
#define VOID void;
#endif
#endif

#ifndef BOOL
typedef int BOOL;
#endif


#ifndef SUCCESS
#define SUCCESS   ((WORD32)(0))
#endif

#ifndef FAIL
#define FAIL      ((WORD32)(0xFFFFFFFF))
#endif

#ifndef FALSE
#define FALSE   ((WORD32)(0))
#endif

#ifndef TRUE
#define TRUE    ((WORD32)(1))
#endif

#ifndef INVALID_LWORD
#define INVALID_LWORD   ((WORD64)(0xFFFFFFFFFFFFFFFF))
#endif

#ifndef INVALID_DWORD
#define INVALID_DWORD   ((WORD32)(0xFFFFFFFF))
#endif

#ifndef INVALID_WORD
#define INVALID_WORD    ((WORD16)(0xFFFF))
#endif

#ifndef INVALID_BYTE
#define INVALID_BYTE    ((BYTE)(0xFF))
#endif

#ifndef NULL
	#ifdef __cplusplus
		#define NULL  0
	#else
		#define NULL  ((VOID *) 0)
	#endif
#endif


#define IS_POWER2(X)              (((X) > 0) && (((X) & (~(X) + 1u)) == (X)))


/* 向上取整为Align的倍数 */
#define ROUND_UP(Value, Align)    (((Value) + ((Align) - 1)) & (~((Align) - 1)))

/* 向下取整为Align的倍数 */
#define ROUND_DWON(Value, Align)  ((Value) & (~((Align) - 1)))


#define ADDR_ALIGN64(Addr)        ((VOID *)((((WORD64)(Addr)) & 0xFFFFFFFFFFFFFFC0) + 64))


#ifndef     MIN
#define     MIN(a, b)             (((a) < (b)) ? (a) : (b))
#endif

#ifndef     MAX
#define     MAX(a, b)             (((a) > (b)) ? (a) : (b))
#endif


/**
 * Check if a branch is likely to be taken.
 *
 * This compiler builtin allows the developer to indicate if a branch is
 * likely to be taken. Example:
 *
 *   if (likely(x > 1))
 *      do_stuff();
 *
 */
#ifndef likely
#define likely(x)	__builtin_expect(!!(x), 1)
#endif /* likely */

/**
 * Check if a branch is unlikely to be taken.
 *
 * This compiler builtin allows the developer to indicate if a branch is
 * unlikely to be taken. Example:
 *
 *   if (unlikely(x < 1))
 *      do_stuff();
 *
 */
#ifndef unlikely
#define unlikely(x)	__builtin_expect(!!(x), 0)
#endif /* unlikely */


#ifdef __cplusplus
}
#endif


#endif


