

#ifndef _BASE_RING_MESSAGE_H_
#define _BASE_RING_MESSAGE_H_


#include <iostream>

#include "base_lock.h"
#include "base_ring.h"


#define BURST_NUM                    ((WORD32)(32))
#define SINGLE_RING_THRESHOLD        ((WORD32)(16))
#define SINGLE_RING_NUM              ((WORD32)(128))
#define PACKET_RING_NUM              ((WORD32)(8))


/* 回调消息处理 */
using PMsgCallBack = WORD32 (*)(VOID *pArg, VOID *pMsg);


template <WORD32 POWER_NUM>
class CMsgRing : public CBaseRingTpl <POWER_NUM>
{
public :
    CMsgRing ();
    virtual ~CMsgRing();

    virtual WORD32 Initialize();

    /* 一次调用多次回调, 将Ring中的消息一次性处理完
       pObj  : 回调对象
       pFunc : 回调函数
    */
    WORD32 Dequeue(VOID *pObj, PMsgCallBack pFunc);

    /* 一次调用多次回调, 指定回调次数
       a. 当RingBuffer中的消息数量少于指定的dwNum时, 读完所有消息
       b. 当RingBuffer中的消息数量大于指定的dwNum时, 读指定的dwNum条消息
       dwNum     : 控制最大回调次数
       pObj      : 回调对象
       pFunc     : 回调函数
       rdwRcvNum : 实际回调次数
       返回值: RingBuffer中剩余的消息数量
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


/* 一次调用多次回调, 将Ring中的消息一次性处理完
   pObj  : 回调对象
   pFunc : 回调函数
   返回值: 回调的消息数量
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


/* 一次调用多次回调, 指定回调次数
   a. 当RingBuffer中的消息数量少于指定的dwNum时, 读完所有消息
   b. 当RingBuffer中的消息数量大于指定的dwNum时, 读指定的dwNum条消息
   dwNum     : 控制最大回调次数
   pObj      : 回调对象
   pFunc     : 回调函数
   rdwRcvNum : 实际回调次数
   返回值: RingBuffer中剩余的消息数量
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

    /* 仅在业务线程创建时调用 */
    CSTRing * CreateSTRing(WORD32 dwRingID);

    /* 注册数据面报文回调函数 
     * pObj      : 注册App对象(pObj将作为回调函数的第一个出参pArg)
     * pFunc     : 注册回调函数
     * rdwRingID : 出参, 返回创建成功的RingID(作为Enqueue的指定参数)
     */
    CSTRing * CreateSTRing(VOID         *pObj,
                           PMsgCallBack  pFunc,
                           WORD32       &rdwRingID);

    /* 必须按照创建的逆序删除 */
    WORD32 DelSTRing(WORD32 dwRingID);

    /* 失败返回0; 成功返回1 */
    WORD32 Enqueue(WORD32 dwRingID, VOID *pBuf, WORD16 wLoopThreshold = SINGLE_RING_THRESHOLD);

    /* 返回Enqueue成功的消息条数 
     * dwRingID : 使用创建CreatePacketRing接口的出参(rdwRingID)
     * dwNum    : 待入队的报文数量
     * pBuf     : 报文指针数组
     */
    WORD32 Enqueue(WORD32 dwRingID, WORD32 dwNum, VOID **pBuf);

    /* 一次调用多次回调, 将Ring中的消息一次性处理完
       pObj      : 回调对象
       pFunc     : 回调函数
    */
    WORD32 Dequeue(VOID *pObj, PMsgCallBack pFunc);

    /* 一次调用多次回调, 指定回调次数
       a. 当RingBuffer中的消息数量少于指定的dwNum时, 读完所有消息
       b. 当RingBuffer中的消息数量大于指定的dwNum时, 读指定的dwNum条消息
       dwNum     : 控制最大回调次数
       pObj      : 回调对象
       pFunc     : 回调函数
       rdwRcvNum : 实际回调次数
       返回值: RingBuffer中剩余的消息数量
    */
    WORD32 Dequeue(WORD32        dwNum,
                   VOID         *pObj,
                   PMsgCallBack  pFunc,
                   WORD32       &rdwRcvNum);

    /* 一次调用多次回调, 将Ring中的消息一次性处理完; (使用注册的对象) */
    WORD32 Dequeue();

    /* 一次调用多次回调, 指定回调次数; (使用注册的对象)
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
        /* 重复创建也会assert */
        assert(0);
    }

    CSTRing *pRing = m_cArray.Create(dwQueueID);
    if (NULL == pRing)
    {
        assert(0);
    }

    pRing->Initialize(NULL, NULL);

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

    m_apOriRing[dwQueueID] = pRing;

    m_cLock.UnLock();

    return pRing;
}


/* 注册数据面报文回调函数 
 * pObj      : 注册App对象(pObj将作为回调函数的第一个出参pArg)
 * pFunc     : 注册回调函数
 * rdwRingID : 出参, 返回创建成功的RingID(作为Enqueue的指定参数)
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
        /* 重复创建也会assert */
        assert(0);
    }

    CSTRing *pRing = m_cArray.Create(rdwRingID);
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


/* 必须按照创建的逆序删除 */
template <WORD32 POWER_NUM, WORD32 RING_NUM>
WORD32 CMultiRing<POWER_NUM, RING_NUM>::DelSTRing(WORD32 dwRingID)
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


/* 失败返回0; 成功返回1 */
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


/* 返回Enqueue成功的消息条数 
 * dwRingID : 使用创建CreatePacketRing接口的出参(rdwRingID)
 * dwNum    : 待入队的报文数量
 * pBuf     : 报文指针数组
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


/* 一次调用多次回调, 将Ring中的消息一次性处理完
   pObj      : 回调对象
   pFunc     : 回调函数
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


/* 一次调用多次回调, 指定回调次数
   a. 当RingBuffer中的消息数量少于指定的dwNum时, 读完所有消息
   b. 当RingBuffer中的消息数量大于指定的dwNum时, 读指定的dwNum条消息
   dwNum     : 控制最大回调次数
   pObj      : 回调对象
   pFunc     : 回调函数
   rdwRcvNum : 实际回调次数
   返回值: RingBuffer中剩余的消息数量
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


/* 一次调用多次回调, 将Ring中的消息一次性处理完; (使用注册的对象) */
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


/* 一次调用多次回调, 指定回调次数; (使用注册的对象)
 * a. 当RingBuffer中的消息数量少于指定的dwNum时, 读完所有消息
 * b. 当RingBuffer中的消息数量大于指定的dwNum时, 读指定的dwNum条消息
 * dwNum     : 控制最大回调次数
 * rdwRcvNum : 实际回调次数
 * 返回值: RingBuffer中剩余的消息数量
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


/* MISO方式运行, 即多个生产者线程, 一个消费者线程 */
typedef CMsgRing<MSG_POWER_NUM>    CMessageRing;

/* 每个ST-Ring以SISO方式运行, 共同组成一个集合以MISO方式运行; 如: 定时器线程 or 日志线程 */
typedef CMultiRing<MSG_POWER_NUM>  CMultiMessageRing;


/* 每个Data-Ring以SISO方式运行, 共同组成一个集合以MISO方式运行; 如: 用户面线程 */
typedef CMultiRing<PACKET_POWER_NUM, PACKET_RING_NUM>    CDataPlaneRing;


#endif


