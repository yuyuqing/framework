

#ifndef _DPDK_ARP_TABLE_H_
#define _DPDK_ARP_TABLE_H_


#include "dpdk_ip_table.h"


#define ARP_MAC_ADDR_LEN                ((WORD32)(6))
#define MAX_ARP_NUM                     ((WORD32)(256))


typedef struct tagT_MacAddr
{
    BYTE  aucMacAddr[ARP_MAC_ADDR_LEN];
}T_MacAddr;


class CArpTable;


class CArpInst : public CBaseData
{
public :
    friend class CArpTable;

public :
    CArpInst (WORD32        dwDevID,
              E_IPAddrType  eType,
              T_IPAddr     &rtIPAddr,
              BYTE         *pMacAddr);

    virtual ~CArpInst();

    BOOL operator== (BYTE *pMacAddr);

    VOID Update(WORD32 dwDevID);
    VOID Update(BYTE *pMacAddr);

    VOID Dump();

protected :
    WORD32        m_dwDeviceID;  /* 记录设备ID(本地MAC地址所属设备 or 对端MAC地址所连接设备) */
    E_IPAddrType  m_eAddrType;
    T_IPAddr      m_tIPAddr;
    T_MacAddr     m_tMacAddr;
};


inline BOOL CArpInst::operator== (BYTE *pMacAddr)
{
    return ((m_tMacAddr.aucMacAddr[0] == pMacAddr[0])
         && (m_tMacAddr.aucMacAddr[1] == pMacAddr[1])
         && (m_tMacAddr.aucMacAddr[2] == pMacAddr[2])
         && (m_tMacAddr.aucMacAddr[3] == pMacAddr[3])
         && (m_tMacAddr.aucMacAddr[4] == pMacAddr[4])
         && (m_tMacAddr.aucMacAddr[5] == pMacAddr[5]));
}


inline VOID CArpInst::Update(WORD32 dwDevID)
{
    m_dwDeviceID = dwDevID;
}


inline VOID CArpInst::Update(BYTE *pMacAddr)
{
    memcpy(m_tMacAddr.aucMacAddr, pMacAddr, ARP_MAC_ADDR_LEN);
}


class CArpTable : public CBaseList<CArpInst, MAX_ARP_NUM, FALSE>
{
protected :
    using CBaseList<CArpInst, MAX_ARP_NUM, FALSE>::Initialize;

public :
    CArpTable ();
    virtual ~CArpTable();

    WORD32 Initialize(CIPTable &rIPTable);

    CArpInst * RegistArp(WORD32        dwDeviceID,
                         E_IPAddrType  eType,
                         T_IPAddr     &rtIPAddr,
                         T_MacAddr    &rtMacAddr);

    CArpInst * FindArp(WORD32 dwIPv4);

    CArpInst * FindArp(T_IPAddr &rtIPAddr);

protected :
    CSpinLock    m_cLock;
};


#endif


