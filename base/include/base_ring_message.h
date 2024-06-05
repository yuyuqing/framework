

#ifndef _BASE_RING_MESSAGE_H_
#define _BASE_RING_MESSAGE_H_


#include <iostream>

#include "base_lock.h"
#include "base_ring_array.h"
#include "base_variable.h"


#define BURST_NUM        ((WORD32)(32))


/* �ص���Ϣ���� */
using PMsgCallBack = WORD32 (*)(VOID *pArg, VOID *pMsg);


template <WORD32 POWER_NUM>
class CMsgRing : public CBaseRingTpl <POWER_NUM>
{
public :
    CMsgRing ();
    virtual ~CMsgRing();

    virtual WORD32 Initialize();

    /* ʧ�ܷ���0; �ɹ�����1 */
    WORD32 Enqueue(VOID *pBuf, WORD16 wLoopThreshold);

    /* һ�ε��ö�λص�, ��Ring�е���Ϣһ���Դ�����
       pObj  : �ص�����
       pFunc : �ص�����
    */
    WORD32 Dequeue(VOID *pObj, PMsgCallBack pFunc);

    /* һ�ε��ö�λص�, ָ���ص�����
       a. ��RingBuffer�е���Ϣ��������ָ����dwNumʱ, ����������Ϣ
       b. ��RingBuffer�е���Ϣ��������ָ����dwNumʱ, ��ָ����dwNum����Ϣ
       dwNum     : �������ص�����
       pObj      : �ص�����
       pFunc     : �ص�����
       rdwRcvNum : ʵ�ʻص�����
       ����ֵ: RingBuffer��ʣ�����Ϣ����
    */
    WORD32 Dequeue(WORD32 dwNum, VOID *pObj, PMsgCallBack pFunc, WORD32 &rdwRcvNum);    
};


template <WORD32 POWER_NUM>
CMsgRing<POWER_NUM>::CMsgRing ()
{
}


template <WORD32 POWER_NUM>
CMsgRing<POWER_NUM>::~CMsgRing()
{
}


template <WORD32 POWER_NUM>
WORD32 CMsgRing<POWER_NUM>::Initialize()
{
    return CBaseRingTpl<POWER_NUM>::Initialize();
}


template <WORD32 POWER_NUM>
inline WORD32 CMsgRing<POWER_NUM>::Enqueue(VOID *pBuf, WORD16 wLoopThreshold)
{
    if (unlikely(NULL == pBuf))
    {
        return 0;
    }

    WORD32 dwNum  = 0;
    WORD32 dwLoop = 0;

    do
    {
        dwNum = CBaseRingTpl<POWER_NUM>::Enqueue(pBuf);
        dwLoop++;
    }while ((0 == dwNum) && (dwLoop < wLoopThreshold));

    return dwNum;
}


/* һ�ε��ö�λص�, ��Ring�е���Ϣһ���Դ�����
   pObj  : �ص�����
   pFunc : �ص�����
   ����ֵ: �ص�����Ϣ����
*/
template <WORD32 POWER_NUM>
inline WORD32 CMsgRing<POWER_NUM>::Dequeue(VOID *pObj, PMsgCallBack pFunc)
{
    VOID   *apBuf[BURST_NUM] = {0, };
    WORD32  dwNum            = 0;
    WORD32  dwTotalNum       = 0;
    WORD32  dwRemainSize     = 0;

    do
    {
        dwNum = CBaseRingTpl<POWER_NUM>::DequeueBurst(BURST_NUM, apBuf, dwRemainSize);

        for (WORD32 dwIndex = 0; dwIndex < dwNum; dwIndex++)
        {
            __builtin_prefetch(apBuf[dwIndex], 1, 2);
            (*pFunc)(pObj, (apBuf[dwIndex]));
        }

        dwTotalNum += dwNum;
    } while (dwRemainSize > 0);

    return dwTotalNum;
}


/* һ�ε��ö�λص�, ָ���ص�����
   a. ��RingBuffer�е���Ϣ��������ָ����dwNumʱ, ����������Ϣ
   b. ��RingBuffer�е���Ϣ��������ָ����dwNumʱ, ��ָ����dwNum����Ϣ
   dwNum     : �������ص�����
   pObj      : �ص�����
   pFunc     : �ص�����
   rdwRcvNum : ʵ�ʻص�����
   ����ֵ: RingBuffer��ʣ�����Ϣ����
*/
template <WORD32 POWER_NUM>
inline WORD32 CMsgRing<POWER_NUM>::Dequeue(WORD32        dwNum, 
                                           VOID         *pObj, 
                                           PMsgCallBack  pFunc, 
                                           WORD32       &rdwRcvNum)
{
    VOID   *apBuf[dwNum] = {0, };
    WORD32  dwRemainSize = 0;

    rdwRcvNum = CBaseRingTpl<POWER_NUM>::DequeueBurst(dwNum, apBuf, dwRemainSize);

    for (WORD32 dwIndex = 0; dwIndex < rdwRcvNum; dwIndex++)
    {
        __builtin_prefetch(apBuf[dwIndex], 1, 2);
        (*pFunc)(pObj, (apBuf[dwIndex]));
    }

    return dwRemainSize;
}


template <WORD32 POWER_NUM>
class CMultiRing : public CBaseData
{
public :
    enum {SINGLE_RING_NUM = 128};

    typedef CBaseRingTpl<POWER_NUM>                  CSTRing;
    typedef CBaseArray<CSTRing, SINGLE_RING_NUM>     CSTRingArray;

public :
    CMultiRing ();
    virtual ~CMultiRing();

    virtual WORD32 Initialize();

    BOOL isEmpty();

    /* ����ҵ���̴߳���ʱ���� */
    CSTRing * CreateSTRing(WORD32 dwRingID);

    /* ʧ�ܷ���0; �ɹ�����1 */
    WORD32 Enqueue(VOID *pBuf, WORD16 wLoopThreshold);

    /* һ�ε��ö�λص�, ��Ring�е���Ϣһ���Դ�����
       pObj      : �ص�����
       pFunc     : �ص�����
    */
    WORD32 Dequeue(VOID *pObj, PMsgCallBack pFunc);

    /* һ�ε��ö�λص�, ָ���ص�����
       a. ��RingBuffer�е���Ϣ��������ָ����dwNumʱ, ����������Ϣ
       b. ��RingBuffer�е���Ϣ��������ָ����dwNumʱ, ��ָ����dwNum����Ϣ
       dwNum     : �������ص�����
       pObj      : �ص�����
       pFunc     : �ص�����
       rdwRcvNum : ʵ�ʻص�����
       ����ֵ: RingBuffer��ʣ�����Ϣ����
    */
    WORD32 Dequeue(WORD32        dwNum,
                   VOID         *pObj,
                   PMsgCallBack  pFunc,
                   WORD32       &rdwRcvNum);

protected :
    CSTRingArray     m_cArray;

    CAtomicLock      m_cLock;
    volatile WORD32  m_dwRingNum;
    CSTRing         *m_apRing[SINGLE_RING_NUM];
    CSTRing         *m_apOriRing[SINGLE_RING_NUM];
};


template <WORD32 POWER_NUM>
CMultiRing<POWER_NUM>::CMultiRing ()
{
    m_dwRingNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < SINGLE_RING_NUM; dwIndex++)
    {
        m_apRing[dwIndex]    = NULL;
        m_apOriRing[dwIndex] = NULL;
    }
}


template <WORD32 POWER_NUM>
CMultiRing<POWER_NUM>::~CMultiRing()
{
    m_dwRingNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < SINGLE_RING_NUM; dwIndex++)
    {
        m_apRing[dwIndex]    = NULL;
        m_apOriRing[dwIndex] = NULL;
    }
}


template <WORD32 POWER_NUM>
WORD32 CMultiRing<POWER_NUM>::Initialize()
{
    m_cArray.Initialize();

    return SUCCESS;
}


template <WORD32 POWER_NUM>
inline BOOL CMultiRing<POWER_NUM>::isEmpty()
{
    BOOL bFlag = TRUE;

    for (WORD32 dwIndex = 0; dwIndex < m_dwRingNum; dwIndex++)
    {
        bFlag = m_apRing[dwIndex]->isEmpty();
        if (FALSE == bFlag)
        {
            break ;
        }
    }

    return bFlag;
}


template <WORD32 POWER_NUM>
typename CMultiRing<POWER_NUM>::CSTRing * 
CMultiRing<POWER_NUM>::CreateSTRing(WORD32 dwRingID)
{
    WORD32 dwQueueID = dwRingID % SINGLE_RING_NUM;

    m_cLock.Lock();

    if ((NULL != m_apOriRing[dwQueueID]) || (m_dwRingNum >= SINGLE_RING_NUM))
    {
        /* �ظ�����Ҳ��assert */
        assert(0);
    }

    CSTRing *pRing = m_cArray.Create(dwQueueID);
    if (NULL == pRing)
    {
        assert(0);
    }

    pRing->Initialize();

    m_apRing[m_dwRingNum] = pRing;

    /* �ڴ����� */
#ifdef ARCH_ARM64
    asm volatile("dmb " "ishld" : : : "memory");  /* ���Arm64ָ�� */
#else
    asm volatile ("" : : : "memory");             /* ���X86ָ�� */
#endif

    m_dwRingNum++;

    /* �ڴ����� */
#ifdef ARCH_ARM64
    asm volatile("dmb " "ishld" : : : "memory");  /* ���Arm64ָ�� */
#else
    asm volatile ("" : : : "memory");             /* ���X86ָ�� */
#endif

    m_apOriRing[dwQueueID] = pRing;

    m_cLock.UnLock();

    return pRing;
}


/* ʧ�ܷ���0; �ɹ�����1 */
template <WORD32 POWER_NUM>
inline WORD32 CMultiRing<POWER_NUM>::Enqueue(VOID *pBuf, WORD16 wLoopThreshold)
{
    WORD32 dwQueueID = m_dwSelfRingID % SINGLE_RING_NUM;

    if (unlikely((NULL == pBuf) || (NULL == m_apOriRing[dwQueueID])))
    {
        return 0;
    }

    return m_apOriRing[dwQueueID]->Enqueue(pBuf, wLoopThreshold);
}


/* һ�ε��ö�λص�, ��Ring�е���Ϣһ���Դ�����
   pObj      : �ص�����
   pFunc     : �ص�����
*/
template <WORD32 POWER_NUM>
inline WORD32 CMultiRing<POWER_NUM>::Dequeue(VOID         *pObj, 
                                             PMsgCallBack  pFunc)
{
    WORD32 dwTotalNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < m_dwRingNum; dwIndex++)
    {
        if (m_apRing[dwIndex]->isEmpty())
        {
            continue ;
        }

        VOID   *apBuf[BURST_NUM] = {0, };
        WORD32  dwNum            = 0;
        WORD32  dwRemainSize     = 0;

        do
        {
            dwNum = m_apRing[dwIndex]->DequeueBurst(BURST_NUM, apBuf, dwRemainSize);

            for (WORD32 dwIndex1 = 0; dwIndex1 < dwNum; dwIndex1++)
            {
                __builtin_prefetch(apBuf[dwIndex1], 1, 2);
                (*pFunc)(pObj, (apBuf[dwIndex1]));
            }

            dwTotalNum += dwNum;
        } while (dwRemainSize > 0);
    }

    return dwTotalNum;
}


/* һ�ε��ö�λص�, ָ���ص�����
   a. ��RingBuffer�е���Ϣ��������ָ����dwNumʱ, ����������Ϣ
   b. ��RingBuffer�е���Ϣ��������ָ����dwNumʱ, ��ָ����dwNum����Ϣ
   dwNum     : �������ص�����
   pObj      : �ص�����
   pFunc     : �ص�����
   rdwRcvNum : ʵ�ʻص�����
   ����ֵ: RingBuffer��ʣ�����Ϣ����
*/
template <WORD32 POWER_NUM>
inline WORD32 CMultiRing<POWER_NUM>::Dequeue(WORD32        dwNum,
                                             VOID         *pObj,
                                             PMsgCallBack  pFunc,
                                             WORD32       &rdwRcvNum)
{
    WORD32 dwRemainSize = 0;
    WORD32 dwRecvCount  = 0;

    rdwRcvNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < m_dwRingNum; dwIndex++)
    {
        if (m_apRing[dwIndex]->isEmpty())
        {
            continue ;
        }

        VOID   *apBuf[dwNum] = {0, };
        WORD32  dwRemainTmp  = 0;

        dwRecvCount = m_apRing[dwIndex]->DequeueBurst(dwNum, apBuf, dwRemainTmp);

        for (WORD32 dwIndex1 = 0; dwIndex1 < dwRecvCount; dwIndex1++)
        {
            __builtin_prefetch(apBuf[dwIndex1], 1, 2);
            (*pFunc)(pObj, (apBuf[dwIndex1]));
        }

        dwRemainSize += dwRemainTmp;
        rdwRcvNum    += dwRecvCount;
    }

    return dwRemainSize;
}


#define MSG_POWER_NUM    ((WORD32)(10))

/* MISO��ʽ����, ������������߳�, һ���������߳� */
typedef CMsgRing<MSG_POWER_NUM>    CMessageRing;

/* ÿ��ST-Ring��SISO��ʽ����, ��ͬ���һ��������MISO��ʽ����; ��: ��ʱ���߳� or ��־�߳� */
typedef CMultiRing<MSG_POWER_NUM>  CMultiMessageRing;


#endif


