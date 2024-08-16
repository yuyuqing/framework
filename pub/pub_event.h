

#ifndef _PUB_EVENT_H_
#define _PUB_EVENT_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "pub_typedef.h"

/******************************************************************************
 * 定义各个子系统消息号段 
 *****************************************************************************/
/* 每个子系统消息号段的长度 */
#define EV_MSG_SEGMENT_LEN                         ((WORD32)(512))

#define EV_BASE_BEGIN                              ((WORD32)(0x0000))

/* FrameWork号段 */
#define EV_FRAMEWORK_START                         ((WORD32)(EV_BASE_BEGIN + 0x0000))
#define EV_FRAMEWORK_END                           ((WORD32)(EV_BASE_BEGIN + 0x01FF))

/* 平台消息号段(OAM/...) */
#define EV_OAM_START                               ((WORD32)(EV_BASE_BEGIN + 0x0200))
#define EV_OAM_END                                 ((WORD32)(EV_BASE_BEGIN + 0x03FF))

/* DBS消息号段(DBS/...) */
#define EV_DBS_START                               ((WORD32)(EV_BASE_BEGIN + 0x0400))
#define EV_DBS_END                                 ((WORD32)(EV_BASE_BEGIN + 0x05FF))

/* 控制面Common消息号段(Cell/SCTP/...) */
#define EV_RRC_COMMON_START                        ((WORD32)(EV_BASE_BEGIN + 0x0600))
#define EV_RRC_COMMON_END                          ((WORD32)(EV_BASE_BEGIN + 0x07FF))

/* 控制面Dedicate消息号段(UE/...) */
#define EV_RRC_DEDICATE_START                      ((WORD32)(EV_BASE_BEGIN + 0x0800))
#define EV_RRC_DEDICATE_END                        ((WORD32)(EV_BASE_BEGIN + 0x09FF))

/* 用户面消息号段(公共/专用/UE/...) */
#define EV_DATA_PLANE_START                        ((WORD32)(EV_BASE_BEGIN + 0x0A00))
#define EV_DATA_PLANE_END                          ((WORD32)(EV_BASE_BEGIN + 0x0BFF))

/* CMAC消息号段(公共/专用/UE/...) */
#define EV_CMAC_START                              ((WORD32)(EV_BASE_BEGIN + 0x0C00))
#define EV_CMAC_END                                ((WORD32)(EV_BASE_BEGIN + 0x0DFF))

/* PHY消息号段(公共/专用/UE/...) */
#define EV_PHY_START                               ((WORD32)(EV_BASE_BEGIN + 0x0E00))
#define EV_PHY_END                                 ((WORD32)(EV_BASE_BEGIN + 0x0FFF))
/******************************************************************************
 * 定义各个子系统消息号段 
 *****************************************************************************/


/******************************************************************************
 * 定义FrameWork内部各个子模块消息号段 (0 ~ 511)
 * EV_FRAMEWORK_START ~ EV_FRAMEWORK_END
 *****************************************************************************/
/* 线程池框架消息号段 ( 0 ~ 63) */
#define EV_BASE_THREAD_START                       ((WORD32)(EV_FRAMEWORK_START + 0x000))
#define EV_BASE_THREAD_END                         ((WORD32)(EV_FRAMEWORK_START + 0x00F))

/* App池框架消息号段  (64 ~ 95) */
#define EV_BASE_APP_START                          ((WORD32)(EV_FRAMEWORK_START + 0x040))
#define EV_BASE_APP_END                            ((WORD32)(EV_FRAMEWORK_START + 0x05F))

/* FT框架消息号段     (96 ~ 127) */
#define EV_BASE_FT_START                           ((WORD32)(EV_FRAMEWORK_START + 0x060))
#define EV_BASE_FT_END                             ((WORD32)(EV_FRAMEWORK_START + 0x07F))

/******************************************************************************/
/******************************************************************************/
/* 线程级处理消息 */
#define EV_BASE_THREAD_CANCEL_ID                   ((WORD32)(EV_BASE_THREAD_START + 0))
#define EV_BASE_THREAD_RESET_APP_ID                ((WORD32)(EV_BASE_THREAD_START + 1))

/* APP Base级处理消息 */
#define EV_BASE_APP_STARTUP_ID                     ((WORD32)(EV_BASE_APP_START + 0))
#define EV_BASE_APP_SHUTDOWN_ID                    ((WORD32)(EV_BASE_APP_START + 1))
#define EV_BASE_APP_SHUTDOWN_COMPLETE              ((WORD32)(EV_BASE_APP_START + 2))
#define EV_BASE_APP_TIMEOUT_ID                     ((WORD32)(EV_BASE_APP_START + 3))

/* APP FT级处理消息 */
#define EV_BASE_FT_START_ID                        ((WORD32)(EV_BASE_FT_START + 0))
#define EV_BASE_FT_CONTINUE_ID                     ((WORD32)(EV_BASE_FT_START + 1))
#define EV_BASE_FT_FINISH_ID                       ((WORD32)(EV_BASE_FT_START + 2))
#define EV_BASE_FT_STEP_ID                         ((WORD32)(EV_BASE_FT_START + 3))
/******************************************************************************
 * 定义FrameWork内部各个子模块消息号段 (0 ~ 511)
 * EV_FRAMEWORK_START ~ EV_FRAMEWORK_END
 *****************************************************************************/


/******************************************************************************
 * 定义平台消息号段内部各个子模块消息号段 (512 ~ 1023)
 * EV_OAM_START ~ EV_OAM_END
 *****************************************************************************/
/*****************************************************************************/
/* 线程池框架OAM消息号段 (512 ~ 543) */
#define EV_FRAMEWORK_OAM_START                     ((WORD32)(EV_OAM_START + 0x000))
#define EV_FRAMEWORK_OAM_END                       ((WORD32)(EV_OAM_START + 0x01F))

/* DuOamAgent消息号段    (544 ~ 575) */
#define EV_DU_OAM_AGENT_START                      ((WORD32)(EV_OAM_START + 0x020))
#define EV_DU_OAM_AGENT_END                        ((WORD32)(EV_OAM_START + 0x03F))

/* DUMGR->RRU消息号段    (640 ~ 655)   起始消息ID : 640 */
#define EV_DUMGR_RRU_START                         ((WORD32)(EV_OAM_START + 0x080))
#define EV_DUMGR_RRU_END                           ((WORD32)(EV_OAM_START + 0x08F))

/* 日志线程处理消息(COamApp处理) */
#define EV_BASE_LOG_SET_GLOBAL_SWITCH_ID           ((WORD32)(EV_FRAMEWORK_OAM_START + 0))
#define EV_BASE_LOG_SET_WRITE_PERIOD_ID            ((WORD32)(EV_FRAMEWORK_OAM_START + 1))
#define EV_BASE_LOG_SET_MODULE_SWITCH_ID           ((WORD32)(EV_FRAMEWORK_OAM_START + 2))
#define EV_OAM_REGIST_CALLBACK_ID                  ((WORD32)(EV_FRAMEWORK_OAM_START + 3))
#define EV_OAM_REMOVE_CALLBACK_ID                  ((WORD32)(EV_FRAMEWORK_OAM_START + 4))

/* 定时器线程处理消息 */
#define EV_TIMER_START_TIMER_ID                    ((WORD32)(EV_FRAMEWORK_OAM_START + 5))
#define EV_TIMER_STOP_TIMER_ID                     ((WORD32)(EV_FRAMEWORK_OAM_START + 6))
#define EV_TIMER_RESET_TIMER_ID                    ((WORD32)(EV_FRAMEWORK_OAM_START + 7))
#define EV_TIMER_SLOT_TTI_ID                       ((WORD32)(EV_FRAMEWORK_OAM_START + 8))

/* DuOamAgent消息号段    (544 ~ 575) */
#define EV_OAM_START_REQUEST                       ((WORD32)(EV_DU_OAM_AGENT_START + 0))
#define EV_OAM_CONFIG_REQUEST                      ((WORD32)(EV_DU_OAM_AGENT_START + 1))
#define EV_OAM_DU_CONFIG_REQUEST                   ((WORD32)(EV_DU_OAM_AGENT_START + 2))
#define EV_OAM_DU_CONFIG_RESPONSE                  ((WORD32)(EV_DU_OAM_AGENT_START + 3))
#define EV_OAM_CELL_CONFIG_REQUEST                 ((WORD32)(EV_DU_OAM_AGENT_START + 4))
#define EV_OAM_CELL_CONFIG_RESPONSE                ((WORD32)(EV_DU_OAM_AGENT_START + 5))


/* DUMGR->RRU消息号段    (640 ~ 655)   起始消息ID : 640 */
#define EV_DUMGR_RRU_CELL_CFG_REQ                  ((WORD32)(EV_DUMGR_RRU_START + 0))
#define EV_DUMGR_RRU_CELL_RECFG_REQ                ((WORD32)(EV_DUMGR_RRU_START + 1))
#define EV_DUMGR_RRU_CELL_DELETE_REQ               ((WORD32)(EV_DUMGR_RRU_START + 2))

/*****************************************************************************/
/******************************************************************************
 * 定义平台消息号段内部各个子模块消息号段 (512 ~ 1023)
 * EV_OAM_START ~ EV_OAM_END
 *****************************************************************************/


/******************************************************************************
 * 定义DBS消息号段内部各个子模块消息号段 (1024 ~ 1535)
 * EV_DBS_START ~ EV_DBS_END
 *****************************************************************************/ 
#define EV_DBS_GetGnbID_REQ                        ((WORD32)(EV_DBS_START + 0))
/*****************************************************************************/
/*****************************************************************************/
/******************************************************************************
 * 定义DBS消息号段内部各个子模块消息号段 (1024 ~ 1535)
 * EV_DBS_START ~ EV_DBS_END
 *****************************************************************************/


/******************************************************************************
 * 定义控制面Common消息号段内部各个子模块消息号段 (1536 ~ 2047)
 * EV_RRC_COMMON_START ~ EV_RRC_COMMON_END
 *****************************************************************************/
/* DBS->DUMGR消息号段  (  0 ~ 127) 起始消息ID : 1536 */
#define EV_DBS_DUMGR_START                         ((WORD32)(EV_RRC_COMMON_START + 0x000))
#define EV_DBS_DUMGR_END                           ((WORD32)(EV_RRC_COMMON_START + 0x07F))

/* RRU->DUMGR消息号段  (128 ~ 143) 起始消息ID : 1664 */
#define EV_RRU_DUMGR_START                         ((WORD32)(EV_RRC_COMMON_START + 0x080))
#define EV_RRU_DUMGR_END                           ((WORD32)(EV_RRC_COMMON_START + 0x08F))

/* DUMGR->DUMGR消息号段(144 ~ 159) 起始消息ID : 1680 */
#define EV_DUMGR_DUMGR_START                       ((WORD32)(EV_RRC_COMMON_START + 0x090))
#define EV_DUMGR_DUMGR_END                         ((WORD32)(EV_RRC_COMMON_START + 0x09F))

/* F1AP->DUMGR消息号段 (160 ~ 223) 起始消息ID : 1696 */
#define EV_F1AP_DUMGR_START                        ((WORD32)(EV_RRC_COMMON_START + 0x0A0))
#define EV_F1AP_DUMGR_END                          ((WORD32)(EV_RRC_COMMON_START + 0x0DF))

/* RRM->DUMGR消息号段  (224 ~ 287) 起始消息ID : 1760 */
#define EV_RRM_DUMGR_START                         ((WORD32)(EV_RRC_COMMON_START + 0x0E0))
#define EV_RRM_DUMGR_END                           ((WORD32)(EV_RRC_COMMON_START + 0x11F))

/* SCH->DUMGR消息号段  (288 ~ 319) 起始消息ID : 1824 */
#define EV_SCH_DUMGR_START                         ((WORD32)(EV_RRC_COMMON_START + 0x120))
#define EV_SCH_DUMGR_END                           ((WORD32)(EV_RRC_COMMON_START + 0x13F))

/* CL->DUMGR消息号段   (320 ~ 335) 起始消息ID : 1856 */
#define EV_CL_DUMGR_START                          ((WORD32)(EV_RRC_COMMON_START + 0x140))
#define EV_CL_DUMGR_END                            ((WORD32)(EV_RRC_COMMON_START + 0x14F))

/* DUMGR->RRM消息号段  (336 ~ 399) 起始消息ID : 1872 */
#define EV_DUMGR_RRM_START                         ((WORD32)(EV_RRC_COMMON_START + 0x150))
#define EV_DUMGR_RRM_END                           ((WORD32)(EV_RRC_COMMON_START + 0x18F))

/* SCTP->F1AP消息号段  (400 ~ 415) 起始消息ID : 1888 */
#define EV_SCTP_F1AP_START                         ((WORD32)(EV_RRC_COMMON_START + 0x190))
#define EV_SCTP_F1AP_END                           ((WORD32)(EV_RRC_COMMON_START + 0x19F))

/* DUMGR->F1AP消息号段 (416 ~ 431) 起始消息ID : 1904 */
#define EV_DUMGR_F1AP_START                        ((WORD32)(EV_RRC_COMMON_START + 0x200))
#define EV_DUMGR_F1AP_END                          ((WORD32)(EV_RRC_COMMON_START + 0x20F))


/* DBS->DUMGR消息号段  (  0 ~ 127) 起始消息ID : 1536 */
#define EV_DBS_DUMGR_OVERALL_PARA_CFG_REQ          ((WORD32)(EV_DBS_DUMGR_START + 0))
#define EV_DBS_DUMGR_CELL_ADD_REQ                  ((WORD32)(EV_DBS_DUMGR_START + 1))
#define EV_DBS_DUMGR_ALL_CELL_RESET_REQ            ((WORD32)(EV_DBS_DUMGR_START + 2))
#define EV_DBS_DUMGR_CELL_RESET_REQ                ((WORD32)(EV_DBS_DUMGR_START + 3))
#define EV_DBS_DUMGR_CELL_DELETE_REQ               ((WORD32)(EV_DBS_DUMGR_START + 4))
#define EV_DBS_DUMGR_ALL_CELL_RECFG_REQ            ((WORD32)(EV_DBS_DUMGR_START + 5))
#define EV_DBS_DUMGR_CELL_RECFG_REQ                ((WORD32)(EV_DBS_DUMGR_START + 6))
#define EV_DBS_DUMGR_CELL_MIB_RECFG_REQ            ((WORD32)(EV_DBS_DUMGR_START + 7))

/* RRU->DUMGR消息号段  (128 ~ 143) 起始消息ID : 1664 */
#define EV_RRU_DUMGR_CELL_CFG_RSP                  ((WORD32)(EV_RRU_DUMGR_START + 0))
#define EV_RRU_DUMGR_CELL_RECFG_RSP                ((WORD32)(EV_RRU_DUMGR_START + 1))
#define EV_RRU_DUMGR_CELL_DELETE_RSP               ((WORD32)(EV_RRU_DUMGR_START + 2))

/* DUMGR->DUMGR消息号段(144 ~ 159) 起始消息ID : 1680 */
#define EV_PER_SECOND_TRIG_ACTION                  ((WORD32)(EV_DUMGR_DUMGR_START + 0))
#define EV_LCL_CELL_ADD_REQ                        ((WORD32)(EV_DUMGR_DUMGR_START + 1))
#define EV_ADD_CELL_TRANS_GUARD_TIMER              ((WORD32)(EV_DUMGR_DUMGR_START + 2))
#define EV_LCL_CELL_DELETE_REQ                     ((WORD32)(EV_DUMGR_DUMGR_START + 3))
#define EV_DELETE_CELL_TRANS_GUARD_TIMER           ((WORD32)(EV_DUMGR_DUMGR_START + 4))
#define EV_LCL_CELL_RECFG_REQ                      ((WORD32)(EV_DUMGR_DUMGR_START + 5))
#define EV_SCTP_ASSOC_UP_NOTIFY                    ((WORD32)(EV_DUMGR_DUMGR_START + 6))

/* F1AP->DUMGR消息号段 (160 ~ 223) 起始消息ID : 1696 */
#define EV_F1AP_DUMGR_MGMT_MSG_RX                  ((WORD32)(EV_F1AP_DUMGR_START + 0))
#define EV_F1AP_DUMGR_UE_MSG_RX                    ((WORD32)(EV_F1AP_DUMGR_START + 1))

/* RRM->DUMGR消息号段  (224 ~ 287) 起始消息ID : 1760 */
#define EV_RRM_DUMGR_CELL_CFG_RSP                  ((WORD32)(EV_RRM_DUMGR_START + 0))
#define EV_RRM_DUMGR_CELL_RECFG_RSP                ((WORD32)(EV_RRM_DUMGR_START + 1))
#define EV_RRM_DUMGR_CELL_DELETE_RSP               ((WORD32)(EV_RRM_DUMGR_START + 2))

/* SCH->DUMGR消息号段  (288 ~ 319) 起始消息ID : 1824 */
#define EV_SCH_DUMGR_CELL_CFG_RSP                  ((WORD32)(EV_SCH_DUMGR_START + 0))
#define EV_SCH_DUMGR_SYMBOL_OFF_CFG_RSP            ((WORD32)(EV_SCH_DUMGR_START + 1))
#define EV_SCH_DUMGR_SYMBOL_OFF_NOTIFY             ((WORD32)(EV_SCH_DUMGR_START + 2))
#define EV_SCH_DUMGR_SYMBOL_OFF_ENABLE_REQ         ((WORD32)(EV_SCH_DUMGR_START + 3))

/* CL->DUMGR消息号段   (320 ~ 335) 起始消息ID : 1856 */
#define EV_CL_DUMGR_CELL_CFG_RSP                   ((WORD32)(EV_CL_DUMGR_START + 0))

/* DUMGR->RRM消息号段  (336 ~ 399) 起始消息ID : 1872 */
#define EV_DUMGR_RRM_CELL_CFG_REQ                  ((WORD32)(EV_DUMGR_RRM_START + 0))
#define EV_DUMGR_RRM_CELL_RECFG_REQ                ((WORD32)(EV_DUMGR_RRM_START + 1))
#define EV_DUMGR_RRM_CELL_DELETE_REQ               ((WORD32)(EV_DUMGR_RRM_START + 2))

/* SCTP->F1AP消息号段  (400 ~ 415) 起始消息ID : 1888 */
#define EV_SCTP_F1AP_USER_PAYLOAD_INDICATION       ((WORD32)(EV_SCTP_F1AP_START + 0))


/******************************************************************************
 * 定义控制面Dedicate消息号段内部各个子模块消息号段 (2048 ~ 2559)
 * EV_RRC_DEDICATE_START ~ EV_RRC_DEDICATE_END
 *****************************************************************************/
/* SCH->APPUE消息号段  (  0 ~ 15 ) 起始消息ID : 2048 */
#define EV_SCH_APPUE_DEDICATE_START                ((WORD32)(EV_RRC_DEDICATE_START + 0x000))
#define EV_SCH_APPUE_DEDICATE_END                  ((WORD32)(EV_RRC_DEDICATE_START + 0x00F))

/* RLC->APPUE消息号段  ( 16 ~ 31 ) 起始消息ID : 2064 */
#define EV_RLC_APPUE_DEDICATE_START                ((WORD32)(EV_RRC_DEDICATE_START + 0x010))
#define EV_RLC_APPUE_DEDICATE_END                  ((WORD32)(EV_RRC_DEDICATE_START + 0x01F))

/* NRUP->APPUE消息号段 ( 32 ~ 47 ) 起始消息ID : 2080 */
#define EV_NRUP_APPUE_DEDICATE_START               ((WORD32)(EV_RRC_DEDICATE_START + 0x020))
#define EV_NRUP_APPUE_DEDICATE_END                 ((WORD32)(EV_RRC_DEDICATE_START + 0x02F))

/* RRM->APPUE消息号段  ( 48 ~ 63 ) 起始消息ID : 2096 */
#define EV_RRM_APPUE_DEDICATE_START                ((WORD32)(EV_RRC_DEDICATE_START + 0x030))
#define EV_RRM_APPUE_DEDICATE_END                  ((WORD32)(EV_RRC_DEDICATE_START + 0x03F))

/* F1AP->APPUE消息号段 ( 64 ~ 79 ) 起始消息ID : 2112 */
#define EV_F1AP_APPUE_DEDICATE_START               ((WORD32)(EV_RRC_DEDICATE_START + 0x040))
#define EV_F1AP_APPUE_DEDICATE_END                 ((WORD32)(EV_RRC_DEDICATE_START + 0x04F))

/* APPUE->APPUE消息号段( 80 ~ 95 ) 起始消息ID : 2128 */
#define EV_APPUE_APPUE_DEDICATE_START              ((WORD32)(EV_RRC_DEDICATE_START + 0x050))
#define EV_APPUE_APPUE_DEDICATE_END                ((WORD32)(EV_RRC_DEDICATE_START + 0x05F))

/* APPUE->RRM消息号段  ( 96 ~ 111) 起始消息ID : 2144 */
#define EV_APPUE_RRM_DEDICATE_START                ((WORD32)(EV_RRC_DEDICATE_START + 0x060))
#define EV_APPUE_RRM_DEDICATE_END                  ((WORD32)(EV_RRC_DEDICATE_START + 0x06F))

/* APPUE->F1AP消息号段 (112 ~ 127) 起始消息ID : 2160 */
#define EV_APPUE_F1AP_DEDICATE_START               ((WORD32)(EV_RRC_DEDICATE_START + 0x070))
#define EV_APPUE_F1AP_DEDICATE_END                 ((WORD32)(EV_RRC_DEDICATE_START + 0x07F))


/* SCH->APPUE消息号段  (  0 ~ 15 ) 起始消息ID : 2048 */
#define EV_SCH_APPUE_CONFIG_CONFIRM                ((WORD32)(EV_SCH_APPUE_DEDICATE_START + 0))
#define EV_SCH_APPUE_CRI_RSRP_RPT_INDICATION       ((WORD32)(EV_SCH_APPUE_DEDICATE_START + 1))
#define EV_SCH_APPUE_BEAM_INFO_INDICATION          ((WORD32)(EV_SCH_APPUE_DEDICATE_START + 2))
#define EV_SCH_APPUE_BWP_SWITCH_INDICATION         ((WORD32)(EV_SCH_APPUE_DEDICATE_START + 3))
#define EV_SCH_APPUE_STATUS_INDICATION             ((WORD32)(EV_SCH_APPUE_DEDICATE_START + 4))
#define EV_SCH_APPUE_CCCH_DATA_INDICATION          ((WORD32)(EV_SCH_APPUE_DEDICATE_START + 5))

/* RLC->APPUE消息号段  ( 16 ~ 31 ) 起始消息ID : 2064 */
#define EV_RLC_APPUE_CONFIG_CONFIRM                ((WORD32)(EV_RLC_APPUE_DEDICATE_START + 0))
#define EV_RLC_APPUE_UE_ID_CHANGE_CONFIRM          ((WORD32)(EV_RLC_APPUE_DEDICATE_START + 1))
#define EV_RLC_APPUE_DATA_INDICATION               ((WORD32)(EV_RLC_APPUE_DEDICATE_START + 2))
#define EV_RLC_APPUE_STATUS_INDICATION             ((WORD32)(EV_RLC_APPUE_DEDICATE_START + 3))

/* NRUP->APPUE消息号段 ( 32 ~ 47 ) 起始消息ID : 2080 */
#define EV_NRUP_APPUE_ADD_RAB_RESPONSE             ((WORD32)(EV_NRUP_APPUE_DEDICATE_START + 0))
#define EV_NRUP_APPUE_ADD_RAB_FAIL                 ((WORD32)(EV_NRUP_APPUE_DEDICATE_START + 1))
#define EV_NRUP_APPUE_DEL_RAB_RESPONSE             ((WORD32)(EV_NRUP_APPUE_DEDICATE_START + 2))
#define EV_NRUP_APPUE_UE_REL_RESPONSE              ((WORD32)(EV_NRUP_APPUE_DEDICATE_START + 3))
#define EV_NRUP_APPUE_INIT_DDDS_RESPONSE           ((WORD32)(EV_NRUP_APPUE_DEDICATE_START + 4))
#define EV_NRUP_APPUE_UE_RESET_RESPONSE            ((WORD32)(EV_NRUP_APPUE_DEDICATE_START + 5))
#define EV_NRUP_APPUE_UE_RESUME_RESPONSE           ((WORD32)(EV_NRUP_APPUE_DEDICATE_START + 6))

/* RRM->APPUE消息号段  ( 48 ~ 63 ) 起始消息ID : 2096 */
#define EV_RRM_APPUE_UE_ADMIT_CONFIRM              ((WORD32)(EV_RRM_APPUE_DEDICATE_START + 0))
#define EV_RRM_APPUE_UE_RECONFIG_RESPONSE          ((WORD32)(EV_RRM_APPUE_DEDICATE_START + 1))
#define EV_RRM_APPUE_UE_REL_RESPONSE               ((WORD32)(EV_RRM_APPUE_DEDICATE_START + 2))
#define EV_RRM_APPUE_DED_PREAMBLE_ALLOC_RESPONSE   ((WORD32)(EV_RRM_APPUE_DEDICATE_START + 3))
#define EV_RRM_APPUE_UE_ID_CHANGE_RESPONSE         ((WORD32)(EV_RRM_APPUE_DEDICATE_START + 4))

/* F1AP->APPUE消息号段 ( 64 ~ 79 ) 起始消息ID : 2112 */
#define EV_F1AP_APPUE_UEID_ALLOC_RESPONSE          ((WORD32)(EV_F1AP_APPUE_DEDICATE_START + 0))
#define EV_F1AP_APPUE_UE_REL_RESPONSE              ((WORD32)(EV_F1AP_APPUE_DEDICATE_START + 1))
#define EV_F1AP_APPUE_UE_MSG_RX                    ((WORD32)(EV_F1AP_APPUE_DEDICATE_START + 2))

/* APPUE->APPUE消息号段( 80 ~ 95 ) 起始消息ID : 2128 */
#define EV_APPUE_APPUE_UE_REL_RRC_CON_TRANS        ((WORD32)(EV_APPUE_APPUE_DEDICATE_START + 0))
#define EV_APPUE_APPUE_DU_INITIATED_UE_RELEASE     ((WORD32)(EV_APPUE_APPUE_DEDICATE_START + 1))

/* APPUE->RRM消息号段  ( 96 ~ 111) 起始消息ID : 2144 */
#define EV_APPUE_RRM_UE_ADMIT_REQUEST              ((WORD32)(EV_APPUE_RRM_DEDICATE_START + 0))
#define EV_APPUE_RRM_UE_RECONFIG_REQUEST           ((WORD32)(EV_APPUE_RRM_DEDICATE_START + 1))
#define EV_APPUE_RRM_UE_DEL_REQUEST                ((WORD32)(EV_APPUE_RRM_DEDICATE_START + 2))
#define EV_APPUE_RRM_DED_PREAMBLE_ALLOC_REQUEST    ((WORD32)(EV_APPUE_RRM_DEDICATE_START + 3))
#define EV_APPUE_RRM_DED_PREAMBLE_REL_IND          ((WORD32)(EV_APPUE_RRM_DEDICATE_START + 4))
#define EV_APPUE_RRM_UE_ID_CHANGE_REQUEST          ((WORD32)(EV_APPUE_RRM_DEDICATE_START + 5))

/* APPUE->F1AP消息号段 (112 ~ 127) 起始消息ID : 2160 */
#define EV_APPUE_F1AP_UEID_ALLOC_REQUEST           ((WORD32)(EV_APPUE_F1AP_DEDICATE_START + 0))
#define EV_APPUE_F1AP_UE_REL_REQUEST               ((WORD32)(EV_APPUE_F1AP_DEDICATE_START + 1))
#define EV_APPUE_F1AP_UE_MSG_TX                    ((WORD32)(EV_APPUE_F1AP_DEDICATE_START + 2))


/******************************************************************************
 * 定义用户面消息号段内部各个子模块消息号段 (2560 ~ 3071)
 * EV_DATA_PLANE_START ~ EV_DATA_PLANE_END
 *****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/******************************************************************************
 * 定义用户面消息号段内部各个子模块消息号段 (2560 ~ 3071)
 * EV_DATA_PLANE_START ~ EV_DATA_PLANE_END
 *****************************************************************************/
/* SCH发给DMAC消息号段 (  0 ~  31) 起始消息ID : 2560 */
#define EV_DATA_PLANE_DMAC_START                   ((WORD32)(EV_DATA_PLANE_START + 0x000))
#define EV_DATA_PLANE_DMAC_END                     ((WORD32)(EV_DATA_PLANE_START + 0x01F))

/* NRUP消息号段        ( 32 ~  63) 起始消息ID : 2592 */
#define EV_DATA_PLANE_NRUP_START                   ((WORD32)(EV_DATA_PLANE_START + 0x020))
#define EV_DATA_PLANE_NRUP_END                     ((WORD32)(EV_DATA_PLANE_START + 0x03F))

/* RLC消息号段         ( 64 ~  95) 起始消息ID : 2624 */
#define EV_DATA_PLANE_RLC_START                    ((WORD32)(EV_DATA_PLANE_START + 0x040))
#define EV_DATA_PLANE_RLC_END                      ((WORD32)(EV_DATA_PLANE_START + 0x05F))

/* SCH发给DMAC消息号段 ( 96 ~ 127) 起始消息ID : 2656 */
#define EV_SCH_TO_DMAC_START                       ((WORD32)(EV_DATA_PLANE_START + 0x060))
#define EV_SCH_TO_DMAC_END                         ((WORD32)(EV_DATA_PLANE_START + 0x07F))

/* CL发给DMAC消息号段  (128 ~ 159) 起始消息ID : 2688 */
#define EV_CL_TO_DMAC_START                        ((WORD32)(EV_DATA_PLANE_START + 0x080))
#define EV_CL_TO_DMAC_END                          ((WORD32)(EV_DATA_PLANE_START + 0x09F))

/* SCH发给DMAC消息号段 (  0 ~  31) 起始消息ID : 2560 */
#define EVT_SCH_DMAC_REL_TB_IND                    ((WORD32)(EV_DATA_PLANE_DMAC_START + 0))
#define EVT_PHY_DMAC_PUSCH_PDU_IND                 ((WORD32)(EV_DATA_PLANE_DMAC_START + 1))

/* NRUP消息号段        ( 32 ~  63) 起始消息ID : 2592 */
#define EVT_DUMGR_EGTP_INIT_REQ                    ((WORD32)(EV_DATA_PLANE_NRUP_START + 0))
#define EVT_APPUE_NRUP_EGTP_ADD_RAB_REQ            ((WORD32)(EV_DATA_PLANE_NRUP_START + 1))
#define EVT_APPUE_NRUP_EGTP_DEL_RAB_REQ            ((WORD32)(EV_DATA_PLANE_NRUP_START + 2))
#define EVT_APPUE_NRUP_EGTP_UE_REL_REQ             ((WORD32)(EV_DATA_PLANE_NRUP_START + 3))
#define EVT_APPUE_NRUP_EGTP_UE_RESET_REQ           ((WORD32)(EV_DATA_PLANE_NRUP_START + 4))
#define EVT_APPUE_NRUP_EGTP_UE_RESUME_REQ          ((WORD32)(EV_DATA_PLANE_NRUP_START + 5))
#define EVT_APPUE_NRUP_INIT_DDDS_REQ               ((WORD32)(EV_DATA_PLANE_NRUP_START + 6))
#define EVT_RLC_NRUP_FLOW_CTRL_INFO_RSP            ((WORD32)(EV_DATA_PLANE_NRUP_START + 7))
#define EVT_NRUP_DEBUG_CMD                         ((WORD32)(EV_DATA_PLANE_NRUP_START + 8))

/* RLC消息号段         ( 64 ~  95) 起始消息ID : 2624 */
#define EVT_APPUE_RLC_CFG_REQ                      ((WORD32)(EV_DATA_PLANE_RLC_START + 0))
#define EVT_APPUE_RLC_UEID_CHG_REQ                 ((WORD32)(EV_DATA_PLANE_RLC_START + 1))
#define EVT_NRUP_RLC_FLOW_CTRL_INFO_REQ            ((WORD32)(EV_DATA_PLANE_RLC_START + 2))
#define EVT_APPUE_RLC_DLDCCH_MSG_IND               ((WORD32)(EV_DATA_PLANE_RLC_START + 3))
#define EVT_CL_RLC_TTI_IND                         ((WORD32)(EV_DATA_PLANE_RLC_START + 4))
#define EVT_SCH_RLC_DL_SCHEDULE_IND                ((WORD32)(EV_DATA_PLANE_RLC_START + 5))

/* CL发给DMAC消息号段  (128 ~ 159) 起始消息ID : 2688 */
/* CL给DMAC的上行数据消息 */
#define EVT_TFU_UL_PDU_REQUEST                     ((WORD32)(EV_CL_TO_DMAC_START + 0))


/******************************************************************************
 * 定义CMAC消息号段内部各个子模块消息号段 (3072 ~ 3583)
 * EV_CMAC_START ~ EV_CMAC_END
 *****************************************************************************/
/* PHY发给CL消息号段  ( 0 ~  31)  起始消息ID : 3072 */
#define EV_PHY_TO_CL_START                         ((WORD32)(EV_CMAC_START + 0x000))
#define EV_PHY_TO_CL_END                           ((WORD32)(EV_CMAC_START + 0x01F))

/* PHY发给SCH消息号段  ( 32 ~ 63)  起始消息ID : 3104 */
#define EV_PHY_TO_SCH_START                        ((WORD32)(EV_CMAC_START + 0x020))
#define EV_PHY_TO_SCH_END                          ((WORD32)(EV_CMAC_START + 0x03F))

/* DMAC发给CL消息号段 ( 64 ~ 95)  起始消息ID : 3136 */
#define EV_DMAC_TO_CL_START                        ((WORD32)(EV_CMAC_START + 0x040))
#define EV_DMAC_TO_CL_END                          ((WORD32)(EV_CMAC_START + 0x05F))

/* RRC发给CL消息号段  ( 96 ~ 127) 起始消息ID : 3168 */
#define EV_RRC_TO_CL_START                         ((WORD32)(EV_CMAC_START + 0x060))
#define EV_RRC_TO_CL_END                           ((WORD32)(EV_CMAC_START + 0x07F))

/* CL发给SCH消息号段  (128 ~ 159) 起始消息ID : 3200 */
#define EV_CL_TO_SCH_START                         ((WORD32)(EV_CMAC_START + 0x080))
#define EV_CL_TO_SCH_END                           ((WORD32)(EV_CMAC_START + 0x09F))

/* RRC发给SCH消息号段 (160 ~ 191) 起始消息ID : 3232 */
#define EV_RRC_TO_SCH_START                        ((WORD32)(EV_CMAC_START + 0x0A0))
#define EV_RRC_TO_SCH_END                          ((WORD32)(EV_CMAC_START + 0x0BF))


/* PHY发往CL消息, 起始消息ID : 3072 */
#define EVT_TFU_SLOT_TTI_IND                       ((WORD32)(EV_PHY_TO_CL_START + 0))
#define EVT_PHY_CELL_MGR_IND                       ((WORD32)(EV_PHY_TO_CL_START + 1))


/* PHY发往SCH消息, 起始消息ID : 3104 */
#define EVT_PHY_SCH_SLOT_TTI_IND                   ((WORD32)(EV_PHY_TO_SCH_START + 0))
#define EVT_PHY_SCH_RACH_IND                       ((WORD32)(EV_PHY_TO_SCH_START + 1))
#define EVT_PHY_SCH_RX_PUSCH_IND                   ((WORD32)(EV_PHY_TO_SCH_START + 2))
#define EVT_PHY_SCH_RX_UCI_IND                     ((WORD32)(EV_PHY_TO_SCH_START + 3))
#define EVT_PHY_SCH_SRS_IND                        ((WORD32)(EV_PHY_TO_SCH_START + 4))
#define EVT_PHY_SCH_CRC_IND                        ((WORD32)(EV_PHY_TO_SCH_START + 5))
#define EVT_PHY_SCH_TIMING_ADV_IND                 ((WORD32)(EV_PHY_TO_SCH_START + 6))
#define EVT_PHY_SCH_PUCCH_DELTA_POWER              ((WORD32)(EV_PHY_TO_SCH_START + 7))
#define EVT_PHY_SCH_SINR_SMO_IND                   ((WORD32)(EV_PHY_TO_SCH_START + 8))
#define EVT_PHY_SCH_FREQ_SEL_IND                   ((WORD32)(EV_PHY_TO_SCH_START + 9))


/* DMAC给CL的下行数据消息, 起始消息ID : 3136 */
#define EVT_TFU_DL_PDU_REQUEST                     ((WORD32)(EV_DMAC_TO_CL_START + 0))

/* RRC配置CL消息, 起始消息ID : 3168 */
#define EVT_RRC_CELL_CONFIG_REQUEST                ((WORD32)(EV_RRC_TO_CL_START + 0))

/* CL发往SCH消息, 起始消息ID : 3200 */


/* RRC配置SCH消息, 起始消息ID : 3232 */
#define EVT_RRC_SCH_GNB_CONFIG_REQUEST             ((WORD32)(EV_RRC_TO_SCH_START + 0))
#define EVT_RRC_SCH_LVL1_CELL_CONFIG_REQUEST       ((WORD32)(EV_RRC_TO_SCH_START + 1))
#define EVT_RRC_SCH_LVL1_CELL_RECONFIG_REQUEST     ((WORD32)(EV_RRC_TO_SCH_START + 2))
#define EVT_RRC_SCH_LVL1_CELL_DELETE_REQUEST       ((WORD32)(EV_RRC_TO_SCH_START + 3))
#define EVT_RRC_SCH_LVL2_CELL_CONFIG_REQUEST       ((WORD32)(EV_RRC_TO_SCH_START + 4))
#define EVT_RRC_SCH_LVL2_CELL_DELETE_REQUEST       ((WORD32)(EV_RRC_TO_SCH_START + 5))
#define EVT_RRC_SCH_MIB_CONFIG_REQUEST             ((WORD32)(EV_RRC_TO_SCH_START + 6))
#define EVT_RRC_SCH_SIB1_CONFIG_REQUEST            ((WORD32)(EV_RRC_TO_SCH_START + 7))
#define EVT_RRC_SCH_SI_CONFIG_REQUEST              ((WORD32)(EV_RRC_TO_SCH_START + 8))
#define EVT_RRC_SCH_PAGING_CONFIG_REQUEST          ((WORD32)(EV_RRC_TO_SCH_START + 9))
#define EVT_RRC_SCH_CCCH_DATA_REQUEST              ((WORD32)(EV_RRC_TO_SCH_START + 10))
#define EVT_RRC_SCH_DED_RACH_COMPLETE_INDICATION   ((WORD32)(EV_RRC_TO_SCH_START + 11))
#define EVT_RRC_SCH_SYMBOL_OFF_CONFIG_REQUEST      ((WORD32)(EV_RRC_TO_SCH_START + 12))
#define EVT_RRC_SCH_SYMBOL_OFF_ENABLE_RESPONSE     ((WORD32)(EV_RRC_TO_SCH_START + 13))
#define EVT_EGTP_ICMP_OPTIMIZE_REQUEST             ((WORD32)(EV_RRC_TO_SCH_START + 14))
#define EVT_RRC_SCH_TRF_PATTERN_CONFIG_REQUEST     ((WORD32)(EV_RRC_TO_SCH_START + 15))

#define EVT_RRC_SCH_LVL1_UE_CONFIG_REQUEST         ((WORD32)(EV_RRC_TO_SCH_START + 16))
#define EVT_RRC_SCH_LVL1_UE_RECONFIG_REQUEST       ((WORD32)(EV_RRC_TO_SCH_START + 17))
#define EVT_RRC_SCH_LVL1_UE_DELETE_REQUEST         ((WORD32)(EV_RRC_TO_SCH_START + 18))
#define EVT_RRC_SCH_LVL1_LCH_CONFIG_REQUEST        ((WORD32)(EV_RRC_TO_SCH_START + 19))
#define EVT_RRC_SCH_LVL1_LCH_RECONFIG_REQUEST      ((WORD32)(EV_RRC_TO_SCH_START + 20))
#define EVT_RRC_SCH_LVL1_LCH_DELETE_REQUEST        ((WORD32)(EV_RRC_TO_SCH_START + 21))
#define EVT_RRC_SCH_LVL1_UL_DATA_TRANS_ACTN        ((WORD32)(EV_RRC_TO_SCH_START + 22))
#define EVT_RRC_SCH_LVL1_RECONFIG_COMPLETE_IND     ((WORD32)(EV_RRC_TO_SCH_START + 23))

#define EVT_RRC_SCH_LVL1_SCELL_ACT                 ((WORD32)(EV_RRC_TO_SCH_START + 24))
#define EVT_RRC_SCH_LVL1_SCELL_DEACT               ((WORD32)(EV_RRC_TO_SCH_START + 25))
#define EVT_RRC_SCH_LVL1_SCELL_READY               ((WORD32)(EV_RRC_TO_SCH_START + 26))
#define EVT_RRC_SCH_LVL1_SCELL_INACTIVE            ((WORD32)(EV_RRC_TO_SCH_START + 27))
#define EVT_RRC_SCH_LVL1_SCELL_PUCCH_MAP_ADD_DEL   ((WORD32)(EV_RRC_TO_SCH_START + 28))


/******************************************************************************
 * 定义PHY消息号段内部各个子模块消息号段 (3584 ~ 4095)
 * EV_PHY_START ~ EV_PHY_END
 *****************************************************************************/
/* CL发给PHY消息号段 (  0 ~ 31) */
#define EV_CL_TO_PHY_START                         ((WORD32)(EV_PHY_START + 0x000))
#define EV_CL_TO_PHY_END                           ((WORD32)(EV_PHY_START + 0x01F))


#ifdef __cplusplus
}
#endif


#endif


