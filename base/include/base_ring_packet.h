

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

    /* 注册数据面报文回调函数 
     * pObj      : 注册App对象(pObj将作为回调函数的第一个出参pArg)
     * pFunc     : 注册回调函数
     * rdwRingID : 出参, 返回创建成功的RingID(作为Enqueue的指定参数)
     */
    CPacketObject * CreatePacketRing(VOID         *pObj,
                                     PMsgCallBack  pFunc,
                                     WORD32       &rdwRingID);

    /* 去注册数据面报文回调函数(必须按照注册的逆序去注册) */
    WORD32 DelPacketRing(WORD32 dwRingID);

    WORD32 Count();

    /* 失败返回0; 成功返回1(即返回Enqueue成功的消息条数) 
     * dwRingID : 使用创建CreatePacketRing接口的出参(rdwRingID)
     * pBuf     : 报文指针
     */
    WORD32 Enqueue(WORD32 dwRingID, VOID *pBuf);

    /* 返回Enqueue成功的消息条数 
     * dwRingID : 使用创建CreatePacketRing接口的出参(rdwRingID)
     * dwNum    : 待入队的报文数量
     * pBuf     : 报文指针数组
     */
    WORD32 Enqueue(WORD32 dwRingID, WORD32 dwNum, VOID **pBuf);

    /* 一次调用多次回调, 将Ring中的消息一次性处理完 */
    WORD32 Dequeue();

    /* 一次调用多次回调, 指定回调次数
     * a. 当RingBuffer中的消息数量少于指定的dwNum时, 读完所有消息
     * b. 当RingBuffer中的消息数量大于指定的dwNum时, 读指定的dwNum条消息
     * dwNum     : 控制最大回调次数
     * rdwRcvNum : 实际回调次数
     * 返回值: RingBuffer中剩余的消息数量
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


/* 注册数据面报文回调函数 
 * pObj      : 注册App对象(pObj将作为回调函数的第一个出参pArg)
 * pFunc     : 注册回调函数
 * rdwRingID : 出参, 返回创建成功的RingID(作为Enqueue的指定参数)
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
        /* 重复创建也会assert */
        assert(0);
    }

    CPacketObject *pRing = m_cArray.Create(rdwRingID);
    if (NULL == pRing)
    {
        assert(0);
    }

    pRing->Initialize(pObj, pFunc);

    m_apRing[m_dwRingNum] = pRing;

    /* 内存屏障 */
#ifdef ARCH_ARM64
    asm volatile("dmb " "ishld" : : : "memory");  /* 针对Arm64指令 */
#else
    asm volatile ("" : : : "memory");             /* 针对X86指令 */
#endif

    m_dwRingNum++;

    /* 内存屏障 */
#ifdef ARCH_ARM64
    asm volatile("dmb " "ishld" : : : "memory");  /* 针对Arm64指令 */
#else
    asm volatile ("" : : : "memory");             /* 针对X86指令 */
#endif

    m_apOriRing[rdwRingID] = pRing;

    m_cLock.UnLock();

    return pRing;
}


/* 去注册数据面报文回调函数(必须按照注册的逆序去注册) */
template <WORD32 POWER_NUM>
WORD32 CPacketRing<POWER_NUM>::DelPacketRing(WORD32 dwRingID)
{
    m_cLock.Lock();

    if (dwRingID != (m_dwRingNum - 1))
    {
        assert(0);
    }

    m_apOriRing[dwRingID] = NULL;

    /* 内存屏障 */
#ifdef ARCH_ARM64
    asm volatile("dmb " "ishld" : : : "memory");  /* 针对Arm64指令 */
#else
    asm volatile ("" : : : "memory");             /* 针对X86指令 */
#endif

    m_dwRingNum--;

    /* 内存屏障 */
#ifdef ARCH_ARM64
    asm volatile("dmb " "ishld" : : : "memory");  /* 针对Arm64指令 */
#else
    asm volatile ("" : : : "memory");             /* 针对X86指令 */
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


/* 失败返回0; 成功返回1(即返回Enqueue成功的消息条数) 
 * dwRingID : 使用创建CreatePacketRing接口的出参(rdwRingID)
 * pBuf     : 报文指针
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


/* 返回Enqueue成功的消息条数 
 * dwRingID : 使用创建CreatePacketRing接口的出参(rdwRingID)
 * dwNum    : 待入队的报文数量
 * pBuf     : 报文指针数组
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


/* 一次调用多次回调, 将所有Ring中的消息一次性处理完 */
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


/* 一次调用多次回调, 指定回调次数(针对每个Ring的回调次数)
 * a. 当RingBuffer中的消息数量少于指定的dwNum时, 读完所有消息
 * b. 当RingBuffer中的消息数量大于指定的dwNum时, 读指定的dwNum条消息
 * dwNum     : 控制最大回调次数
 * rdwRcvNum : 实际回调次数
 * 返回值: RingBuffer中剩余的消息数量
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

/* 每个Data-Ring以SISO方式运行, 共同组成一个集合以MISO方式运行; 如: 用户面线程 */
typedef CPacketRing<PACKET_POWER_NUM>    CDataPlaneRing;


#endif


