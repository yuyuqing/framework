

#ifndef _DPDK_NET_INTERFACE_H_
#define _DPDK_NET_INTERFACE_H_


#include "dpdk_device.h"

#include "base_list.h"


class CLocalInterface : public CBaseData
{
public :
    CLocalInterface(WORD32 dwDeviceID, WORD32 dwPortID, CBaseDevice *pDevice);
    virtual ~CLocalInterface();

    operator CBaseDevice * ()
    {
        return m_pDevice;
    }

    BOOL operator== (WORD32 dwDeviceID);

    WORD32 GetDeviceID();
    WORD16 GetPortID();

protected :
    WORD32        m_dwDeviceID;
    WORD32        m_dwPortID;
    CBaseDevice  *m_pDevice;
};


inline BOOL CLocalInterface::operator== (WORD32 dwDeviceID)
{
    return (dwDeviceID == m_dwDeviceID);
}


inline WORD32 CLocalInterface::GetDeviceID()
{
    return m_dwDeviceID;
}


inline WORD16 CLocalInterface::GetPortID()
{
    return m_dwPortID;
}


/* 本地网络接口 */
class CNetIntfHandler : public CBaseList<CLocalInterface, MAX_DEV_PORT_NUM, FALSE>
{
public :
    CNetIntfHandler ();
    virtual ~CNetIntfHandler();

    virtual WORD32 Initialize();

    CLocalInterface * Find(WORD32 dwDeviceID);

    WORD32 RegistDevice(WORD32 dwDeviceID, WORD32 dwPortID, CBaseDevice *pDevice);
    WORD32 RemoveDevice(WORD32 dwDeviceID);
};


inline CLocalInterface * CNetIntfHandler::Find(WORD32 dwDeviceID)
{
    CLocalInterface *pData    = NULL;
    T_LinkHeader    *pCurHead = m_ptHeader;

    while (pCurHead)
    {
        pData = (*pCurHead);
        if ((*pData) == dwDeviceID)
        {
            return pData;
        }

        pCurHead = pCurHead->m_pNext;
    }

    return NULL;
}


#endif


