

#ifndef _BASE_SNPRINTF_H_
#define _BASE_SNPRINTF_H_


#include <sys/types.h>
#include <stdarg.h>

#include "pub_typedef.h"


#define INTEGER_TO_STR_LEN    ((WORD32)(24))


typedef enum tagE_Decimal
{
    E_DECIMAL_10 = 10,
    E_DECIMAL_16 = 16,    
}E_Decimal;


typedef enum tagE_Integer
{
    E_INTEGER_INVALID = 0,
    E_INTEGER_32,
    E_INTEGER_64,
}E_Integer;


/* 不指定宽度 */
extern WORD32 IntToStr(CHAR *pDst, WORD64 lwValue, WORD32 dwBase);


extern WORD32 IntToStr(CHAR   *pDst, 
                       WORD64  lwValue, 
                       WORD32  dwBase, 
                       WORD32  dwWidth, 
                       CHAR    ucPad   = ' ');


extern WORD32 base_vsnprintf(CHAR       *pDst, 
                             WORD32      dwSize, 
                             CHAR        ucPad, 
                             const CHAR *pFormat, 
                             va_list     tParamList);

extern WORD32 base_snprintf(CHAR       *pDst, 
                            WORD32      dwSize, 
                            CHAR        ucPad, 
                            const CHAR *pFormat, ...);


extern WORD32 base_assemble(CHAR       *pDst, 
                            WORD32      dwSize, 
                            WORD16      wStrLen, 
                            const CHAR *pFormatStr, 
                            BYTE        ucParamNum,
                            WORD64     *plwParams);


#endif


