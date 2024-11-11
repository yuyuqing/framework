

#ifndef _PUB_MESSAGE_RRC_H_
#define _PUB_MESSAGE_RRC_H_


/* 与C兼容 */
#ifdef __cplusplus
extern "C"{
#endif


#include "pub_event.h"
#include "pub_global_def.h"


/******************************************************************************/
/*******************************    OAM 消息    *******************************/
/******************************************************************************/
/* 起站后DuMgr向OamAgent发送启动消息 */
typedef struct tagT_DuMgrOamStartRequest
{
    WORD32  dwMaxCellNum;
}T_DuMgrOamStartRequest;


/* SysRepo向OamAgent发送配置请求, 触发OamAgent发送DuConfigRequest消息 */
typedef struct tagT_DuOamConfigRequest
{
}T_DuOamConfigRequest;


/* OamAgent向DuMgr发送DU配置请求, 触发起站后的小区建立流程 */
typedef struct tagT_OamDuMgrDuConfigRequest
{
}T_OamDuMgrDuConfigRequest;


/* DuMgr向OamAgent返回DuConfigResponse */
typedef struct tagT_DuMgrOamDuConfigResponse
{
}T_DuMgrOamDuConfigResponse;


/* OamAgent向DuMgr发送Cell配置请求, 触发起站后的小区建立流程 */
typedef struct tagT_OamDuMgrCellConfigRequest
{
}T_OamDuMgrCellConfigRequest;


/* DuMgr向OamAgent返回CellConfigResponse */
typedef struct tagT_DuMgrOamCellConfigResponse
{
}T_DuMgrOamCellConfigResponse;


/******************************************************************************/
/*******************************    RRU 消息    *******************************/
/******************************************************************************/
typedef struct tagT_RrcRruConfigRequest
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcRruConfigRequest;


typedef struct tagT_RruRrcConfigResponse
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RruRrcConfigResponse;


/******************************************************************************/
/*******************************    SCTP消息    *******************************/
/******************************************************************************/
/* DuMgr向Sctp发送SCTP配置请求消息 */
typedef struct tagT_SctpConfigRequest
{
}T_SctpConfigRequest;


/* Sctp向DuMgr返回响应消息 */
typedef struct tagT_SctpConfigResponse
{
}T_SctpConfigResponse;


/* SCTP向DuMgr发送偶联通知 */
typedef struct tagT_SctpAssocStatusNotify
{
    BYTE  ucStatus;
}T_SctpAssocStatusNotify;


/******************************************************************************/
/*******************************    DBS 消息    *******************************/
/******************************************************************************/
/* 数据库触发小区建立请求(OamAgent发给DuMgr) */
typedef struct tagT_DbsCellAddReq
{
    WORD32    dwCellID;
}T_DbsCellAddReq;


/* 数据库触发小区删除请求(OamAgent发给DuMgr) */
typedef struct tagT_DbsCellDelReq
{
    WORD32    dwCellID;
}T_DbsCellDelReq;


/* 数据库触发小区Reset请求(OamAgent发给DuMgr) */
typedef struct tagT_DbsCellResetReq
{
    WORD32    dwCellID;
}T_DbsCellResetReq;


/* 数据库触发小区重配请求(OamAgent发给DuMgr) */
typedef struct tagT_DbsMibRecfgReq
{
    WORD32    dwCellID;
}T_DbsMibRecfgReq;


/* 数据库触发小区重配请求(OamAgent发给DuMgr) */
typedef struct tagT_DbsAmcRecfgReq
{
    WORD32    dwCellID;
}T_DbsAmcRecfgReq;


/* 数据库触发小区重配请求(OamAgent发给DuMgr) */
typedef struct tagT_DbsRankAdaptRecfgReq
{
    WORD32    dwCellID;
}T_DbsRankAdaptRecfgReq;


/* 数据库触发小区重配请求(OamAgent发给DuMgr) */
typedef struct tagT_DbsTaRecfgReq
{
    WORD32    dwCellID;
}T_DbsTaRecfgReq;


/******************************************************************************/
/*******************************    RRM 消息    *******************************/
/******************************************************************************/
typedef struct tagT_RrcRrmConfigRequest
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcRrmConfigRequest;


typedef struct tagT_RrmRrcConfigResponse
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrmRrcConfigResponse;


typedef struct tagT_RrcRrmReConfigRequest
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcRrmReConfigRequest;


typedef struct tagT_RrmRrcReConfigResponse
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrmRrcReConfigResponse;


typedef struct tagT_RrcRrmDeleteRequest
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcRrmDeleteRequest;


typedef struct tagT_RrmRrcDeleteResponse
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrmRrcDeleteResponse;


/******************************************************************************/
/*******************************    MAC 消息    *******************************/
/******************************************************************************/
typedef struct tagT_PhyDuMgrSlotInd
{
    WORD16    wSFN;
    BYTE      ucSlot;
}T_PhyDuMgrSlotInd;


/* 配置GNB */
typedef struct tagT_DuMgrSchGnbCfgReq
{
    BYTE    ucDlSchdType;
    BYTE    ucUlSchdType;
    BYTE    ucMaxDlUePerTti;
    BYTE    ucMaxUlUePerTti;
    BYTE    ucNumCells;
}T_DuMgrSchGnbCfgReq;


/* 配置GNB */
typedef struct tagT_SchDuMgrGnbCfgConfirm
{
}T_SchDuMgrGnbCfgConfirm;


/* 配置Lv1小区 */
typedef struct tagT_RrcSchCellLv1CfgReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;

    VOID     *ptCellCfg;
}T_RrcSchCellLv1CfgReq;


/* 配置Lv1小区 */
typedef struct tagT_SchRrcCellLv1CfgConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_SchRrcCellLv1CfgConfirm;


/* 配置Lv2小区 */
typedef struct tagT_RrcSchCellLv2CfgReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;

    VOID     *ptCellCfg;
}T_RrcSchCellLv2CfgReq;


/* 配置Lv2小区 */
typedef struct tagT_SchRrcCellLv2CfgConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_SchRrcCellLv2CfgConfirm;


/* 配置小区MIB */
typedef struct tagT_RrcSchMibCfgReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcSchMibCfgReq;


/* 配置小区MIB */
typedef struct tagT_SchRrcMibCfgConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_SchRrcMibCfgConfirm;


/* 配置小区SIB1 */
typedef struct tagT_RrcSchSib1CfgReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcSchSib1CfgReq;


/* 配置小区SIB1 */
typedef struct tagT_SchRrcSib1CfgConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_SchRrcSib1CfgConfirm;


/* 配置小区SI */
typedef struct tagT_RrcSchSibXCfgReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcSchSibXCfgReq;


/* 配置小区SI */
typedef struct tagT_SchRrcSibXCfgConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_SchRrcSibXCfgConfirm;


/* 配置小区CL */
typedef struct tagT_RrcClCellCfgReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcClCellCfgReq;


/* 配置小区CL */
typedef struct tagT_ClRrcCellCfgConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_ClRrcCellCfgConfirm;


/* 重配Lv1小区 */
typedef struct tagT_RrcSchCellLv1ReCfgReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;

    VOID     *ptCellReCfg;
}T_RrcSchCellLv1ReCfgReq;


/* 重配Lv1小区 */
typedef struct tagT_SchRrcCellLv1ReCfgConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_SchRrcCellLv1ReCfgConfirm;


/* 重配小区CL */
typedef struct tagT_RrcClCellReCfgReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcClCellReCfgReq;


/* 重配小区CL */
typedef struct tagT_ClRrcCellReCfgConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_ClRrcCellReCfgConfirm;


/* 删除Lv1小区 */
typedef struct tagT_RrcSchCellLv1DelReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcSchCellLv1DelReq;


/* 删除Lv1小区 */
typedef struct tagT_SchRrcCellLv1DelConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_SchRrcCellLv1DelConfirm;


/* 删除Lv2小区 */
typedef struct tagT_RrcSchCellLv2DelReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcSchCellLv2DelReq;


/* 删除Lv2小区 */
typedef struct tagT_SchRrcCellLv2DelConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_SchRrcCellLv2DelConfirm;


/* 删除小区CL */
typedef struct tagT_RrcClCellDelReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcClCellDelReq;


/* 删除小区CL */
typedef struct tagT_ClRrcCellDelConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_ClRrcCellDelConfirm;


/* 寻呼 */
typedef struct tagT_RrcSchPagingReq
{
    WORD32    dwCellID;
    WORD16    wPF;
    BYTE      ucIS;
}T_RrcSchPagingReq;


/* 寻呼 */
typedef struct tagT_RrcSchPagingConfirm
{
    WORD32    dwCellID;
}T_RrcSchPagingConfirm;



#ifdef __cplusplus
}
#endif


#endif


