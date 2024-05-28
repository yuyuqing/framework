

#ifndef _BASE_SORT_H_
#define _BASE_SORT_H_


#include "pub_typedef.h"


/* all compare */
template <class T>
using PCompareAFunc = BOOL (*)(T &rLeft, T &rRight);

/* part compare */
template <class T, typename VT>
using PComparePFunc = BOOL (*)(T &rLeft, T &rRight, VT T::* pValue);


template <class T>
inline BOOL Less(T &rLeft, T &rRight)
{
    return rLeft < rRight;
}


template <class T>
inline BOOL Greater(T &rLeft, T &rRight)
{
    return rLeft > rRight;
}


template <class T, typename VT>
inline BOOL LessV(T &rLeft, T &rRight, VT T::* pValue)
{
    // VT vL = rLeft.*pValue;
    // VT vR = rRight.*pValue;
    
    return (rLeft.*pValue) < (rRight.*pValue);
}


template <class T, typename VT>
inline BOOL GreaterV(T &rLeft, T &rRight, VT T::* pValue)
{
    return (rLeft.*pValue) > (rRight.*pValue);
}


template <class T>
inline VOID Swap(T &rLeft, T &rRight)
{
    T tmp = rLeft;

    rLeft  = rRight;
    rRight = tmp;
}


template <typename T>
VOID BubbleSort(T                *pBegin, 
                WORD32            dwNum, 
                PCompareAFunc<T>  pFunc)
{
    if (dwNum <= 1)
    {
        return ;
    }

    for (WORD32 dwI = (dwNum - 1); dwI > 0; dwI--)
    {
        BOOL bFalg = FALSE;

        for (WORD32 dwJ = 0; (dwJ + 1) <= dwI; dwJ++)
        {
            if (TRUE == ((*pFunc) (*(pBegin + dwJ), *(pBegin + dwJ + 1))))
            {
                Swap(*(pBegin + dwJ), *(pBegin + dwJ + 1));
                bFalg = TRUE;
            }
        }

        if (!bFalg)
        {
            break ;
        }
    }
}


template <typename T, typename VT>
VOID BubbleSort(T      *pBegin, 
                WORD32  dwNum, 
                VT T::* pValue, 
                PComparePFunc<T, VT> pFunc)
{
    if (dwNum <= 1)
    {
        return ;
    }

    for (WORD32 dwI = (dwNum - 1); dwI > 0; dwI--)
    {
        BOOL bFalg = FALSE;

        for (WORD32 dwJ = 0; (dwJ + 1) <= dwI; dwJ++)
        {
            if (TRUE == ((*pFunc) (*(pBegin + dwJ), *(pBegin + dwJ + 1), pValue)))
            {
                Swap(*(pBegin + dwJ), *(pBegin + dwJ + 1));
                bFalg = TRUE;
            }
        }

        if (!bFalg)
        {
            break ;
        }
    }
}


template <typename T>
VOID InsertSort(T                *pBegin, 
                WORD32            dwNum, 
                PCompareAFunc<T>  pFunc)
{
    for (SWORD32 dwI = 1; dwI < dwNum; dwI++)
    {
        T tmp = *(pBegin + dwI);
        
        SWORD32 dwJ = (dwI - 1);
        for (; dwJ >= 0; dwJ--)
        {
            if (TRUE == ((*pFunc) (*(pBegin + dwJ), tmp)))
            {
                *(pBegin + dwJ + 1) = *(pBegin + dwJ);
            }
            else
            {
                break ;
            }
        }
        
        *(pBegin + (dwJ+1)) = tmp;
    }
}


template <typename T, typename VT>
VOID InsertSort(T      *pBegin, 
                WORD32  dwNum, 
                VT T::* pValue, 
                PComparePFunc<T, VT> pFunc)
{
    for (SWORD32 dwI = 1; dwI < dwNum; dwI++)
    {
        T tmp = *(pBegin + dwI);
        
        SWORD32 dwJ = (dwI - 1);
        for (; dwJ >= 0; dwJ--)
        {
            if (TRUE == ((*pFunc) (*(pBegin + dwJ), tmp, pValue)))
            {
                *(pBegin + dwJ + 1) = *(pBegin + dwJ);
            }
            else
            {
                break ;
            }
        }
        
        *(pBegin + (dwJ+1)) = tmp;
    }
}


template <typename T>
VOID Push_down(T                *pBegin,
               WORD32            dwStart,
               WORD32            dwNum,
               PCompareAFunc<T>  pFunc)
{
    WORD32 dwLeft    = 2 * dwStart + 1;
    WORD32 dwRight   = 2 * dwStart + 2;
    WORD32 dwLargest = 0;

    if ( (dwLeft < dwNum)
      && ((*pFunc) (*(pBegin + dwLeft), (*(pBegin + dwStart)))))
    {
        dwLargest = dwLeft;
    }
    else
    {
        dwLargest = dwStart;
    }

    if ( (dwRight < dwNum)
      && ((*pFunc) (*(pBegin + dwRight), (*(pBegin + dwLargest)))))
    {
        dwLargest = dwRight;
    }

    if (dwLargest != dwStart)
    {
        Swap(*(pBegin + dwStart), *(pBegin + dwLargest));
        Push_down(pBegin, dwLargest, dwNum, pFunc);
    }    
}


template <typename T>
VOID HeapSort(T                *pBegin, 
              WORD32            dwNum, 
              PCompareAFunc<T>  pFunc)
{
    SWORD32 iIndex = 0;

    for (iIndex = (dwNum / 2 - 1); iIndex >= 0; iIndex--)
    {
        Push_down(pBegin, iIndex, dwNum, pFunc);
    }

    for (iIndex = (dwNum - 1); iIndex > 0; iIndex--)
    {
        Swap(*(pBegin), *(pBegin + iIndex));
        Push_down(pBegin, 0, iIndex, pFunc);
    }
}


template <typename T, typename VT>
VOID Push_down(T       *pBegin,
               WORD32   dwStart,
               WORD32   dwNum,
               VT T::*  pValue,
               PComparePFunc<T, VT>  pFunc)
{
    WORD32 dwLeft    = 2 * dwStart + 1;
    WORD32 dwRight   = 2 * dwStart + 2;
    WORD32 dwLargest = 0;

    if ( (dwLeft < dwNum)
      && ((*pFunc) (*(pBegin + dwLeft), (*(pBegin + dwStart)), pValue)))
    {
        dwLargest = dwLeft;
    }
    else
    {
        dwLargest = dwStart;
    }

    if ( (dwRight < dwNum)
      && ((*pFunc) (*(pBegin + dwRight), (*(pBegin + dwLargest)), pValue)))
    {
        dwLargest = dwRight;
    }

    if (dwLargest != dwStart)
    {
        Swap(*(pBegin + dwStart), *(pBegin + dwLargest));
        Push_down(pBegin, dwLargest, dwNum, pValue, pFunc);
    }    
}


template <typename T, typename VT>
VOID HeapSort(T       *pBegin, 
              WORD32   dwNum,
              VT T::*  pValue,
              PComparePFunc<T, VT>  pFunc)
{
    SWORD32 iIndex = 0;

    for (iIndex = (dwNum / 2 - 1); iIndex >= 0; iIndex--)
    {
        Push_down(pBegin, iIndex, dwNum, pValue, pFunc);
    }

    for (iIndex = (dwNum - 1); iIndex > 0; iIndex--)
    {
        Swap(*(pBegin), *(pBegin + iIndex));
        Push_down(pBegin, 0, iIndex, pValue, pFunc);
    }
}


template <typename T, typename VT>
VOID SelectSort(T      *pBegin, 
                WORD32  dwNum, 
                VT T::* pValue, 
                PComparePFunc<T, VT> pFunc)
{
    for (WORD32 dwI = 0; dwI < dwNum; dwI++)
    {
        for (WORD32 dwJ = (dwI + 1); dwJ <= dwNum; dwJ++)
        {
            if (TRUE == ((*pFunc) (*(pBegin + dwI), *(pBegin + dwJ), pValue)))
            {
                Swap(*(pBegin + dwI), *(pBegin + dwJ));
            }
        }
    }
}


template <typename T>
SWORD32 Partition(T                *pBegin,
                  SWORD32           iLow,
                  SWORD32           iHigh,
                  PCompareAFunc<T>  pFunc)
{
    SWORD32  iPivot       = iHigh;
    SWORD32  iSwapCounter = iLow;

    for (SWORD32 dwI = iLow; dwI < iHigh; dwI++)
    {
        if (TRUE == ((*pFunc) (*(pBegin + dwI), *(pBegin + iPivot))))
        {
            Swap(*(pBegin + dwI), *(pBegin + iSwapCounter));
            iSwapCounter++;
        }
    }

    Swap(*(pBegin + iSwapCounter), *(pBegin + iHigh));

    return iSwapCounter;
}


template <typename T>
VOID QuickSort(T                *pBegin,
               SWORD32           iLow,
               SWORD32           iHigh,
               PCompareAFunc<T>  pFunc)
{
    if (iLow < iHigh)
    {
        SWORD32 iPosition = Partition(pBegin, iLow, iHigh, pFunc);
        QuickSort(pBegin, iLow, (iPosition - 1), pFunc);
        QuickSort(pBegin, (iPosition + 1), iHigh, pFunc);
    }
}


template <typename T, typename VT>
SWORD32 Partition(T                    *pBegin,
                  SWORD32               iLow,
                  SWORD32               iHigh,
                  VT T::*               pValue,
                  PComparePFunc<T, VT>  pFunc)
{
    SWORD32  iPivot       = iHigh;
    SWORD32  iSwapCounter = iLow;

    for (SWORD32 dwI = iLow; dwI < iHigh; dwI++)
    {
        if (TRUE == ((*pFunc) (*(pBegin + dwI), *(pBegin + iPivot), pValue)))
        {
            Swap(*(pBegin + dwI), *(pBegin + iSwapCounter));
            iSwapCounter++;
        }
    }

    Swap(*(pBegin + iSwapCounter), *(pBegin + iHigh));

    return iSwapCounter;
}


template <typename T, typename VT>
VOID QuickSort(T                    *pBegin,
               SWORD32               iLow,
               SWORD32               iHigh,
               VT T::*               pValue,
               PComparePFunc<T, VT>  pFunc)
{
    if (iLow < iHigh)
    {
        SWORD32 iPosition = Partition(pBegin, iLow, iHigh, pValue, pFunc);
        QuickSort(pBegin, iLow, (iPosition - 1), pValue, pFunc);
        QuickSort(pBegin, (iPosition + 1), iHigh, pValue, pFunc);
    }
}


#endif


