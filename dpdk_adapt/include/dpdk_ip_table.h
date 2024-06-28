

#ifndef _DPDK_IP_TABLE_H_
#define _DPDK_IP_TABLE_H_


#include "base_list.h"
#include "base_config_file.h"


#define IPV6_ADDR_LEN     ((WORD32)(16))
#define MAX_IP_NUM        ((WORD32)(256))


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

    WORD32 SetIPv6(CHAR *pIPv4Addr)
    {
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

protected :
    WORD32                    m_dwDeviceID;
    BOOL                      m_bVlanFlag;
    WORD32                    m_dwVlanID;
    E_IPAddrType              m_eAddrType;
    T_IPAddr                  m_tAddr;

    CString<IPV6_STRING_LEN>  m_cIPStr;
};


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


