

#ifndef _DPDK_DEVICE_BB_H_
#define _DPDK_DEVICE_BB_H_


#include "dpdk_device.h"


class CBBDevice : public CBaseDevice
{
public :
    static SWORD32 LsiEventCallBack(WORD16          wPortID,
                                    E_EthEventType  eType,
                                    VOID           *pParam,
                                    VOID           *pRetParam);

public :
    CBBDevice (const T_DeviceParam &rtParam);
    virtual ~CBBDevice();

    /* ��������������RxConf���� */
    virtual WORD32 SetRxConf();

    /* ��������������TxConf���� */
    virtual WORD32 SetTxConf();

    virtual WORD32 Initialize();

protected :
};


#endif


