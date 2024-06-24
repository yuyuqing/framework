

#ifndef _BASE_TIMER_H_
#define _BASE_TIMER_H_


#include "base_call_back.h"
#include "base_time.h"
#include "base_timer_wrapper.h"


class CTimerNode : public CBaseData
{
public :
    CTimerNode ();

    /* ��Զ�ʱ��, dwTick��λ : 0.5ms(��TTI�жϴ���) */
    CTimerNode (WORD32     dwTick,
                CCBObject *pObj,
                PCBFUNC    pFunc,
                WORD32     dwID,
                WORD32     dwExtendID,
                WORD32     dwTransID,
                WORD32     dwResvID,
                VOID      *pContext,
                VOID      *pUserData);

    /* ���Զ�ʱ�� */
    CTimerNode (BYTE       ucHour,
                BYTE       ucMinute,
                BYTE       ucSecond,
                WORD16     wMillSec,
                CCBObject *pObj,
                PCBFUNC    pFunc,
                WORD32     dwID,
                WORD32     dwExtendID,
                WORD32     dwTransID,
                WORD32     dwResvID,
                VOID      *pContext,
                VOID      *pUserData);

    virtual ~CTimerNode();

    CTimerNode & operator = (const CTimerNode &rNode);

    BOOL operator > (CTimerNode &rNode);

    /* �ж��Ƿ�ʱ; (TRUE : ��ʱ, FALSE : δ��ʱ) */
    BOOL IsTimeOut(WORD64 lwCurUs);

    VOID CallBack();

    VOID UpdateClock(WORD64 lwMicroSec, WORD32 dwTick);

public :
    CTimerNode  *m_pNext;
    CTimerNode  *m_pPrev;

protected :
    WORD64          m_lwTimeoutUs;    /* ��ʱʱ��(����ʱ��, ��λ:us) */
    CCBObject      *m_pObj;           /* �ص����� */
    PCBFUNC         m_pFunc;          /* �ص����� */

    T_TimerParam    m_tCBParam;       /* �ص����� */
};


inline CTimerNode & CTimerNode::operator = (const CTimerNode &rNode)
{
    if (&rNode != this)
    {
        m_lwTimeoutUs         = rNode.m_lwTimeoutUs;
        m_pObj                = rNode.m_pObj;
        m_pFunc               = rNode.m_pFunc;
        m_tCBParam.dwID       = rNode.m_tCBParam.dwID;
        m_tCBParam.dwExtendID = rNode.m_tCBParam.dwExtendID;
        m_tCBParam.dwTransID  = rNode.m_tCBParam.dwTransID;
        m_tCBParam.dwResvID   = rNode.m_tCBParam.dwResvID;
        m_tCBParam.pContext   = rNode.m_tCBParam.pContext;
        m_tCBParam.pUserData  = rNode.m_tCBParam.pUserData;
    }

    return *this;
}


inline BOOL CTimerNode::operator > (CTimerNode &rNode)
{
    return (m_lwTimeoutUs > rNode.m_lwTimeoutUs);
}


/* �ж��Ƿ�ʱ; (TRUE : ��ʱ, FALSE : δ��ʱ) */
inline BOOL CTimerNode::IsTimeOut(WORD64 lwCurUs)
{
    return (lwCurUs >= m_lwTimeoutUs);
}


inline VOID CTimerNode::CallBack()
{
    if (likely((NULL != m_pObj) && (NULL != m_pFunc)))
    {
        (m_pObj->*(m_pFunc)) (&(m_tCBParam), sizeof(m_tCBParam));
    }
}


inline VOID CTimerNode::UpdateClock(WORD64 lwMicroSec, WORD32 dwTick)
{
    WORD64 lwTickUs = dwTick * 1000;

    m_lwTimeoutUs = lwMicroSec + lwTickUs;
}


template <WORD32 TIMER_NODE_NUM>
class CTimerList
{
public :
    typedef CBaseDataContainer<CTimerNode, TIMER_NODE_NUM>  CTimerNodeList;

public :
    CTimerList ();
    virtual ~CTimerList();

    virtual WORD32 Initialize();

    /* ��ն�ʱ�� */
    WORD32 Clear();

    /* ��ʱ��ע��ӿ� : ��ʱ��ö�ʱ���Զ�ɾ��, �����ٵ���KillTimerɾ��
     * dwTick : ��ʱʱ��, ��λ(1ms)
     * pObj   : �ص�����
     * pFunc  : �ص�����
     * 
     */
    WORD32 RegisterTimer(WORD32     dwTick,
                         CCBObject *pObj,
                         PCBFUNC    pFunc,
                         WORD32     dwID,
                         WORD32     dwExtendID = INVALID_DWORD,
                         WORD32     dwTransID  = INVALID_DWORD,
                         WORD32     dwResvID   = INVALID_DWORD,
                         VOID      *pContext   = NULL,
                         VOID      *pUserData  = NULL);

    /* ��ʱ��ע��ӿ� : ��ʱ��ö�ʱ���Զ�ɾ��, �����ٵ���KillTimerɾ��
     * dwTick : ��ʱʱ��, ��λ(1ms)
     * pObj   : �ص�����
     * pFunc  : �ص�����
     * 
     */
    WORD32 RegisterTimer(WORD32       dwTick,
                         CTimerNode **pNode,
                         CCBObject   *pObj,
                         PCBFUNC      pFunc,
                         WORD32       dwID,
                         WORD32       dwExtendID = INVALID_DWORD,
                         WORD32       dwTransID  = INVALID_DWORD,
                         WORD32       dwResvID   = INVALID_DWORD,
                         VOID        *pContext   = NULL,
                         VOID        *pUserData  = NULL);

    /* ���Զ�ʱ�� */
    WORD32 RegisterTimer(BYTE       ucHour,
                         BYTE       ucMinute,
                         BYTE       ucSecond,
                         WORD16     wMillSec,
                         CCBObject *pObj,
                         PCBFUNC    pFunc,
                         WORD32     dwID,
                         WORD32     dwExtendID = INVALID_DWORD,
                         WORD32     dwTransID  = INVALID_DWORD,
                         WORD32     dwResvID   = INVALID_DWORD,
                         VOID      *pContext   = NULL,
                         VOID      *pUserData  = NULL);

    WORD32 KillTimer(WORD32 dwTimerID);

    WORD32 Insert(CTimerNode *pNode);

    WORD32 Remove(CTimerNode *pNode);

    WORD32 Decrease(WORD64 lwCurUs, WORD32 dwThreshCount, WORD64 &rlwTimeOutNum);

    WORD32 GetCount();

protected :
    WORD32           m_dwCount;
    CTimerNode      *m_pHeader;
    CTimerNodeList   m_List;
};


template <WORD32 TIMER_NODE_NUM>
CTimerList<TIMER_NODE_NUM>::CTimerList ()
{
    m_dwCount = 0;
    m_pHeader = NULL;
}


template <WORD32 TIMER_NODE_NUM>
CTimerList<TIMER_NODE_NUM>::~CTimerList()
{
    CTimerNode *pCur  = m_pHeader;
    CTimerNode *pNext = NULL;

    while (NULL != pCur)
    {
        pNext         = pCur->m_pNext;
        pCur->m_pNext = NULL;
        pCur->m_pPrev = NULL;

        m_List.Free(pCur);
        delete pCur;

        pCur = pNext;
    }

    m_dwCount = 0;
    m_pHeader = NULL;
}


template <WORD32 TIMER_NODE_NUM>
WORD32 CTimerList<TIMER_NODE_NUM>::Initialize()
{
    m_List.Initialize();

    return SUCCESS;
}


/* ��ն�ʱ�� */
template <WORD32 TIMER_NODE_NUM>
WORD32 CTimerList<TIMER_NODE_NUM>::Clear()
{
    CTimerNode *pCur = m_pHeader;

    while (m_pHeader)
    {
        m_pHeader     = pCur->m_pNext;
        pCur->m_pNext = NULL;
        pCur->m_pPrev = NULL;

        m_List.Free(pCur);
        delete pCur;

        m_dwCount--;
    }

    return SUCCESS;
}


/* ��ʱ��ע��ӿ� : ��ʱ��ö�ʱ���Զ�ɾ��, �����ٵ���KillTimerɾ��
 * dwTick : ��ʱʱ��, ��λ(0.5ms, TTI�ж�)
 * pObj   : �ص�����
 * pFunc  : �ص�����
 * 
 */
template <WORD32 TIMER_NODE_NUM>
inline WORD32 CTimerList<TIMER_NODE_NUM>::RegisterTimer(WORD32     dwTick,
                                                        CCBObject *pObj,
                                                        PCBFUNC    pFunc,
                                                        WORD32     dwID,
                                                        WORD32     dwExtendID,
                                                        WORD32     dwTransID,
                                                        WORD32     dwResvID,
                                                        VOID      *pContext,
                                                        VOID      *pUserData)
{
    if (unlikely((NULL == pObj) || (NULL == pFunc)))
    {
        return INVALID_DWORD;
    }

    WORD32 dwTimerID = INVALID_DWORD;

    CTimerNode *pTimer = m_List.Malloc(dwTimerID);
    if (unlikely(NULL == pTimer))
    {
        return INVALID_DWORD;
    }

    new (pTimer) CTimerNode(dwTick,
                            pObj,
                            pFunc,
                            dwID,
                            dwExtendID,
                            dwTransID,
                            dwResvID,
                            pContext,
                            pUserData);

    Insert(pTimer);

    m_dwCount++;

    return dwTimerID;
}


/* ��ʱ��ע��ӿ� : ��ʱ��ö�ʱ���Զ�ɾ��, �����ٵ���KillTimerɾ��
 * dwTick : ��ʱʱ��, ��λ(1ms)
 * pObj   : �ص�����
 * pFunc  : �ص�����
 * 
 */
template <WORD32 TIMER_NODE_NUM>
inline WORD32 CTimerList<TIMER_NODE_NUM>::RegisterTimer(WORD32       dwTick,
                                                        CTimerNode **pNode,
                                                        CCBObject   *pObj,
                                                        PCBFUNC      pFunc,
                                                        WORD32       dwID,
                                                        WORD32       dwExtendID,
                                                        WORD32       dwTransID,
                                                        WORD32       dwResvID,
                                                        VOID        *pContext,
                                                        VOID        *pUserData)
{
    if (unlikely((NULL == pObj) || (NULL == pFunc)))
    {
        return INVALID_DWORD;
    }

    WORD32 dwTimerID = INVALID_DWORD;

    CTimerNode *pTimer = m_List.Malloc(dwTimerID);
    if (unlikely(NULL == pTimer))
    {
        return INVALID_DWORD;
    }

    new (pTimer) CTimerNode(dwTick,
                            pObj,
                            pFunc,
                            dwID,
                            dwExtendID,
                            dwTransID,
                            dwResvID,
                            pContext,
                            pUserData);

    Insert(pTimer);

    m_dwCount++;

    *pNode = pTimer;

    return dwTimerID;
}


template <WORD32 TIMER_NODE_NUM>
inline WORD32 CTimerList<TIMER_NODE_NUM>::RegisterTimer(BYTE       ucHour,
                                                        BYTE       ucMinute,
                                                        BYTE       ucSecond,
                                                        WORD16     wMillSec,
                                                        CCBObject *pObj,
                                                        PCBFUNC    pFunc,
                                                        WORD32     dwID,
                                                        WORD32     dwExtendID,
                                                        WORD32     dwTransID,
                                                        WORD32     dwResvID,
                                                        VOID      *pContext,
                                                        VOID      *pUserData)
{
    if (unlikely((NULL == pObj)
              || (NULL == pFunc)))
    {
        return INVALID_DWORD;
    }

    WORD32 dwTimerID = INVALID_DWORD;

    CTimerNode *pNode = m_List.Malloc(dwTimerID);
    if (unlikely(NULL == pNode))
    {
        return INVALID_DWORD;
    }

    new (pNode) CTimerNode(ucHour, ucMinute, ucSecond, wMillSec, 
                           pObj, pFunc, 
                           dwID, dwExtendID, dwTransID, dwResvID,
                           pContext, pUserData);

    Insert(pNode);
    
    m_dwCount++;

    return dwTimerID;
}


template <WORD32 TIMER_NODE_NUM>
inline WORD32 CTimerList<TIMER_NODE_NUM>::KillTimer(WORD32 dwTimerID)
{
    if (unlikely(dwTimerID >= TIMER_NODE_NUM))
    {
        return FAIL;
    }

    CTimerNode *pNode = (m_List(dwTimerID));
    CTimerNode *pPrev = NULL;
    CTimerNode *pNext = NULL;

    if (NULL == pNode)
    {
        return FAIL;
    }

    pPrev = pNode->m_pPrev;
    pNext = pNode->m_pNext;

    if (pPrev == NULL)    /* pNode��ͷ��� */
    {
        m_pHeader = pNext;
    }
    else                  /* pNode����ͷ��� */
    {
        pPrev->m_pNext = pNext;
    }

    if (pNext == NULL)    /* pNode��β��� */
    {
    }
    else                  /* pNode����β��� */
    {
        pNext->m_pPrev = pPrev;
    }

    /* ɾ��pNode�ڵ� */
    pNode->m_pNext = NULL;
    pNode->m_pPrev = NULL;
    m_List.Free(pNode);
    delete pNode;

    m_dwCount--;

    return SUCCESS;
}


template <WORD32 TIMER_NODE_NUM>
inline WORD32 CTimerList<TIMER_NODE_NUM>::Insert(CTimerNode *pNode)
{
    if (unlikely(NULL == pNode))
    {
        return FAIL;
    }

    CTimerNode *pCur  = m_pHeader;
    CTimerNode *pPrev = NULL;

    while (pCur)
    {
        if ((*pNode) > (*pCur))
        {
            pPrev = pCur;
            pCur  = pCur->m_pNext;
        }
        else
        {
            break ;
        }
    }

    pNode->m_pNext = pCur;
    pNode->m_pPrev = pPrev;

    if (NULL == pPrev)
    {
        m_pHeader = pNode;
    }
    else
    {
        pPrev->m_pNext = pNode;
    }

    if (NULL != pCur)
    {
        pCur->m_pPrev = pNode;
    }

    return SUCCESS;
}


template <WORD32 TIMER_NODE_NUM>
inline WORD32 CTimerList<TIMER_NODE_NUM>::Remove(CTimerNode *pNode)
{
    if (unlikely(NULL == pNode))
    {
        return FAIL;
    }

    CTimerNode *pPrev = pNode->m_pPrev;
    CTimerNode *pNext = pNode->m_pNext;

    if (pPrev == NULL)    /* pNode��ͷ��� */
    {
        m_pHeader = pNext;
    }
    else                  /* pNode����ͷ��� */
    {
        pPrev->m_pNext = pNext;
    }

    if (pNext == NULL)    /* pNode��β��� */
    {
    }
    else                  /* pNode����β��� */
    {
        pNext->m_pPrev = pPrev;
    }

    /* ɾ��pNode�ڵ� */
    pNode->m_pNext = NULL;
    pNode->m_pPrev = NULL;

    return SUCCESS;
}


template <WORD32 TIMER_NODE_NUM>
inline WORD32 CTimerList<TIMER_NODE_NUM>::Decrease(WORD64  lwCurUs,
                                                   WORD32  dwThreshCount,
                                                   WORD64 &rlwTimeOutNum)
{
    CTimerNode *pCur = m_pHeader;

    rlwTimeOutNum = 0;

    while (pCur)
    {
        /* ���ƶ�ʱ����ʱ����, �������Ļص����º�ʱ����Ԥ�� */
        if (rlwTimeOutNum >= dwThreshCount)
        {
            break ;
        }

        if (TRUE == pCur->IsTimeOut(lwCurUs))
        {
            rlwTimeOutNum++;

            pCur->CallBack();

            m_pHeader     = pCur->m_pNext;
            pCur->m_pNext = NULL;

            m_List.Free(pCur);
            delete pCur;

            pCur = m_pHeader;
            if (pCur)
            {
                pCur->m_pPrev = NULL;
            }

            m_dwCount--;
        }
        else
        {
            break ;
        }
    }

    return SUCCESS;
}


template <WORD32 TIMER_NODE_NUM>
inline WORD32 CTimerList<TIMER_NODE_NUM>::GetCount()
{
    return m_dwCount;
}


#endif


