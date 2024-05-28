

#ifndef _BASE_JSON_PARSE_H_
#define _BASE_JSON_PARSE_H_


#include <cstddef>
#include <iostream>
#include <istream>
#include <stack>
#include <string>
#include <map>

#include "base_string.h"
#include "base_data_container.h"
#include "base_snprintf.h"


#define JSON_MAX_STRING_LENGTH      ((WORD16)(128))

typedef CString<JSON_MAX_STRING_LENGTH>  CJsonString;


typedef enum tagE_JsonValueType
{
    E_NULL_VALUE = 0,
    E_BOOL_VALUE,
    E_DWORD_VALUE,
    E_STRING_VALUE,
    E_ARRAY_VALUE,
    E_OBJECT_VALUE,
}E_JsonValueType;


/* Array/Object类型的对象被组织存放在map中, map的第一个元素由CZString描述 */
class CZString
{
public :
    CZString (WORD32 dwIndex);
    CZString (const CHAR *paucData);
    CZString (const CZString &rStr);
    virtual ~CZString();

    CZString & operator=(const CZString &rOther);

    BOOL operator< (const CZString &rOther) const;
    BOOL operator==(const CZString &rOther) const;

    WORD32 Index();
    WORD32 Length();

    CHAR * Data();

protected :
    WORD32       m_dwIndex;
    CJsonString  m_cString;
};


class CJsonValue
{
public :
    typedef std::map<CZString, CJsonValue>  CObjectValues;

    typedef union tagT_ValueHolder
    {
        BOOL           bValue;
        WORD32         dwValue;
        CJsonString   *pString;
        CObjectValues *pMap;
    }T_ValueHolder;

    static CJsonValue const & NullSingleton();

protected :
    WORD32 InitValue();

public :
    CJsonValue(E_JsonValueType eType = E_NULL_VALUE);
    CJsonValue(BOOL bValue);
    CJsonValue(WORD32 dwValue);
    CJsonValue(CJsonString &rValue);

    virtual ~CJsonValue();

    CJsonValue & operator=(CJsonValue &rOther);

    VOID SwapPayload(CJsonValue &rOther);

    VOID SetOffsetStart(std::ptrdiff_t lwStart);
    VOID SetOffsetLimit(std::ptrdiff_t lwLimit);

    BOOL AsBOOL();
    WORD32 AsDWORD();
    const CHAR * AsString();

    WORD32 size();

    CJsonValue & operator[] (WORD32 dwIndex);
    CJsonValue & operator[] (const CHAR *pName);
    CJsonValue & operator[] (CJsonString &rStr);

public :
    E_JsonValueType  m_eValueType;
    T_ValueHolder    m_tValue;

    std::ptrdiff_t   m_lwStart;
    std::ptrdiff_t   m_lwLimit;

private :
    CHAR             m_aucData[INTEGER_TO_STR_LEN];
};


typedef enum tagE_JsonTokenType
{
    E_TOKEN_EndOfStream = 0,
    E_TOKEN_ObjectBegin,
    E_TOKEN_ObjectEnd,
    E_TOKEN_ArrayBegin,
    E_TOKEN_ArrayEnd,
    E_TOKEN_String,
    E_TOKEN_Number,
    E_TOKEN_TRUE,
    E_TOKEN_FALSE,
    E_TOKEN_NULL,
    E_TOKEN_ArraySeparator,
    E_TOKEN_MemberSeparator,
    E_TOKEN_Comment,
    E_TOKEN_ERROR,
}E_JsonTokenType;


class CJsonToken
{
public :
    CJsonToken();
    virtual ~CJsonToken();

public :
    E_JsonTokenType   m_eType;
    const CHAR       *m_pStart;
    const CHAR       *m_pEnd;
};


typedef std::stack<CJsonValue *>  CJsonNodes;


class CJsonReader
{
public :
    enum { JSON_MAX_FILE_LENGTH        = 65536 };
    enum { JSON_DEPRECATED_STACK_LIMIT =  1024 };

public :
    CJsonReader();
    virtual ~CJsonReader();

    BOOL Parse(std::istream &rFile, CJsonValue &rRoot);

protected :
    BOOL Parse(const CHAR *pBegin,
               const CHAR *pEnd,
               CJsonValue &rRoot);

    BOOL ReadValue();

    VOID SkipCommentTokens(CJsonToken &rToken);

    BOOL ReadToken(CJsonToken &rToken);
    BOOL ReadObject(CJsonToken &rToken);
    BOOL ReadArray(CJsonToken &rToken);
    BOOL DecodeNumber(CJsonToken &rToken);
    BOOL DecodeNumber(CJsonToken &rToken, WORD32 &rdwValue);
    BOOL DecodeString(CJsonToken &rToken);
    BOOL DecodeString(CJsonToken &rToken, CJsonString &rStr);
    VOID DecodeTRUE(CJsonToken &rToken);
    VOID DecodeFALSE(CJsonToken &rToken);
    VOID DecodeNULL(CJsonToken &rToken);

    BOOL Match(const CHAR *pPattern, WORD32 dwLen);
    VOID ReadNumber();
    BOOL ReadComment();
    BOOL ReadCStyleComment();
    BOOL ReadCppStyleComment();
    BOOL ReadString();

    VOID SkipSpaces();

    CHAR NextChar();

    CJsonValue & CurrentValue();

protected :
    CHAR         m_acDoc[JSON_MAX_FILE_LENGTH];

    CJsonNodes   m_cNodes;

    const CHAR  *m_pBegin;
    const CHAR  *m_pEnd;
    const CHAR  *m_pCurrent;
    const CHAR  *m_pLastValueEnd;
    CJsonValue  *m_pLastValue;
};


#endif


