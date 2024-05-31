

#ifndef _BASE_RING_PACKET_H_
#define _BASE_RING_PACKET_H_


#include <iostream>

#include "base_lock.h"
#include "base_ring_array.h"


#define PACKET_RING_NUM            ((WORD32)(8))
#define PACKET_THRESHOLD           ((WORD32)(16))
#define PACKET_BURST_NUM           ((WORD32)(32))


template <WORD32 POWER_NUM>
class CPacketRing : public CBaseData
{
public :
    typedef CBaseRingTpl<POWER_NUM>    CDataRing;

    class CPacketObject : public CBaseData
    {
    public :
        VOID          *m_pObj;
        PMsgCallBack   m_pFunc;
        CDataRing      m_cRing;

        CPacketObject ()
        {
            m_pObj  = NULL;
            m_pFunc = NULL;
        }

        virtual ~CPacketObject()
        {
            m_pObj  = NULL;
            m_pFunc = NULL;
        }

        WORD32 Initialize(VOID *pObj, PMsgCallBack pFunc)
        {
            m_pObj  = pObj;
            m_pFunc = pFunc;
            m_cRing.Initialize();
            return SUCCESS;
        }
    };

    typedef CBaseArray<CPacketObject, PACKET_RING_NUM>   CPacketRingArray;

public :
    CPacketRing ();
    virtual ~CPacketRing();

    virtual WORD32 Initialize();

    BOOL isEmpty();

    /* ע�������汨�Ļص����� 
     * pObj      : ע��App����(pObj����Ϊ�ص������ĵ�һ������pArg)
     * pFunc     : ע��ص�����
     * rdwRingID : ����, ���ش����ɹ���RingID(��ΪEnqueue��ָ������)
     */
    CPacketObject * CreatePacketRing(VOID         *pObj,
                                     PMsgCallBack  pFunc,
                                     WORD32       &rdwRingID);

    /* ȥע�������汨�Ļص�����(���밴��ע�������ȥע��) */
    WORD32 DelPacketRing(WORD32 dwRingID);

    WORD32 Count();

    /* ʧ�ܷ���0; �ɹ�����1(������Enqueue�ɹ�����Ϣ����) 
     * dwRingID : ʹ�ô���CreatePacketRing�ӿڵĳ���(rdwRingID)
     * pBuf     : ����ָ��
     */
    WORD32 Enqueue(WORD32 dwRingID, VOID *pBuf);

    /* ����Enqueue�ɹ�����Ϣ���� 
     * dwRingID : ʹ�ô���CreatePacketRing�ӿڵĳ���(rdwRingID)
     * dwNum    : ����ӵı�������
     * pBuf     : ����ָ������
     */
    WORD32 Enqueue(WORD32 dwRingID, WORD32 dwNum, VOID **pBuf);

    /* һ�ε��ö�λص�, ��Ring�е���Ϣһ���Դ����� */
    WORD32 Dequeue();

    /* һ�ε��ö�λص�, ָ���ص�����
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
    CPacketRingArray    m_cArray;

    CAtomicLock         m_cLock;
    volatile WORD32     m_dwRingNum;
    CPacketObject      *m_apRing[PACKET_RING_NUM];
    CPacketObject      *m_apOriRing[PACKET_RING_NUM];
};


template <WORD32 POWER_NUM>
CPacketRing<POWER_NUM>::CPacketRing ()
{
    m_dwRingNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < PACKET_RING_NUM; dwIndex++)
    {
        m_apRing[dwIndex]    = NULL;
        m_apOriRing[dwIndex] = NULL;
    }
}


template <WORD32 POWER_NUM>
CPacketRing<POWER_NUM>::~CPacketRing()
{
    m_dwRingNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < PACKET_RING_NUM; dwIndex++)
    {
        m_apRing[dwIndex]    = NULL;
        m_apOriRing[dwIndex] = NULL;
    }
}


template <WORD32 POWER_NUM>
WORD32 CPacketRing<POWER_NUM>::Initialize()
{
    m_cArray.Initialize();

    return SUCCESS;
}


template <WORD32 POWER_NUM>
inline BOOL CPacketRing<POWER_NUM>::isEmpty()
{
    BOOL bFlag = TRUE;

    for (WORD32 dwIndex = 0; dwIndex < m_dwRingNum; dwIndex++)
    {
        bFlag = m_apRing[dwIndex]->m_cRing.isEmpty();
        if (FALSE == bFlag)
        {
            break ;
        }
    }

    return bFlag;
}


/* ע�������汨�Ļص����� 
 * pObj      : ע��App����(pObj����Ϊ�ص������ĵ�һ������pArg)
 * pFunc     : ע��ص�����
 * rdwRingID : ����, ���ش����ɹ���RingID(��ΪEnqueue��ָ������)
 */
template <WORD32 POWER_NUM>
typename CPacketRing<POWER_NUM>::CPacketObject * 
CPacketRing<POWER_NUM>::CreatePacketRing(VOID         *pObj,
                                         PMsgCallBack  pFunc,
                                         WORD32       &rdwRingID)
{
    m_cLock.Lock();

    rdwRingID = m_dwRingNum % PACKET_RING_NUM;

    if ((NULL != m_apOriRing[rdwRingID]) || (m_dwRingNum >= PACKET_RING_NUM))
    {
        /* �ظ�����Ҳ��assert */
        assert(0);
    }

    CPacketObject *pRing = m_cArray.Create(rdwRingID);
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


/* ȥע�������汨�Ļص�����(���밴��ע�������ȥע��) */
template <WORD32 POWER_NUM>
WORD32 CPacketRing<POWER_NUM>::DelPacketRing(WORD32 dwRingID)
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


template <WORD32 POWER_NUM>
inline WORD32 CPacketRing<POWER_NUM>::Count()
{
    WORD32 dwCount = 0;

    for (WORD32 dwIndex = 0; dwIndex < m_dwRingNum; dwIndex++)
    {
        dwCount += m_apRing[dwIndex]->m_cRing.Count();
    }

    return dwCount;
}


/* ʧ�ܷ���0; �ɹ�����1(������Enqueue�ɹ�����Ϣ����) 
 * dwRingID : ʹ�ô���CreatePacketRing�ӿڵĳ���(rdwRingID)
 * pBuf     : ����ָ��
 */
template <WORD32 POWER_NUM>
inline WORD32 CPacketRing<POWER_NUM>::Enqueue(WORD32 dwRingID, VOID *pBuf)
{
    if (unlikely((dwRingID >= PACKET_RING_NUM)
              || (NULL == pBuf)
              || (NULL == m_apOriRing[dwRingID])))
    {
        return 0;
    }

    return m_apOriRing[dwRingID]->m_cRing.Enqueue(pBuf, PACKET_THRESHOLD);
}


/* ����Enqueue�ɹ�����Ϣ���� 
 * dwRingID : ʹ�ô���CreatePacketRing�ӿڵĳ���(rdwRingID)
 * dwNum    : ����ӵı�������
 * pBuf     : ����ָ������
 */
template <WORD32 POWER_NUM>
inline WORD32 CPacketRing<POWER_NUM>::Enqueue(WORD32 dwRingID, WORD32 dwNum, VOID **pBuf)
{
    if (unlikely((dwRingID >= PACKET_RING_NUM)
              || (NULL == pBuf)
              || (NULL == m_apOriRing[dwRingID])))
    {
        return 0;
    }

    WORD32 dwSuccNum  = 0;
    WORD32 dwTotalNum = 0;
    WORD32 dwFreeSize = 0;

    CDataRing &rRing = m_apOriRing[dwRingID]->m_cRing;

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


/* һ�ε��ö�λص�, ������Ring�е���Ϣһ���Դ����� */
template <WORD32 POWER_NUM>
inline WORD32 CPacketRing<POWER_NUM>::Dequeue()
{
    WORD32 dwTotalNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < m_dwRingNum; dwIndex++)
    {
        CPacketObject &rObj = *m_apRing[dwIndex];

        if (rObj.m_cRing.isEmpty())
        {
            continue ;
        }

        VOID   *apBuf[PACKET_BURST_NUM] = {0, };
        WORD32  dwNum                   = 0;
        WORD32  dwRemainSize            = 0;

        do
        {
            dwNum = rObj.m_cRing.DequeueBurst(PACKET_BURST_NUM, apBuf, dwRemainSize);

            for (WORD32 dwIndex1 = 0; dwIndex1 < dwNum; dwIndex1++)
            {
                __builtin_prefetch(apBuf[dwIndex1], 1, 2);
                (*(rObj.m_pFunc))(rObj.m_pObj, (apBuf[dwIndex1]));
            }

            dwTotalNum += dwNum;
        } while (dwRemainSize > 0);
    }

    return dwTotalNum;
}


/* һ�ε��ö�λص�, ָ���ص�����(���ÿ��Ring�Ļص�����)
 * a. ��RingBuffer�е���Ϣ��������ָ����dwNumʱ, ����������Ϣ
 * b. ��RingBuffer�е���Ϣ��������ָ����dwNumʱ, ��ָ����dwNum����Ϣ
 * dwNum     : �������ص�����
 * rdwRcvNum : ʵ�ʻص�����
 * ����ֵ: RingBuffer��ʣ�����Ϣ����
 */
template <WORD32 POWER_NUM>
inline WORD32 CPacketRing<POWER_NUM>::Dequeue(WORD32 dwNum, WORD32 &rdwRcvNum)
{
    WORD32 dwRemainSize = 0;
    WORD32 dwRecvCount  = 0;

    rdwRcvNum = 0;

    for (WORD32 dwIndex = 0; dwIndex < m_dwRingNum; dwIndex++)
    {
        CPacketObject &rObj = *m_apRing[dwIndex];

        if (rObj.m_cRing.isEmpty())
        {
            continue ;
        }

        VOID   *apBuf[dwNum] = {0, };
        WORD32  dwRemainTmp  = 0;

        dwRecvCount = rObj.m_cRing.DequeueBurst(dwNum, apBuf, dwRemainTmp);

        for (WORD32 dwIndex1 = 0; dwIndex1 < dwRecvCount; dwIndex1++)
        {
            __builtin_prefetch(apBuf[dwIndex1], 1, 2);
            (*(rObj.m_pFunc))(rObj.m_pObj, (apBuf[dwIndex1]));
        }

        dwRemainSize += dwRemainTmp;
        rdwRcvNum    += dwRecvCount;
    }

    return dwRemainSize;
}


template <WORD32 POWER_NUM>
inline VOID CPacketRing<POWER_NUM>::SnapShot(WORD32          &rdwRingNum,
                                             T_RingHeadTail  *ptProd,
                                             T_RingHeadTail  *ptCons)
{
    rdwRingNum = MIN(m_dwRingNum, PACKET_RING_NUM);

    for (WORD32 dwIndex = 0; dwIndex < rdwRingNum; dwIndex++)
    {
        CPacketObject  &rObj       = *m_apRing[dwIndex];
        T_RingHeadTail &rtProdItem = *(ptProd + dwIndex);
        T_RingHeadTail &rtConsItem = *(ptCons + dwIndex);

        rObj.m_cRing.SnapShot(rtProdItem, rtConsItem);
    }
}


#define PACKET_POWER_NUM    ((WORD32)(14))

/* ÿ��Data-Ring��SISO��ʽ����, ��ͬ���һ��������MISO��ʽ����; ��: �û����߳� */
typedef CPacketRing<PACKET_POWER_NUM>    CDataPlaneRing;


#endif


