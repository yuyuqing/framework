

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


class CArpInst
{
public :
    friend class CArpTable;

public :
    CArpInst (E_IPAddrType  eType,
              T_IPAddr     &rtIPAddr,
              BYTE         *pMacAddr);

    virtual ~CArpInst();

protected :
    E_IPAddrType  m_eAddrType;
    T_IPAddr      m_tIPAddr;
    T_MacAddr     m_tMacAddr;
};


class CArpTable : public CBaseList<CArpInst, MAX_ARP_NUM, FALSE>
{
public :
    CArpTable ();
    virtual ~CArpTable();

    WORD32 Initialize(CIPTable &rIPTable);

    WORD32 RegistArp(E_IPAddrType  eType,
                     T_IPAddr     &rtIPAddr,
                     T_MacAddr    &rtMacAddr);

    CArpInst * FindArp(WORD32 dwIPv4);

    CArpInst * FindArp(T_IPAddr &rtIPAddr);

protected :
    CSpinLock    m_cLock;
};


#endif


