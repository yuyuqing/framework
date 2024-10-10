

#ifndef _BASE_BITSET_H_
#define _BASE_BITSET_H_


#include <iostream>

#include "base_bitmap.h"


#define BITSET_DWORD(N)    \
    ((N) < 1 ? 1 : (((N) + BIT_NUM_PER_DWORD - 1) / BIT_NUM_PER_DWORD))

#define BITSET_LWORD(N)    \
    ((N) < 1 ? 1 : (((N) + BIT_NUM_PER_LWORD - 1) / BIT_NUM_PER_LWORD))


template <WORD32 BIT_NUM>
class CBaseBitSetTpl
{
public :
    enum { BITSET_DWORD_NUM = BITSET_DWORD(BIT_NUM) };
    enum { BITSET_LWORD_NUM = BITSET_LWORD(BIT_NUM) };

    static WORD32 WhichWord(WORD32 dwPos)
    {
        return dwPos / BIT_NUM_PER_DWORD;
    }

    static WORD32 WhichByte(WORD32 dwPos)
    {
        return (dwPos % BIT_NUM_PER_DWORD) / BIT_NUM_PER_BYTE;
    }

    static WORD32 WhichBit(WORD32 dwPos)
    {
        return dwPos % BIT_NUM_PER_DWORD;
    }

    static WORD32 MaskBit(WORD32 dwPos)
    {
        return (WORD32)(1 << WhichBit(dwPos));
    }

    template<WORD32 ExtraBits>
    static VOID DoSanitize(WORD32 &rdwVal)
    {
        rdwVal &= ~((~static_cast<WORD32>(0)) << ExtraBits);
    }

    class CReference;
    friend class CReference;

    class CReference
    {
    public :
        friend class CBaseBitSetTpl;

        CReference(CBaseBitSetTpl &rBitSet, WORD32 dwPos)
        {
            m_pdwBitSet = &rBitSet.GetWord(dwPos);
            m_dwBitPos  = CBaseBitSetTpl<BIT_NUM>::WhichBit(dwPos);
        }

        ~CReference() {}

        CReference & operator=(BOOL bVal)
        {
            if (bVal)
            {
                *m_pdwBitSet |= CBaseBitSetTpl<BIT_NUM>::MaskBit(m_dwBitPos);
            }
            else
            {
                *m_pdwBitSet &= ~CBaseBitSetTpl<BIT_NUM>::MaskBit(m_dwBitPos);
            }

            return *this;
        }

        CReference & operator=(const CReference &rRef)
        {
            if ( (*(rRef.m_pdwBitSet) & CBaseBitSetTpl<BIT_NUM>::MaskBit(rRef.m_dwBitPos)) )
            {
                *m_pdwBitSet |= CBaseBitSetTpl<BIT_NUM>::MaskBit(m_dwBitPos);
            }
            else
            {
                *m_pdwBitSet &= ~CBaseBitSetTpl<BIT_NUM>::MaskBit(m_dwBitPos);
            }

            return *this;
        }

        BOOL operator~() const
        {
            return (*(m_pdwBitSet) & CBaseBitSetTpl<BIT_NUM>::MaskBit(m_dwBitPos)) == 0;
        }

        operator bool() const
        {
            return (*(m_pdwBitSet) & CBaseBitSetTpl<BIT_NUM>::MaskBit(m_dwBitPos)) != 0;
        }

        CReference & Flip()
        {
            *m_pdwBitSet ^= CBaseBitSetTpl<BIT_NUM>::MaskBit(m_dwBitPos);
            return *this;
        }

    protected :
        CReference();

    protected :
        WORD32    *m_pdwBitSet;
        WORD32     m_dwBitPos;
    };

public :
    CBaseBitSetTpl ();
    CBaseBitSetTpl (WORD32 dwVal);
    virtual ~CBaseBitSetTpl();

    WORD32 & GetWord(WORD32 dwPos);
    WORD32   GetWord(WORD32 dwPos) const;
    WORD32 & GetHigh();
    WORD32   GetHigh() const;

    /* Prev bit位的左1bit位取值 */
    BOOL LeftBit(WORD32 dwPrev);

    /* 从低到高查找第一个bit位为1的bit(返回该bit的Pos), 未找到时返回无效值 */
    WORD32 FindFirst();

    /* 从低到高查找第一个bit位为0的bit(返回该bit的Pos), 未找到时返回无效值 */
    WORD32 FindFirst0();

    /* 从Prev bit位开始, 从低到高查找下一个bit位为1的bit(返回该bit的Pos), 未找到时返回无效值 */
    WORD32 FindNext(WORD32 dwPrev);

    /* 从Prev bit位开始, 从低到高查找下一个bit位为0的bit(返回该bit的Pos), 未找到时返回无效值 */
    WORD32 FindNext0(WORD32 dwPrev);

    /* 从Prev bit位开始, 从低到高查找连续bit位为1的最后1个bit(返回该bit的Pos), 未找到时返回dwPrev */
    WORD32 FindLast(WORD32 dwPrev);

    /* 从Prev bit位开始, 从低到高查找连续bit位为0的最后1个bit(返回该bit的Pos), 未找到时返回dwPrev */
    WORD32 FindLast0(WORD32 dwPrev);

    CBaseBitSetTpl<BIT_NUM> & operator&=(const CBaseBitSetTpl<BIT_NUM> &rBitSet);
    CBaseBitSetTpl<BIT_NUM> & operator|=(const CBaseBitSetTpl<BIT_NUM> &rBitSet);
    CBaseBitSetTpl<BIT_NUM> & operator^=(const CBaseBitSetTpl<BIT_NUM> &rBitSet);
    CBaseBitSetTpl<BIT_NUM> & operator<<=(WORD32 dwShift);
    CBaseBitSetTpl<BIT_NUM> & operator>>=(WORD32 dwShift);

    /* 设置bit位为1 */
    CBaseBitSetTpl<BIT_NUM> & Set(WORD32 dwPos);

    /* 设置bit位为指定值 */
    CBaseBitSetTpl<BIT_NUM> & Set(WORD32 dwPos, BOOL bVal);

    /* 设置bit位为0 */
    CBaseBitSetTpl<BIT_NUM> & ReSet(WORD32 dwPos);

    /* 设置bit位为取反 */
    CBaseBitSetTpl<BIT_NUM> & Flip(WORD32 dwPos);

    /* 按位为取反 */
    CBaseBitSetTpl<BIT_NUM> & Flip();

    CReference operator[] (WORD32 dwPos)
    {
        return CReference(*this, dwPos);
    }

    BOOL operator[] (WORD32 dwPos) const;
    BOOL operator & (WORD32 dwPos) const;
    BOOL operator==(const CBaseBitSetTpl<BIT_NUM> &rBitSet) const;
    BOOL operator!=(const CBaseBitSetTpl<BIT_NUM> &rBitSet) const;
    CBaseBitSetTpl<BIT_NUM> operator~() const;
    CBaseBitSetTpl<BIT_NUM> operator<<(WORD32 dwShift) const;
    CBaseBitSetTpl<BIT_NUM> operator>>(WORD32 dwShift) const;

    WORD32 Count();
    BOOL IsAny();

private :
    VOID Santitize();
    VOID DoFlip();
    VOID DoSet();
    VOID DoReSet();

protected :
    WORD32  m_adwBitSet[BITSET_DWORD_NUM];
};


template <WORD32 BIT_NUM>
CBaseBitSetTpl<BIT_NUM>::CBaseBitSetTpl ()
{
    DoReSet();
}


template <WORD32 BIT_NUM>
CBaseBitSetTpl<BIT_NUM>::CBaseBitSetTpl (WORD32 dwVal)
{
    DoReSet();
    m_adwBitSet[0] = dwVal;
    Santitize();
}


template <WORD32 BIT_NUM>
CBaseBitSetTpl<BIT_NUM>::~CBaseBitSetTpl()
{
}


template <WORD32 BIT_NUM>
inline WORD32 & CBaseBitSetTpl<BIT_NUM>::GetWord(WORD32 dwPos)
{
    return m_adwBitSet[CBaseBitSetTpl<BIT_NUM>::WhichWord(dwPos)];
}


template <WORD32 BIT_NUM>
inline WORD32 CBaseBitSetTpl<BIT_NUM>::GetWord(WORD32 dwPos) const
{
    return m_adwBitSet[CBaseBitSetTpl<BIT_NUM>::WhichWord(dwPos)];
}


template <WORD32 BIT_NUM>
inline WORD32 & CBaseBitSetTpl<BIT_NUM>::GetHigh()
{
    return m_adwBitSet[BITSET_DWORD_NUM - 1];
}


template <WORD32 BIT_NUM>
inline WORD32 CBaseBitSetTpl<BIT_NUM>::GetHigh() const
{
    return m_adwBitSet[BITSET_DWORD_NUM - 1];
}


/* Prev bit位的左1bit位取值 */
template <WORD32 BIT_NUM>
inline BOOL CBaseBitSetTpl<BIT_NUM>::LeftBit(WORD32 dwPrev)
{
    dwPrev++;
    if (dwPrev >= BIT_NUM)
    {
        return FALSE;
    }

    WORD32 dwPos = CBaseBitSetTpl<BIT_NUM>::WhichWord(dwPrev);
    WORD32 dwBit = CBaseBitSetTpl<BIT_NUM>::WhichBit(dwPrev);
    WORD32 dwVal = m_adwBitSet[dwPos];

    dwVal >>= dwBit;

    return (BOOL)(dwVal & 0x00000001);
}


/* 从低到高查找第一个bit位为1的bit(返回该bit的Pos), 未找到时返回无效值 */
template <WORD32 BIT_NUM>
inline WORD32 CBaseBitSetTpl<BIT_NUM>::FindFirst()
{
    WORD32 dwCurVal = 0;
    BYTE   ucCurVal = 0;

    for (WORD32 dwIndex = 0; dwIndex < BITSET_DWORD_NUM; dwIndex++)
    {
        dwCurVal = m_adwBitSet[dwIndex];
        if (dwCurVal != 0)
        {
            for (WORD32 dwIndex1 = 0; dwIndex1 < sizeof(WORD32); dwIndex1++)
            {
                ucCurVal = (BYTE)(dwCurVal & 0x000000FF);

                if (ucCurVal)
                {
                    return (dwIndex * BIT_NUM_PER_DWORD)
                         + (dwIndex1 * BIT_NUM_PER_BYTE)
                         + s_aucFirstOne[ucCurVal];
                }

                dwCurVal >>= BIT_NUM_PER_BYTE;
            }
        }
    }

    return INVALID_DWORD;
}


/* 从低到高查找第一个bit位为0的bit(返回该bit的Pos), 未找到时返回无效值 */
template <WORD32 BIT_NUM>
inline WORD32 CBaseBitSetTpl<BIT_NUM>::FindFirst0()
{
    WORD32 dwCurVal = 0;
    BYTE   ucCurVal = 0;

    for (WORD32 dwIndex = 0; dwIndex < BITSET_DWORD_NUM; dwIndex++)
    {
        dwCurVal = ~m_adwBitSet[dwIndex];
        if (dwCurVal != 0)
        {
            for (WORD32 dwIndex1 = 0; dwIndex1 < sizeof(WORD32); dwIndex1++)
            {
                ucCurVal = (BYTE)(dwCurVal & 0x000000FF);

                if (ucCurVal)
                {
                    return (dwIndex * BIT_NUM_PER_DWORD)
                         + (dwIndex1 * BIT_NUM_PER_BYTE)
                         + s_aucFirstOne[ucCurVal];
                }

                dwCurVal >>= BIT_NUM_PER_BYTE;
            }
        }
    }

    return INVALID_DWORD;
}


/* 从Prev bit位开始, 从低到高查找下一个bit位为1的bit(返回该bit的Pos), 未找到时返回无效值 */
template <WORD32 BIT_NUM>
inline WORD32 CBaseBitSetTpl<BIT_NUM>::FindNext(WORD32 dwPrev)
{
    dwPrev++;
    if (dwPrev >= BIT_NUM)
    {
        return INVALID_DWORD;
    }

    WORD32 dwIndex  = CBaseBitSetTpl<BIT_NUM>::WhichWord(dwPrev);
    WORD32 dwIndex1 = 0;
    WORD32 dwCurVal = m_adwBitSet[dwIndex];
    BYTE   ucCurVal = 0;

    dwCurVal &= (~static_cast<WORD32>(0)) << CBaseBitSetTpl<BIT_NUM>::WhichBit(dwPrev);
    if (dwCurVal != 0)
    {
        dwIndex1   = CBaseBitSetTpl<BIT_NUM>::WhichByte(dwPrev);
        dwCurVal >>= dwIndex1 * BIT_NUM_PER_BYTE;

        for (; dwIndex1 < sizeof(WORD32); dwIndex1++)
        {
            ucCurVal = (BYTE)(dwCurVal & 0x000000FF);

            if (ucCurVal)
            {
                return (dwIndex * BIT_NUM_PER_DWORD)
                     + (dwIndex1 * BIT_NUM_PER_BYTE)
                     + s_aucFirstOne[ucCurVal];
            }

            dwCurVal >>= BIT_NUM_PER_BYTE;
        }
    }

    dwIndex++;
    for (; dwIndex < BITSET_DWORD_NUM; dwIndex++)
    {
        dwCurVal = m_adwBitSet[dwIndex];
        if (dwCurVal != 0)
        {
            for (dwIndex1 = 0; dwIndex1 < sizeof(WORD32); dwIndex1++)
            {
                ucCurVal = (BYTE)(dwCurVal & 0x000000FF);

                if (ucCurVal)
                {
                    return (dwIndex * BIT_NUM_PER_DWORD)
                         + (dwIndex1 * BIT_NUM_PER_BYTE)
                         + s_aucFirstOne[ucCurVal];
                }

                dwCurVal >>= BIT_NUM_PER_BYTE;
            }
        }
    }

    return INVALID_DWORD;
}


/* 从Prev bit位开始, 从低到高查找下一个bit位为0的bit(返回该bit的Pos), 未找到时返回无效值 */
template <WORD32 BIT_NUM>
inline WORD32 CBaseBitSetTpl<BIT_NUM>::FindNext0(WORD32 dwPrev)
{
    dwPrev++;
    if (dwPrev >= BIT_NUM)
    {
        return INVALID_DWORD;
    }

    WORD32 dwIndex  = CBaseBitSetTpl<BIT_NUM>::WhichWord(dwPrev);
    WORD32 dwIndex1 = 0;
    WORD32 dwCurVal = ~m_adwBitSet[dwIndex];
    BYTE   ucCurVal = 0;

    dwCurVal &= (~static_cast<WORD32>(0)) << CBaseBitSetTpl<BIT_NUM>::WhichBit(dwPrev);
    if (dwCurVal != 0)
    {
        dwIndex1   = CBaseBitSetTpl<BIT_NUM>::WhichByte(dwPrev);
        dwCurVal >>= dwIndex1 * BIT_NUM_PER_BYTE;

        for (; dwIndex1 < sizeof(WORD32); dwIndex1++)
        {
            ucCurVal = (BYTE)(dwCurVal & 0x000000FF);

            if (ucCurVal)
            {
                return (dwIndex * BIT_NUM_PER_DWORD)
                     + (dwIndex1 * BIT_NUM_PER_BYTE)
                     + s_aucFirstOne[ucCurVal];
            }

            dwCurVal >>= BIT_NUM_PER_BYTE;
        }
    }

    dwIndex++;
    for (; dwIndex < BITSET_DWORD_NUM; dwIndex++)
    {
        dwCurVal = ~m_adwBitSet[dwIndex];
        if (dwCurVal != 0)
        {
            for (dwIndex1 = 0; dwIndex1 < sizeof(WORD32); dwIndex1++)
            {
                ucCurVal = (BYTE)(dwCurVal & 0x000000FF);

                if (ucCurVal)
                {
                    return (dwIndex * BIT_NUM_PER_DWORD)
                         + (dwIndex1 * BIT_NUM_PER_BYTE)
                         + s_aucFirstOne[ucCurVal];
                }

                dwCurVal >>= BIT_NUM_PER_BYTE;
            }
        }
    }

    return INVALID_DWORD;
}


/* 从Prev bit位开始, 从低到高查找连续bit位为1的最后1个bit(返回该bit的Pos), 未找到时返回dwPrev */
template <WORD32 BIT_NUM>
inline WORD32 CBaseBitSetTpl<BIT_NUM>::FindLast(WORD32 dwPrev)
{
    dwPrev++;
    if (dwPrev >= BIT_NUM)
    {
        return (dwPrev - 1);
    }

    WORD32 dwIndex  = CBaseBitSetTpl<BIT_NUM>::WhichWord(dwPrev);
    WORD32 dwIndex1 = 0;
    WORD32 dwEndPos = 0;
    WORD32 dwCurVal = m_adwBitSet[dwIndex];
    BYTE   ucCurVal = 0;

    dwCurVal  = ~dwCurVal;
    dwCurVal &= (~static_cast<WORD32>(0)) << CBaseBitSetTpl<BIT_NUM>::WhichBit(dwPrev);

    if (dwCurVal != 0)
    {
        dwIndex1   = CBaseBitSetTpl<BIT_NUM>::WhichByte(dwPrev);
        dwCurVal >>= dwIndex1 * BIT_NUM_PER_BYTE;

        for (; dwIndex1 < sizeof(WORD32); dwIndex1++)
        {
            ucCurVal = (BYTE)(dwCurVal & 0x000000FF);

            if (ucCurVal)
            {
                dwEndPos = (dwIndex * BIT_NUM_PER_DWORD)
                         + (dwIndex1 * BIT_NUM_PER_BYTE)
                         + s_aucFirstOne[ucCurVal];

                return dwEndPos ? (dwEndPos - 1) : 0;
            }

            dwCurVal >>= BIT_NUM_PER_BYTE;
        }
    }

    dwIndex++;
    for (; dwIndex < BITSET_DWORD_NUM; dwIndex++)
    {
        dwCurVal = ~(m_adwBitSet[dwIndex]);
        if (dwCurVal != 0)
        {
            for (dwIndex1 = 0; dwIndex1 < sizeof(WORD32); dwIndex1++)
            {
                ucCurVal = (BYTE)(dwCurVal & 0x000000FF);

                if (ucCurVal)
                {
                    dwEndPos = (dwIndex * BIT_NUM_PER_DWORD)
                             + (dwIndex1 * BIT_NUM_PER_BYTE)
                             + s_aucFirstOne[ucCurVal];

                    return dwEndPos ? (dwEndPos - 1) : 0;
                }

                dwCurVal >>= BIT_NUM_PER_BYTE;
            }
        }
    }

    return (dwPrev - 1);
}


/* 从Prev bit位开始, 从低到高查找连续bit位为0的最后1个bit(返回该bit的Pos), 未找到时返回dwPrev */
template <WORD32 BIT_NUM>
inline WORD32 CBaseBitSetTpl<BIT_NUM>::FindLast0(WORD32 dwPrev)
{
    dwPrev++;
    if (dwPrev >= BIT_NUM)
    {
        return (dwPrev - 1);
    }

    WORD32 dwIndex  = CBaseBitSetTpl<BIT_NUM>::WhichWord(dwPrev);
    WORD32 dwIndex1 = 0;
    WORD32 dwEndPos = 0;
    WORD32 dwCurVal = m_adwBitSet[dwIndex];
    BYTE   ucCurVal = 0;

    dwCurVal &= (~static_cast<WORD32>(0)) << CBaseBitSetTpl<BIT_NUM>::WhichBit(dwPrev);

    if (dwCurVal != 0)
    {
        dwIndex1   = CBaseBitSetTpl<BIT_NUM>::WhichByte(dwPrev);
        dwCurVal >>= dwIndex1 * BIT_NUM_PER_BYTE;

        for (; dwIndex1 < sizeof(WORD32); dwIndex1++)
        {
            ucCurVal = (BYTE)(dwCurVal & 0x000000FF);

            if (ucCurVal)
            {
                dwEndPos = (dwIndex * BIT_NUM_PER_DWORD)
                         + (dwIndex1 * BIT_NUM_PER_BYTE)
                         + s_aucFirstOne[ucCurVal];

                return dwEndPos ? (dwEndPos - 1) : 0;
            }

            dwCurVal >>= BIT_NUM_PER_BYTE;
        }
    }

    dwIndex++;
    for (; dwIndex < BITSET_DWORD_NUM; dwIndex++)
    {
        dwCurVal = m_adwBitSet[dwIndex];
        if (dwCurVal != 0)
        {
            for (dwIndex1 = 0; dwIndex1 < sizeof(WORD32); dwIndex1++)
            {
                ucCurVal = (BYTE)(dwCurVal & 0x000000FF);

                if (ucCurVal)
                {
                    dwEndPos = (dwIndex * BIT_NUM_PER_DWORD)
                             + (dwIndex1 * BIT_NUM_PER_BYTE)
                             + s_aucFirstOne[ucCurVal];

                    return dwEndPos ? (dwEndPos - 1) : 0;
                }

                dwCurVal >>= BIT_NUM_PER_BYTE;
            }
        }
    }

    return (dwPrev - 1);
}


template <WORD32 BIT_NUM>
inline CBaseBitSetTpl<BIT_NUM> & CBaseBitSetTpl<BIT_NUM>::operator&=(const CBaseBitSetTpl<BIT_NUM> &rBitSet)
{
    for (WORD32 dwIndex = 0; dwIndex < BITSET_DWORD_NUM; dwIndex++)
    {
        m_adwBitSet[dwIndex] &= rBitSet.m_adwBitSet[dwIndex];
    }

    return *this;
}


template <WORD32 BIT_NUM>
inline CBaseBitSetTpl<BIT_NUM> & CBaseBitSetTpl<BIT_NUM>::operator|=(const CBaseBitSetTpl<BIT_NUM> &rBitSet)
{
    for (WORD32 dwIndex = 0; dwIndex < BITSET_DWORD_NUM; dwIndex++)
    {
        m_adwBitSet[dwIndex] |= rBitSet.m_adwBitSet[dwIndex];
    }

    return *this;
}


template <WORD32 BIT_NUM>
inline CBaseBitSetTpl<BIT_NUM> & CBaseBitSetTpl<BIT_NUM>::operator^=(const CBaseBitSetTpl<BIT_NUM> &rBitSet)
{
    for (WORD32 dwIndex = 0; dwIndex < BITSET_DWORD_NUM; dwIndex++)
    {
        m_adwBitSet[dwIndex] ^= rBitSet.m_adwBitSet[dwIndex];
    }

    return *this;
}


template <WORD32 BIT_NUM>
inline CBaseBitSetTpl<BIT_NUM> & CBaseBitSetTpl<BIT_NUM>::operator<<=(WORD32 dwShift)
{
    if (0 != dwShift)
    {
        SWORD32 dwShiftDW   = dwShift / BIT_NUM_PER_DWORD;
        WORD32  dwOffset    = dwShift % BIT_NUM_PER_DWORD;
        WORD32  dwSubOffset = BIT_NUM_PER_DWORD - dwOffset;

        if (0 == dwOffset)
        {
            for (SWORD32 dwIndex = BITSET_DWORD_NUM - 1;
                 dwIndex >= dwShiftDW;
                 dwIndex--)
            {
                m_adwBitSet[dwIndex] = m_adwBitSet[dwIndex - dwShiftDW];
            }
        }
        else
        {
            for (SWORD32 dwIndex = BITSET_DWORD_NUM - 1;
                 dwIndex > dwShiftDW;
                 dwIndex--)
            {
                m_adwBitSet[dwIndex] = (m_adwBitSet[dwIndex - dwShiftDW] << dwOffset)
                                     | (m_adwBitSet[dwIndex - dwShiftDW - 1] >> dwSubOffset);
            }

            m_adwBitSet[dwShiftDW] = m_adwBitSet[0] << dwOffset;
        }

        std::fill(m_adwBitSet + 0, m_adwBitSet + dwShiftDW, static_cast<WORD32>(0));
    }

    Santitize();
    return *this;
}


template <WORD32 BIT_NUM>
inline CBaseBitSetTpl<BIT_NUM> & CBaseBitSetTpl<BIT_NUM>::operator>>=(WORD32 dwShift)
{
    if (0 != dwShift)
    {
        WORD32 dwShiftDW   = dwShift / BIT_NUM_PER_DWORD;
        WORD32 dwOffset    = dwShift % BIT_NUM_PER_DWORD;
        WORD32 dwSubOffset = BIT_NUM_PER_DWORD - dwOffset;
        WORD32 dwLimit     = BITSET_DWORD_NUM - dwShiftDW - 1;

        if (0 == dwOffset)
        {
            for (WORD32 dwIndex = 0; dwIndex <= dwLimit; dwIndex++)
            {
                m_adwBitSet[dwIndex] = m_adwBitSet[dwIndex + dwShiftDW];
            }
        }
        else
        {
            for (WORD32 dwIndex = 0; dwIndex < dwLimit; dwIndex++)
            {
                m_adwBitSet[dwIndex] = (m_adwBitSet[dwIndex + dwShiftDW] >> dwOffset)
                                     | (m_adwBitSet[dwIndex + dwShiftDW + 1] << dwSubOffset);
            }

            m_adwBitSet[dwLimit] = m_adwBitSet[BITSET_DWORD_NUM - 1] >> dwOffset;
        }

        std::fill(m_adwBitSet + dwLimit + 1, m_adwBitSet + BITSET_DWORD_NUM, static_cast<WORD32>(0));
    }

    Santitize();
    return *this;
}


/* 设置bit位为1 */
template <WORD32 BIT_NUM>
inline CBaseBitSetTpl<BIT_NUM> & CBaseBitSetTpl<BIT_NUM>::Set(WORD32 dwPos)
{
    if (likely(dwPos < BIT_NUM))
    {
        this->GetWord(dwPos) |= CBaseBitSetTpl<BIT_NUM>::MaskBit(dwPos);
    }

    return *this;
}


/* 设置bit位为指定值 */
template <WORD32 BIT_NUM>
inline CBaseBitSetTpl<BIT_NUM> & CBaseBitSetTpl<BIT_NUM>::Set(WORD32 dwPos, BOOL bVal)
{
    if (likely(dwPos < BIT_NUM))
    {
        if (bVal)
        {
            this->GetWord(dwPos) |= CBaseBitSetTpl<BIT_NUM>::MaskBit(dwPos);
        }
        else
        {
            this->GetWord(dwPos) &= ~CBaseBitSetTpl<BIT_NUM>::MaskBit(dwPos);
        }
    }

    return *this;
}


/* 设置bit位为0 */
template <WORD32 BIT_NUM>
inline CBaseBitSetTpl<BIT_NUM> & CBaseBitSetTpl<BIT_NUM>::ReSet(WORD32 dwPos)
{
    if (likely(dwPos < BIT_NUM))
    {
        this->GetWord(dwPos) &= ~CBaseBitSetTpl<BIT_NUM>::MaskBit(dwPos);
    }

    return *this;
}


/* 设置bit位为取反 */
template <WORD32 BIT_NUM>
inline CBaseBitSetTpl<BIT_NUM> & CBaseBitSetTpl<BIT_NUM>::Flip(WORD32 dwPos)
{
    if (likely(dwPos < BIT_NUM))
    {
        this->GetWord(dwPos) ^= CBaseBitSetTpl<BIT_NUM>::MaskBit(dwPos);
    }

    return *this;
}


/* 按位为取反 */
template <WORD32 BIT_NUM>
inline CBaseBitSetTpl<BIT_NUM> & CBaseBitSetTpl<BIT_NUM>::Flip()
{
    DoFlip();
    Santitize();
    return *this;
}


template <WORD32 BIT_NUM>
inline BOOL CBaseBitSetTpl<BIT_NUM>::operator[] (WORD32 dwPos) const
{
    if (unlikely(dwPos>= BIT_NUM))
    {
        return FALSE;
    }

    return (this->GetWord(dwPos) & CBaseBitSetTpl<BIT_NUM>::MaskBit(dwPos)) != 0;
}


template <WORD32 BIT_NUM>
inline BOOL CBaseBitSetTpl<BIT_NUM>::operator & (WORD32 dwPos) const
{
    if (unlikely(dwPos>= BIT_NUM))
    {
        return FALSE;
    }

    return (this->GetWord(dwPos) & CBaseBitSetTpl<BIT_NUM>::MaskBit(dwPos)) != 0;
}


template <WORD32 BIT_NUM>
inline BOOL CBaseBitSetTpl<BIT_NUM>::operator==(const CBaseBitSetTpl<BIT_NUM> &rBitSet) const
{
    for (WORD32 dwIndex = 0; dwIndex < BITSET_DWORD_NUM; dwIndex++)
    {
        if (m_adwBitSet[dwIndex] != rBitSet.m_adwBitSet[dwIndex])
        {
            return FALSE;
        }
    }

    return TRUE;
}


template <WORD32 BIT_NUM>
inline BOOL CBaseBitSetTpl<BIT_NUM>::operator!=(const CBaseBitSetTpl<BIT_NUM> &rBitSet) const
{
    for (WORD32 dwIndex = 0; dwIndex < BITSET_DWORD_NUM; dwIndex++)
    {
        if (m_adwBitSet[dwIndex] != rBitSet.m_adwBitSet[dwIndex])
        {
            return TRUE;
        }
    }

    return FALSE;
}


template <WORD32 BIT_NUM>
inline CBaseBitSetTpl<BIT_NUM> CBaseBitSetTpl<BIT_NUM>::operator~() const
{
    return CBaseBitSetTpl<BIT_NUM>(*this).Flip();
}


template <WORD32 BIT_NUM>
inline CBaseBitSetTpl<BIT_NUM> CBaseBitSetTpl<BIT_NUM>::operator<<(WORD32 dwShift) const
{
    return CBaseBitSetTpl<BIT_NUM>(*this) <<= dwShift;
}


template <WORD32 BIT_NUM>
inline CBaseBitSetTpl<BIT_NUM> CBaseBitSetTpl<BIT_NUM>::operator>>(WORD32 dwShift) const
{
    return CBaseBitSetTpl<BIT_NUM>(*this) >>= dwShift;
}


template <WORD32 BIT_NUM>
inline WORD32 CBaseBitSetTpl<BIT_NUM>::Count()
{
    WORD32  dwCount  = 0;
    BYTE   *pByteVal = (BYTE *)m_adwBitSet;
    BYTE   *pByteEnd = (BYTE *)(m_adwBitSet + BITSET_DWORD_NUM);

    while (pByteVal < pByteEnd)
    {
        dwCount += s_aucBitCount[*pByteVal];
        pByteVal++;
    }

    return dwCount;
}


template <WORD32 BIT_NUM>
inline BOOL CBaseBitSetTpl<BIT_NUM>::IsAny()
{
    for (WORD32 dwIndex = 0; dwIndex < BITSET_DWORD_NUM; dwIndex++)
    {
        if (m_adwBitSet[dwIndex] != 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}


template <WORD32 BIT_NUM>
inline VOID CBaseBitSetTpl<BIT_NUM>::Santitize()
{
    if (0 == (BIT_NUM % BIT_NUM_PER_DWORD))
    {
        return ;
    }

    CBaseBitSetTpl<BIT_NUM>::DoSanitize<BIT_NUM % BIT_NUM_PER_DWORD>(this->GetHigh());
}


template <WORD32 BIT_NUM>
inline VOID CBaseBitSetTpl<BIT_NUM>::DoFlip()
{
    for (WORD32 dwIndex = 0; dwIndex < BITSET_DWORD_NUM; dwIndex++)
    {
        m_adwBitSet[dwIndex] = ~m_adwBitSet[dwIndex];
    }
}


template <WORD32 BIT_NUM>
inline VOID CBaseBitSetTpl<BIT_NUM>::DoSet()
{
    for (WORD32 dwIndex = 0; dwIndex < BITSET_DWORD_NUM; dwIndex++)
    {
        m_adwBitSet[dwIndex] = ~static_cast<WORD32>(0);
    }
}


template <WORD32 BIT_NUM>
inline VOID CBaseBitSetTpl<BIT_NUM>::DoReSet()
{
    memset(m_adwBitSet, 0, BITSET_DWORD_NUM * sizeof(WORD32));
}


#endif


