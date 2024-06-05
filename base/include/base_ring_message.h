

#ifndef _BASE_RING_MESSAGE_H_
#define _BASE_RING_MESSAGE_H_


#include <iostream>

#include "base_lock.h"
#include "base_ring_array.h"
#include "base_variable.h"


#define BURST_NUM        ((WORD32)(32))


/* 回调消息处理 */
using PMsgCallBack = WORD32 (*)(VOID *pArg, VOID *pMsg);


template <WORD32 POWER_NUM>
class CMsgRing : public CBaseRingTpl <POWER_NUM>
{
public :
    CMsgRing ();
    virtual ~CMsgRing();

    virtual WORD32 Initialize();

    /* 失败返回0; 成功返回1 */
    WORD32 Enqueue(VOID *pBuf, WORD16 wLoopThreshold);

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

    /* 仅在业务线程创建时调用 */
    CSTRing * CreateSTRing(WORD32 dwRingID);

    /* 失败返回0; 成功返回1 */
    WORD32 Enqueue(VOID *pBuf, WORD16 wLoopThreshold);

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
        /* 重复创建也会assert */
        assert(0);
    }

    CSTRing *pRing = m_cArray.Create(dwQueueID);
    if (NULL == pRing)
    {
        assert(0);
    }

    pRing->Initialize();

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


/* 失败返回0; 成功返回1 */
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


/* 一次调用多次回调, 将Ring中的消息一次性处理完
   pObj      : 回调对象
   pFunc     : 回调函数
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

/* MISO方式运行, 即多个生产者线程, 一个消费者线程 */
typedef CMsgRing<MSG_POWER_NUM>    CMessageRing;

/* 每个ST-Ring以SISO方式运行, 共同组成一个集合以MISO方式运行; 如: 定时器线程 or 日志线程 */
typedef CMultiRing<MSG_POWER_NUM>  CMultiMessageRing;


#endif


