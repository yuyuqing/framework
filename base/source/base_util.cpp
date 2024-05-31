

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#include "base_util.h"


WORD32 Rand(WORD32 dwMod)
{
    WORD32 dwSeed = 0;

    FILE * pFile = fopen("/dev/urandom", "r");
    if (NULL != pFile)
    {
        if (fread(&dwSeed, sizeof(dwSeed), 1, pFile) <= 0)
        {
            dwSeed = time(NULL);
        }

        fclose(pFile);
    }
    else
    {
        dwSeed = time(NULL);
    }

    srand(dwSeed);
    return (rand() % dwMod);
}


WORD32 SocketID(WORD32 dwCoreIndex)
{
    WORD32 dwSocket = 0;

    for (; dwSocket < MAX_CPU_CORE_NUM; dwSocket++)
    {
        CHAR acPath[PATH_MAX] = {0};

        snprintf(acPath, 
                 sizeof(acPath), 
                 "%s/node%u/cpu%u", 
                 NUMA_NODE_PATH,
                 dwSocket, 
                 dwCoreIndex);

        if (access(acPath, F_OK) == 0)
        {
            return dwSocket;
        }
    }

    return 0;
}


WORD32 CoreID(WORD32 dwCoreIndex)
{
    CHAR    acPath[PATH_MAX] = {0};
    CHAR    acBuf[BUFSIZ]    = {0};
    CHAR   *pEnd             = NULL;
    WORD64  lwID             = 0;

    SWORD32 sdwLen = snprintf(acPath, 
                              sizeof(acPath), 
                              SYS_CPU_DIR "/%s", 
                              dwCoreIndex, 
                              CORE_ID_FILE);
    if ((sdwLen <= 0) || (((WORD32)(sdwLen)) >= sizeof(acPath)))
    {
        return 0;
    }

    FILE *pFile = fopen(acPath, "r");
    if (NULL == pFile)
    {
        return 0;
    }

    if (fgets(acBuf, sizeof(acBuf), pFile) == NULL) 
    {
        fclose(pFile);
        return 0;
    }

    lwID = strtoul(acBuf, &pEnd, 0);
    if ((acBuf[0] == '\0') || (pEnd == NULL)) 
    {
        fclose(pFile);
        return 0;
    }

    if (*pEnd != '\n')
    {
        fclose(pFile);
        return 0;
    }

    fclose(pFile);

    return ((WORD32)lwID);
}


BOOL CpuDetected(WORD32 dwCoreIndex)
{
    CHAR    acPath[PATH_MAX] = {0};
    SWORD32 sdwLen = snprintf(acPath, 
                              sizeof(acPath), 
                              SYS_CPU_DIR "/" CORE_ID_FILE, 
                              dwCoreIndex);

    if ((sdwLen <= 0) || (((WORD32)(sdwLen)) >= sizeof(acPath)))
    {
        return FALSE;
    }

    if (access(acPath, F_OK) != 0)
    {
        return FALSE;
    }

    return TRUE;
}


BOOL IsRoot()
{
    return (0 == geteuid());
}


BYTE GetHostEncoding()
{
    static const SWORD32 s_iTmp = 1;

    if (1 == *((const CHAR *)(&s_iTmp)))
    {
        return (BYTE)1;
    }
    else
    {
        return (BYTE)2;
    }
}


WORD32 Hash(const BYTE *pName)
{
    WORD32 dwH = 0;
    WORD32 dwG = 0;

    while (*pName)
    {
        dwH = (dwH << 4) + *pName++;
        dwG = dwH & 0xF0000000;

        if (dwG != 0)
        {
            dwH ^= dwG >> 24;
        }

        dwH &= ~dwG;
    }

    return dwH;
}


WORD32 LocalTime(time_t *ptSrcTime, tm &rDstTime)
{
    SWORD64 lwHpery     = 0;
    SWORD64 lwPass4Year = 0;

    const static WORD32 s_dwOneYearHours = 8760;
    const static CHAR   s_aucDays[12]    = {31, 28, 31, 30, 
                                            31, 30, 31, 31, 
                                            30, 31, 30, 31};

    time_t tTime = *ptSrcTime;

    tTime += 28800;
    rDstTime.tm_isdst = 0;

    if (tTime < 0)
    {
        tTime = 0;
    }

    rDstTime.tm_sec = (SWORD32)(tTime % 60);
    tTime /= 60;
    rDstTime.tm_min = (SWORD32)(tTime % 60);
    tTime /= 60;
    rDstTime.tm_wday = (tTime/24 + 4) % 7;

    lwPass4Year = (WORD32)(tTime / (1461L * 24L));
    rDstTime.tm_year = (lwPass4Year << 2) + 70;

    tTime %= (1461L * 24L);
    rDstTime.tm_yday = (tTime / 24) % 365;

    while (TRUE)
    {
        lwHpery = s_dwOneYearHours;

        if (0 == (rDstTime.tm_year & 3))
        {
            lwHpery += 24;
        }

        if (tTime < lwHpery)
        {
            break ;
        }

        rDstTime.tm_year++;
        tTime -= lwHpery;
    }

    rDstTime.tm_hour = (SWORD32)(tTime % 24);
    tTime /= 24;
    tTime++;

    if (0 == (rDstTime.tm_year & 3))
    {
        if (tTime > 60)
        {
            tTime--;
        }
        else
        {
            if (tTime == 60)
            {
                rDstTime.tm_mon  = 1;
                rDstTime.tm_mday = 29;

                return SUCCESS;
            }
        }
    }

    for (rDstTime.tm_mon = 0; s_aucDays[rDstTime.tm_mon] < tTime; rDstTime.tm_mon++)
    {
        tTime -= s_aucDays[rDstTime.tm_mon];
    }

    rDstTime.tm_mday = (SWORD32)(tTime);

    return SUCCESS;
}


/* ¼ÆËã¶ÔÊýlog(N) */
BYTE CalcLog32BitNBase2Ceil(WORD32 dwNumber)
{
    WORD32  adwB[] = {0x02, 0x0C, 0xF0, 0xFF00, 0xFFFF0000};
    WORD32  adwS[] = {1,    2,    4,    8,      16};
    WORD32  dwTmp  = dwNumber;
    BYTE    ucRet  = 0;

    for (WORD32 dwIndex = 4; dwIndex >= 0; dwIndex--)
    {
        if (dwNumber & adwB[dwIndex])
        {
            dwNumber >>= adwS[dwIndex];
            ucRet |= adwS[dwIndex];
        }
    }

    if (dwTmp)
    {
        if (dwTmp & (dwTmp - 1))
        {
            ucRet++;
        }
    }

    return ucRet;
}


CEndianConvertor::CEndianConvertor ()
{
    m_bNeedConvert = FALSE;
}


CEndianConvertor::~CEndianConvertor()
{
    m_bNeedConvert = FALSE;
}


VOID CEndianConvertor::Setup(BYTE ucElfFileEncode)
{
    m_bNeedConvert = (ucElfFileEncode != GetHostEncoding());
}


WORD64 CEndianConvertor::operator() (WORD64 lwValue) const
{
    if (!m_bNeedConvert)
    {
        return lwValue;
    }

    lwValue = ( ( lwValue & 0x00000000000000FFull ) << 56 ) |
              ( ( lwValue & 0x000000000000FF00ull ) << 40 ) |
              ( ( lwValue & 0x0000000000FF0000ull ) << 24 ) |
              ( ( lwValue & 0x00000000FF000000ull ) << 8 ) |
              ( ( lwValue & 0x000000FF00000000ull ) >> 8 ) |
              ( ( lwValue & 0x0000FF0000000000ull ) >> 24 ) |
              ( ( lwValue & 0x00FF000000000000ull ) >> 40 ) |
              ( ( lwValue & 0xFF00000000000000ull ) >> 56 );

    return lwValue;
}


SWORD64 CEndianConvertor::operator() (SWORD64 slwValue) const
{
    return (SWORD64)((*this)((WORD64)(slwValue)));
}


WORD32 CEndianConvertor::operator() (WORD32 dwValue) const
{
    if (!m_bNeedConvert)
    {
        return dwValue;
    }

    dwValue = (( dwValue & 0x000000FF ) << 24 ) 
            | (( dwValue & 0x0000FF00 ) << 8 )
            | (( dwValue & 0x00FF0000 ) >> 8 )
            | (( dwValue & 0xFF000000 ) >> 24 );
    return dwValue;
}


SWORD32 CEndianConvertor::operator() (SWORD32 sdwValue) const
{
    return (SWORD32)((*this)((WORD32)(sdwValue)));
}


WORD16 CEndianConvertor::operator() (WORD16 wValue) const
{
    if (!m_bNeedConvert)
    {
        return wValue;
    }

    wValue = (( wValue & 0x00FF ) << 8 )
           | (( wValue & 0xFF00 ) >> 8 );
    return wValue;
}


SWORD16 CEndianConvertor::operator() (SWORD16 swValue) const
{
    return (SWORD16)((*this)((WORD16)(swValue)));
}


BYTE CEndianConvertor::operator() (BYTE ucValue) const
{
    return ucValue;
}


CHAR CEndianConvertor::operator() (CHAR cValue) const
{
    return cValue;
}


