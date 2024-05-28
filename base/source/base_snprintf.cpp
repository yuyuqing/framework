

#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

#include "base_snprintf.h"


const CHAR g_aucHex[] = "0123456789abcdef";


/* 将无符号整数转换为字符串
 * dwBase  : 10进制 or 16进制
 */
inline CHAR * IntegerToString(WORD32  dwBase, 
                              WORD64  lwVal, 
                              CHAR   *pBuf)
{
    *pBuf-- = 0;

    do
    {        
        *pBuf-- = g_aucHex[lwVal % dwBase];
    }while ((lwVal /= dwBase) != 0);

    return pBuf + 1;
}


/* 将无符号整数转换为字符串
 * dwBase  : 10进制 or 16进制
 */
inline CHAR * IntegerToString(WORD32  dwBase, 
                              WORD32  dwVal, 
                              CHAR   *pBuf)
{
    *pBuf-- = 0;

    do
    {        
        *pBuf-- = g_aucHex[dwVal % dwBase];
    }while ((dwVal /= dwBase) != 0);

    return pBuf + 1;
}


/* 将无符号整数转换为字符串
 * dwBase  : 10进制 or 16进制
 * dwWidth : 指定宽度
 */
inline CHAR * IntegerToString(WORD32  dwBase, 
                              WORD64  lwVal,  
                              CHAR   *pBuf,
                              WORD32  dwWidth,
                              CHAR    ucPad)
{
    *pBuf-- = 0;

    do
    { 
        if (dwWidth == 0)
        {
            break ;
        }

        *pBuf-- = g_aucHex[lwVal % dwBase];
        dwWidth--;
    }while ((lwVal /= dwBase) != 0);

    while (dwWidth > 0)
    {
        *pBuf-- = ucPad;
        dwWidth--;
    }

    return pBuf + 1;
}


/* 将有符号整数转换为字符串
 * dwBase  : 只支持10进制
 */
inline CHAR * IntegerToString(WORD32  dwBase, 
                              SWORD32 sdwVal, 
                              CHAR   *pBuf,
                              BOOL    bSigned)
{
    BOOL bIsNeg = (sdwVal < 0);

    *pBuf-- = 0;

    if (bIsNeg)
    {
        sdwVal = -sdwVal;
    }

    do
    {
        *pBuf-- = g_aucHex[sdwVal % dwBase];
    } while((sdwVal /= dwBase) != 0);

    if (bIsNeg)
    {
        *pBuf-- = '-';
    }

    return pBuf + 1;
}


/* 将有符号整数转换为字符串
 * dwBase  : 只支持10进制
 */
inline CHAR * IntegerToString(WORD32   dwBase,
                              SWORD64  slwVal,
                              CHAR    *pBuf,
                              BOOL     bSigned)
{
    BOOL bIsNeg = (slwVal < 0);

    *pBuf-- = 0;

    if (bIsNeg)
    {
        slwVal = -slwVal;
    }

    do
    {
        *pBuf-- = g_aucHex[slwVal % dwBase];
    } while((slwVal /= dwBase) != 0);

    if (bIsNeg)
    {
        *pBuf-- = '-';
    }

    return pBuf + 1;
}


WORD32 IntToStr(CHAR *pDst, WORD64 lwValue, WORD32 dwBase)
{
    WORD32 dwLen = 0;
    CHAR  *pStr  = NULL;
    CHAR   aucInteger[INTEGER_TO_STR_LEN] = {0};
        
    pStr = IntegerToString(dwBase, 
                           lwValue, 
                           &(aucInteger[INTEGER_TO_STR_LEN - 1]));
    while (*pStr)
    {
        *pDst++ = *pStr++;
        dwLen++;
    }

    return dwLen;
}


WORD32 IntToStr(CHAR   *pDst, 
                WORD64  lwValue, 
                WORD32  dwBase, 
                WORD32  dwWidth, 
                CHAR    ucPad)
{
    WORD32  dwLen = 0;
    CHAR   *pStr  = NULL;
    CHAR    aucInteger[INTEGER_TO_STR_LEN] = {0};
        
    pStr = IntegerToString(dwBase, 
                           lwValue, 
                           &(aucInteger[INTEGER_TO_STR_LEN - 1]),
                           dwWidth, 
                           ucPad);
    while (*pStr)
    {
        *pDst++ = *pStr++;
        dwLen++;
    }

    return dwLen;
}


const CHAR * CheckInteger(const CHAR *pFormat, 
                          E_Integer  &eInteger, 
                          WORD32     &dwInteger)
{
    eInteger  = E_INTEGER_INVALID;
    dwInteger = 0;

    for (WORD32 dwIndex = 0; 
         (dwIndex <= 2) && (*pFormat); 
         dwIndex++)
    {
        if ( ((*pFormat) >= '0')
          && ((*pFormat) <= '9'))
        {
            dwInteger = (dwInteger * 10) + ((*pFormat) - '0');
            ++pFormat;
            continue ;
        }
        else if ((*pFormat) == 'l')
        {
            ++pFormat;
            eInteger = E_INTEGER_64;
            continue ;
        }
        else if ((*pFormat) == 'd')
        {
            eInteger = (eInteger == E_INTEGER_INVALID) ? E_INTEGER_32 : eInteger;
            if (dwInteger >= INTEGER_TO_STR_LEN)
            {
                dwInteger = (INTEGER_TO_STR_LEN - 1);
            }
            break ;
        }
        else if ((*pFormat) == 'u')
        {
            eInteger = (eInteger == E_INTEGER_INVALID) ? E_INTEGER_32 : eInteger;
            if (dwInteger >= INTEGER_TO_STR_LEN)
            {
                dwInteger = (INTEGER_TO_STR_LEN - 1);
            }
            break ;
        }
        else
        {
            eInteger = E_INTEGER_INVALID;
            break ;
        }
    }

    return pFormat;
}
    

WORD32 base_vsnprintf(CHAR       *pDst, 
                      WORD32      dwSize, 
                      CHAR        ucPad, 
                      const CHAR *pFormat, 
                      va_list     tParamList)
{
    CHAR      *pStart    = pDst;
    CHAR      *pEnd      = pDst + dwSize - 1;
    E_Integer  eInteger  = E_INTEGER_INVALID;
    WORD32     dwInteger = 0;
    WORD64     lwValue   = 0;
    BOOL       bSigned   = FALSE;
    CHAR      *pStr      = NULL;
    CHAR       aucInteger[INTEGER_TO_STR_LEN] = {0};

    for (; *pFormat; ++pFormat)
    {
        if ((*pFormat) != '%')
        {
            /* 已经到buffer尾端 */
            if (pDst == pEnd)
            {
                break ;
            }

            *pDst++ = *pFormat;
            continue ;
        }

        ++pFormat;  /* 跳过'%' */
        pFormat = CheckInteger(pFormat, eInteger, dwInteger);

        switch (*pFormat)
        {
        case 'u' :
        case 'd' :
        case 'p' :
            {
                bSigned = FALSE;
                
                if (eInteger == E_INTEGER_32)
                {
                    lwValue = va_arg(tParamList, WORD32);
                    
                    if ((*pFormat) == 'd')
                    {
                        bSigned = TRUE;
                    }
                }
                else
                {
                    lwValue = va_arg(tParamList, WORD64);
                }

                if (dwInteger)
                {
                    pStr = IntegerToString(E_DECIMAL_10, 
                                           lwValue, 
                                           &(aucInteger[INTEGER_TO_STR_LEN - 1]),
                                           dwInteger,
                                           ucPad);
                }
                else
                {
                    if (bSigned)
                    {
                        pStr = IntegerToString(E_DECIMAL_10, 
                                               (SWORD32)lwValue, 
                                               &(aucInteger[INTEGER_TO_STR_LEN - 1]),
                                               bSigned);
                    }
                    else
                    {
                        pStr = IntegerToString(E_DECIMAL_10, 
                                               lwValue, 
                                               &(aucInteger[INTEGER_TO_STR_LEN - 1]));
                    }                    
                }
            
                while ((*pStr) && (pDst < pEnd))
                {
                    *pDst++ = *pStr++;
                }
            }
            break ;
            
        case 'c' :
            {
                const CHAR cVal = (CHAR)(va_arg(tParamList, WORD32));
                if (pDst < pEnd)
                {
                    *pDst++ = cVal;
                }
            }
            break ;
            
        case 's' :
            {
                const CHAR *pVal = va_arg(tParamList, CHAR *);
                if (pVal)
                {
                    while ((*pVal) && (pDst < pEnd))
                    {
                        *pDst++ = *pVal++;
                    }
                }
            }
            break ;

        default :
            {
                /* do nothing */
            }
            break ;
        }
    }

    *pDst  = 0;
    return (WORD32)(pDst - pStart);
}


WORD32 base_snprintf(CHAR *pDst, WORD32 dwSize, CHAR ucPad, const CHAR *pFormat, ...)
{
    WORD32 dwResult = INVALID_DWORD;

    va_list tParamList;
    va_start(tParamList, pFormat);
    dwResult = base_vsnprintf(pDst, dwSize, ucPad, pFormat, tParamList);
    va_end(tParamList);

    return dwResult;
}


WORD32 base_assemble(CHAR       *pDst, 
                     WORD32      dwSize, 
                     WORD16      wStrLen, 
                     const CHAR *pFormatStr, 
                     BYTE        ucParamNum,
                     WORD64     *plwParams)
{
    CHAR      *pStart       = pDst;
    CHAR      *pEnd         = pDst + dwSize - 1;
    E_Integer  eInteger     = E_INTEGER_INVALID;
    WORD32     dwInteger    = 0;
    BYTE       ucParamIndex = 0;
    SWORD32    sdwValue     = 0;
    SWORD64    slwValue     = 0;
    WORD64     lwParam      = 0;
    BOOL       bSigned      = FALSE;
    CHAR      *pStr         = NULL;
    CHAR       aucInteger[INTEGER_TO_STR_LEN] = {0};

    for (; *pFormatStr; ++pFormatStr)
    {
        if ((*pFormatStr) != '%')
        {
            /* 已经到buffer尾端 */
            if (pDst == pEnd)
            {
                break ;
            }

            *pDst++ = *pFormatStr;
            continue ;
        }

        ++pFormatStr;  /* 跳过'%' */
        pFormatStr = CheckInteger(pFormatStr, eInteger, dwInteger);

        if ((*pFormatStr) == 'd')
        {
            bSigned = TRUE;
        }
        else if ((*pFormatStr) == 'u')
        {
            bSigned = FALSE;
        }
        else
        {
            *pDst++ = *pFormatStr;
            continue ;
        }

        if (ucParamIndex >= ucParamNum)
        {
            continue ;
        }

        lwParam = plwParams[ucParamIndex];
        ucParamIndex++;

        if (lwParam <= 0xFFFFFFFF)
        {
            sdwValue = (SWORD32)(lwParam);

            if (bSigned)
            {
                pStr = IntegerToString(E_DECIMAL_10, 
                                       sdwValue, 
                                       &(aucInteger[INTEGER_TO_STR_LEN - 1]),
                                       bSigned);
            }
            else
            {
                pStr = IntegerToString(E_DECIMAL_10, 
                                       (WORD32)sdwValue, 
                                       &(aucInteger[INTEGER_TO_STR_LEN - 1]));
            }
        }
        else
        {
            slwValue = (SWORD64)(lwParam);

            if (bSigned)
            {
                pStr = IntegerToString(E_DECIMAL_10, 
                                       (SWORD64)slwValue, 
                                       &(aucInteger[INTEGER_TO_STR_LEN - 1]),
                                       bSigned);
            }
            else
            {
                pStr = IntegerToString(E_DECIMAL_10, 
                                       (WORD64)slwValue, 
                                       &(aucInteger[INTEGER_TO_STR_LEN - 1]));
            }
        }

        while ((*pStr) && (pDst < pEnd))
        {
            *pDst++ = *pStr++;
        }
    }
    
    *pDst  = 0;
    return (WORD32)(pDst - pStart);
}


