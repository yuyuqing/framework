

#ifndef _PUB_MESSAGE_BASE_TYPE_H_
#define _PUB_MESSAGE_BASE_TYPE_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "pub_typedef.h"


typedef struct tagT_BaseMessage
{
    WORD32   dwMsgID;          /* 消息ID */
    WORD32   dwSrcAppID;       /* 发送方AppID */
    WORD32   dwDstAppID;       /* 接收方AppID */
    WORD32   dwSrcThreadID;    /* 发送线程ID */
    WORD32   dwDstThreadID;    /* 接收线程ID */
    WORD16   wResved;
    WORD16   wMsgLen;
    WORD64   lwStartCycle;     /* 入队列时Cycle时间戳 */
    WORD64   lwEndCycle;       /* 出队列时Cycle时间戳 */
    CHAR     aValue[];
}T_BaseMessage;


#ifdef __cplusplus
}
#endif


#endif


