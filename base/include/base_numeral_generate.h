

#ifndef _BASE_NUMERAL_GENERATE_H_
#define _BASE_NUMERAL_GENERATE_H_


#include "base_ring_array.h"


typedef struct tagT_NumeralNode
{
    volatile BOOL    bFree;
    volatile WORD32  dwValue;
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
inline WORD32 CNumeralArray<POWER_NUM>::Generate()
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
inline WORD32 CNumeralArray<POWER_NUM>::Retrieve(WORD32 dwValue)
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


template <WORD32 POWER_NUM>
class CNumeralGenerator
{
public :
    static const WORD32 s_dwNumeralNum = (1 << POWER_NUM);
    static const WORD32 s_dwRingPower  = (POWER_NUM + 1);

    typedef CSimpleRing<s_dwRingPower>  CNumeralRing;

public :
    CNumeralGenerator ();
    virtual ~CNumeralGenerator();

    WORD32 Initialize(WORD32 dwStartValue, WORD32 dwMaxNum);

    WORD32 Generate();
    WORD32 Retrieve(WORD32 dwValue);

protected :
    WORD32         m_dwStartValue;
    WORD32         m_dwMaxNum;

    T_NumeralNode  m_atNode[s_dwNumeralNum];
    CNumeralRing   m_cRing;
};


template <WORD32 POWER_NUM>
CNumeralGenerator<POWER_NUM>::CNumeralGenerator ()
{
    m_dwStartValue = 0;
    m_dwMaxNum     = s_dwNumeralNum;
}


template <WORD32 POWER_NUM>
CNumeralGenerator<POWER_NUM>::~CNumeralGenerator()
{
    m_dwStartValue = 0;
    m_dwMaxNum     = s_dwNumeralNum;
}


template <WORD32 POWER_NUM>
WORD32 CNumeralGenerator<POWER_NUM>::Initialize(WORD32 dwStartValue, WORD32 dwMaxNum)
{
    m_dwStartValue = dwStartValue;
    m_dwMaxNum     = MIN(dwMaxNum, s_dwNumeralNum);

    m_cRing.Initialize();

    for (WORD32 dwIndex = 0; dwIndex < m_dwMaxNum; dwIndex++)
    {
        m_atNode[dwIndex].bFree   = TRUE;
        m_atNode[dwIndex].dwValue = dwIndex;

        m_cRing.Enqueue((VOID *)(&(m_atNode[dwIndex])));
    }

    return SUCCESS;
}


template <WORD32 POWER_NUM>
inline WORD32 CNumeralGenerator<POWER_NUM>::Generate()
{
    WORD32         dwNum = 0;
    T_NumeralNode *pObj  = NULL;

    dwNum = m_cRing.Dequeue((VOID **)(&pObj));
    if (0 == dwNum)
    {
        return INVALID_DWORD;
    }

    pObj->bFree = FALSE;

    return (pObj->dwValue + m_dwStartValue);
}


template <WORD32 POWER_NUM>
inline WORD32 CNumeralGenerator<POWER_NUM>::Retrieve(WORD32 dwValue)
{
    WORD32 dwSucces = 0;
    BOOL   bOldFree = FALSE;
    BOOL   bNewFree = TRUE;

    dwValue -= m_dwStartValue;

    if (unlikely(dwValue >= m_dwMaxNum))
    {
        return FAIL;
    }

    T_NumeralNode *pObj = &(m_atNode[dwValue]);

    /* 若bFree取值与bOldFree相等, 则将bNewFree的值写入bFree */
    /* 若bFree取值与bOldFree不等, 则将bFree的值写入bOldFree */
    dwSucces = __atomic_compare_exchange_n(&(pObj->bFree),
                                           &bOldFree,
                                           bNewFree,
                                           0,
                                           __ATOMIC_RELAXED,
                                           __ATOMIC_RELAXED);
    if (0 == dwSucces)
    {
        return FAIL;
    }

    m_cRing.Enqueue((VOID *)pObj);

    return SUCCESS;
}


#endif


