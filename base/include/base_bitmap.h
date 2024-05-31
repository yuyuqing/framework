

#ifndef _BASE_BITMAP_H_
#define _BASE_BITMAP_H_


#include "pub_typedef.h"


#define BIT_NUM_PER_BYTE    ((WORD32)(8))
#define BYTE_NUM_PER_MB     ((WORD64)(1024 * 1024))


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

    /* dwBitIndex 取值小于 BITMAP_BIT_NUM */
    VOID SetBitMap(WORD32 dwBitIndex);    

    /* dwBitIndex 取值小于 BITMAP_BIT_NUM */
    VOID RemoveBitMap(WORD32 dwBitIndex);

    VOID ClearBitMap();

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


