

#ifndef _PUB_GLOBAL_DEF_H_
#define _PUB_GLOBAL_DEF_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "pub_typedef.h"


#define MAX_LOG_LEVEL_NUM                           ((BYTE)(8))
#define MAX_CELL_PER_GNB                            ((WORD32)(4))

#define MAX_UE_NUM_PER_GNB                          ((WORD32)(1600))
#define MAX_UE_NUM_PER_APP                          ((WORD32)(1600))

#define LOG_MODULE_LEN                              ((BYTE)(32))
#define LOG_FILE_NAME_LEN                           ((WORD32)(80))

#define BIT_NUM_PER_BYTE                            ((WORD32)(8))
#define BYTE_NUM_PER_MB                             ((WORD64)(1024 * 1024))

#define BITMAP_BYTE_NUM_02                          ((BYTE)(2))
#define BITMAP_BYTE_NUM_32                          ((BYTE)(32))

#define MAX_DEV_NUM                                 ((BYTE)(2))
#define MAX_FAPI_CELL_NUM                           ((BYTE)(2))


typedef enum tagE_PROC
{
    E_PROC_SCS = 0,
    E_PROC_OAM,
    E_PROC_CU,
    E_PROC_DU,
    E_PROC_L1,
    E_PROC_DHCP,
    E_PROC_TR069,
    E_PROC_MONITOR,
    E_PROC_LTE,
    E_PROC_FT,
    E_PROC_NUM,
}E_PROC;


typedef enum tagE_LOG_MODULE
{
    E_BASE_FRAMEWORK = 0,
        
    /* OSS模块 */
    E_OSS,

    /* SCS模块 */
    E_SCS,
    E_SCS_NETCONF_CLI,
    E_SCS_NETCONF_SERVER,

    /* OAM模块 */
    E_OAM = 5,
    E_OAM_DELAY,
    E_OAM_ES,
    E_OAM_KPI,
    E_OAM_UPGRADE,
    E_OAM_ALARM,
    E_OAM_AUTOSTART = 11,

    /* DHCP模块 */
    E_DHCP,

    /* TR069模块 */
    E_TR069,

    /* MONITOR模块 */
    E_MONITOR,

    /* CU模块 */
    E_CU_APP = 15,
    E_CU_COMMON,
    E_CU_OAM_AGENT,
    E_CU_GNB_MGR,
    E_CU_UP_MGR,
    E_CU_RM = 20,
    E_CU_UE_CONN_MGR,
    E_CU_BEARER_MGR,
    E_CU_CODEC_COMMON,
    E_CU_X2AP_CODEC,
    E_CU_F1AP_CODEC = 25,
    E_CU_RRC_CODEC,
    E_CU_NGAP_CODEC,
    E_CU_XNAP_CODEC,
    E_CU_E1AP_CODEC,
    E_CU_SCTP_COMMON = 30,
    E_CU_SCTP_CNTRL,
    E_CU_SCTP_TX,
    E_CU_EGTPU_COMMON,
    E_CU_EGTPU_UPPER_TX_CNTRL,
    E_CU_EGTPU_UPPER_RX_CNTRL = 35,
    E_CU_EGTPU_UPPER_TX,
    E_CU_EGTPU_UPPER_RX,
    E_CU_EGTPU_LOWER_TX_CNTRL,
    E_CU_EGTPU_LOWER_RX_CNTRL,
    E_CU_EGTPU_LOWER_TX = 40,
    E_CU_EGTPU_LOWER_RX,
    E_CU_PDCP_COMMON,
    E_CU_PDCP_TX_CNTRL,
    E_CU_PDCP_RX_CNTRL,
    E_CU_PDCP_TX = 45,
    E_CU_PDCP_RX,
    E_CU_UDP_CNTRL,
    E_CU_UDP_TX,
    E_CU_UDP_RX,
    E_CU_NRUP_CODEC = 50,
    E_CU_SDAP_COMMON,
    E_CU_SDAP_TX_CNTRL,
    E_CU_SDAP_RX_CNTRL,
    E_CU_SDAP_TX,
    E_CU_SDAP_RX = 55,
    E_CU_SDAP_CODEC,
    E_CU_TIMER,
    E_CU_EGTPU_TIMER,
    E_CU_CRYPTO_RX,
    E_CU_PM = 60,
    E_CU_NGP_MEM,
    E_CU_NGP_BUF,
    E_CU_NGP_STATS,
    E_CU_NGP_TIMER,
    E_CU_NGP_STHREAD = 65,
    E_CU_NGP_CTHREAD,
    E_CU_NGP_SYS,
    E_CU_NGP_EXCP,
    E_CU_NGP_COMM,
    E_CU_NGP_SCTP = 70,
    E_CU_NGP_UDP,
    E_CU_NGP_TCP,
    E_CU_NGP_MSGQ,
    E_CU_NGP_PRIOQ,
    E_CU_NGP_WORKQ = 75,
    E_CU_NGP_PERF,
    E_CU_USERP_EXTRA,

    /* DU模块 */
    E_DU_MGR2 = 88,
    E_DU_APP_UE2 = 89,
    E_DU_OAM_AGENT = 90,
    E_DU_MGR,
    E_DU_RRM,
    E_DU_APP_UE,
    E_DU_CODEC,
    E_DU_EVENT = 95,
    E_DU_EGTPU,
    E_DU_UDP,
    E_DU_CMN,
    E_DU_MAC,
    E_DU_SCHL1 = 100,
    E_DU_SCHL2,
    E_DU_DUCL,
    E_DU_NS,
    E_DU_FSPKT,
    E_DU_NRUP = 105,
    E_DU_F1AP,
    E_DU_SCTP,

    /* mac start */
    E_DU_CELL_CMN,
    E_DU_UE_CMN,
    E_DU_CFG_CMN = 110,    
    E_DU_CL_UL,
    E_DU_CL_DL,
    E_DU_CL_UCI,
    E_DU_CL_CMN,
    E_DU_CL_UL_TO_L1 = 115,
    E_DU_CL_DL_TO_L1,
    E_DU_LVL2_UL,
    E_DU_LVL2_DL,
    E_DU_CMN_SCH,
    E_DU_LVL1_UL = 120,
    E_DU_LVL1_DL,
    E_DU_LVL1_MEAS,
    E_DU_SCH,
    E_DU_RACH,
    E_DU_TA = 125,
    E_DU_LA,
    E_DU_DRX,
    E_DU_MAC_UL,
    E_DU_MAC_DL,
    E_DU_MAC_COMMON = 130,
    /* mac end */

    E_DU_COMMON_UMN,
    E_DU_RLC_UL,
    E_DU_RLC_DL,
    E_DU_RLC_COMMON,
    E_DU_NGP_MEM = 135,
    E_DU_NGP_BUF,
    E_DU_NGP_STATS,
    E_DU_NGP_TIMER,
    E_DU_NGP_STHREAD,
    E_DU_NGP_CTHREAD = 140,
    E_DU_NGP_SYS,
    E_DU_NGP_EXCP,
    E_DU_NGP_COMM,
    E_DU_NGP_SCTP,
    E_DU_NGP_UDP = 145,
    E_DU_NGP_TCP,
    E_DU_NGP_MSGQ,
    E_DU_NGP_PRIOQ,
    E_DU_NGP_WORKQ,
    E_DU_NGP_PERF = 150,
    E_DU_QOS,
    E_DU_MAC1 = 152,

    /* L1模块 */
    E_L1 = 170,
    E_L1_CPA,
    E_L1_BBUPOOL,
    E_L1_API,
    E_L1_PDSCH,
    E_L1_PDCCH = 175,
    E_L1_PUSCH,
    E_L1_PUCCH,
    E_L1_PRACH,
    E_L1_SRS,

    E_LOG_MODULE_NUM,
}E_LOG_MODULE;


typedef enum tagE_MemPoolMallocPoint
{
    E_MP_MALLOC_POINT_00 = 0,
    E_MP_MALLOC_POINT_01,
    E_MP_MALLOC_POINT_02,
    E_MP_MALLOC_POINT_03,
    E_MP_MALLOC_POINT_04,
    E_MP_MALLOC_POINT_05,
    E_MP_MALLOC_POINT_06,
    E_MP_MALLOC_POINT_07,
    E_MP_MALLOC_POINT_08,
    E_MP_MALLOC_POINT_09,
    E_MP_MALLOC_POINT_10,
    E_MP_MALLOC_POINT_11,
    E_MP_MALLOC_POINT_12,
    E_MP_MALLOC_POINT_13,
    E_MP_MALLOC_POINT_14,
    E_MP_MALLOC_POINT_15,
    E_MP_MALLOC_POINT_16,
    E_MP_MALLOC_POINT_17,
    E_MP_MALLOC_POINT_18,
    E_MP_MALLOC_POINT_19,
    E_MP_MALLOC_POINT_20,
    E_MP_MALLOC_POINT_21,
    E_MP_MALLOC_POINT_22,
    E_MP_MALLOC_POINT_23,
    E_MP_MALLOC_POINT_24,
    E_MP_MALLOC_POINT_25,
    E_MP_MALLOC_POINT_26,
    E_MP_MALLOC_POINT_27,
    E_MP_MALLOC_POINT_28,
    E_MP_MALLOC_POINT_29,
    E_MP_MALLOC_POINT_30,
    E_MP_MALLOC_POINT_31,
    E_MP_MALLOC_POINT_32,
    E_MP_MALLOC_POINT_33,
    E_MP_MALLOC_POINT_34,
    E_MP_MALLOC_POINT_35,
    E_MP_MALLOC_POINT_36,
    E_MP_MALLOC_POINT_37,
    E_MP_MALLOC_POINT_38,
    E_MP_MALLOC_POINT_39,
    E_MP_MALLOC_POINT_40,
    E_MP_MALLOC_POINT_41,
    E_MP_MALLOC_POINT_42,
    E_MP_MALLOC_POINT_43,
    E_MP_MALLOC_POINT_44,
    E_MP_MALLOC_POINT_45,
    E_MP_MALLOC_POINT_46,
    E_MP_MALLOC_POINT_47,
    E_MP_MALLOC_POINT_48,
    E_MP_MALLOC_POINT_49,
    E_MP_MALLOC_POINT_50,

    E_MP_MALLOC_POINT_NUM,
}E_MemPoolMallocPoint;


#ifdef __cplusplus
}
#endif


#endif


