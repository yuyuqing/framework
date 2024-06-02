

#ifndef _BASE_LOG_WRAPPER_H_
#define _BASE_LOG_WRAPPER_H_


/* 与C兼容 */
#ifdef __cplusplus
extern "C"{
#endif


#include "pub_typedef.h"
#include "pub_global_def.h"


#define PRINTF_BINARY_8                 "%c%c%c%c%c%c%c%c"

#define PRINTF_BINARY_16                \
    PRINTF_BINARY_8    PRINTF_BINARY_8

#define PRINTF_BINARY_32                \
    PRINTF_BINARY_16    PRINTF_BINARY_16

#define PRINTF_BINARY_64                \
    PRINTF_BINARY_32    PRINTF_BINARY_32

#define PRINTF_BINARY_128               \
    PRINTF_BINARY_64    PRINTF_BINARY_64

#define PRINTF_BINARY_256               \
    PRINTF_BINARY_128    PRINTF_BINARY_128

#define PRINTF_BYTE_TO_BINARY(Value)    \
    (((Value) & 0x80) ? '1' : '0'),     \
    (((Value) & 0x40) ? '1' : '0'),     \
    (((Value) & 0x20) ? '1' : '0'),     \
    (((Value) & 0x10) ? '1' : '0'),     \
    (((Value) & 0x08) ? '1' : '0'),     \
    (((Value) & 0x04) ? '1' : '0'),     \
    (((Value) & 0x02) ? '1' : '0'),     \
    (((Value) & 0x01) ? '1' : '0')


#define COL(X)     "\033[;" #X "m"
#define RED        COL(31)
#define GREEN      COL(32)
#define YELLOW     COL(33)
#define BLUE       COL(34)
#define MAGENTA    COL(35)
#define CYAN       COL(36)
#define WHITE      COL(0)
#define GRAY       "\033[0m"


#define LOG_NAME_LEN         ((BYTE)(64))
#define LOG_PREFIX_LEN       ((BYTE)(256))
#define LOG_LEVEL_LEN        ((BYTE)(8))
#define LOG_MODULE_LEN       ((BYTE)(32))


/* 写日志时切换文件的提前时间量, 单位:ms */
#define LOG_PERIOD_AHEAD     ((WORD32)(100))

#define __FILE_NAME_LEN__    ((WORD32)(40))
#define LOG_MAX_PARAM_NUM    ((BYTE)(31))


typedef enum tagE_LOG_LEVEL
{
    E_LOG_LEVEL_TRACE = 0,
    E_LOG_LEVEL_DEBUG,
    E_LOG_LEVEL_INFO,
    E_LOG_LEVEL_WARN,
    E_LOG_LEVEL_ERROR,
    E_LOG_LEVEL_ASSERT,
    E_LOG_LEVEL_FATAL,
    E_LOG_LEVEL_NUM,
    E_LOG_LEVEL_VERBOSE = 0xFFFF,
}E_LOG_LEVEL;


extern const BYTE s_aucLevel[E_LOG_LEVEL_NUM][LOG_LEVEL_LEN];
extern const BYTE s_aucColor[E_LOG_LEVEL_NUM][LOG_LEVEL_LEN];


typedef struct tagT_LogBinaryMessage
{
    WORD64   lwSeconds;

    WORD32   dwMicroSec;    

    WORD16   wModuleID;
    BYTE     ucLevelID;
    CHAR     ucCell;

    BYTE     ucFileLen;
    BYTE     ucParamNum;
    WORD16   wStrLen;
    WORD32   dwResved1;

    CHAR     aucFile[__FILE_NAME_LEN__];

    WORD64   lwTotalCount;

    WORD64   alwParams[LOG_MAX_PARAM_NUM];

    CHAR     aucFormat[];
}T_LogBinaryMessage;


/* 需要用到NGP的内存池时, 需要回调注册函数 */
typedef VOID (*CB_RegistMemPool) ();


/* 在主进程入口处调用 */
extern WORD32 LogInit_Process(WORD32 dwProcID, CB_RegistMemPool pFunc);

extern WORD32 LogExit_Process();


/* 在线程入口处调用 */
extern WORD32 LogInit_Thread();

extern WORD32 LogExit_Thread();


extern WORD32 SendMessageToLogThread(WORD32 dwMsgID, const VOID *ptMsg, WORD16 wLen);


extern VOID LogAssert();


/* 普通日志打印接口 */
extern VOID LogPrintf(WORD32        dwModuleID,
                      WORD32        dwCellID,
                      WORD32        dwLevelID,
                      BOOL          bForced,
                      const CHAR   *pchPrtInfo,
                      ...);


/* Fast日志打印接口 */
extern VOID FastLogPrintf(WORD32      dwModuleID,
                          WORD32      dwCellID,
                          WORD32      dwLevelID,
                          BOOL        bForced,
                          const CHAR *pFile,
                          WORD32      dwLine,
                          BYTE        ucParamNum,
                          const CHAR *pchPrtInfo,
                          ...);


#define INTERNAL_GET_ARG_COUNT_PRIVATE(                     \
    e0,  e1,  e2,  e3,  e4,  e5,  e6,  e7,  e8,  e9,        \
    e10, e11, e12, e13, e14, e15, e16, e17, e18, e19,       \
    e20, e21, e22, e23, e24, e25, e26, e27, e28, e29,       \
    e30, e31, e32, e33, e34, e35, e36, e37, e38, e39,       \
    e40, e41, e42, e43, e44, e45, e46, e47, e48, e49,       \
    e50, e51, e52, e53, e54, e55, e56, e57, e58, e59,       \
    e60, e61, e62, e63, e64, e65, e66, e67, e68, e69,       \
    e70, e71, e72, e73, e74, e75, e76, e77, e78, e79,       \
    e80, e81, e82, e83, e84, e85, e86, e87, e88, e89,       \
    e90, e91, e92, e93, e94, e95, e96, e97, e98, e99,       \
    e100, count, ...)    count


#define GET_ARG_COUNT(...)                                  \
    INTERNAL_GET_ARG_COUNT_PRIVATE(                         \
        unused, ##__VA_ARGS__, 100,                         \
        99, 98, 97, 96, 95, 94, 93, 92, 91, 90,             \
        89, 88, 87, 86, 85, 84, 83, 82, 81, 80,             \
        79, 78, 77, 76, 75, 74, 73, 72, 71, 70,             \
        69, 68, 67, 66, 65, 64, 63, 62, 61, 60,             \
        59, 58, 57, 56, 55, 54, 53, 52, 51, 50,             \
        49, 48, 47, 46, 45, 44, 43, 42, 41, 40,             \
        39, 38, 37, 36, 35, 34, 33, 32, 31, 30,             \
        29, 28, 27, 26, 25, 24, 23, 22, 21, 20,             \
        19, 18, 17, 16, 15, 14, 13, 12, 11, 10,             \
         9,  8,  7,  6,  5,  4,  3,  2,  1,  0)


#define LOG_VPRINTF(ModuleID, CellID, Level, Flag, INFOSTR, ...)          \
    do                                                                    \
    {                                                                     \
        LogPrintf((WORD32)(ModuleID),                                     \
                  (WORD32)(CellID),                                       \
                  (WORD32)(Level),                                        \
                  (BOOL)(Flag),                                           \
                  "[%s:%d] " INFOSTR,                                     \
                  __FILE__, __LINE__,                                     \
                  ##__VA_ARGS__);                                         \
    } while(0)


#define FAST_LOG_PRINTF(ModuleID, CellID, Level, Flag, INFOSTR, ...)      \
    do                                                                    \
    {                                                                     \
        FastLogPrintf(ModuleID, CellID, Level, Flag, __FILE__, __LINE__,  \
                      GET_ARG_COUNT(__VA_ARGS__), INFOSTR,                \
                      ##__VA_ARGS__);                                     \
    } while(0)


#ifdef __cplusplus
}
#endif


#endif


