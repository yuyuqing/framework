

#ifndef _DPDK_COMMON_H_
#define _DPDK_COMMON_H_


#include <rte_version.h>
#include <rte_ethdev.h>
#include <rte_eal.h>
#include <rte_byteorder.h>

#include "base_call_back.h"
#include "base_factory_tpl.h"
#include "base_config_file.h"
#include "base_mem_pool.h"


#if RTE_BYTE_ORDER == RTE_LITTLE_ENDIAN
#define HTONS(x)  ((WORD16)((((x) & 0x00ffU) << 8) | (((x) & 0xff00U) >> 8)))
#else
#define HTONS(x)  (x)
#endif


#define MAX_DEV_QUEUE_NUM        ((WORD32)(4))
#define MBUF_BURST_NUM           ((WORD32)(32))
#define RX_DESC_DEFAULT          ((WORD32)(1024))
#define TX_DESC_DEFAULT          ((WORD32)(1024))
#define MBUF_NAME_LEN            ((WORD32)(64))
#define MBUF_NUM_PER_PORT        ((WORD32)(16384))
#define MBUF_CACHE_SIZE          ((WORD32)(128))
#define MBUF_PRIV_SIZE           ((WORD32)(0))
#define MBUF_DATA_ROOM_SIZE      ((WORD32)(2048 + 128))


typedef struct rte_mbuf          T_MBuf;
typedef enum rte_eth_event_type  E_EthEventType;
typedef struct rte_ether_hdr     T_EthHead;
typedef struct rte_vlan_hdr      T_VlanHead;
typedef struct rte_arp_hdr       T_ArpHead;
typedef struct rte_ipv4_hdr      T_Ipv4Head;
typedef struct rte_ipv6_hdr      T_Ipv6Head;
typedef struct rte_icmp_hdr      T_IcmpHead;
typedef struct rte_udp_hdr       T_UdpHead;
typedef struct rte_tcp_hdr       T_TcpHead;


/* 报文回调处理函数 */
using PMBufCallBack = WORD32 (*)(VOID   *pArg,
                                 WORD32  dwDevID,
                                 WORD32  dwPortID,
                                 WORD32  dwQueueID,
                                 T_MBuf *pMBuf);


typedef enum tagE_DeviceType
{
    E_DEV_INVALID = 0,

    E_ETH_DEVICE,       /* 以太网设备 */
    E_BB_DEVICE,        /* 基带设备 */
}E_DeviceType;


typedef enum tagE_TrafficType
{
    E_TRAFFIC_INVALID = 0,

    E_ETH_TRAFFIC,          /* 基带设备(BKQ/AR)的以太网业务 */
    E_OAM_TRAFFIC,          /* 基带设备(BKQ/AR)的OAM业务 */
    E_FAPI_DATA_TRAFFIC,    /* 基带设备(BKQ/AR)的FAPI数据业务 */
    E_FAPI_CTRL_TRAFFIC,    /* 基带设备(BKQ/AR)的FAPI控制业务 */
}E_TrafficType;


typedef struct tagT_OffloadInfo
{
    WORD32    dwDeviceID;
    WORD32    dwPortID;
    WORD32    dwQueueID;

    WORD16    wEthType;
    WORD16    wL2Len;
    WORD32    dwVlanID;
    WORD16    wL3Len;
    WORD16    wL4Len;
    BYTE      ucL4Proto;
    BYTE      ucTunnelFlag;
    WORD16    wOuterEthType;
    WORD16    wOuterL2Len;
    WORD16    wOuterL3Len;
    BYTE      ucOuterL4Proto;
    WORD32    dwPktLen;
}T_OffloadInfo;


#endif


