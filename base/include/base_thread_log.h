

#ifndef _BASE_THREAD_LOG_H_
#define _BASE_THREAD_LOG_H_


#include "base_oam_app.h"
#include "base_thread_singleton.h"


#define LOG_RING_BURST_NUM        ((WORD32)(32))


typedef struct tagT_LogFileInfo
{
    BYTE        ucFilePos;      /* ��־�ļ����±�, ��pFile��Ӧ */
    BYTE        ucResved[3];
    WORD32      dwFileSize;     /* ��־�ļ���С */

    FILE       *pFile;          /* ��־�ļ�ID */
    WORD64      lwWriteTotal;   /* д�ļ������ֽ��� */

    /* ���ڴ�ӡͳ����Ϣ */
    CLogInfo   *pLogger;       
}T_LogFileInfo;


typedef struct tagT_FastLogMeasure
{
    WORD64  lwCount;                     /* Fast�ӿڵ����ܴ���(��Ч��ӡ��־����) */
    WORD64  lwRingTotalTime;             /* ҵ���̵߳���Fast�ӿ��ܺ�ʱ(��λ : 0.1us) */
    WORD32  adwStat[BIT_NUM_OF_WORD32];  /* ����Fast�ӿں�ʱ�ֲ�(��λ : 0.1us) */
}T_FastLogMeasure;


class CLogThread : public CSingletonThread<CLogThread, CLogMemPool>
{
public :

    enum { LOG_FILE_NUM         = 64 };
    enum { LOG_FILE_SIZE        = 128 * 1024 * 1024 };
    enum { LOG_TICK_GRANULARITY = 200 }; /* 0.2ms���� */

    /* ������ͨ�ӿ�+Fast�ӿ���־ */
    static WORD32 RecvLog(VOID *pObj, VOID *pMsg);

public :
    CLogThread (const T_ThreadParam &rtParam);
    virtual ~CLogThread();

    /* �����߳�ʵ����ִ�г�ʼ��(�����߳�ջ�ռ�ִ��) */
    WORD32 Initialize();

    WORD32 RegistLogFile(FILE     *pFile, 
                         CLogInfo *pLogger, 
                         BYTE      ucPos,
                         WORD32    dwFileSize);

    WORD32 Cancel();

    /* ��ͨ��־�ӿ� */
    WORD32 NormalWrite(FILE    *pFile,
                       WORD16   wModuleID,
                       WORD16   wLen,
                       CHAR    *pchPrtInfo,
                       WORD16   wLoopThreshold,
                       WORD64   lwAddr,
                       CMultiMessageRing::CSTRing *pRing);

    /* Fast��־�ӿ� */
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

    /* ��־�߳������־���̽ӿ� */
    WORD32 WriteBackSelf(FILE    *pFile, 
                         CHAR    *ptMsg, 
                         WORD32   dwMsgLen);

    /* ҵ���߳������־���̽ӿ� */
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

    BOOL                           m_bMeasSwitch;  /* ��־ά�⿪�� */
    WORD32                         m_dwFileSize;   /* ά����־�ļ� */

    T_FastLogMeasure               m_tFastMeasure; /* Fast��־�ӿ�ά����Ϣ */
};


#endif


