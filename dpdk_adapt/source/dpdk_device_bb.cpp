

#include "dpdk_device_bb.h"
#include "base_log.h"


DEFINE_DEVICE(CBBDevice);


CBBDevice::CBBDevice (const T_DeviceParam &rtParam)
    : CBaseDevice(E_BB_DEVICE, rtParam)
{
}


CBBDevice::~CBBDevice()
{
}


WORD32 CBBDevice::Initialize()
{
    return SUCCESS;
}


