

#ifndef _DPDK_VLAN_TABLE_H_
#define _DPDK_VLAN_TABLE_H_


#include "dpdk_net_ip_table.h"


#define MAX_VLAN_NUM        ((WORD32)(64))


typedef struct tagT_VlanKey
{
    WORD32  dwDeviceID;
    WORD32  dwVlanID;

    BOOL operator== (const tagT_VlanKey &rKey)
    {
        return ((this->dwDeviceID == rKey.dwDeviceID) && (this->dwVlanID == rKey.dwVlanID));
    }
}T_VlanKey;


class CVlanInst
{
public :
    CVlanInst ();
    virtual ~CVlanInst();

    WORD32 Initialize(WORD32                     dwDeviceID,
                      WORD32                     dwVlanID,
                      WORD32                     dwPriority,
                      E_IPAddrType               eType,
                      CString<IPV6_STRING_LEN>  &rIPAddr);

    WORD32 GetDeviceID();
    WORD32 GetVlanID();
    WORD32 GetPriority();

    T_IPAddr & GetIPAddr();

protected :
    WORD32        m_dwDeviceID;       /* 设备ID */
    WORD32        m_dwVlanID;         /* (VLANID1 << 12) | (VLANID2) */
    WORD32        m_dwPriority;       /* 优先级 */
    T_IPAddr      m_tIPAddr;
};


inline WORD32 CVlanInst::GetDeviceID()
{
    return m_dwDeviceID;
}


inline WORD32 CVlanInst::GetVlanID()
{
    return m_dwVlanID;
}


inline WORD32 CVlanInst::GetPriority()
{
    return m_dwPriority;
}


inline T_IPAddr & CVlanInst::GetIPAddr()
{
    return m_tIPAddr;
}


class CVlanTable : public CBaseSequence<T_VlanKey, CVlanInst, MAX_VLAN_NUM>
{
public :
    CVlanTable ();
    virtual ~CVlanTable();

    WORD32 Initialize();

    WORD32 RegistVlan(T_DpdkEthDevJsonCfg &rtCfg);

    CVlanInst * FindVlan(WORD32 dwDeviceID, WORD32 dwVlanID);
};


#endif


