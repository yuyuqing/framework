

#include "dpdk_net_ip_table.h"


CIPInst::CIPInst (WORD32                     dwDeviceID,
                  BOOL                       bVlanFlag,
                  WORD32                     dwVlanID,
                  E_IPAddrType               eType,
                  CString<IPV6_STRING_LEN>  &rIPAddr)
    : m_cIPStr(rIPAddr)
{
    m_dwDeviceID = dwDeviceID;
    m_bVlanFlag  = bVlanFlag;
    m_dwVlanID   = dwVlanID;
    m_eAddrType  = eType;

    memset(&m_tAddr, 0x00, sizeof(m_tAddr));

    if (E_IPV4_TYPE == eType)
    {
        m_tAddr.SetIPv4(rIPAddr.toChar());
    }
    else
    {
        m_tAddr.SetIPv6(rIPAddr.toChar());
    }
}


CIPInst::~CIPInst()
{
}


CIPTable::CIPTable ()
{
}


CIPTable::~CIPTable()
{
}


WORD32 CIPTable::Initialize()
{
    return CBaseList<CIPInst, MAX_IP_NUM, FALSE>::Initialize();
}


WORD32 CIPTable::RegistIP(T_DpdkEthDevJsonCfg &rtCfg)
{
    WORD32    dwDeviceID = rtCfg.dwDeviceID;
    WORD32    dwIPNum    = rtCfg.dwIpNum;
    WORD32    dwVlanNum  = rtCfg.dwVlanNum;
    WORD32    dwVlanID   = INVALID_DWORD;

    for (WORD32 dwIndex = 0; dwIndex < dwIPNum; dwIndex++)
    {
        T_DpdkEthIPJsonCfg       &rtIPCfg = rtCfg.atIP[dwIndex];
        CString<IPV6_STRING_LEN>  cIPStr;

        CIPInst *pInst = CreateTail();
        if (NULL == pInst)
        {
            return FAIL;
        }

        if (E_IPV4_TYPE == rtIPCfg.dwIPType)
        {
            cIPStr = rtIPCfg.aucIpv4Addr;
        }
        else
        {
            cIPStr = rtIPCfg.aucIpv6Addr;
        }

        new (pInst) CIPInst(dwDeviceID,
                            FALSE,
                            INVALID_DWORD,
                            (E_IPAddrType)(rtIPCfg.dwIPType),
                            cIPStr);
    }

    for (WORD32 dwIndex = 0; dwIndex < dwVlanNum; dwIndex++)
    {
        T_DpdkEthVlanJsonCfg     &rtVlanCfg = rtCfg.atVlan[dwIndex];
        CString<IPV6_STRING_LEN>  cIPStr;

        CIPInst *pInst = CreateTail();
        if (NULL == pInst)
        {
            return FAIL;
        }

        if (E_IPV4_TYPE == rtVlanCfg.tIP.dwIPType)
        {
            cIPStr = rtVlanCfg.tIP.aucIpv4Addr;
        }
        else
        {
            cIPStr = rtVlanCfg.tIP.aucIpv6Addr;
        }

        dwVlanID = rtVlanCfg.dwVlanID;

        new (pInst) CIPInst(dwDeviceID,
                            TRUE,
                            dwVlanID,
                            (E_IPAddrType)(rtVlanCfg.tIP.dwIPType),
                            cIPStr);
    }

    return SUCCESS;
}


/* dwIP : °´ÍøÂç×Ö½ÚÐò */
CIPInst * CIPTable::FindByIPv4(WORD32 dwIP)
{
    CIPInst *pCur = GetHead();

    while (pCur)
    {
        if ( (E_IPV4_TYPE == pCur->m_eAddrType)
          && (dwIP == pCur->m_tAddr.dwIPv4))
        {
            return pCur;
        }

        pCur = Next(pCur);
    }

    return NULL;
}


CIPInst * CIPTable::FindByIPv6(T_IPAddr &rtIPAddr)
{
    CIPInst *pCur = GetHead();

    while (pCur)
    {
        if ( (E_IPV6_TYPE == pCur->m_eAddrType)
          && (rtIPAddr == pCur->m_tAddr))
        {
            return pCur;
        }

        pCur = Next(pCur);
    }

    return NULL;
}


