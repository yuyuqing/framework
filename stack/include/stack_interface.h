

#ifndef _STACK_INTERFACE_H_
#define _STACK_INTERFACE_H_


#include "base_app_interface.h"


/* 定义内部协议标识, 仅内部使用
 * 区别于标准协议类型 : 0x8100(VLAN)/0x0806(ARP)/0x0800(IPv4)/ 
 */
typedef enum tagE_ProtocolPrivType
{
    E_PRIV_ETH_ID = 0,
    E_PRIV_VLAN_ID,
    E_PRIV_ARP_ID,
    E_PRIV_IPV4_ID,
    E_PRIV_IPV6_ID,
    E_PRIV_ICMP_ID,
    E_PRIV_UDP_ID,
    E_PRIV_TCP_ID,
    E_PRIV_SCTP_ID,
    E_PRIV_VXLAN_ID,
    E_PRIV_GTPU_ID,
    E_PRIV_GTPC_ID,
}E_ProtocolPrivType;


typedef struct tagT_StackParam
{
    BYTE             ucPrivType;
    CCentralMemPool *pMemPool;
}T_StackParam;


class CStackInterface : public CBaseData, public CCBObject
{
public :
    CStackInterface (const T_StackParam &rtParam);
    virtual ~CStackInterface();

protected :
    CCentralMemPool  &m_rCentralMemPool;

    BYTE              m_ucPrivType;
};


class CAbstractVLan;
class CAbstractArp;
class CAbstractIPv4;
class CAbstractIcmp;
class CAbstractUdp;
class CAbstractSctp;
class CAbstractTcp;
class CAbstractGtpuU;
class CAbstractGtpuC;
class CAbstractVxLan;


class CAbstractEth : public CStackInterface
{
public :
    CAbstractEth (const T_StackParam &rtParam);
    virtual ~CAbstractEth();

protected :
    BYTE              m_ucLayer;  /* 0~2; 0:m_pVlan为NULL, 2:m_pLower为NULL */

    CAbstractVLan    *m_pVlan;
    CAbstractArp     *m_pArp;
    CAbstractIPv4    *m_pIPv4;

    CStackInterface  *m_pLower;   /* 低层协议栈(NULL or Eth or VXLAN/) */
};


class CAbstractVLan : public CAbstractEth
{
public :
    CAbstractVLan (const T_StackParam &rtParam);
    virtual ~CAbstractVLan();

protected :
    WORD16            m_wVlanID;
};


class CAbstractArp : public CStackInterface
{
public :
    CAbstractArp (const T_StackParam &rtParam);
    virtual ~CAbstractArp();

protected :
    CAbstractEth  *m_pLower;
};


class CAbstractIPv4 : public CStackInterface
{
public :
    CAbstractIcmp (const T_StackParam &rtParam);
    virtual ~CAbstractIcmp();

protected :
    BYTE           m_ucLayer;  /* 0:Underlay, 1:Overlay */

    CAbstractIcmp *m_pIcmp;
    CAbstractUdp  *m_pUdp;
    CAbstractSctp *m_pSctp;
    CAbstractTcp  *m_pTcp;

    CAbstractEth  *m_pLower;
};


class CAbstractIcmp : public CStackInterface
{
public :
    CAbstractIcmp (const T_StackParam &rtParam);
    virtual ~CAbstractIcmp();

protected :
    CAbstractIPv4  *m_pLower;
};


class CAbstractUdp : public CStackInterface
{
public :
    CAbstractUdp (const T_StackParam &rtParam);
    virtual ~CAbstractUdp();

protected :
    BOOL            m_bTunnel;  /* 当m_pLayer为Overlay时, 为隧道报文 */

    CAbstractGtpuU *m_pGTPU_U;
    CAbstractGtpuC *m_pGTPU_C;
    CAbstractVxLan *m_pVXLan;

    CAbstractIPv4  *m_pLower;
};


class CAbstractSctp : public CStackInterface
{
public :
    CAbstractSctp (const T_StackParam &rtParam);
    virtual ~CAbstractSctp();

protected :
    CAbstractIPv4  *m_pLower;
};


class CAbstractTcp : public CStackInterface
{
public :
    CAbstractTcp (const T_StackParam &rtParam);
    virtual ~CAbstractTcp();

protected :
    CAbstractIPv4  *m_pLower;
};


class CAbstractGtpuU : public CStackInterface
{
public :
    CAbstractGtpuU (const T_StackParam &rtParam);
    virtual ~CAbstractGtpuU();

protected :
    CAbstractUdp  *m_pLower;
};


class CAbstractGtpuC : public CStackInterface
{
public :
    CAbstractGtpuC (const T_StackParam &rtParam);
    virtual ~CAbstractGtpuC();

protected :
    CAbstractUdp  *m_pLower;
};


class CAbstractVxLan : public CStackInterface
{
public :
    CAbstractVxLan (const T_StackParam &rtParam);
    virtual ~CAbstractVxLan();

protected :
    CAbstractEth  *m_pEth;

    CAbstractUdp  *m_pLower;
};


class CFactoryStack : public CFactoryTpl<CFactoryStack>
{
public :
    CFactoryStack ();
    virtual ~CFactoryStack();

    VOID Dump();
};


#define DEFINE_STACK(V)    \
    WORD32 __attribute__((used)) __dwStack_##V##_ = CFactoryStack::DefineProduct<V, T_StackParam>(#V)


#endif


