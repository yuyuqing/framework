

#ifndef _PUB_MESSAGE_RRC_H_
#define _PUB_MESSAGE_RRC_H_


/* ��C���� */
#ifdef __cplusplus
extern "C"{
#endif


#include "pub_event.h"
#include "pub_global_def.h"


/******************************************************************************/
/*******************************    OAM ��Ϣ    *******************************/
/******************************************************************************/
/* ��վ��DuMgr��OamAgent����������Ϣ */
typedef struct tagT_DuMgrOamStartRequest
{
    WORD32  dwMaxCellNum;
}T_DuMgrOamStartRequest;


/* SysRepo��OamAgent������������, ����OamAgent����DuConfigRequest��Ϣ */
typedef struct tagT_DuOamConfigRequest
{
}T_DuOamConfigRequest;


/* OamAgent��DuMgr����DU��������, ������վ���С���������� */
typedef struct tagT_OamDuMgrDuConfigRequest
{
}T_OamDuMgrDuConfigRequest;


/* DuMgr��OamAgent����DuConfigResponse */
typedef struct tagT_DuMgrOamDuConfigResponse
{
}T_DuMgrOamDuConfigResponse;


/* OamAgent��DuMgr����Cell��������, ������վ���С���������� */
typedef struct tagT_OamDuMgrCellConfigRequest
{
}T_OamDuMgrCellConfigRequest;


/* DuMgr��OamAgent����CellConfigResponse */
typedef struct tagT_DuMgrOamCellConfigResponse
{
}T_DuMgrOamCellConfigResponse;


/******************************************************************************/
/*******************************    RRU ��Ϣ    *******************************/
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
/*******************************    SCTP��Ϣ    *******************************/
/******************************************************************************/
/* DuMgr��Sctp����SCTP����������Ϣ */
typedef struct tagT_SctpConfigRequest
{
}T_SctpConfigRequest;


/* Sctp��DuMgr������Ӧ��Ϣ */
typedef struct tagT_SctpConfigResponse
{
}T_SctpConfigResponse;


/* SCTP��DuMgr����ż��֪ͨ */
typedef struct tagT_SctpAssocStatusNotify
{
    BYTE  ucStatus;
}T_SctpAssocStatusNotify;


/******************************************************************************/
/*******************************    DBS ��Ϣ    *******************************/
/******************************************************************************/
/* ���ݿⴥ��С����������(OamAgent����DuMgr) */
typedef struct tagT_DbsCellAddReq
{
    WORD32    dwCellID;
}T_DbsCellAddReq;


/* ���ݿⴥ��С��ɾ������(OamAgent����DuMgr) */
typedef struct tagT_DbsCellDelReq
{
    WORD32    dwCellID;
}T_DbsCellDelReq;


/* ���ݿⴥ��С��Reset����(OamAgent����DuMgr) */
typedef struct tagT_DbsCellResetReq
{
    WORD32    dwCellID;
}T_DbsCellResetReq;


/* ���ݿⴥ��С����������(OamAgent����DuMgr) */
typedef struct tagT_DbsMibRecfgReq
{
    WORD32    dwCellID;
}T_DbsMibRecfgReq;


/* ���ݿⴥ��С����������(OamAgent����DuMgr) */
typedef struct tagT_DbsAmcRecfgReq
{
    WORD32    dwCellID;
}T_DbsAmcRecfgReq;


/* ���ݿⴥ��С����������(OamAgent����DuMgr) */
typedef struct tagT_DbsRankAdaptRecfgReq
{
    WORD32    dwCellID;
}T_DbsRankAdaptRecfgReq;


/* ���ݿⴥ��С����������(OamAgent����DuMgr) */
typedef struct tagT_DbsTaRecfgReq
{
    WORD32    dwCellID;
}T_DbsTaRecfgReq;


/******************************************************************************/
/*******************************    RRM ��Ϣ    *******************************/
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
/*******************************    MAC ��Ϣ    *******************************/
/******************************************************************************/
typedef struct tagT_PhyDuMgrSlotInd
{
    WORD16    wSFN;
    BYTE      ucSlot;
}T_PhyDuMgrSlotInd;


/* ����GNB */
typedef struct tagT_DuMgrSchGnbCfgReq
{
    BYTE    ucDlSchdType;
    BYTE    ucUlSchdType;
    BYTE    ucMaxDlUePerTti;
    BYTE    ucMaxUlUePerTti;
    BYTE    ucNumCells;
}T_DuMgrSchGnbCfgReq;


/* ����GNB */
typedef struct tagT_SchDuMgrGnbCfgConfirm
{
}T_SchDuMgrGnbCfgConfirm;


/* ����Lv1С�� */
typedef struct tagT_RrcSchCellLv1CfgReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;

    VOID     *ptCellCfg;
}T_RrcSchCellLv1CfgReq;


/* ����Lv1С�� */
typedef struct tagT_SchRrcCellLv1CfgConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_SchRrcCellLv1CfgConfirm;


/* ����Lv2С�� */
typedef struct tagT_RrcSchCellLv2CfgReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;

    VOID     *ptCellCfg;
}T_RrcSchCellLv2CfgReq;


/* ����Lv2С�� */
typedef struct tagT_SchRrcCellLv2CfgConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_SchRrcCellLv2CfgConfirm;


/* ����С��MIB */
typedef struct tagT_RrcSchMibCfgReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcSchMibCfgReq;


/* ����С��MIB */
typedef struct tagT_SchRrcMibCfgConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_SchRrcMibCfgConfirm;


/* ����С��SIB1 */
typedef struct tagT_RrcSchSib1CfgReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcSchSib1CfgReq;


/* ����С��SIB1 */
typedef struct tagT_SchRrcSib1CfgConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_SchRrcSib1CfgConfirm;


/* ����С��SI */
typedef struct tagT_RrcSchSibXCfgReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcSchSibXCfgReq;


/* ����С��SI */
typedef struct tagT_SchRrcSibXCfgConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_SchRrcSibXCfgConfirm;


/* ����С��CL */
typedef struct tagT_RrcClCellCfgReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcClCellCfgReq;


/* ����С��CL */
typedef struct tagT_ClRrcCellCfgConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_ClRrcCellCfgConfirm;


/* ����Lv1С�� */
typedef struct tagT_RrcSchCellLv1ReCfgReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;

    VOID     *ptCellReCfg;
}T_RrcSchCellLv1ReCfgReq;


/* ����Lv1С�� */
typedef struct tagT_SchRrcCellLv1ReCfgConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_SchRrcCellLv1ReCfgConfirm;


/* ����С��CL */
typedef struct tagT_RrcClCellReCfgReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcClCellReCfgReq;


/* ����С��CL */
typedef struct tagT_ClRrcCellReCfgConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_ClRrcCellReCfgConfirm;


/* ɾ��Lv1С�� */
typedef struct tagT_RrcSchCellLv1DelReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcSchCellLv1DelReq;


/* ɾ��Lv1С�� */
typedef struct tagT_SchRrcCellLv1DelConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_SchRrcCellLv1DelConfirm;


/* ɾ��Lv2С�� */
typedef struct tagT_RrcSchCellLv2DelReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcSchCellLv2DelReq;


/* ɾ��Lv2С�� */
typedef struct tagT_SchRrcCellLv2DelConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_SchRrcCellLv2DelConfirm;


/* ɾ��С��CL */
typedef struct tagT_RrcClCellDelReq
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_RrcClCellDelReq;


/* ɾ��С��CL */
typedef struct tagT_ClRrcCellDelConfirm
{
    WORD64    lwTransID;
    WORD32    dwCellID;
}T_ClRrcCellDelConfirm;


/* Ѱ�� */
typedef struct tagT_RrcSchPagingReq
{
    WORD32    dwCellID;
    WORD16    wPF;
    BYTE      ucIS;
}T_RrcSchPagingReq;


/* Ѱ�� */
typedef struct tagT_RrcSchPagingConfirm
{
    WORD32    dwCellID;
}T_RrcSchPagingConfirm;



#ifdef __cplusplus
}
#endif


#endif

