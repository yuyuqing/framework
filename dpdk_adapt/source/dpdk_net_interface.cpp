

#include "dpdk_net_interface.h"


CLocalInterface::CLocalInterface(WORD32       dwDeviceID,
                                 WORD32       dwPortID,
                                 CBaseDevice *pDevice)
{
    m_dwDeviceID = dwDeviceID;
    m_dwPortID   = dwPortID;
    m_pDevice    = pDevice;
}


CLocalInterface::~CLocalInterface()
{
    m_dwDeviceID = INVALID_DWORD;
    m_dwPortID   = INVALID_DWORD;
    m_pDevice    = NULL;
}


CNetIntfHandler::CNetIntfHandler ()
{
}


CNetIntfHandler::~CNetIntfHandler()
{
}


WORD32 CNetIntfHandler::Initialize()
{
    CBaseList<CLocalInterface, MAX_DEV_PORT_NUM, FALSE>::Initialize();

    return SUCCESS;
}


WORD32 CNetIntfHandler::RegistDevice(WORD32       dwDeviceID,
                                     WORD32       dwPortID,
                                     CBaseDevice *pDevice)
{
    CLocalInterface *pIntf = CreateTail();
    if (NULL == pIntf)
    {
        return FAIL;
    }

    new (pIntf) CLocalInterface(dwDeviceID, dwPortID, pDevice);

    return SUCCESS;
}


WORD32 CNetIntfHandler::RemoveDevice(WORD32 dwDeviceID)
{
    return Remove<WORD32>(dwDeviceID);
}


