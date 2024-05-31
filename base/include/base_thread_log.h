

#ifndef _BASE_THREAD_LOG_H_
#define _BASE_THREAD_LOG_H_


#include "base_oam_app.h"
#include "base_thread_singleton.h"


#define LOG_RING_BURST_NUM        ((WORD32)(32))


typedef struct tagT_LogFileInfo
{
    BYTE        ucFilePos;      /* 日志文件的下标, 与pFile对应 */
    BYTE        ucResved[3];
    WORD32      dwFileSize;     /* 日志文件大小 */

    FILE       *pFile;          /* 日志文件ID */
    WORD64      lwWriteTotal;   /* 写文件的总字节数 */

    /* 用于打印统计信息 */
    CLogInfo   *pLogger;       
}T_LogFileInfo;


typedef struct tagT_FastLogMeasure
{
    WORD64  lwCount;                     /* Fast接口调用总次数(有效打印日志次数) */
    WORD64  lwRingTotalTime;             /* 业务线程调用Fast接口总耗时(单位 : 0.1us) */
    WORD32  adwStat[BIT_NUM_OF_WORD32];  /* 调用Fast接口耗时分布(单位 : 0.1us) */
}T_FastLogMeasure;


class CLogThread : public CSingletonThread<CLogThread, CLogMemPool>
{
public :

    enum { LOG_FILE_NUM         = 64 };
    enum { LOG_FILE_SIZE        = 128 * 1024 * 1024 };
    enum { LOG_TICK_GRANULARITY = 200 }; /* 0.2ms粒度 */

    /* 处理普通接口+Fast接口日志 */
    static WORD32 RecvLog(VOID *pObj, VOID *pMsg);

public :
    CLogThread (const T_ThreadParam &rtParam);
    virtual ~CLogThread();

    /* 创建线程实例后执行初始化(在主线程栈空间执行) */
    WORD32 Initialize();

    WORD32 RegistLogFile(FILE     *pFile, 
                         CLogInfo *pLogger, 
                         BYTE      ucPos,
                         WORD32    dwFileSize);

    WORD32 Cancel();

    /* 普通日志接口 */
    WORD32 NormalWrite(FILE    *pFile,
                       WORD16   wModuleID,
                       WORD16   wLen,
                       CHAR    *pchPrtInfo,
                       WORD16   wLoopThreshold,
                       WORD64   lwAddr,
                       CMultiMessageRing::CSTRing *pRing);

    /* Fast日志接口 */
    WORD32 FastWrite(FILE               *pFile,
                     WORD16              wModuleID,
                     T_LogBinaryMessage *ptMsg,
                     WORD16              wLoopThreshold,
                     WORD64              lwAddr,
                     CMultiMessageRing::CSTRing *pRing);

    WORD32 Flush();

    virtual VOID Dump();

protected : 
    virtual VOID DoRun();

    /* 日志线程输出日志落盘接口 */
    WORD32 WriteBackSelf(FILE    *pFile, 
                         CHAR    *ptMsg, 
                         WORD32   dwMsgLen);

    /* 业务线程输出日志落盘接口 */
    WORD32 WriteBack(FILE    *pFile, 
                     CHAR    *ptMsg, 
                     WORD32   dwMsgLen);

    WORD32 Assemble(FILE *pFile, T_LogBinaryMessage *ptMsg);

    T_LogFileInfo * Find(FILE *pFile);

protected :
    CSpinLock                      m_cLock;
    WORD32                         m_dwFileNum;
    T_LogFileInfo                  m_atLogFile[LOG_FILE_NUM];
    
    CHAR                           m_aucAssemble[MAX_LOG_STR_LEN];

    std::atomic<WORD64>            m_lwEnqueCount;

    BOOL                           m_bMeasSwitch;  /* 日志维测开关 */
    WORD32                         m_dwFileSize;   /* 维测日志文件 */

    T_FastLogMeasure               m_tFastMeasure; /* Fast日志接口维测信息 */
};


#endif


