

#include "dpdk_net_vlan_table.h"


CVlanInst::CVlanInst ()
{
    m_dwDeviceID = INVALID_DWORD;
    m_dwVlanID   = INVALID_DWORD;
    m_dwPriority = INVALID_DWORD;

    memset(&m_tIPAddr, 0x00, sizeof(m_tIPAddr));
}


CVlanInst::~CVlanInst()
{
    m_dwDeviceID = INVALID_DWORD;
    m_dwVlanID   = INVALID_DWORD;
    m_dwPriority = INVALID_DWORD;

    memset(&m_tIPAddr, 0x00, sizeof(m_tIPAddr));
}


WORD32 CVlanInst::Initialize(WORD32                     dwDeviceID,
                             WORD32                     dwVlanID,
                             WORD32                     dwPriority,
                             E_IPAddrType               eType,
                             CString<IPV6_STRING_LEN>  &rIPAddr)
{
    m_dwDeviceID = dwDeviceID;
    m_dwVlanID   = dwVlanID;
    m_dwPriority = dwPriority;

    memset(&m_tIPAddr, 0x00, sizeof(m_tIPAddr));

    if (E_IPV4_TYPE == eType)
    {
        m_tIPAddr.SetIPv4(rIPAddr.toChar());
    }
    else
    {
        m_tIPAddr.SetIPv6(rIPAddr.toChar());
    }

    return SUCCESS;
}


CVlanTable::CVlanTable ()
{
}


CVlanTable::~CVlanTable()
{
}


WORD32 CVlanTable::Initialize()
{
    return CBaseSequence<T_VlanKey, CVlanInst, MAX_VLAN_NUM>::Initialize();
}


WORD32 CVlanTable::RegistVlan(T_DpdkEthDevJsonCfg &rtCfg)
{
    WORD32    dwDeviceID = rtCfg.dwDeviceID;
    WORD32    dwVlanNum  = rtCfg.dwVlanNum;
    WORD32    dwVlanID   = INVALID_DWORD;
    WORD32    dwPriority = INVALID_DWORD;
    T_VlanKey tKey;

    for (WORD32 dwIndex = 0; dwIndex < dwVlanNum; dwIndex++)
    {
        T_DpdkEthVlanJsonCfg     &rtVlanCfg = rtCfg.atVlan[dwIndex];
        CString<IPV6_STRING_LEN>  cIPStr;

        dwVlanID   = rtVlanCfg.dwVlanID;
        dwPriority = rtVlanCfg.dwPriority;

        tKey.dwDeviceID = dwDeviceID;
        tKey.dwVlanID   = dwVlanID;

        CVlanInst *pInst = CreateTail(tKey);
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

        pInst->Initialize(dwDeviceID,
                          dwVlanID,
                          dwPriority,
                          (E_IPAddrType)(rtVlanCfg.tIP.dwIPType),
                          cIPStr);
    }

    return SUCCESS;
}


CVlanInst * CVlanTable::FindVlan(WORD32 dwDeviceID, WORD32 dwVlanID)
{
    T_VlanKey tKey;

    tKey.dwDeviceID = dwDeviceID;
    tKey.dwVlanID   = dwVlanID;

    return Find(tKey);
}


