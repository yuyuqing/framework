

#ifndef _DPDK_MGR_H_
#define _DPDK_MGR_H_


#include "dpdk_device.h"


#define EAL_ARG_NUM             ((WORD32)(2))


class CDpdkMgr : public CSingleton<CDpdkMgr>, public CBaseData
{
public :
    CDpdkMgr();
    virtual ~CDpdkMgr();

    WORD32 Initialize(CCentralMemPool *pMemInterface,
                      const CHAR      *pArg0,
                      T_DpdkJsonCfg   &rtCfg);

    T_DeviceInfo * FindDevInfo(WORD32 dwDeviceID);
    CBaseDevice  * FindDevice(WORD32 dwDeviceID);
    CBaseDevice  * FindDevice(E_DeviceType eType, WORD16 wPortID);

    WORD32 GetDeviceNum();

    VOID Dump();

protected :
    WORD32 InitDpdkEal(const CHAR *pArg0, CHAR *pArgCore);
    WORD32 InitDevice(T_DpdkJsonCfg &rtCfg);

    T_DeviceInfo * CreateInfo(WORD32            dwDeviceID,
                              WORD32            dwPortID,
                              WORD32            dwQueueNum,
                              T_ProductDefInfo *ptDefInfo);

protected :
    CCentralMemPool    *m_pMemInterface;
    WORD32              m_dwDevNum;
    T_DeviceInfo        m_atDevInfo[MAX_DEV_PORT_NUM];
};


extern CDpdkMgr *g_pDpdkMgr;


#endif


