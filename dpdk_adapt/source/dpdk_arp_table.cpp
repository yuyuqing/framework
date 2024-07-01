

#include "dpdk_device_eth.h"
#include "dpdk_mgr.h"


CArpInst::CArpInst (WORD32        dwDevID,
                    E_IPAddrType  eType,
                    T_IPAddr     &rtIPAddr,
                    BYTE         *pMacAddr)
{
    m_dwDeviceID = dwDevID;
    m_eAddrType  = eType;

    memcpy(&m_tIPAddr,  &rtIPAddr, sizeof(T_IPAddr));
    memcpy(m_tMacAddr.aucMacAddr, pMacAddr, ARP_MAC_ADDR_LEN);
}


CArpInst::~CArpInst()
{
    m_dwDeviceID = INVALID_DWORD;
    m_eAddrType  = E_IPV4_TYPE;

    memset(&m_tIPAddr,  0x00, sizeof(m_tIPAddr));
    memset(&m_tMacAddr, 0x00, sizeof(m_tMacAddr));
}


VOID CArpInst::Dump()
{
    CHAR aucMacAddr[24] = {0,};

    sprintf(aucMacAddr,
            "%02x:%02x:%02x:%02x:%02x:%02x",
            m_tMacAddr.aucMacAddr[0], m_tMacAddr.aucMacAddr[1],
            m_tMacAddr.aucMacAddr[2], m_tMacAddr.aucMacAddr[3],
            m_tMacAddr.aucMacAddr[4], m_tMacAddr.aucMacAddr[5]);

    CString <IPV6_STRING_LEN>  tIPAddr;

    m_tIPAddr.toStr(m_eAddrType, tIPAddr);

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "DeviceID : %d, Type : %d, IPAddr : %s, MacAddr : %s\n",
               m_dwDeviceID,
               m_eAddrType,
               tIPAddr.toChar(),
               aucMacAddr);
}


CArpTable::CArpTable ()
{
}


CArpTable::~CArpTable()
{
}


WORD32 CArpTable::Initialize(CIPTable &rIPTable)
{
    CBaseList<CArpInst, MAX_ARP_NUM, FALSE>::Initialize();

    CIPInst *pCur = rIPTable.GetHead();
    while (pCur)
    {
        WORD32        dwDeviceID = pCur->GetDeviceID();
        E_IPAddrType  eType      = pCur->GetIPType();
        T_IPAddr     &rtIPAddr   = pCur->GetIPAddr();
        CEthDevice   *pDevice    = (CEthDevice *)(g_pDpdkMgr->FindDevice(dwDeviceID));
        BYTE         *pMacAddr   = pDevice->GetMacAddr();
        CArpInst     *pInst      = CreateTail();
        if (NULL == pInst)
        {
            return FAIL;
        }

        new (pInst) CArpInst(dwDeviceID, eType, rtIPAddr, pMacAddr);

        pCur = rIPTable.Next(pCur);
    }

    return SUCCESS;
}


CArpInst * CArpTable::RegistArp(WORD32        dwDeviceID,
                                E_IPAddrType  eType,
                                T_IPAddr     &rtIPAddr,
                                T_MacAddr    &rtMacAddr)
{
    CGuardLock<CSpinLock> cGuard(m_cLock);

    CArpInst *pInst = CreateTail();
    if (NULL == pInst)
    {
        return NULL;
    }

    new (pInst) CArpInst(dwDeviceID, eType, rtIPAddr, rtMacAddr.aucMacAddr);

    return pInst;
}


CArpInst * CArpTable::FindArp(WORD32 dwIPv4)
{
    CGuardLock<CSpinLock> cGuard(m_cLock);

    CArpInst *pCur = GetHead();
    while (pCur)
    {
        if ( (E_IPV4_TYPE == pCur->m_eAddrType)
          && (dwIPv4 == pCur->m_tIPAddr.dwIPv4))
        {
            return pCur;
        }

        pCur = Next(pCur);
    }

    return NULL;
}


CArpInst * CArpTable::FindArp(T_IPAddr &rtIPAddr)
{
    CGuardLock<CSpinLock> cGuard(m_cLock);

    CArpInst *pCur = GetHead();
    while (pCur)
    {
        if (rtIPAddr == pCur->m_tIPAddr)
        {
            return pCur;
        }

        pCur = Next(pCur);
    }

    return NULL;
}


