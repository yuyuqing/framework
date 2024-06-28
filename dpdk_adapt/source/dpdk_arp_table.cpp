

#include "dpdk_device_eth.h"
#include "dpdk_mgr.h"


CArpInst::CArpInst (E_IPAddrType  eType,
                    T_IPAddr     &rtIPAddr,
                    BYTE         *pMacAddr)
{
    m_eAddrType = eType;

    memcpy(&m_tIPAddr,  &rtIPAddr, sizeof(T_IPAddr));
    memcpy(m_tMacAddr.aucMacAddr, pMacAddr, ARP_MAC_ADDR_LEN);
}


CArpInst::~CArpInst()
{
    m_eAddrType = E_IPV4_TYPE;

    memset(&m_tIPAddr,  0x00, sizeof(m_tIPAddr));
    memset(&m_tMacAddr, 0x00, sizeof(m_tMacAddr));
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

        new (pInst) CArpInst(eType, rtIPAddr, pMacAddr);

        pCur = rIPTable.Next(pCur);
    }

    return SUCCESS;
}


WORD32 CArpTable::RegistArp(E_IPAddrType  eType,
                            T_IPAddr     &rtIPAddr,
                            T_MacAddr    &rtMacAddr)
{
    CGuardLock<CSpinLock> cGuard(m_cLock);

    CArpInst *pInst = CreateTail();
    if (NULL == pInst)
    {
        return FAIL;
    }

    new (pInst) CArpInst(eType, rtIPAddr, rtMacAddr.aucMacAddr);

    return SUCCESS;
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


