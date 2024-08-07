

#include "dpdk_net_ip_table.h"

#include "base_log.h"


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


VOID CIPInst::Dump()
{
    CString<IPV6_STRING_LEN> cIPAddr;
    m_tAddr.toStr(cIPAddr);

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "m_dwDeviceID : %d, m_bVlanFlag : %d, m_dwVlanID : %d, "
               "IPAddr : %s, IPStr : %s\n",
               m_dwDeviceID,
               m_bVlanFlag,
               m_dwVlanID,
               cIPAddr.toChar(),
               m_cIPStr.toChar());
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


WORD32 CIPTable::RegistIP(T_IPv6Addr *ptAddr,
                          WORD32      dwDeviceID,
                          BOOL        bVlanFlag,
                          WORD32      dwVlanID)
{
    T_IPAddr                  tIPAddr;
    CString<IPV6_STRING_LEN>  cIPStr;

    tIPAddr.eType = E_IPV6_TYPE;
    memcpy(&(tIPAddr.tIPv6), ptAddr, sizeof(T_IPv6Addr));

    tIPAddr.toStr(cIPStr);

    CIPInst *pInst = CreateTail();
    if (NULL == pInst)
    {
        return FAIL;
    }

    new (pInst) CIPInst(dwDeviceID, bVlanFlag, dwVlanID, E_IPV6_TYPE, cIPStr);

    return SUCCESS;
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


/* dwIP : �������ֽ��� */
CIPInst * CIPTable::FindByIPv4(WORD32 dwDeviceID, WORD32 dwIP)
{
    CIPInst *pCur = GetHead();

    while (pCur)
    {
        if ( (dwDeviceID == pCur->m_dwDeviceID)
          && (E_IPV4_TYPE == pCur->m_tAddr.eType)
          && (dwIP == pCur->m_tAddr.tIPv4.dwIPAddr))
        {
            return pCur;
        }

        pCur = Next(pCur);
    }

    return NULL;
}


CIPInst * CIPTable::FindByIPv6(WORD32 dwDeviceID, T_IPAddr &rtIPAddr)
{
    CIPInst *pCur = GetHead();

    while (pCur)
    {
        if ( (dwDeviceID == pCur->m_dwDeviceID)
          && (E_IPV6_TYPE == pCur->m_tAddr.eType)
          && (rtIPAddr == pCur->m_tAddr))
        {
            return pCur;
        }

        pCur = Next(pCur);
    }

    return NULL;
}


VOID CIPTable::Dump()
{
    TRACE_STACK("CIPTable::Dump()");

    CIPInst *pCur = GetHead();

    while (pCur)
    {
        pCur->Dump();
        pCur = Next(pCur);
    }
}


