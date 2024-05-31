

#ifndef _BASE_LOG_CONTEXT_H_
#define _BASE_LOG_CONTEXT_H_


#include "base_log_wrapper.h"
#include "base_bitmap.h"


#define CONST_LOG_PRE_LEN         ((WORD32)(4))
#define MAX_LOG_STR_LEN           ((WORD32)(800))


class CLogInfo;


class CLogContext
{    
public :
    enum { UNABLE_FLAG = 0 };
    enum { ENABLE_FLAG = 1 };

public :        
    CLogContext (CHAR     *pcFileName,
                 WORD32    dwLineNum,
                 CHAR     *ptr,
                 CLogInfo *pLogInfo);

    virtual ~CLogContext();    

    CLogInfo * GetLogger();

protected:
    CLogInfo     *m_pLogInfo;
    CHAR         *m_pcFileName;
    WORD32        m_dwLineNum;
    WORD16        m_wFlag;
    WORD16        m_wLen;
    BYTE          m_aucData[MAX_LOG_STR_LEN];
};


inline CLogInfo * CLogContext::GetLogger()
{
    return m_pLogInfo;
}


class CModuleLogInfo
{
public :
    CModuleLogInfo ();
    virtual ~CModuleLogInfo();

public : 
    CBaseBitMap02         m_cExtModuleBitMap;  /* 扩展模块bit掩码, 例如用于控制小区级别的日志输出 */
    CBaseBitMap02         m_cLevelBitMap;      /* 日志打印级别bit掩码, 用于控制不同等级的日志输出 */ 

    std::atomic<WORD64>   m_lwTotalCount;
    std::atomic<WORD64>   m_lwForcedCount;     /* 由于消息队列满强制丢消息计数 */
    std::atomic<WORD64>   m_lwMallocFail;      /* 分配内存失败丢弃消息 */
    std::atomic<WORD64>   m_lwEnqueueFail;     /* 入队列失败丢弃消息 */
    std::atomic<WORD64>   m_lwSuccCount;       /* 消息发送成功次数 */
};


#endif


