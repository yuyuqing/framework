

#ifndef _DPDK_BB_TRAFFIC_H_
#define _DPDK_BB_TRAFFIC_H_


#include "dpdk_device.h"


typedef struct tagT_TrafficParam
{
    WORD32  dwDeviceID;
    WORD32  dwPortID;
    WORD32  dwQueueID;
    WORD32  dwTrafficID;
    WORD32  dwFAPICellID;
    WORD32  dwBindCellID;
}T_TrafficParam;


class CBaseTraffic : public CCBObject
{
public :
    CBaseTraffic (E_TrafficType eType, const T_TrafficParam &rtParam);
    virtual ~CBaseTraffic();

    virtual WORD32 Initialize();

protected :
    E_TrafficType    m_eType;

    WORD32           m_dwDeviceID;
    WORD32           m_dwPortID;
    WORD32           m_dwQueueID;
    WORD32           m_dwTrafficID;
    WORD32           m_dwFAPICellID;
    WORD32           m_dwBindCellID;
};


typedef struct tagT_TrafficInfo
{
    CHAR             aucName[BB_TRAFFIC_NAME_LEN];
    WORD32           dwDeviceID;
    WORD32           dwPortID;
    WORD32           dwQueueID;
    WORD32           dwTrafficID;
    WORD32           dwFAPICellID;
    WORD32           dwBindCellID;
    WORD32           dwMemSize;
    PCreateProduct   pCreateFunc;
    PDestroyProduct  pDestroyFunc;
    BYTE            *pMem;
    CBaseTraffic    *pTraffic;
}T_TrafficInfo;


class CFactoryTraffic : public CFactoryTpl<CFactoryTraffic>
{
public :
    CFactoryTraffic () {}
    virtual ~CFactoryTraffic() {}

    VOID Dump();
};


#define DEFINE_TRAFFIC(V)    \
    WORD32 __attribute__((used)) __dwTraffic_##V##_ = CFactoryTraffic::DefineProduct<V, T_TrafficParam>(#V)


#endif


