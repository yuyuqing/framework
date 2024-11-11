

#ifndef _BASE_RING_TPL_H_
#define _BASE_RING_TPL_H_


#include "base_mem_pool.h"
#include "base_ring_message.h"


template <class T, WORD32 POWER_NUM>
class CRingTpl : public CBaseData
{
public :
    static const WORD16 s_wThreshold  = 4;
    static const WORD32 s_dwNodeSize  = sizeof(T);
    static const WORD32 s_dwRingPower = (POWER_NUM + 1);

    typedef CObjectMemPool<POWER_NUM>  CNodeMemPool;
    typedef CMsgRing<s_dwRingPower>    CNodeRing;

public :
    CRingTpl (CCentralMemPool &rCentralMemPool);
    virtual ~CRingTpl();

    virtual WORD32 Initialize();

    T * Malloc();
    WORD32 Free(T *pBuf);

    /* Ring队列消息数量 */
    WORD32 Count();

    /* Ring队列是否为空 */
    BOOL IsEmpty();

    /* 失败返回0; 成功返回1 */
    WORD32 Enqueue(VOID *pBuf, WORD16 wLoopThreshold = s_wThreshold);

    /* 一次调用多次回调, 将Ring中的消息一次性处理完
     * pObj  : 回调对象
     * pFunc : 回调函数
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

protected :
    CNodeMemPool  m_cMemPool;
    CNodeRing     m_cRing;
};


template <class T, WORD32 POWER_NUM>
CRingTpl<T, POWER_NUM>::CRingTpl (CCentralMemPool &rCentralMemPool)
    : m_cMemPool(rCentralMemPool)
{
}


template <class T, WORD32 POWER_NUM>
CRingTpl<T, POWER_NUM>::~CRingTpl()
{
}


template <class T, WORD32 POWER_NUM>
WORD32 CRingTpl<T, POWER_NUM>::Initialize()
{
    m_cMemPool.Initialize(POWER_NUM, s_dwNodeSize);
    m_cRing.Initialize();

    return SUCCESS;
}


template <class T, WORD32 POWER_NUM>
inline T * CRingTpl<T, POWER_NUM>::Malloc()
{
    BYTE *pBuf = m_cMemPool.Malloc(s_dwNodeSize);
    return (T *)pBuf;
}


template <class T, WORD32 POWER_NUM>
inline WORD32 CRingTpl<T, POWER_NUM>::Free(T *pBuf)
{
    return m_cMemPool.Free((BYTE *)pBuf);
}


/* Ring队列消息数量 */
template <class T, WORD32 POWER_NUM>
inline WORD32 CRingTpl<T, POWER_NUM>::Count()
{
    return m_cRing.Count();
}


/* Ring队列是否为空 */
template <class T, WORD32 POWER_NUM>
inline BOOL CRingTpl<T, POWER_NUM>::IsEmpty()
{
    return (0 == m_cRing.Count());
}


/* 失败返回0; 成功返回1 */
template <class T, WORD32 POWER_NUM>
inline WORD32 CRingTpl<T, POWER_NUM>::Enqueue(VOID *pBuf, WORD16 wLoopThreshold)
{
    return m_cRing.Enqueue(pBuf, wLoopThreshold);
}


/* 一次调用多次回调, 将Ring中的消息一次性处理完
 * pObj  : 回调对象
 * pFunc : 回调函数
 */
template <class T, WORD32 POWER_NUM>
inline WORD32 CRingTpl<T, POWER_NUM>::Dequeue(VOID *pObj, PMsgCallBack pFunc)
{
    return m_cRing.Dequeue(pObj, pFunc);
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
template <class T, WORD32 POWER_NUM>
inline WORD32 CRingTpl<T, POWER_NUM>::Dequeue(WORD32        dwNum,
                                              VOID         *pObj,
                                              PMsgCallBack  pFunc,
                                              WORD32       &rdwRcvNum)
{
    return m_cRing.Dequeue(dwNum, pObj, pFunc, rdwRcvNum);
}


#endif


