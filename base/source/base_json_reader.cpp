

#include <assert.h>
#include <utility>

#include "base_json_reader.h"


WORD32 CJsonValue::InitValue()
{
    switch (m_eValueType)
    {
    case E_NULL_VALUE :
        break ;

    case E_BOOL_VALUE :
        m_tValue.bValue = FALSE;
        break ;

    case E_DWORD_VALUE :
        m_tValue.dwValue = 0;
        break ;

    case E_STRING_VALUE :
        m_tValue.pString = new CJsonString();
        break ;

    case E_ARRAY_VALUE :
    case E_OBJECT_VALUE :
        m_tValue.pMap = new CObjectValues();
        break ;

    default :
        assert(0);
        break ;
    }

    return SUCCESS;
}


CJsonValue::CJsonValue(E_JsonValueType eType)
{
    m_eValueType = eType;
    m_lwStart    = 0;
    m_lwLimit    = 0;

    InitValue();
}


CJsonValue::CJsonValue(BOOL bValue)
{
    m_eValueType = E_BOOL_VALUE;
    m_lwStart    = 0;
    m_lwLimit    = 0;

    m_tValue.bValue = bValue;
}


CJsonValue::CJsonValue(WORD32 dwValue)
{
    m_eValueType = E_DWORD_VALUE;
    m_lwStart    = 0;
    m_lwLimit    = 0;

    m_tValue.dwValue = dwValue;
}


CJsonValue::CJsonValue(CJsonString &rValue)
{
    m_eValueType = E_STRING_VALUE;
    m_lwStart    = 0;
    m_lwLimit    = 0;

    m_tValue.pString = new CJsonString(rValue);
}


CJsonValue::~CJsonValue()
{
    switch (m_eValueType)
    {
    case E_STRING_VALUE :
        {
            if (NULL != m_tValue.pString)
            {
                delete m_tValue.pString;
                m_tValue.pString = NULL;
            }
        }
        break ;

    case E_ARRAY_VALUE :
    case E_OBJECT_VALUE :
        if (NULL != m_tValue.pMap)
        {
            delete m_tValue.pMap;
            m_tValue.pMap = NULL;
        }
        break ;

    default :
        break ;
    }
}


VOID CJsonValue::SwapPayload(CJsonValue &rOther)
{
    std::swap(m_eValueType, rOther.m_eValueType);
    std::swap(m_tValue, rOther.m_tValue);
}


VOID CJsonValue::SetOffsetStart(std::ptrdiff_t lwStart)
{
    m_lwStart = lwStart;
}


VOID CJsonValue::SetOffsetLimit(std::ptrdiff_t lwLimit)
{
    m_lwLimit = lwLimit;
}


BOOL CJsonValue::AsBOOL()
{
    switch (m_eValueType)
    {
    case E_BOOL_VALUE :
        return m_tValue.bValue;

    case E_NULL_VALUE :
        break ;

    case E_DWORD_VALUE :
        return (m_tValue.dwValue != 0);

    default :
        assert(0);
        break ;
    }

    return FALSE;
}


WORD32 CJsonValue::AsDWORD()
{
    switch (m_eValueType)
    {
    case E_BOOL_VALUE :
        return (m_tValue.bValue) ? 1 : 0;

    case E_NULL_VALUE :
        return 0;

    case E_DWORD_VALUE :
        return m_tValue.dwValue;

    default :
        assert(0);
        break ;
    }

    return 0;
}


const CHAR * CJsonValue::AsString()
{
    switch (m_eValueType)
    {
    case E_BOOL_VALUE :
        return (m_tValue.bValue) ? "true" : "false";

    case E_NULL_VALUE :
        return NULL;

    case E_DWORD_VALUE :
        {
            memset(m_aucData, 0x00, sizeof(m_aucData));
            IntToStr(m_aucData, m_tValue.dwValue, E_DECIMAL_10);
            return m_aucData;
        }
        break ;

    case E_STRING_VALUE :
        return (m_tValue.pString->toChar());

    default :
        break ;
    }

    return NULL;
}


WORD32 CJsonValue::size()
{
    switch (m_eValueType)
    {
    case E_NULL_VALUE :
    case E_BOOL_VALUE :
    case E_DWORD_VALUE :
    case E_STRING_VALUE :
        return 0;

    case E_ARRAY_VALUE :
        if (!m_tValue.pMap->empty())
        {
            CObjectValues::const_iterator itLast = m_tValue.pMap->end();
            --itLast;
            return (*itLast).first.index() + 1;
        }
        return 0;

    case E_OBJECT_VALUE :
        return (WORD32)(m_tValue.pMap->size());

    default :
        break ;
    }

    return 0;
}


CJsonValue & CJsonValue::operator[] (WORD32 dwIndex)
{
    if (m_eValueType != E_ARRAY_VALUE)
    {
        assert(0);
    }

    CJsonString cKey(dwIndex);
    CObjectValues::const_iterator it = m_tValue.pMap->find(cKey);
    if (it == m_tValue.pMap->end())
    {
        return *(CJsonValue *)NULL;
    }

    return (*it).second;
}


CJsonValue & CJsonValue::operator[] (const CHAR *pKey)
{
    if (m_eValueType != E_OBJECT_VALUE)
    {
        assert(0);
    }

    CJsonString cKey(pKey);
    CObjectValues::const_iterator it = m_tValue.pMap->find(cKey);
    if (it == m_tValue.pMap->end())
    {
        return *(CJsonValue *)NULL;
    }

    return (*it).second;
}


CJsonValue & CJsonValue::operator[] (const CJsonString &rKey)
{
    if (m_eValueType != E_OBJECT_VALUE)
    {
        assert(0);
    }

    CObjectValues::const_iterator it = m_tValue.pMap->find(rKey);
    if (it == m_tValue.pMap->end())
    {
        return *(CJsonValue *)NULL;
    }

    return (*it).second;
}


CJsonToken::CJsonToken()
{
    m_eType  = E_TOKEN_EndOfStream;
    m_pStart = NULL;
    m_pEnd   = NULL;
}


CJsonToken::CJsonToken()
{
    m_eType  = E_TOKEN_EndOfStream;
    m_pStart = NULL;
    m_pEnd   = NULL;
}


CJsonReader::CJsonReader()
{
    memset(m_acDoc, 0x00, sizeof(m_acDoc));

    m_pBegin        = NULL;
    m_pEnd          = NULL;
    m_pCurrent      = NULL;
    m_pLastValueEnd = NULL;
    m_pLastValue    = NULL;
}


CJsonReader::~CJsonReader()
{
    while (!m_cNodes.empty())
    {
        m_cNodes.pop();
    }

    m_pBegin        = NULL;
    m_pEnd          = NULL;
    m_pCurrent      = NULL;
    m_pLastValueEnd = NULL;
    m_pLastValue    = NULL;
}


BOOL CJsonReader::Parse(std::istream &rFile,
                        CJsonValue   &rRoot)
{
    WORD32 dwLength = 0;

    rFile.seekg(0, rFile.end);
    dwLength = rFile.tellg();
    rFile.seekg(0, rFile.beg);

    if (dwLength >= JSON_MAX_FILE_LENGTH)
    {
        return FALSE;
    }

    rFile.read(m_acDoc, dwLength);

    return Parse(&(m_acDoc[0]), &(m_acDoc[dwLength]), rRoot);
}


BOOL CJsonReader::Parse(const CHAR *pBegin,
                        const CHAR *pEnd,
                        CJsonValue &rRoot)
{
    m_pBegin        = pBegin;
    m_pEnd          = pEnd;
    m_pCurrent      = pBegin;
    m_pLastValueEnd = NULL;
    m_pLastValue    = NULL;

    while (!m_cNodes.empty())
    {
        m_cNodes.pop();
    }

    m_cNodes.push(&rRoot);

    BOOL       bResult = ReadValue();
    CJsonToken cToken;
    SkipCommentTokens(cToken);

    return bResult;
}


BOOL CJsonReader::ReadValue()
{
    if (m_cNodes.size() > JSON_DEPRECATED_STACK_LIMIT)
    {
        return FALSE;
    }

    BOOL       bResult = TRUE;
    CJsonToken cToken;
    SkipCommentTokens(cToken);

    switch (cToken.m_eType)
    {
    case E_TOKEN_ObjectBegin :
        {
            bResult = ReadObject(cToken);
            CurrentValue().SetOffsetLimit(m_pCurrent - m_pBegin);
        }
        break ;

    case E_TOKEN_ArrayBegin :
        {
            bResult = ReadArray(cToken);
            CurrentValue().SetOffsetLimit(m_pCurrent - m_pBegin);
        }
        break ;

    case E_TOKEN_Number :
        {
            bResult = DecodeNumber(cToken);
        }
        break ;

    case E_TOKEN_String :
        {
            bResult = DecodeString(cToken);
        }
        break ;

    case E_TOKEN_TRUE :
        {
            DecodeTRUE(cToken);
        }
        break ;

    case E_TOKEN_FALSE :
        {
            DecodeFALSE(cToken);
        }
        break ;

    case E_TOKEN_NULL :
        {
            DecodeNULL(cToken);
        }
        break ;

    case E_TOKEN_ArraySeparator :
    case E_TOKEN_ArrayEnd :
    case E_TOKEN_ObjectEnd :
    default :
        {
            bResult = FALSE;
        }
        break ;
    }

    return bResult;
}


VOID CJsonReader::SkipCommentTokens(CJsonToken &rToken)
{
    do
    {
        ReadToken(rToken);
    }while (rToken.m_eType == E_TOKEN_Comment);
}


BOOL CJsonReader::ReadToken(CJsonToken &rToken)
{
    SkipSpaces();

    rToken.m_pStart = m_pCurrent;

    BOOL bResult = TRUE;
    CHAR cValue  = NextChar();
    switch (cValue)
    {
    case '{' :
        rToken.m_eType = E_TOKEN_ObjectBegin;
        break ;
    case '}' :
        rToken.m_eType = E_TOKEN_ObjectEnd;
        break ;
    case '[' :
        rToken.m_eType = E_TOKEN_ArrayBegin;
        break ;
    case ']' :
        rToken.m_eType = E_TOKEN_ArrayEnd;
        break ;
    case '"' :
        rToken.m_eType = E_TOKEN_String;
        bResult = ReadString();
        break ;
    case '/' :
        rToken.m_eType = E_TOKEN_Comment;
        bResult = ReadComment();
        break ;
    case '0' :
    case '1' :
    case '2' :
    case '3' :
    case '4' :
    case '5' :
    case '6' :
    case '7' :
    case '8' :
    case '9' :
        rToken.m_eType = E_TOKEN_Number;
        ReadNumber();
        break ;
    case 't' :
        rToken.m_eType = E_TOKEN_TRUE;
        bResult = Match("rue", 3);
        break ;
    case 'f' :
        rToken.m_eType = E_TOKEN_FALSE;
        bResult = Match("alse", 4);
        break ;
    case 'n' :
        rToken.m_eType = E_TOKEN_NULL;
        bResult = Match("ull", 3);
        break ;
    case ',' :
        rToken.m_eType = E_TOKEN_ArraySeparator;
        break ;
    case ':' :
        rToken.m_eType = E_TOKEN_MemberSeparator;
        break ;
    case 0 :
        rToken.m_eType = E_TOKEN_EndOfStream;
        break ;
    default :
        bResult = FALSE;
        break ;
    }

    rToken.m_pEnd = m_pCurrent;

    return bResult;
}


BOOL CJsonReader::ReadObject(CJsonToken &rToken)
{
    CJsonToken  cTokenName;
    CJsonString cName;
    CJsonValue  cInit(E_OBJECT_VALUE);

    CurrentValue().SwapPayload(cInit);
    CurrentValue().SetOffsetStart(rToken.m_pStart - m_pBegin);

    BOOL bResult  = FALSE;
    BOOL bFinFlag = TRUE;

    while (ReadToken(cTokenName))
    {
        BOOL bInitResult = TRUE;
        while ((cTokenName.m_eType == E_TOKEN_Comment) && (bInitResult))
        {
            bInitResult = ReadToken(cTokenName);
        }

        if (!bInitResult)
        {
            break ;
        }

        if ((cTokenName.m_eType == E_TOKEN_ObjectEnd) && (cName.IsEmpty()))
        {
            return TRUE;
        }

        cName.Reset();
        if (cTokenName.m_eType == E_TOKEN_String)
        {
            if (FALSE == DecodeString(cTokenName, cName))
            {
                return FALSE;
            }
        }
        else
        {
            break ;
        }

        CJsonToken cColon;  /* 冒号分隔符 */
        if ( (FALSE == ReadToken(cColon))
          || (cColon.m_eType != E_TOKEN_MemberSeparator))
        {
            return FALSE;
        }

        CJsonValue &rValue = CurrentValue()[cName];
        m_cNodes.push(&rValue);
        bResult = ReadValue();
        m_cNodes.pop();
        if (!bResult)
        {
            return FALSE;
        }

        CJsonToken cComma;  /* 逗号分隔符 */
        if ( (FALSE == ReadToken(cComma))
          || ( (cComma.m_eType != E_TOKEN_ObjectEnd)
            && (cComma.m_eType != E_TOKEN_ArraySeparator)
            && (cComma.m_eType != E_TOKEN_Comment)))
        {
            return FALSE;
        }

        bFinFlag = TRUE;
        while ((cComma.m_eType == E_TOKEN_Comment) && (bFinFlag))
        {
            bFinFlag = ReadToken(cComma);
        }

        if (cComma.m_eType == E_TOKEN_ObjectEnd)
        {
            return TRUE;
        }
    }

    return FALSE;
}


BOOL CJsonReader::ReadArray(CJsonToken &rToken)
{
    CJsonValue cInit(E_ARRAY_VALUE);

    CurrentValue().SwapPayload(cInit);
    CurrentValue().SetOffsetStart(rToken.m_pStart - m_pBegin);

    SkipSpaces();

    /* empty array */
    if ((m_pCurrent != m_pEnd) && (*m_pCurrent == ']'))
    {
        CJsonToken cEndArray;
        ReadToken(cEndArray);

        return TRUE;
    }

    WORD32 dwIndex = 0;
    BOOL   bResult = FALSE;

    while(TRUE)
    {
        CJsonValue &rValue = CurrentValue()[dwIndex++];
        m_cNodes.push(&rValue);
        bResult = ReadValue();
        m_cNodes.pop();
        if (!bResult)
        {
            return FALSE;
        }

        CJsonToken cCurToken;
        bResult = ReadToken(cCurToken);
        while ((cCurToken.m_eType == E_TOKEN_Comment) && (bResult))
        {
            bResult = ReadToken(cCurToken);
        }

        BOOL bBadTokenType = (cCurToken.m_eType != E_TOKEN_ArraySeparator)
                          && (cCurToken.m_eType != E_TOKEN_ArrayEnd);
        if (!bResult || bBadTokenType)
        {
            return FALSE;
        }

        if (cCurToken.m_eType == E_TOKEN_ArrayEnd)
        {
            break ;
        }
    }

    return TRUE;
}


BOOL CJsonReader::DecodeNumber(CJsonToken &rToken)
{
    WORD32 dwValue = 0;
    if (!DecodeNumber(rToken, dwValue))
    {
        return FALSE;
    }

    CJsonValue cValue(dwValue);

    CurrentValue().SwapPayload(cValue);
    CurrentValue().SetOffsetStart(rToken.m_pStart - m_pBegin);
    CurrentValue().SetOffsetLimit(rToken.m_pEnd - m_pBegin);

    return TRUE;
}


BOOL CJsonReader::DecodeNumber(CJsonToken &rToken, WORD32 &rdwValue)
{
    const CHAR *pCurrent = rToken.m_pStart;
    const CHAR *pEnd     = rToken.m_pEnd;

    WORD32 dwTmpValue = 0;

    while (pCurrent < pEnd)
    {
        /* 数值溢出 */
        if (dwTmpValue >= 0x19999999)
        {
            return FALSE;
        }

        CHAR cValue = *pCurrent++;
        if ((cValue < '0') || (cValue > '9'))
        {
            return FALSE;
        }

        WORD32 dwDigit = cValue - '0';

        dwTmpValue = dwTmpValue * 10 + dwDigit;
    }

    rdwValue = dwTmpValue;

    return TRUE;
}


BOOL CJsonReader::DecodeString(CJsonToken &rToken)
{
    CJsonString  cStr;
    if (!DecodeString(rToken, cStr))
    {
        return FALSE;
    }

    CJsonValue cValue(cStr);

    CurrentValue().SwapPayload(cValue);
    CurrentValue().SetOffsetStart(rToken.m_pStart - m_pBegin);
    CurrentValue().SetOffsetLimit(rToken.m_pEnd - m_pBegin);

    return TRUE;
}


BOOL CJsonReader::DecodeString(CJsonToken &rToken, CJsonString &rStr)
{
    const CHAR *pCurrent = rToken.m_pStart + 1;  /* 跳过'"' */
    const CHAR *pEnd     = rToken.m_pEnd - 1;    /* 不包含'"' */

    while (pCurrent != pEnd)
    {
        CHAR cValue = *pCurrent++;
        if (cValue == '"')
        {
            break ;
        }

        if (cValue == '\\')
        {
            return FALSE;
        }
        else
        {
            rStr += cValue;
        }
    }

    return TRUE;
}


VOID CJsonReader::DecodeTRUE(CJsonToken &rToken)
{
    CJsonValue cValue(TRUE);

    CurrentValue().SwapPayload(cValue);
    CurrentValue().SetOffsetStart(rToken.m_pStart - m_pBegin);
    CurrentValue().SetOffsetLimit(rToken.m_pEnd - m_pBegin);
}


VOID CJsonReader::DecodeFALSE(CJsonToken &rToken)
{
    CJsonValue cValue(FALSE);

    CurrentValue().SwapPayload(cValue);
    CurrentValue().SetOffsetStart(rToken.m_pStart - m_pBegin);
    CurrentValue().SetOffsetLimit(rToken.m_pEnd - m_pBegin);
}


VOID CJsonReader::DecodeNULL(CJsonToken &rToken)
{
    CJsonValue cValue;

    CurrentValue().SwapPayload(cValue);
    CurrentValue().SetOffsetStart(rToken.m_pStart - m_pBegin);
    CurrentValue().SetOffsetLimit(rToken.m_pEnd - m_pBegin);
}


BOOL CJsonReader::Match(const CHAR *pPattern, WORD32 dwLen)
{
    if ((m_pEnd - m_pCurrent) < dwLen)
    {
        return FALSE;
    }

    WORD32 dwIndex = dwLen;
    while (dwIndex--)
    {
        if (m_pCurrent[dwIndex] != pPattern[dwIndex])
        {
            return FALSE;
        }
    }

    m_pCurrent += dwLen;

    return TRUE;
}


VOID CJsonReader::ReadNumber()
{
    CHAR        cValue = '0';
    const CHAR *pTmp   = m_pCurrent;

    while ((cValue >= '0') && (cValue <= '9'))
    {
        cValue = ((m_pCurrent = pTmp) < m_pEnd) ? *pTmp++ : '\0';
    }
}


BOOL CJsonReader::ReadComment()
{
    const CHAR *pBegin  = m_pCurrent - 1;
    CHAR        cValue  = NextChar();
    BOOL        bResult = FALSE;

    if (cValue == '*')
    {
        bResult = ReadCStyleComment();
    }
    else if (cValue == '/')
    {
        bResult = ReadCppStyleComment();
    }
    else
    {
        bResult = FALSE;
    }

    return bResult;
}


BOOL CJsonReader::ReadCStyleComment()
{
    while ((m_pCurrent + 1) < m_pEnd)
    {
        CHAR cValue = NextChar();
        if ((cValue == '*') && (*m_pCurrent == '/'))
        {
            break ;
        }
    }

    return (NextChar() == '/');
}


BOOL CJsonReader::ReadCppStyleComment()
{
    while (m_pCurrent != m_pEnd)
    {
        CHAR cValue = NextChar();
        if (cValue == '\n')
        {
            break ;
        }

        if (cValue == '\r')
        {
            if ((m_pCurrent != m_pEnd) && (*m_pCurrent == '\n'))
            {
                NextChar();
            }

            break ;
        }
    }

    return TRUE;
}


BOOL CJsonReader::ReadString()
{
    while (m_pCurrent != m_pEnd)
    {
        CHAR cValue = *m_pCurrent++;
        if (cValue == '\\')
        {
            NextChar();
        }
        else if (cValue == '"')
        {
            return TRUE;
        }
        else
        {
        }
    }

    return FALSE;
}


VOID CJsonReader::SkipSpaces()
{
    while (m_pCurrent != m_pEnd)
    {
        CHAR cValue = *m_pCurrent;
        if ( (cValue == ' ')
          || (cValue == '\t')
          || (cValue == '\r')
          || (cValue == '\n'))
        {
            m_pCurrent++;
        }
        else
        {
            break ;
        }
    }
}


CHAR CJsonReader::NextChar()
{
    if (m_pCurrent != m_pEnd)
    {
        return *m_pCurrent++;
    }
    else
    {
        return 0;
    }
}


CJsonValue & CJsonReader::CurrentValue()
{
    return *(m_cNodes.top());
}


