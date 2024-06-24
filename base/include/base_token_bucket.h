

#ifndef _BASE_TOKEN_BUCKET_H_
#define _BASE_TOKEN_BUCKET_H_


#include "base_variable.h"
#include "base_time.h"


class CTokenBucket
{
public :
    enum { TOKEN_GRADE_PER_SECOND = 1000000000 };  /* ʱ����λ : ���� */

public :
    /* lwRate : ÿ�봦���Token����; lwBurstSize : ÿ��ͻ���������ѵ�Token���� */
    CTokenBucket(WORD64 lwRate, WORD64 lwBurstSize);
    virtual ~CTokenBucket();

    BOOL Consume(WORD64 lwTokens);

protected :
    WORD64    m_lwTimePerToken;  /* ÿ����1 Token��Ҫ��ʱ�� */
    WORD64    m_lwTimePerBurst;  /* ÿ��ͻ����Ҫ��ʱ�� */
    WORD64    m_lwBurstTime;     /* Burstʱ��; ��λ:���� */
};


/* lwRate : ÿ�봦���Token����; lwBurstSize : ÿ��ͻ���������ѵ�Token���� */
CTokenBucket::CTokenBucket(WORD64 lwRate, WORD64 lwBurstSize)
{
    WORD64 lwCycle   = 0;
    WORD64 lwNanoSec = TOKEN_GRADE_PER_SECOND;

    lwRate = MIN(lwRate, TOKEN_GRADE_PER_SECOND);

    m_lwTimePerToken = lwNanoSec / lwRate;
    m_lwTimePerBurst = lwBurstSize * m_lwTimePerToken;

    g_pGlobalClock->GetTime3(m_lwBurstTime, lwCycle);

    m_lwBurstTime = m_lwBurstTime * 1000;
}


CTokenBucket::~CTokenBucket()
{
    m_lwTimePerToken = 0;
    m_lwTimePerBurst = 0;
    m_lwBurstTime    = 0;
}


BOOL CTokenBucket::Consume(WORD64 lwTokens)
{
    WORD64 lwCycle    = 0;
    WORD64 lwNowCLock = 0;
    WORD64 lwMinTime  = 0;
    WORD64 lwNewTime  = 0;
    WORD64 lwOldTime  = m_lwBurstTime;
    WORD64 lwTimeNeed = lwTokens * m_lwTimePerToken;

    g_pGlobalClock->GetTime3(lwNowCLock, lwCycle);

    lwNowCLock = lwNowCLock * 1000;
    lwMinTime  = lwNowCLock - m_lwTimePerBurst;
    lwNewTime  = (lwMinTime > lwOldTime) ? lwMinTime : lwOldTime;
    lwNewTime += lwTimeNeed;

    if (lwNewTime > lwNowCLock)
    {
        return FALSE;
    }

    m_lwBurstTime = lwNewTime;

    return TRUE;
}


#endif


