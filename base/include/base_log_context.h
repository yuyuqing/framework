

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
    CBaseBitMap02         m_cExtModuleBitMap;  /* ��չģ��bit����, �������ڿ���С���������־��� */
    CBaseBitMap02         m_cLevelBitMap;      /* ��־��ӡ����bit����, ���ڿ��Ʋ�ͬ�ȼ�����־��� */ 

    std::atomic<WORD64>   m_lwTotalCount;
    std::atomic<WORD64>   m_lwForcedCount;     /* ������Ϣ������ǿ�ƶ���Ϣ���� */
    std::atomic<WORD64>   m_lwMallocFail;      /* �����ڴ�ʧ�ܶ�����Ϣ */
    std::atomic<WORD64>   m_lwEnqueueFail;     /* �����ʧ�ܶ�����Ϣ */
    std::atomic<WORD64>   m_lwSuccCount;       /* ��Ϣ���ͳɹ����� */
};


#endif


