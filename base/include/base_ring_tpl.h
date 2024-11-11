

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

    /* Ring������Ϣ���� */
    WORD32 Count();

    /* Ring�����Ƿ�Ϊ�� */
    BOOL IsEmpty();

    /* ʧ�ܷ���0; �ɹ�����1 */
    WORD32 Enqueue(VOID *pBuf, WORD16 wLoopThreshold = s_wThreshold);

    /* һ�ε��ö�λص�, ��Ring�е���Ϣһ���Դ�����
     * pObj  : �ص�����
     * pFunc : �ص�����
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


/* Ring������Ϣ���� */
template <class T, WORD32 POWER_NUM>
inline WORD32 CRingTpl<T, POWER_NUM>::Count()
{
    return m_cRing.Count();
}


/* Ring�����Ƿ�Ϊ�� */
template <class T, WORD32 POWER_NUM>
inline BOOL CRingTpl<T, POWER_NUM>::IsEmpty()
{
    return (0 == m_cRing.Count());
}


/* ʧ�ܷ���0; �ɹ�����1 */
template <class T, WORD32 POWER_NUM>
inline WORD32 CRingTpl<T, POWER_NUM>::Enqueue(VOID *pBuf, WORD16 wLoopThreshold)
{
    return m_cRing.Enqueue(pBuf, wLoopThreshold);
}


/* һ�ε��ö�λص�, ��Ring�е���Ϣһ���Դ�����
 * pObj  : �ص�����
 * pFunc : �ص�����
 */
template <class T, WORD32 POWER_NUM>
inline WORD32 CRingTpl<T, POWER_NUM>::Dequeue(VOID *pObj, PMsgCallBack pFunc)
{
    return m_cRing.Dequeue(pObj, pFunc);
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
template <class T, WORD32 POWER_NUM>
inline WORD32 CRingTpl<T, POWER_NUM>::Dequeue(WORD32        dwNum,
                                              VOID         *pObj,
                                              PMsgCallBack  pFunc,
                                              WORD32       &rdwRcvNum)
{
    return m_cRing.Dequeue(dwNum, pObj, pFunc, rdwRcvNum);
}


#endif


