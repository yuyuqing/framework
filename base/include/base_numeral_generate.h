

#ifndef _BASE_NUMERAL_GENERATE_H_
#define _BASE_NUMERAL_GENERATE_H_


#include "base_ring_array.h"


typedef struct tagT_NumeralNode
{
    BOOL    bFree;
    WORD32  dwValue;
}T_NumeralNode;


template <WORD32 POWER_NUM>
class CNumeralArray
{
public :
    static const WORD32 s_dwNumeralNum = (1 << POWER_NUM);
    static const WORD32 s_dwRingPower  = (POWER_NUM + 1);

    typedef CSimpleRing<s_dwRingPower>  CNumeralRing;

public :
    CNumeralArray ();
    virtual ~CNumeralArray();

    WORD32 Initialize();

    WORD32 Generate();
    WORD32 Retrieve(WORD32 dwValue);

protected :
    T_NumeralNode  m_atNode[s_dwNumeralNum];
    CNumeralRing   m_cRing;
};


template <WORD32 POWER_NUM>
CNumeralArray<POWER_NUM>::CNumeralArray ()
{
}


template <WORD32 POWER_NUM>
CNumeralArray<POWER_NUM>::~CNumeralArray()
{
}


template <WORD32 POWER_NUM>
WORD32 CNumeralArray<POWER_NUM>::Initialize()
{
    m_cRing.Initialize();

    for (WORD32 dwIndex = 0; dwIndex < s_dwNumeralNum; dwIndex++)
    {
        m_atNode[dwIndex].bFree   = TRUE;
        m_atNode[dwIndex].dwValue = dwIndex;

        m_cRing.Enqueue((VOID *)(&(m_atNode[dwIndex])));
    }

    return SUCCESS;
}


template <WORD32 POWER_NUM>
WORD32 CNumeralArray<POWER_NUM>::Generate()
{
    WORD32         dwNum = 0;
    T_NumeralNode *pObj  = NULL;

    dwNum = m_cRing.Dequeue((VOID **)(&pObj));
    if (0 == dwNum)
    {
        return INVALID_DWORD;
    }

    pObj->bFree = FALSE;

    return pObj->dwValue;
}


template <WORD32 POWER_NUM>
WORD32 CNumeralArray<POWER_NUM>::Retrieve(WORD32 dwValue)
{
    if (dwValue >= s_dwNumeralNum)
    {
        return FAIL;
    }

    T_NumeralNode *pObj = &(m_atNode[dwValue]);
    if (TRUE == pObj->bFree)
    {
        return FAIL;
    }

    pObj->bFree = TRUE;

    m_cRing.Enqueue((VOID *)pObj);

    return SUCCESS;
}


#endif


