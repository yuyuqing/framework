

#include "dpdk_net_ndp_table.h"
#include "dpdk_net_interface.h"


CNdpInst::CNdpInst (WORD32      dwDeviceID,
                    T_IPv6Addr *ptIPAddr,
                    T_MacAddr  *ptMacAddr)
{
    m_dwDeviceID = dwDeviceID;

    memcpy(&m_tIPAddr,  ptIPAddr,  sizeof(T_IPv6Addr));
    memcpy(&m_tMacAddr, ptMacAddr, sizeof(T_MacAddr));
}


CNdpInst::~CNdpInst()
{
}


BOOL CNdpInst::operator==(const T_IPv6Addr &rtIP)
{
    return ((rtIP.alwIPAddr[0] == m_tIPAddr.alwIPAddr[0])
         && (rtIP.alwIPAddr[1] == m_tIPAddr.alwIPAddr[1]));
}


BOOL CNdpInst::IsMatch(BYTE *pMacAddr)
{
    return ((pMacAddr[0] == m_tMacAddr.aucMacAddr[0])
         && (pMacAddr[1] == m_tMacAddr.aucMacAddr[1])
         && (pMacAddr[2] == m_tMacAddr.aucMacAddr[2])
         && (pMacAddr[3] == m_tMacAddr.aucMacAddr[3])
         && (pMacAddr[4] == m_tMacAddr.aucMacAddr[4])
         && (pMacAddr[5] == m_tMacAddr.aucMacAddr[5]));
}


VOID CNdpInst::UpdateMacAddr(BYTE *pMacAddr)
{
    memcpy(m_tMacAddr.aucMacAddr, pMacAddr, ARP_MAC_ADDR_LEN);
}


CDevNdpTable::CDevNdpTable (CNdpContainer &rContainer, WORD32 dwDeviceID)
    : CShareList<CNdpInst, MAX_NDP_NUM, FALSE>(rContainer),
      m_dwDeviceID(dwDeviceID)
{
}


CDevNdpTable::~CDevNdpTable()
{
    m_dwDeviceID = INVALID_DWORD;
}


CNdpInst * CDevNdpTable::FindNdpInst(const T_IPv6Addr &rtIP)
{
    CNdpInst *pCur = GetHead();

    while (pCur)
    {
        if ((*pCur) == rtIP)
        {
            return pCur;
        }

        pCur = Next(pCur);
    }

    return NULL;
}


CNdpInst * CDevNdpTable::CreateNdpInst(WORD32 dwDevID, T_IPv6Addr &rtIP, BYTE *pMacAddr)
{
    CNdpTable &rNdpTable = g_pNetIntfHandler->GetNdpTable();
    CNdpInst  *pNdpInst  = NULL;
    T_MacAddr  tMacAddr;

    /* 临界区, 需要对资源加锁 */
    rNdpTable.LockTable();
    pNdpInst = CreateTail();
    rNdpTable.UnLockTable();

    if (NULL == pNdpInst)
    {
        return NULL;
    }

    memcpy(tMacAddr.aucMacAddr, pMacAddr, ARP_MAC_ADDR_LEN);

    new (pNdpInst) CNdpInst(dwDevID, &rtIP, &tMacAddr);

    return pNdpInst;
}


CNdpTable::CNdpTable ()
{
    for (WORD32 dwIndex = 0; dwIndex < MAX_DEV_PORT_NUM; dwIndex++)
    {
        m_apDevTable[dwIndex] = NULL;
    }
}


CNdpTable::~CNdpTable()
{
    for (WORD32 dwIndex = 0; dwIndex < MAX_DEV_PORT_NUM; dwIndex++)
    {
        if (NULL != m_apDevTable[dwIndex])
        {
            delete m_apDevTable[dwIndex];
            this->Remove(m_apDevTable[dwIndex]);
        }

        m_apDevTable[dwIndex] = NULL;
    }
}


WORD32 CNdpTable::Initialize()
{
    CBaseList<CDevNdpTable, MAX_DEV_PORT_NUM, TRUE>::Initialize();

    m_cNdpContainer.Initialize();

    return SUCCESS;
}


/* 在创建每个EthDevice时, 为每个EthDevice创建1个CDevNdpTable实例 */
CDevNdpTable * CNdpTable::CreateDevTable(WORD32 dwDeviceID)
{
    if (dwDeviceID >= MAX_DEV_PORT_NUM)
    {
        return NULL;
    }

    if (NULL != m_apDevTable[dwDeviceID])
    {
        return m_apDevTable[dwDeviceID];
    }

    CDevNdpTable *pNdpTable = CreateTail();
    if (NULL == pNdpTable)
    {
        return NULL;
    }

    new (pNdpTable) CDevNdpTable(m_cNdpContainer, dwDeviceID);

    m_apDevTable[dwDeviceID] = pNdpTable;

    return m_apDevTable[dwDeviceID];
}


