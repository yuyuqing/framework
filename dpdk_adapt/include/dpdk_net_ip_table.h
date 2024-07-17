

#ifndef _DPDK_IP_TABLE_H_
#define _DPDK_IP_TABLE_H_


#include "dpdk_common.h"

#include "base_list.h"
#include "base_config_file.h"


#define MAX_IP_NUM                      ((WORD32)(128))

#define ARP_MAC_ADDR_LEN                ((WORD32)(6))
#define IPV6_ADDR_LEN                   ((WORD32)(16))
#define IPV6_ADDR_WLEN                  ((WORD32)(8))
#define IPV6_ADDR_DWLEN                 ((WORD32)(4))
#define IPV6_ADDR_LWLEN                 ((WORD32)(2))


typedef struct tagT_MacAddr
{
    BYTE  aucMacAddr[ARP_MAC_ADDR_LEN];
}T_MacAddr;


typedef enum tagE_IPAddrType
{
    E_IPV4_TYPE = 0,
    E_IPV6_TYPE,
}E_IPAddrType;


typedef struct tagT_IPv4Addr
{
    WORD32    dwIPAddr;    /* °´ÍøÂç×Ö½ÚÐò */
}T_IPv4Addr;


typedef struct tagT_IPv6Addr
{
    union
    {
        BYTE     aucIPAddr[IPV6_ADDR_LEN];    /* °´ÍøÂç×Ö½ÚÐò */
        WORD16   awIPAddr[IPV6_ADDR_WLEN];    /* °´ÍøÂç×Ö½ÚÐò */
        WORD32   adwIPAddr[IPV6_ADDR_DWLEN];  /* °´ÍøÂç×Ö½ÚÐò */
        WORD64   alwIPAddr[IPV6_ADDR_LWLEN];  /* °´ÍøÂç×Ö½ÚÐò */
    };
}T_IPv6Addr;


typedef struct tagT_IPAddr
{
    union
    {
        T_IPv4Addr  tIPv4;  /* °´ÍøÂç×Ö½ÚÐò */
        T_IPv6Addr  tIPv6;  /* °´ÍøÂç×Ö½ÚÐò */
    };

    E_IPAddrType    eType;

    BOOL operator== (tagT_IPAddr &rtIPAddr)
    {
        if (rtIPAddr.eType != this->eType)
        {
            return FALSE;
        }

        if (E_IPV4_TYPE == rtIPAddr.eType)
        {
            return (rtIPAddr.tIPv4.dwIPAddr == this->tIPv4.dwIPAddr);
        }
        else
        {
            return ((rtIPAddr.tIPv6.alwIPAddr[0] == this->tIPv6.alwIPAddr[0])
                 && (rtIPAddr.tIPv6.alwIPAddr[1] == this->tIPv6.alwIPAddr[1]));
        }
    }

    WORD32 SetIPv4(CHAR *pIPv4Addr)
    {
        this->eType = E_IPV4_TYPE;

        WORD32  dwLen = strlen(pIPv4Addr);
        CHAR   *pData = pIPv4Addr;

        WORD32 dwByteNum = 0;
        WORD32 dwValue   = 0;
        WORD32 adwIP[4]  = {0, 0, 0, 0};

        for (WORD32 dwIndex = 0; dwIndex < dwLen; dwIndex++)
        {
            WORD32 dwByte = pData[dwIndex];

            if (dwByte == '.')
            {
                adwIP[dwByteNum++] = dwValue;
                dwValue = 0;
            }
            else
            {
                dwByte -= '0';
                dwValue = (dwValue * 10) + dwByte;

                if (dwIndex == (dwLen - 1))
                {
                    adwIP[dwByteNum++] = dwValue;
                    break ;
                }
            }
        }

        this->tIPv4.dwIPAddr = (adwIP[3] << 24) | (adwIP[2] << 16) | (adwIP[1] << 8) | adwIP[0];

        return SUCCESS;
    }

    WORD32 SetIPv6(CHAR *pIPv6Addr)
    {
        this->eType              = E_IPV6_TYPE;
        this->tIPv6.alwIPAddr[0] = 0;
        this->tIPv6.alwIPAddr[1] = 0;

        WORD32  dwLen = strlen(pIPv6Addr);
        CHAR    cLast = 0;
        CHAR   *pData = pIPv6Addr;

        WORD32 dwByteNum = 0;
        WORD16 wColonIdx = INVALID_WORD;
        WORD16 wColonNum = 0;
        WORD16 wSkipNum  = 0;
        WORD16 wValue    = 0;
        WORD16 awIP[IPV6_ADDR_WLEN]   = {0, 0, 0, 0, 0, 0, 0, 0};

        for (WORD32 dwIndex = 0; dwIndex < dwLen; dwIndex++)
        {
            WORD16 wByte = pData[dwIndex];

            if ((cLast == ':') && (wByte == ':'))
            {
                if (wColonIdx != INVALID_WORD)
                {
                    assert(0);
                }

                wColonIdx = (WORD16)dwByteNum;
            }
            else if (wByte == ':')
            {
                awIP[dwByteNum++] = wValue;
                wValue = 0;
            }
            else
            {
                if ((wByte >= '0') && (wByte <= '9'))
                {
                    wByte -= '0';
                }
                else if ((wByte >= 'a') && (wByte <= 'f'))
                {
                    wByte = 10 + (wByte - 'a');
                }
                else if ((wByte >= 'A') && (wByte <= 'F'))
                {
                    wByte = 10 + (wByte - 'A');
                }
                else
                {
                    assert(0);
                }

                wValue  = (wValue << 4) + wByte;

                if (dwIndex == (dwLen - 1))
                {
                    awIP[dwByteNum++] = wValue;
                    break ;
                }
            }

            cLast = pData[dwIndex];

            if (dwByteNum >= IPV6_ADDR_WLEN)
            {
                assert(0);
            }
        }

        if (wColonIdx != INVALID_WORD)
        {
            for (WORD32 dwIndex = 0; dwIndex < wColonIdx; dwIndex++)
            {
                this->tIPv6.awIPAddr[dwIndex] = HTONS(awIP[dwIndex]);
            }

            wSkipNum = IPV6_ADDR_WLEN - dwByteNum;

            for (WORD32 dwIndex = 0; dwIndex < (dwByteNum - wColonIdx); dwIndex++)
            {
                this->tIPv6.awIPAddr[dwIndex + wColonIdx + wSkipNum] = HTONS(awIP[dwIndex + wColonIdx]);
            }
        }
        else
        {
            for (WORD32 dwIndex = 0; dwIndex < IPV6_ADDR_WLEN; dwIndex++)
            {
                this->tIPv6.awIPAddr[dwIndex] = HTONS(awIP[dwIndex]);
            }
        }

        return SUCCESS;
    }

    WORD32 toStr(CString<IPV6_STRING_LEN> &rIPAddr)
    {
        WORD32 dwLen      = 0;
        WORD32 dwTotalLen = 0;

        if (E_IPV4_TYPE == this->eType)
        {
            WORD32 dwIP     = this->tIPv4.dwIPAddr;
            WORD32 adwIP[4] = {0, 0, 0, 0};
            CHAR   aucData[IPV4_STRING_LEN] = {0,};

            adwIP[0] = (dwIP & 0xFF);
            adwIP[1] = ((dwIP >>  8) & 0xFF);
            adwIP[2] = ((dwIP >> 16) & 0xFF);
            adwIP[3] = ((dwIP >> 24) & 0xFF);

            dwLen       = IntToStr(&(aucData[dwTotalLen]), adwIP[0], E_DECIMAL_10);
            dwTotalLen += dwLen;

            aucData[dwTotalLen++] = '.';
            dwLen       = IntToStr(&(aucData[dwTotalLen]), adwIP[1], E_DECIMAL_10);
            dwTotalLen += dwLen;

            aucData[dwTotalLen++] = '.';
            dwLen    = IntToStr(&(aucData[dwTotalLen]), adwIP[2], E_DECIMAL_10);
            dwTotalLen += dwLen;

            aucData[dwTotalLen++] = '.';
            dwLen    = IntToStr(&(aucData[dwTotalLen]), adwIP[3], E_DECIMAL_10);
            dwTotalLen += dwLen;

            rIPAddr = aucData;
        }
        else
        {
            CHAR aucIP[IPV6_ADDR_LEN];
            memcpy(aucIP, this->tIPv6.aucIPAddr, IPV6_ADDR_LEN);

            for (WORD32 dwIndex = 0; dwIndex < IPV6_ADDR_LEN; dwIndex++)
            {
                CHAR aucData[4] = {0, 0, 0, 0};

                dwLen    = IntToStr(aucData, aucIP[0], E_DECIMAL_16, 2, '0');
                rIPAddr += aucData;

                if ((dwIndex + 1) == IPV6_ADDR_LEN)
                {
                    break ;
                }
                rIPAddr += ':';
            }
        }

        return SUCCESS;
    }
}T_IPAddr;


class CIPTable;


class CIPInst : public CBaseData
{
public :
    friend class CIPTable;

public :
    CIPInst (WORD32                     dwDeviceID,
             BOOL                       bVlanFlag,
             WORD32                     dwVlanID,
             E_IPAddrType               eType,
             CString<IPV6_STRING_LEN>  &rIPAddr);

    virtual ~CIPInst();

    WORD32 GetDeviceID();
    WORD32 GetVlanID();
    WORD32 GetIPV4();

    E_IPAddrType GetIPType();

    T_IPAddr & GetIPAddr();

    CHAR * toChar();

protected :
    WORD32                    m_dwDeviceID;
    BOOL                      m_bVlanFlag;
    WORD32                    m_dwVlanID;
    T_IPAddr                  m_tAddr;

    CString<IPV6_STRING_LEN>  m_cIPStr;
};


inline WORD32 CIPInst::GetDeviceID()
{
    return m_dwDeviceID;
}


inline WORD32 CIPInst::GetVlanID()
{
    return (m_bVlanFlag) ? m_dwVlanID : INVALID_DWORD;
}


inline WORD32 CIPInst::GetIPV4()
{
    return (E_IPV4_TYPE == m_tAddr.eType) ? (m_tAddr.tIPv4.dwIPAddr) : 0;
}


inline E_IPAddrType CIPInst::GetIPType()
{
    return m_tAddr.eType;
}


inline T_IPAddr & CIPInst::GetIPAddr()
{
    return m_tAddr;
}


inline CHAR * CIPInst::toChar()
{
    return m_cIPStr.toChar();
}


class CIPTable : public CBaseList<CIPInst, MAX_IP_NUM, FALSE>
{
public :
    CIPTable ();
    virtual ~CIPTable();

    WORD32 Initialize();

    WORD32 RegistIP(T_DpdkEthDevJsonCfg &rtCfg);

    /* dwIP : °´ÍøÂç×Ö½ÚÐò */
    CIPInst * FindByIPv4(WORD32 dwIP);

    CIPInst * FindByIPv6(T_IPAddr &rtIPAddr);
};


#endif


