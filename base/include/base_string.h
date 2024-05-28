

#ifndef _BASE_STRING_H_
#define _BASE_STRING_H_


#include <cstring>

#include "base_snprintf.h"


template <WORD32 STR_LEN>
class CString
{
public :
    CString ();
    CString (WORD32 dwValue);
    CString (BYTE *paucData);    
    CString (CHAR *paucData);
    CString (const CHAR *paucData);
    CString (CString &rStr);
    CString (const CString &rStr);
    virtual ~CString();

    WORD32 Length();

    BOOL IsEmpty();

    VOID Reset();

    CString & operator=(WORD32 dwValue);
    CString & operator=(BYTE *paucData);
    CString & operator=(CHAR *paucData);
    CString & operator=(const CHAR *paucData);
    CString & operator=(CString &rStr);
    CString & operator=(const CString &rStr);

    CHAR & operator[](WORD32 dwIndex);

    BOOL isMatch(const CHAR *paucData);

    BOOL operator==(WORD32 dwValue);
    BOOL operator==(const CHAR cValue);
    BOOL operator==(const CHAR *paucData);
    BOOL operator==(CString &rStr);
    BOOL operator==(const CString &rStr);
    BOOL operator >(CString &rStr);
    BOOL operator <(CString &rStr);

    CString & operator+=(CHAR cValue);
    CString & operator+=(BYTE cValue);

    CString & operator+=(BYTE *pStr);
    CString & operator+=(CHAR *pStr);
    CString & operator+=(CString &rStr);

    BYTE * toByte();

    CHAR * toChar();

    CString subStr(WORD32 dwPos = 0, WORD32 dwLen = 0);

    WORD32 Copy(BYTE *pOuter, WORD32 dwPos, WORD32 dwLen);

    WORD32 Find(const CHAR *pSubStr);

    WORD32 Find(BYTE *pSubStr);

protected :
    CHAR  m_aucData[STR_LEN];
};


template <WORD32 STR_LEN>
CString<STR_LEN>::CString ()
{
    memset(m_aucData, 0x00, sizeof(m_aucData));
}


template <WORD32 STR_LEN>
CString<STR_LEN>::CString (WORD32 dwValue)
{
    memset(m_aucData, 0x00, sizeof(m_aucData));
    IntToStr(m_aucData, dwValue, E_DECIMAL_10);
}


template <WORD32 STR_LEN>
CString<STR_LEN>::CString (BYTE *paucData)
{
    memset(m_aucData, 0x00, sizeof(m_aucData));

    if (NULL != paucData)
    {
        memcpy(m_aucData, 
               paucData, 
               MIN((sizeof(m_aucData) - 1), strlen((char *)paucData)));
    }
}


template <WORD32 STR_LEN>
CString<STR_LEN>::CString (CHAR *paucData)
{
    memset(m_aucData, 0x00, sizeof(m_aucData));

    if (NULL != paucData)
    {
        memcpy(m_aucData, 
               paucData, 
               MIN((sizeof(m_aucData) - 1), strlen(paucData)));
    }
}


template <WORD32 STR_LEN>
CString<STR_LEN>::CString (const CHAR *paucData)
{
    memset(m_aucData, 0x00, sizeof(m_aucData));

    if (NULL != paucData)
    {
        memcpy(m_aucData, 
               paucData, 
               MIN((sizeof(m_aucData) - 1), strlen(paucData)));
    }
}


template <WORD32 STR_LEN>
CString<STR_LEN>::CString (CString<STR_LEN> &rStr)
{
    memset(m_aucData, 0x00, sizeof(m_aucData));
    memcpy(m_aucData, rStr.toByte(), rStr.Length());
}


template <WORD32 STR_LEN>
CString<STR_LEN>::CString (const CString<STR_LEN> &rStr)
{
    memset(m_aucData, 0x00, sizeof(m_aucData));
    memcpy(m_aucData, rStr.m_aucData, strlen((char*)(rStr.m_aucData)));
}


template <WORD32 STR_LEN>
CString<STR_LEN>::~CString()
{
    memset(m_aucData, 0x00, sizeof(m_aucData));
}


template <WORD32 STR_LEN>
inline WORD32 CString<STR_LEN>::Length()
{
    return strlen((char *)m_aucData);
}


template <WORD32 STR_LEN>
inline BOOL CString<STR_LEN>::IsEmpty()
{
    return (0 == m_aucData[0]);
}


template <WORD32 STR_LEN>
inline VOID CString<STR_LEN>::Reset()
{
    memset(m_aucData, 0x00, sizeof(m_aucData));
}


template <WORD32 STR_LEN>
inline CString<STR_LEN> & CString<STR_LEN>::operator=(WORD32 dwValue)
{
    memset(m_aucData, 0x00, sizeof(m_aucData));
    IntToStr(m_aucData, dwValue, E_DECIMAL_10);
}


template <WORD32 STR_LEN>
inline CString<STR_LEN> & CString<STR_LEN>::operator=(BYTE *paucData)
{
    if (NULL == paucData)
    {        
        memset(m_aucData, 0x00, sizeof(m_aucData));
    }
    else
    {
        memset(m_aucData, 0x00, sizeof(m_aucData));
        memcpy(m_aucData, 
               paucData, 
               MIN((sizeof(m_aucData) - 1), strlen((char *)paucData)));
    }

    return *this;
}


template <WORD32 STR_LEN>
inline CString<STR_LEN> & CString<STR_LEN>::operator=(CHAR *paucData)
{
    if (NULL == paucData)
    {
        memset(m_aucData, 0x00, sizeof(m_aucData));
    }
    else
    {
        memset(m_aucData, 0x00, sizeof(m_aucData));
        memcpy(m_aucData, 
               paucData, 
               MIN((sizeof(m_aucData) - 1), strlen(paucData)));
    }

    return *this;
}


template <WORD32 STR_LEN>
inline CString<STR_LEN> & CString<STR_LEN>::operator=(const CHAR *paucData)
{
    if (NULL == paucData)
    {
        memset(m_aucData, 0x00, sizeof(m_aucData));
    }
    else
    {
        memset(m_aucData, 0x00, sizeof(m_aucData));
        memcpy(m_aucData, 
               paucData, 
               MIN((sizeof(m_aucData) - 1), strlen(paucData)));
    }

    return *this;
}


template <WORD32 STR_LEN>
inline CString<STR_LEN> & CString<STR_LEN>::operator=(CString<STR_LEN> &rStr)
{
    if (&rStr != this)
    {
        memset(m_aucData, 0x00, sizeof(m_aucData));
        memcpy(m_aucData, rStr.toByte(), rStr.Length());
    }

    return *this;
}


template <WORD32 STR_LEN>
inline CString<STR_LEN> & CString<STR_LEN>::operator=(const CString<STR_LEN> &rStr)
{
    if (&rStr != this)
    {
        memset(m_aucData, 0x00, sizeof(m_aucData));
        memcpy(m_aucData, rStr.m_aucData, strlen((char*)(rStr.m_aucData)));
    }

    return *this;
}


template <WORD32 STR_LEN>
inline CHAR & CString<STR_LEN>::operator[](WORD32 dwIndex)
{
    return m_aucData[dwIndex];
}


template <WORD32 STR_LEN>
inline BOOL CString<STR_LEN>::isMatch(const CHAR *paucData)
{
    if (unlikely(NULL == paucData))
    {
        return FALSE;
    }

    WORD32 dwDstLen = strlen(paucData);
    WORD32 dwSrcLen = Length();
    WORD32 dwLen    = MIN(dwDstLen, dwSrcLen);
    
    if (dwLen != dwDstLen)
    {
        if (' ' != paucData[dwLen])
        {
            return FALSE;
        }
    }
    
    return (0 == memcmp(m_aucData, 
                        paucData, 
                        dwLen));
}


template <WORD32 STR_LEN>
inline BOOL CString<STR_LEN>::operator==(WORD32 dwValue)
{
    CHAR   aucData[INTEGER_TO_STR_LEN] = {0,};
    WORD32 dwLen = IntToStr(aucData, dwValue, E_DECIMAL_10);

    if (Length() == dwLen)
    {
        return (0 == memcmp(m_aucData, aucData, dwLen));
    }
    else
    {
        return FALSE;
    }
}


template <WORD32 STR_LEN>
inline BOOL CString<STR_LEN>::operator==(const CHAR cValue)
{
    if (Length() != 1)
    {
        return FALSE;
    }
    else
    {
        return (m_aucData[0] == cValue);
    }
}


template <WORD32 STR_LEN>
inline BOOL CString<STR_LEN>::operator==(const CHAR *paucData)
{
    if (unlikely(NULL == paucData))
    {
        return FALSE;
    }

    WORD32 dwLen = strlen(paucData);

    if (Length() == dwLen)
    {
        return (0 == memcmp(m_aucData, paucData, dwLen));
    }
    else
    {
        return FALSE;
    }
}


template <WORD32 STR_LEN>
inline BOOL CString<STR_LEN>::operator==(CString<STR_LEN> &rStr)
{
    if (&rStr == this)
    {
        return TRUE;
    }

    WORD32 dwLen = rStr.Length();

    if (Length() == dwLen)
    {
        return (0 == memcmp(m_aucData, rStr.toByte(), dwLen));
    }
    else
    {
        return FALSE;
    }
}


template <WORD32 STR_LEN>
inline BOOL CString<STR_LEN>::operator==(const CString &rStr)
{
    if (&rStr == this)
    {
        return TRUE;
    }

    WORD32 dwLen = rStr.Length();

    if (Length() == dwLen)
    {
        return (0 == memcmp(m_aucData, rStr.toByte(), dwLen));
    }
    else
    {
        return FALSE;
    }
}


template <WORD32 STR_LEN>
inline BOOL CString<STR_LEN>::operator >(CString &rStr)
{
    if (&rStr == this)
    {
        return FALSE;
    }
    else
    {
        return (0 < memcmp(m_aucData, 
                           rStr.toByte(), 
                           MIN(Length(), rStr.Length())));
    }
}


template <WORD32 STR_LEN>
inline BOOL CString<STR_LEN>::operator <(CString &rStr)
{
    if (&rStr == this)
    {
        return FALSE;
    }
    else
    {
        return (0 > memcmp(m_aucData, 
                           rStr.toByte(), 
                           MIN(Length(), rStr.Length())));
    }
}


template <WORD32 STR_LEN>
inline CString<STR_LEN> & CString<STR_LEN>::operator+=(CHAR cValue)
{
    WORD32 dwPos = Length();
    
    if (dwPos < STR_LEN)
    {
        m_aucData[dwPos] = cValue;
    }
    
    return *this;
}


template <WORD32 STR_LEN>
inline CString<STR_LEN> & CString<STR_LEN>::operator+=(BYTE cValue)
{
    WORD32 dwPos = Length();
    
    if (dwPos < STR_LEN)
    {
        m_aucData[dwPos] = cValue;
    }
    
    return *this;
}


template <WORD32 STR_LEN>
inline CString<STR_LEN> & CString<STR_LEN>::operator+=(BYTE *pStr)
{
    WORD32 dwPos = Length();
    
    if ((NULL != pStr) && (dwPos < STR_LEN))
    {
        WORD32 dwLen = strlen((char *)pStr);

        memcpy((m_aucData + dwPos), 
               pStr, 
               MIN((sizeof(m_aucData) - dwPos - 1), dwLen));
    }

    return *this;
}


template <WORD32 STR_LEN>
inline CString<STR_LEN> & CString<STR_LEN>::operator+=(CHAR *pStr)
{
    WORD32 dwPos = Length();
    
    if ((NULL != pStr) && (dwPos < STR_LEN))
    {
        WORD32 dwLen = strlen(pStr);

        memcpy((m_aucData + dwPos), 
               pStr, 
               MIN((sizeof(m_aucData) - dwPos - 1), dwLen));
    }

    return *this;
}


template <WORD32 STR_LEN>
inline CString<STR_LEN> & CString<STR_LEN>::operator+=(CString<STR_LEN> &rStr)
{
    WORD32 dwPos = Length();
    
    if ((&rStr != this) && (dwPos < STR_LEN))
    {
        WORD32 dwLen = rStr.Length();

        memcpy((m_aucData + dwPos), 
               rStr.toByte(), 
               MIN((sizeof(m_aucData) - dwPos - 1), dwLen));
    }

    return *this;
}


template <WORD32 STR_LEN>
inline BYTE * CString<STR_LEN>::toByte()
{
    return (BYTE *)m_aucData;
}


template <WORD32 STR_LEN>
inline CHAR * CString<STR_LEN>::toChar()
{
    return m_aucData;
}


template <WORD32 STR_LEN>
inline CString<STR_LEN> CString<STR_LEN>::subStr(WORD32 dwPos, WORD32 dwLen)
{
    CString<STR_LEN> cTmp;

    if (dwPos >= Length())
    {
        return cTmp;
    }

    BYTE   aucStr[STR_LEN] = {0};
    WORD32 dwStrLen = MIN((Length() - dwPos), dwLen);

    memcpy(aucStr, &((*this)[dwPos]), dwStrLen);

    cTmp = aucStr;

    return cTmp;
}


template <WORD32 STR_LEN>
inline WORD32 CString<STR_LEN>::Copy(BYTE *pOuter, WORD32 dwPos, WORD32 dwLen)
{
    if ((NULL == pOuter) || (dwPos >= Length()))
    {
        return FAIL;
    }

    WORD32 dwStrLen = MIN((Length() - dwPos), dwLen);

    memcpy(pOuter, &((*this)[dwPos]), dwStrLen);

    return dwStrLen;
}


template <WORD32 STR_LEN>
inline WORD32 CString<STR_LEN>::Find(const CHAR *pSubStr)
{
    if (unlikely(NULL == pSubStr))
    {
        return FAIL;
    }

    BYTE aucData[STR_LEN] = {0};
    memcpy(aucData, pSubStr, strlen(pSubStr));

    return Find(aucData);
}


template <WORD32 STR_LEN>
inline WORD32 CString<STR_LEN>::Find(BYTE *pSubStr)
{
    if (unlikely(NULL == pSubStr))
    {
        return FAIL;
    }

    WORD32 dwStrLen = strlen((char *)pSubStr);
    WORD32 dwLen    = Length();

    CString<STR_LEN> cDst(pSubStr);

    for (WORD32 dwIndex = 0; (dwIndex + dwStrLen) <= dwLen; dwIndex++)
    {
        CString<STR_LEN> cSrc = subStr(dwIndex, dwStrLen).toByte();

        if (cSrc == cDst)
        {
            return dwIndex;
        }
    }

    return FAIL;
}


#endif


