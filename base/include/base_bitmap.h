

#ifndef _BASE_BITMAP_H_
#define _BASE_BITMAP_H_


#include "pub_typedef.h"


#define BIT_NUM_PER_BYTE    ((WORD32)(8))
#define BIT_NUM_PER_DWORD   ((WORD32)(32))
#define BIT_NUM_PER_LWORD   ((WORD32)(64))
#define BYTE_NUM_PER_MB     ((WORD64)(1024 * 1024))


extern const BYTE s_aucBitCount[256];
extern const BYTE s_aucFirstOne[256];


template <BYTE BM_BYTE_NUM>
class CBaseBitMapTpl
{
public :
    enum { BITMAP_BYTE_NUM     = BM_BYTE_NUM };
    enum { BITMAP_BIT_NUM      = BM_BYTE_NUM * BIT_NUM_PER_BYTE };    
    enum { BITMAP_MATCH_SYMBOL = 0xFFFF };

public :
    CBaseBitMapTpl();

    /* dwBitIndex : 第几个bit位置位(0 ~ (BITMAP_BIT_NUM - 1)) */
    CBaseBitMapTpl(WORD32 dwBitIndex);
    
    virtual ~CBaseBitMapTpl();
    
    CBaseBitMapTpl(const CBaseBitMapTpl &rBitMap);
    CBaseBitMapTpl(CBaseBitMapTpl &rBitMap);

    CBaseBitMapTpl & operator = (const CBaseBitMapTpl &rBitMap);
    CBaseBitMapTpl & operator = (CBaseBitMapTpl &rBitMap);

    /* dwBitIndex 取值小于 BITMAP_BIT_NUM */
    BOOL operator & (WORD32 dwBitIndex);

    /* 对所有字节设置为指定Value */
    VOID SetAllValue(BYTE ucValue);

    /* dwBitIndex 取值小于 BITMAP_BIT_NUM */
    VOID SetBitMap(WORD32 dwBitIndex);    

    /* dwBitIndex 取值小于 BITMAP_BIT_NUM */
    VOID RemoveBitMap(WORD32 dwBitIndex);

    VOID ClearBitMap();

    WORD32 FindConsecutiveBit1(WORD32 dwReqBitLen, WORD32 dwTotalBitLen);

    VOID SetConsecutiveBit0(WORD32 dwStartPos, WORD32 dwReqResBitLen);

protected :
    BYTE   m_aucBitMap[BITMAP_BYTE_NUM];
};


template <BYTE BM_BYTE_NUM>
CBaseBitMapTpl<BM_BYTE_NUM>::CBaseBitMapTpl()
{
    for (WORD32 dwIndex = 0; dwIndex < BITMAP_BYTE_NUM; dwIndex++)
    {
        m_aucBitMap[dwIndex] = 0;
    }
}


/* dwBitIndex : 第几个bit位置位(0 ~ (BITMAP_BIT_NUM - 1)) */
template <BYTE BM_BYTE_NUM>
CBaseBitMapTpl<BM_BYTE_NUM>::CBaseBitMapTpl(WORD32 dwBitIndex)
{
    for (WORD32 dwIndex = 0; dwIndex < BITMAP_BYTE_NUM; dwIndex++)
    {
        m_aucBitMap[dwIndex] = 0;
    }

    SetBitMap(dwBitIndex);
}


template <BYTE BM_BYTE_NUM>
CBaseBitMapTpl<BM_BYTE_NUM>::~CBaseBitMapTpl()
{
    for (WORD32 dwIndex = 0; dwIndex < BITMAP_BYTE_NUM; dwIndex++)
    {
        m_aucBitMap[dwIndex] = 0;
    }
}


template <BYTE BM_BYTE_NUM>
CBaseBitMapTpl<BM_BYTE_NUM>::CBaseBitMapTpl(const CBaseBitMapTpl<BM_BYTE_NUM> &rBitMap)
{
    for (WORD32 dwIndex = 0; dwIndex < BITMAP_BYTE_NUM; dwIndex++)
    {
        m_aucBitMap[dwIndex] = rBitMap.m_aucBitMap[dwIndex];
    }
}


template <BYTE BM_BYTE_NUM>
CBaseBitMapTpl<BM_BYTE_NUM>::CBaseBitMapTpl(CBaseBitMapTpl<BM_BYTE_NUM> &rBitMap)
{
    for (WORD32 dwIndex = 0; dwIndex < BITMAP_BYTE_NUM; dwIndex++)
    {
        m_aucBitMap[dwIndex] = rBitMap.m_aucBitMap[dwIndex];
    }
}


template <BYTE BM_BYTE_NUM>
inline CBaseBitMapTpl<BM_BYTE_NUM> & CBaseBitMapTpl<BM_BYTE_NUM>::operator = (const CBaseBitMapTpl<BM_BYTE_NUM> &rBitMap)
{
    if (&rBitMap != this)
    {
        for (WORD32 dwIndex = 0; dwIndex < BITMAP_BYTE_NUM; dwIndex++)
        {
            m_aucBitMap[dwIndex] = rBitMap.m_aucBitMap[dwIndex];
        }
    }

    return *this;
}


template <BYTE BM_BYTE_NUM>
inline CBaseBitMapTpl<BM_BYTE_NUM> & CBaseBitMapTpl<BM_BYTE_NUM>::operator = (CBaseBitMapTpl<BM_BYTE_NUM> &rBitMap)
{
    if (&rBitMap != this)
    {
        for (WORD32 dwIndex = 0; dwIndex < BITMAP_BYTE_NUM; dwIndex++)
        {
            m_aucBitMap[dwIndex] = rBitMap.m_aucBitMap[dwIndex];
        }
    }

    return *this;
}


template <BYTE BM_BYTE_NUM>
inline BOOL CBaseBitMapTpl<BM_BYTE_NUM>::operator & (WORD32 dwBitIndex)
{
    /* 优先匹配通配符 */
    if (BITMAP_MATCH_SYMBOL == dwBitIndex)
    {
        return TRUE;
    }

    if (unlikely(dwBitIndex >= BITMAP_BIT_NUM))
    {
        return FALSE;
    }
    
    WORD32 dwPos   = dwBitIndex / BIT_NUM_PER_BYTE;
    BYTE   ucValue = (BYTE)(1 << (dwBitIndex % BIT_NUM_PER_BYTE));

    return (0 != (m_aucBitMap[dwPos] & ucValue));
}


/* 对所有字节设置为指定Value */
template <BYTE BM_BYTE_NUM>
inline VOID CBaseBitMapTpl<BM_BYTE_NUM>::SetAllValue(BYTE ucValue)
{
    memset(m_aucBitMap, ucValue, sizeof(m_aucBitMap));
}


template <BYTE BM_BYTE_NUM>
inline VOID CBaseBitMapTpl<BM_BYTE_NUM>::SetBitMap(WORD32 dwBitIndex)
{
    if (unlikely(dwBitIndex >= BITMAP_BIT_NUM))
    {
        return ;
    }
    
    WORD32 dwPos   = dwBitIndex / BIT_NUM_PER_BYTE;
    BYTE   ucValue = (BYTE)(1 << (dwBitIndex % BIT_NUM_PER_BYTE));

    m_aucBitMap[dwPos] |= ucValue;
}


template <BYTE BM_BYTE_NUM>
inline VOID CBaseBitMapTpl<BM_BYTE_NUM>::RemoveBitMap(WORD32 dwBitIndex)
{
    if (unlikely(dwBitIndex >= BITMAP_BIT_NUM))
    {
        return ;
    }
    
    WORD32 dwPos   = dwBitIndex / BIT_NUM_PER_BYTE;
    BYTE   ucValue = (BYTE)(1 << (dwBitIndex % BIT_NUM_PER_BYTE));

    m_aucBitMap[dwPos] &= (~ucValue);
}


template <BYTE BM_BYTE_NUM>
inline VOID CBaseBitMapTpl<BM_BYTE_NUM>::ClearBitMap()
{
    memset(m_aucBitMap, 0x00, BITMAP_BYTE_NUM);
}


template <BYTE BM_BYTE_NUM>
inline WORD32 CBaseBitMapTpl<BM_BYTE_NUM>::FindConsecutiveBit1(
    WORD32 dwReqBitLen,
    WORD32 dwTotalBitLen)
{
    BOOL   bFlag        = FALSE;
    WORD32 dwStartPos   = 0;
    WORD32 dwFillBitNum = 0;
    WORD32 dwPos        = 0;
    BYTE   ucValue      = 0;

    for (WORD32 dwIndex = 0; dwIndex < dwTotalBitLen; dwIndex++)
    {
        dwPos   = dwIndex / BIT_NUM_PER_BYTE;
        ucValue = (BYTE)(1 << (dwIndex % BIT_NUM_PER_BYTE));

        if (0 != (m_aucBitMap[dwPos] & ucValue))
        {
            dwFillBitNum++;

            if (dwFillBitNum >= dwReqBitLen)
            {
                bFlag = TRUE;
                break ;
            }
        }
        else
        {
            dwStartPos   = dwIndex + 1;
            dwFillBitNum = 0;
        }
    }

    return bFlag ? dwStartPos : INVALID_DWORD;
}


template <BYTE BM_BYTE_NUM>
inline VOID CBaseBitMapTpl<BM_BYTE_NUM>::SetConsecutiveBit0(
    WORD32 dwStartPos, WORD32 dwReqResBitLen)
{
    WORD32  dwBitIdx = 0;
    WORD32  dwPos    = 0;
    BYTE    ucValue  = 0;

    for (WORD32 dwIndex = 0; dwIndex < dwReqResBitLen; dwIndex++)
    {
        dwBitIdx = dwIndex + dwStartPos;
        dwPos    = dwBitIdx / BIT_NUM_PER_BYTE;
        ucValue  = (BYTE)(1 << (dwBitIdx % BIT_NUM_PER_BYTE));

        m_aucBitMap[dwPos] &= (~ucValue);
    }
}


#define BITMAP_BYTE_NUM_02   ((BYTE)(2))
#define BITMAP_BYTE_NUM_32   ((BYTE)(32))


class CBaseBitMap02 : public CBaseBitMapTpl<BITMAP_BYTE_NUM_02>
{
public :
    VOID Dump(const CHAR *pName);
};


class CBaseBitMap32 : public CBaseBitMapTpl<BITMAP_BYTE_NUM_32>
{
public :
    VOID Dump(const CHAR *pName);
};


#endif


