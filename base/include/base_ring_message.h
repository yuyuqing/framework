

#ifndef _BASE_RING_MESSAGE_H_
#define _BASE_RING_MESSAGE_H_


#include <iostream>

#include "base_lock.h"
#include "base_ring.h"


#define BURST_NUM                    ((WORD32)(32))
#define SINGLE_RING_THRESHOLD        ((WORD32)(16))
#define SINGLE_RING_NUM              ((WORD32)(128))
#define PACKET_RING_NUM              ((WORD32)(8))


/* �ص���Ϣ���� */
using PMsgCallBack = WORD32 (*)(VOID *pArg, VOID *pMsg);


template <WORD32 POWER_NUM>
class CMsgRing : public CBaseRingTpl <POWER_NUM>
{
public :
    CMsgRing ();
    virtual ~CMsgRing();

    virtual WORD32 Initialize();

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


template <WORD32 POWER_NUM, WORD32 RING_NUM = SINGLE_RING_NUM>
class CMultiRing : public CBaseData
{
public :
    class CSTRing : public CBaseRingTpl<POWER_NUM>
    {
    protected :
        using CBaseRingTpl<POWER_NUM>::Initialize;

    public :
        VOID          *m_pObj;
        PMsgCallBack   m_pFunc;

        CSTRing ()
        {
            m_pObj  = NULL;
            m_pFunc = NULL;
        }

        virtual ~CSTRing()
        {
            m_pObj  = NULL;
            m_pFunc = NULL;
        }

        WORD32 Initialize(VOID *pObj, PMsgCallBack pFunc)
        {
            m_pObj  = pObj;
            m_pFunc = pFunc;

            return CBaseRingTpl<POWER_NUM>::Initialize();
        }
    };

    typedef CBaseArray<CSTRing, RING_NUM>  CSTRingArray;

public :
    CMultiRing ();
    virtual ~CMultiRing();

    virtual WORD32 Initialize();

    BOOL isEmpty();

    WORD32 Count();

    /* ����ҵ���̴߳���ʱ���� */
    CSTRing * CreateSTRing(WORD32 dwRingID);

    /* ע�������汨�Ļص����� 
     * pObj      : ע��App����(pObj����Ϊ�ص������ĵ�һ������pArg)
     * pFunc     : ע��ص�����
     * rdwRingID : ����, ���ش����ɹ���RingID(��ΪEnqueue��ָ������)
     */
    CSTRing * CreateSTRing(VOID         *pObj,
                           PMsgCallBack  pFunc,
                           WORD32       &rdwRingID);

    /* ���밴�մ���������ɾ�� */
    WORD32 DelSTRing(WORD32 dwRingID);

    /* ʧ�ܷ���0; �ɹ�����1 */
    WORD32 Enqueue(WORD32 dwRingID, VOID *pBuf, WORD16 wLoopThreshold = SINGLE_RING_THRESHOLD);

    /* ����Enqueue�ɹ�����Ϣ���� 
     * dwRingID : ʹ�ô���CreatePacketRing�ӿڵĳ���(rdwRingID)
     * dwNum    : ����ӵı�������
     * pBuf     : ����ָ������
     */
    WORD32 Enqueue(WORD32 dwRingID, WORD32 dwNum, VOID **pBuf);

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

    /* һ�ε��ö�λص�, ��Ring�е���Ϣһ���Դ�����; (ʹ��ע��Ķ���) */
    WORD32 Dequeue();

    /* һ�ε��ö�λص�, ָ���ص�����; (ʹ��ע��Ķ���)
     * a. ��RingBuffer�е���Ϣ��������ָ����dwNumʱ, ����������Ϣ
     * b. ��RingBuffer�е���Ϣ��������ָ����dwNumʱ, ��ָ����dwNum����Ϣ
     * dwNum     : �������ص�����
     * rdwRcvNum : ʵ�ʻص�����
     * ����ֵ: RingBuffer��ʣ�����Ϣ����
     */
    WORD32 Dequeue(WORD32 dwNum, WORD32 &rdwRcvNum);

    VOID SnapShot(WORD32          &rdwRingNum,
                  T_RingHeadTail  *ptProd,
                  T_RingHeadTail  *ptCons);

protected :
    CSTRingArray     m_cArray;

    CAtomicLock      m_cLock;
    volatile WORD32  m_dwRingNum;
    CSTRing         *m_apRing[RING_NUM];
    CSTRing         *m_apOriRing[RING_NUM];
};


template <WORD32 POWER_NUM, WORD32 RING_NUM>
CMultiRing<POWER_NUM, RING_NUM>::CMultiRing ()
{
    m_dwRingNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < RING_NUM; dwIndex++)
    {
        m_apRing[dwIndex]    = NULL;
        m_apOriRing[dwIndex] = NULL;
    }
}


template <WORD32 POWER_NUM, WORD32 RING_NUM>
CMultiRing<POWER_NUM, RING_NUM>::~CMultiRing()
{
    m_dwRingNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < RING_NUM; dwIndex++)
    {
        m_apRing[dwIndex]    = NULL;
        m_apOriRing[dwIndex] = NULL;
    }
}


template <WORD32 POWER_NUM, WORD32 RING_NUM>
WORD32 CMultiRing<POWER_NUM, RING_NUM>::Initialize()
{
    m_cArray.Initialize();

    return SUCCESS;
}


template <WORD32 POWER_NUM, WORD32 RING_NUM>
inline BOOL CMultiRing<POWER_NUM, RING_NUM>::isEmpty()
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


template <WORD32 POWER_NUM, WORD32 RING_NUM>
inline WORD32 CMultiRing<POWER_NUM, RING_NUM>::Count()
{
    WORD32 dwCount = 0;

    for (WORD32 dwIndex = 0; dwIndex < m_dwRingNum; dwIndex++)
    {
        dwCount += m_apRing[dwIndex]->Count();
    }

    return dwCount;
}


template <WORD32 POWER_NUM, WORD32 RING_NUM>
typename CMultiRing<POWER_NUM, RING_NUM>::CSTRing * 
CMultiRing<POWER_NUM, RING_NUM>::CreateSTRing(WORD32 dwRingID)
{
    WORD32 dwQueueID = dwRingID % RING_NUM;

    m_cLock.Lock();

    if ((NULL != m_apOriRing[dwQueueID]) || (m_dwRingNum >= RING_NUM))
    {
        /* �ظ�����Ҳ��assert */
        assert(0);
    }

    CSTRing *pRing = m_cArray.Create(dwQueueID);
    if (NULL == pRing)
    {
        assert(0);
    }

    pRing->Initialize(NULL, NULL);

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


/* ע�������汨�Ļص����� 
 * pObj      : ע��App����(pObj����Ϊ�ص������ĵ�һ������pArg)
 * pFunc     : ע��ص�����
 * rdwRingID : ����, ���ش����ɹ���RingID(��ΪEnqueue��ָ������)
 */
template <WORD32 POWER_NUM, WORD32 RING_NUM>
typename CMultiRing<POWER_NUM, RING_NUM>::CSTRing * 
CMultiRing<POWER_NUM, RING_NUM>::CreateSTRing(VOID         *pObj,
                                              PMsgCallBack  pFunc,
                                              WORD32       &rdwRingID)
{
    m_cLock.Lock();

    rdwRingID = m_dwRingNum % PACKET_RING_NUM;

    if ((NULL != m_apOriRing[rdwRingID]) || (m_dwRingNum >= RING_NUM))
    {
        /* �ظ�����Ҳ��assert */
        assert(0);
    }

    CSTRing *pRing = m_cArray.Create(rdwRingID);
    if (NULL == pRing)
    {
        assert(0);
    }

    pRing->Initialize(pObj, pFunc);

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

    m_apOriRing[rdwRingID] = pRing;

    m_cLock.UnLock();

    return pRing;
}


/* ���밴�մ���������ɾ�� */
template <WORD32 POWER_NUM, WORD32 RING_NUM>
WORD32 CMultiRing<POWER_NUM, RING_NUM>::DelSTRing(WORD32 dwRingID)
{
    m_cLock.Lock();

    if (dwRingID != (m_dwRingNum - 1))
    {
        assert(0);
    }

    m_apOriRing[dwRingID] = NULL;

    /* �ڴ����� */
#ifdef ARCH_ARM64
    asm volatile("dmb " "ishld" : : : "memory");  /* ���Arm64ָ�� */
#else
    asm volatile ("" : : : "memory");             /* ���X86ָ�� */
#endif

    m_dwRingNum--;

    /* �ڴ����� */
#ifdef ARCH_ARM64
    asm volatile("dmb " "ishld" : : : "memory");  /* ���Arm64ָ�� */
#else
    asm volatile ("" : : : "memory");             /* ���X86ָ�� */
#endif

    m_apRing[m_dwRingNum] = NULL;

    m_cArray.Delete(dwRingID);

    m_cLock.UnLock();

    return SUCCESS;
}


/* ʧ�ܷ���0; �ɹ�����1 */
template <WORD32 POWER_NUM, WORD32 RING_NUM>
inline WORD32 CMultiRing<POWER_NUM, RING_NUM>::Enqueue(
    WORD32  dwRingID,
    VOID   *pBuf,
    WORD16  wLoopThreshold)
{
    if (unlikely((dwRingID >= m_dwRingNum)
              || (NULL == pBuf)
              || (NULL == m_apOriRing[dwRingID])))
    {
        return 0;
    }

    return m_apOriRing[dwRingID]->Enqueue(pBuf, wLoopThreshold);
}


/* ����Enqueue�ɹ�����Ϣ���� 
 * dwRingID : ʹ�ô���CreatePacketRing�ӿڵĳ���(rdwRingID)
 * dwNum    : ����ӵı�������
 * pBuf     : ����ָ������
 */
template <WORD32 POWER_NUM, WORD32 RING_NUM>
inline WORD32 CMultiRing<POWER_NUM, RING_NUM>::Enqueue(
    WORD32 dwRingID, WORD32 dwNum, VOID **pBuf)
{
    if (unlikely((dwRingID >= m_dwRingNum)
              || (NULL == pBuf)
              || (NULL == m_apOriRing[dwRingID])))
    {
        return 0;
    }

    WORD32 dwSuccNum  = 0;
    WORD32 dwTotalNum = 0;
    WORD32 dwFreeSize = 0;

    CSTRing &rRing = *m_apOriRing[dwRingID];

    while (dwNum > 0)
    {
        dwSuccNum   = rRing.EnqueueBurst(dwNum, &(pBuf[dwTotalNum]), dwFreeSize);
        dwNum      -= dwSuccNum;
        dwTotalNum += dwSuccNum;

        if (0 == dwFreeSize)
        {
            break ;
        }
    }

    return dwTotalNum;
}


/* һ�ε��ö�λص�, ��Ring�е���Ϣһ���Դ�����
   pObj      : �ص�����
   pFunc     : �ص�����
*/
template <WORD32 POWER_NUM, WORD32 RING_NUM>
inline WORD32 CMultiRing<POWER_NUM, RING_NUM>::Dequeue(
    VOID *pObj, PMsgCallBack  pFunc)
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
template <WORD32 POWER_NUM, WORD32 RING_NUM>
inline WORD32 CMultiRing<POWER_NUM, RING_NUM>::Dequeue(
    WORD32        dwNum,
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


/* һ�ε��ö�λص�, ��Ring�е���Ϣһ���Դ�����; (ʹ��ע��Ķ���) */
template <WORD32 POWER_NUM, WORD32 RING_NUM>
inline WORD32 CMultiRing<POWER_NUM, RING_NUM>::Dequeue()
{
    WORD32 dwTotalNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < m_dwRingNum; dwIndex++)
    {
        CSTRing &rRing = *m_apRing[dwIndex];
        if (rRing.isEmpty())
        {
            continue ;
        }

        VOID   *apBuf[BURST_NUM] = {0, };
        WORD32  dwNum            = 0;
        WORD32  dwRemainSize     = 0;

        do
        {
            dwNum = rRing.DequeueBurst(BURST_NUM, apBuf, dwRemainSize);

            for (WORD32 dwIndex1 = 0; dwIndex1 < dwNum; dwIndex1++)
            {
                __builtin_prefetch(apBuf[dwIndex1], 1, 2);
                (*(rRing.m_pFunc))(rRing.m_pObj, (apBuf[dwIndex1]));
            }

            dwTotalNum += dwNum;
        } while (dwRemainSize > 0);
    }

    return dwTotalNum;
}


/* һ�ε��ö�λص�, ָ���ص�����; (ʹ��ע��Ķ���)
 * a. ��RingBuffer�е���Ϣ��������ָ����dwNumʱ, ����������Ϣ
 * b. ��RingBuffer�е���Ϣ��������ָ����dwNumʱ, ��ָ����dwNum����Ϣ
 * dwNum     : �������ص�����
 * rdwRcvNum : ʵ�ʻص�����
 * ����ֵ: RingBuffer��ʣ�����Ϣ����
 */
template <WORD32 POWER_NUM, WORD32 RING_NUM>
inline WORD32 CMultiRing<POWER_NUM, RING_NUM>::Dequeue(
    WORD32 dwNum, WORD32 &rdwRcvNum)
{
    WORD32 dwRemainSize = 0;
    WORD32 dwRecvCount  = 0;

    rdwRcvNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < m_dwRingNum; dwIndex++)
    {
        CSTRing &rRing = *m_apRing[dwIndex];
        if (rRing.isEmpty())
        {
            continue ;
        }

        VOID   *apBuf[dwNum] = {0, };
        WORD32  dwRemainTmp  = 0;

        dwRecvCount = rRing.DequeueBurst(dwNum, apBuf, dwRemainTmp);

        for (WORD32 dwIndex1 = 0; dwIndex1 < dwRecvCount; dwIndex1++)
        {
            __builtin_prefetch(apBuf[dwIndex1], 1, 2);
            (*(rRing.m_pFunc))(rRing.m_pObj, (apBuf[dwIndex1]));
        }

        dwRemainSize += dwRemainTmp;
        rdwRcvNum    += dwRecvCount;
    }

    return dwRemainSize;
}


template <WORD32 POWER_NUM, WORD32 RING_NUM>
VOID CMultiRing<POWER_NUM, RING_NUM>::SnapShot(WORD32          &rdwRingNum,
                                               T_RingHeadTail  *ptProd,
                                               T_RingHeadTail  *ptCons)
{
    rdwRingNum = MIN(m_dwRingNum, RING_NUM);

    for (WORD32 dwIndex = 0; dwIndex < rdwRingNum; dwIndex++)
    {
        CSTRing        &rRing      = *m_apRing[dwIndex];
        T_RingHeadTail &rtProdItem = *(ptProd + dwIndex);
        T_RingHeadTail &rtConsItem = *(ptCons + dwIndex);

        rRing.SnapShot(rtProdItem, rtConsItem);
    }
}


#define MSG_POWER_NUM           ((WORD32)(10))
#define PACKET_POWER_NUM        ((WORD32)(14))


/* MISO��ʽ����, ������������߳�, һ���������߳� */
typedef CMsgRing<MSG_POWER_NUM>    CMessageRing;

/* ÿ��ST-Ring��SISO��ʽ����, ��ͬ���һ��������MISO��ʽ����; ��: ��ʱ���߳� or ��־�߳� */
typedef CMultiRing<MSG_POWER_NUM>  CMultiMessageRing;


/* ÿ��Data-Ring��SISO��ʽ����, ��ͬ���һ��������MISO��ʽ����; ��: �û����߳� */
typedef CMultiRing<PACKET_POWER_NUM, PACKET_RING_NUM>    CDataPlaneRing;


#endif


