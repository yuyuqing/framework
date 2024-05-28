

#ifndef _PUB_MESSAGE_BASE_TYPE_H_
#define _PUB_MESSAGE_BASE_TYPE_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "pub_typedef.h"


typedef struct tagT_BaseMessage
{
    WORD32   dwMsgID;          /* ��ϢID */
    WORD32   dwSrcAppID;       /* ���ͷ�AppID */
    WORD32   dwDstAppID;       /* ���շ�AppID */
    WORD32   dwSrcThreadID;    /* �����߳�ID */
    WORD32   dwDstThreadID;    /* �����߳�ID */
    WORD16   wResved;
    WORD16   wMsgLen;
    WORD64   lwStartCycle;     /* �����ʱCycleʱ��� */
    WORD64   lwEndCycle;       /* ������ʱCycleʱ��� */
    CHAR     aValue[];
}T_BaseMessage;


#ifdef __cplusplus
}
#endif


#endif


