

#ifndef _DPDK_IP_TABLE_H_
#define _DPDK_IP_TABLE_H_


#include "base_list.h"
#include "base_config_file.h"


#define IPV6_ADDR_LEN     ((WORD32)(16))
#define MAX_IP_NUM        ((WORD32)(128))


typedef enum tagE_IPAddrType
{
    E_IPV4_TYPE = 0,
    E_IPV6_TYPE,
}E_IPAddrType;


typedef union tagT_IPAddr
{
    WORD32    dwIPv4;                  /* °´ÍøÂç×Ö½ÚÐò */
    BYTE      aucIPv6[IPV6_ADDR_LEN];  /* °´ÍøÂç×Ö½ÚÐò */

    BOOL operator== (tagT_IPAddr &rIPAddr)
    {
        return (0 == memcmp(rIPAddr.aucIPv6, this->aucIPv6, IPV6_ADDR_LEN));
    }

    WORD32 SetIPv4(CHAR *pIPv4Addr)
    {
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

        this->dwIPv4 = (adwIP[3] << 24) | (adwIP[2] << 16) | (adwIP[1] << 8) | adwIP[0];

        return SUCCESS;
    }

    WORD32 SetIPv6(CHAR *pIPv6Addr)
    {
        WORD32  dwLen = strlen(pIPv6Addr);
        CHAR   *pData = pIPv6Addr;

        WORD32 dwByteNum = 0;
        WORD16 wValue    = 0;
        WORD16 awIP[8]   = {0, 0, 0, 0, 0, 0, 0, 0};

        for (WORD32 dwIndex = 0; dwIndex < dwLen; dwIndex++)
        {
            WORD16 wByte = pData[dwIndex];

            if (wByte == ':')
            {
                awIP[dwByteNum++] = wValue;
                wValue = 0;
            }
            else if (wByte > '9')
            {
                switch (wByte)
                {
                case 'a' :
                case 'A' :
                    wByte = 10;
                    break;

                case 'b' :
                case 'B' :
                    wByte = 11;
                    break;

                case 'c' :
                case 'C' :
                    wByte = 12;
                    break;

                case 'd' :
                case 'D' :
                    wByte = 13;
                    break;

                case 'e' :
                case 'E' :
                    wByte = 14;
                    break;

                case 'f' :
                case 'F' :
                    wByte = 15;
                    break;

                default :
                    /* ÅäÖÃ´íÎó */
                    assert(0);
                    break ;
                }

                wValue  = (wValue << 4) + wByte;

                if (dwIndex == (dwLen - 1))
                {
                    awIP[dwByteNum++] = wValue;
                    break ;
                }
            }
            else
            {
                wByte  -= '0';
                wValue  = (wValue << 4) + wByte;

                if (dwIndex == (dwLen - 1))
                {
                    awIP[dwByteNum++] = wValue;
                    break ;
                }
            }
        }

        for (WORD32 dwIndex = 0; dwIndex < 8; dwIndex++)
        {
            this->aucIPv6[(2 * dwIndex) + 0] = (BYTE)(awIP[dwIndex] >> 8);
            this->aucIPv6[(2 * dwIndex) + 1] = (BYTE)(awIP[dwIndex] & 0x00FF);
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
    E_IPAddrType              m_eAddrType;
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
    return (E_IPV4_TYPE == m_eAddrType) ? (m_tAddr.dwIPv4) : 0;
}


inline E_IPAddrType CIPInst::GetIPType()
{
    return m_eAddrType;
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


