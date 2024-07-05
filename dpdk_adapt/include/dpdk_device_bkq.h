

#ifndef _DPDK_DEVICE_BKQ_H_
#define _DPDK_DEVICE_BKQ_H_


#include "dpdk_device.h"


class CBKQDevice : public CBaseDevice
{
public :
    static SWORD32 LsiEventCallBack(WORD16          wPortID,
                                    E_EthEventType  eType,
                                    VOID           *pParam,
                                    VOID           *pRetParam);

public :
    CBKQDevice (const T_DeviceParam &rtParam);
    virtual ~CBKQDevice();

    /* ��������������RxConf���� */
    virtual WORD32 SetRxConf();

    /* ��������������TxConf���� */
    virtual WORD32 SetTxConf();

    virtual WORD32 Initialize();

protected :
};


#endif


