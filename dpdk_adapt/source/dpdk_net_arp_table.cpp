

#include "dpdk_mgr.h"
#include "dpdk_device_eth.h"

#include "base_log.h"


CArpInst::CArpInst (WORD32     dwDevID,
                    T_IPAddr  &rtIPAddr,
                    BYTE      *pMacAddr)
{
    m_dwDeviceID = dwDevID;

    memcpy(&m_tIPAddr,  &rtIPAddr, sizeof(T_IPAddr));
    memcpy(m_tMacAddr.aucMacAddr, pMacAddr, ARP_MAC_ADDR_LEN);
}


CArpInst::~CArpInst()
{
    m_dwDeviceID = INVALID_DWORD;

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

    m_tIPAddr.toStr(tIPAddr);

    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_INFO, TRUE,
               "DeviceID : %d, Type : %d, IPAddr : %s, MacAddr : %s\n",
               m_dwDeviceID,
               m_tIPAddr.eType,
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
        T_IPAddr     &rtIPAddr   = pCur->GetIPAddr();
        CEthDevice   *pDevice    = (CEthDevice *)(g_pDpdkMgr->FindDevice(dwDeviceID));
        BYTE         *pMacAddr   = pDevice->GetMacAddr();
        CArpInst     *pInst      = CreateTail();
        if (NULL == pInst)
        {
            return FAIL;
        }

        new (pInst) CArpInst(dwDeviceID, rtIPAddr, pMacAddr);

        pCur = rIPTable.Next(pCur);
    }

    return SUCCESS;
}


CArpInst * CArpTable::RegistArp(WORD32      dwDeviceID,
                                T_IPAddr   &rtIPAddr,
                                T_MacAddr  &rtMacAddr)
{
    CGuardLock<CSpinLock> cGuard(m_cLock);

    CArpInst *pInst = CreateTail();
    if (NULL == pInst)
    {
        return NULL;
    }

    new (pInst) CArpInst(dwDeviceID, rtIPAddr, rtMacAddr.aucMacAddr);

    return pInst;
}


CArpInst * CArpTable::FindArp(WORD32 dwDevID, WORD32 dwIPv4)
{
    CGuardLock<CSpinLock> cGuard(m_cLock);

    CArpInst *pCur = GetHead();
    while (pCur)
    {
        if ( (dwDevID == pCur->m_dwDeviceID)
          && (E_IPV4_TYPE == pCur->m_tIPAddr.eType)
          && (dwIPv4 == pCur->m_tIPAddr.tIPv4.dwIPAddr))
        {
            return pCur;
        }

        pCur = Next(pCur);
    }

    return NULL;
}


CArpInst * CArpTable::FindArp(WORD32 dwDevID, T_IPAddr &rtIPAddr)
{
    CGuardLock<CSpinLock> cGuard(m_cLock);

    CArpInst *pCur = GetHead();
    while (pCur)
    {
        if ( (dwDevID == pCur->m_dwDeviceID)
          && (rtIPAddr == pCur->m_tIPAddr))
        {
            return pCur;
        }

        pCur = Next(pCur);
    }

    return NULL;
}


VOID CArpTable::Dump()
{
    TRACE_STACK("CArpTable::Dump()");

    CArpInst *pCur = GetHead();

    while (pCur)
    {
        pCur->Dump();
        pCur = Next(pCur);
    }
}


