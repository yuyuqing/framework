

#ifndef _DPDK_DEVICE_BB_H_
#define _DPDK_DEVICE_BB_H_


#include "dpdk_device.h"


class CBBDevice : public CBaseDevice
{
public :
    CBBDevice (const T_DeviceParam &rtParam);
    virtual ~CBBDevice();

    WORD32 Initialize();
};


#endif


