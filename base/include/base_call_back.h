

#ifndef _BASE_CALL_BACK_H_
#define _BASE_CALL_BACK_H_


#include "base_data_array.h"


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


template <WORD32 CB_NUM>
class CEventMap : public CCBObject
{
public :
    class CCallBackItem
    {
    public :
        WORD32     dwSignal;
        CCBObject *pObj;
        PCBFUNC    pFunc;

        CCallBackItem ()
        {
            dwSignal = INVALID_DWORD;
            pObj     = NULL;
            pFunc    = NULL;
        }

        ~CCallBackItem ()
        {
            dwSignal = INVALID_DWORD;
            pObj     = NULL;
            pFunc    = NULL;
        }
    };

    typedef CBaseArray<CCallBackItem, CB_NUM > CCallBackSequence;

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
    WORD32 dwIndex = dwSignal % CB_NUM;

    CCallBackItem *pItem = m_Sequence.Create(dwIndex);
    if (NULL == pItem)
    {
        assert(0);
    }

    pItem->dwSignal = dwSignal;
    pItem->pObj     = pObj;
    pItem->pFunc    = pFunc;

    return SUCCESS;
}


template <WORD32 CB_NUM>
inline WORD32 CEventMap<CB_NUM>::ProcessEvent(WORD32      dwSignal,
                                              const VOID *pIn,
                                              WORD32      dwLen)
{
    WORD32 dwIndex = dwSignal % CB_NUM;

    CCallBackItem *pItem = m_Sequence(dwIndex);
    if (unlikely(NULL == pItem))
    {
        return FAIL;
    }

    if (unlikely((dwSignal != pItem->dwSignal)
              || (NULL == pItem->pObj)
              || (NULL == pItem->pFunc)))
    {
        return FAIL;
    }

    (pItem->pObj->*(pItem->pFunc)) (pIn, dwLen);

    return SUCCESS;
}


template <WORD32 CB_NUM>
VOID CEventMap<CB_NUM>::RemoveProcessor(WORD32 dwSignal)
{
    WORD32 dwIndex = dwSignal % CB_NUM;

    m_Sequence.Delete(dwIndex);
}


#endif


