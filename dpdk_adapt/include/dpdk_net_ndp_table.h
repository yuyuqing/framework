

#ifndef _DPDK_NDP_TABLE_H_
#define _DPDK_NDP_TABLE_H_


#include "dpdk_net_ip_table.h"


#define MAX_NDP_NUM         ((WORD32)(512))


class CNdpInst : public CBaseData, public CCBObject
{
public :
    CNdpInst (WORD32      dwDeviceID,
              T_IPv6Addr *ptIPAddr,
              T_MacAddr  *ptMacAddr);
    virtual ~CNdpInst();

    BOOL operator==(const T_IPv6Addr &rtIP);

    BOOL IsMatch(BYTE *pMacAddr);
    VOID UpdateMacAddr(BYTE *pMacAddr);

protected :
    WORD32        m_dwDeviceID;  /* 设备ID */
    T_IPv6Addr    m_tIPAddr;     /* 设备的邻居IP */
    T_MacAddr     m_tMacAddr;    /* 设备的邻区MAC */
};


typedef CBaseDataContainer<CNdpInst, MAX_NDP_NUM, FALSE>  CNdpContainer;


/* 每个设备接口上都持有1个NDP邻区表 */
class CDevNdpTable : public CShareList<CNdpInst, MAX_NDP_NUM, FALSE>, public CBaseData
{
public :
    CDevNdpTable (CNdpContainer &rContainer, WORD32 dwDeviceID);
    virtual ~CDevNdpTable();

    CNdpInst * FindNdpInst(const T_IPv6Addr &rtIP);

    CNdpInst * CreateNdpInst(WORD32 dwDevID, T_IPv6Addr &rtIP, BYTE *pMacAddr);

protected :
    WORD32    m_dwDeviceID;
};


class CNdpTable : public CBaseList<CDevNdpTable, MAX_DEV_PORT_NUM, TRUE>
{
public :
    CNdpTable ();
    virtual ~CNdpTable();

    virtual WORD32 Initialize();

    CDevNdpTable * CreateDevTable(WORD32 dwDeviceID);
    CDevNdpTable * GetDevTable(WORD32 dwDeviceID);

    VOID LockTable();
    VOID UnLockTable();

protected :
    CSpinLock           m_cLock;
    CNdpContainer       m_cNdpContainer;

    CDevNdpTable       *m_apDevTable[MAX_DEV_PORT_NUM];
};


inline CDevNdpTable * CNdpTable::GetDevTable(WORD32 dwDeviceID)
{
    if (dwDeviceID >= MAX_DEV_PORT_NUM)
    {
        return NULL;
    }

    return m_apDevTable[dwDeviceID];
}


inline VOID CNdpTable::LockTable()
{
    m_cLock.Lock();
}


inline VOID CNdpTable::UnLockTable()
{
    m_cLock.UnLock();
}


#endif


