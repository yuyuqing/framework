

#ifndef _BASE_CALL_BACK_H_
#define _BASE_CALL_BACK_H_


#include <assert.h>
#include "base_data_container.h"


class CCBObject;


typedef VOID (CCBObject::*PCBFUNC)(const VOID *pIn, WORD32 dwLen);
typedef VOID (CCBObject::*PHook) (...);
typedef VOID (CCBObject::*PDFUNC)(VOID *pData);


/* CallBack Object */
class CCBObject
{
public :
    virtual ~CCBObject () {}

    template <typename... CallArgs>
    VOID operator() (PHook pFunc, CallArgs&&... args)
    {
        (this->*(pFunc)) (args...);
    }
};


template <class T, WORD32 CB_NUM>
class CEventContainer
{
public :
    class CEventItem
    {
    public:
        WORD32  m_dwSignal;
        BOOL    m_bFree;
        T       m_tData;

        CEventItem()
        {
            m_bFree = TRUE;
        }

        ~CEventItem()
        {
            m_bFree = TRUE;
        }

        operator T& ()
        {
            return m_tData;
        }

        operator T* ()
        {
            return &m_tData;
        }
    };

public :
    CEventContainer ();
    virtual ~CEventContainer();

    WORD32 Initialize();

    T * CreateInst(WORD32 dwSignal, const T &rItem);

    T * FindInst(WORD32 dwSignal);
    
    WORD32 DelInst(WORD32 dwSignal);

protected :
    BYTE  m_aucData[sizeof(CEventItem) * CB_NUM];
};


template <class T, WORD32 CB_NUM>
CEventContainer<T, CB_NUM>::CEventContainer ()
{
}


template <class T, WORD32 CB_NUM>
CEventContainer<T, CB_NUM>::~CEventContainer()
{
}


template <class T, WORD32 CB_NUM>
WORD32 CEventContainer<T, CB_NUM>::Initialize()
{
    CEventItem *pCur = NULL;

    for (WORD32 dwIndex = 0; dwIndex < CB_NUM; dwIndex++)
    {
        pCur = (CEventItem *)(&(m_aucData[sizeof(CEventItem) * dwIndex]));

        pCur->m_dwSignal = INVALID_DWORD;
        pCur->m_bFree    = TRUE;
    }

    return SUCCESS;
}


template <class T, WORD32 CB_NUM>
inline T * CEventContainer<T, CB_NUM>::CreateInst(WORD32 dwSignal, const T &rItem)
{
    WORD32 dwIndex = dwSignal % CB_NUM;

    CEventItem *pInst = (CEventItem *)(&(m_aucData[sizeof(CEventItem) * dwIndex]));
    if (FALSE == pInst->m_bFree)
    {
        assert(0);
    }

    pInst->m_dwSignal = dwSignal;
    pInst->m_bFree    = FALSE;

    new (&(pInst->m_tData)) T(rItem);

    return &(pInst->m_tData);
}


template <class T, WORD32 CB_NUM>
inline T * CEventContainer<T, CB_NUM>::FindInst(WORD32 dwSignal)
{
    WORD32 dwIndex = dwSignal % CB_NUM;

    CEventItem *pInst = (CEventItem *)(&(m_aucData[sizeof(CEventItem) * dwIndex]));
    if (unlikely((TRUE == pInst->m_bFree) || (dwSignal != pInst->m_dwSignal)))
    {
        return NULL;
    }

    return &(pInst->m_tData);
}


template <class T, WORD32 CB_NUM>
inline WORD32 CEventContainer<T, CB_NUM>::DelInst(WORD32 dwSignal)
{
    WORD32 dwIndex = dwSignal % CB_NUM;

    CEventItem *pInst = (CEventItem *)(&(m_aucData[sizeof(CEventItem) * dwIndex]));
    if (TRUE == pInst->m_bFree)
    {
        assert(0);
    }

    pInst->m_dwSignal = INVALID_DWORD;
    pInst->m_bFree    = TRUE;

    delete (&(pInst->m_tData));

    return SUCCESS;
}


template <WORD32 CB_NUM>
class CEventMap : public CCBObject
{
public :
    class CCallBackItem : public CBaseData
    {
    public :
        CCBObject *pObj;
        PCBFUNC    pFunc;

        CCallBackItem ()
        {
            pObj  = NULL;
            pFunc = NULL;
        }

        CCallBackItem (const CCallBackItem &rItem)
        {
            pObj  = rItem.pObj;
            pFunc = rItem.pFunc;
        }

        virtual ~CCallBackItem ()
        {
            pObj  = NULL;
            pFunc = NULL;
        }
    };

    typedef CEventContainer<CCallBackItem, CB_NUM > CCallBackSequence;

public :
    CEventMap ();
    virtual ~CEventMap();

    virtual WORD32 Initialize();
    
    WORD32 RegisterProcessor(WORD32     dwSignal,
                             CCBObject *pObj,
                             PCBFUNC    pFunc);

    WORD32 ProcessEvent(WORD32      dwSignal,
                        const VOID *pIn,
                        WORD32      dwLen);

    VOID RemoveProcessor(WORD32 dwSignal);
    
private :
    CCallBackSequence   m_Sequence;
};


template <WORD32 CB_NUM>
CEventMap<CB_NUM>::CEventMap ()
{
}


template <WORD32 CB_NUM>
CEventMap<CB_NUM>::~CEventMap()
{
}


template <WORD32 CB_NUM>
WORD32 CEventMap<CB_NUM>::Initialize()
{
    return m_Sequence.Initialize();
}


template <WORD32 CB_NUM>
WORD32 CEventMap<CB_NUM>::RegisterProcessor(WORD32     dwSignal,
                                            CCBObject *pObj,
                                            PCBFUNC    pFunc)
{
    CCallBackItem cItem;
    cItem.pObj  = pObj;
    cItem.pFunc = pFunc;

    CCallBackItem *pNew = m_Sequence.CreateInst(dwSignal, cItem);
    if (NULL == pNew)
    {
        return FAIL;
    }

    return SUCCESS;
}


template <WORD32 CB_NUM>
inline WORD32 CEventMap<CB_NUM>::ProcessEvent(WORD32      dwSignal,
                                              const VOID *pIn,
                                              WORD32      dwLen)
{
    CCallBackItem *pCur = m_Sequence.FindInst(dwSignal);
    if (unlikely(NULL == pCur))
    {
        return FAIL;
    }

    if (unlikely((NULL == pCur->pObj) || (NULL == pCur->pFunc)))
    {
        return FAIL;
    }

    (pCur->pObj->*(pCur->pFunc)) (pIn, dwLen);

    return SUCCESS;
}


template <WORD32 CB_NUM>
VOID CEventMap<CB_NUM>::RemoveProcessor(WORD32 dwSignal)
{
    m_Sequence.DelInst(dwSignal);
}


#endif


