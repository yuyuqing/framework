

#ifndef _BASE_SEM_IPC_H_
#define _BASE_SEM_IPC_H_


#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

#include "pub_typedef.h"


typedef union tagU_Semun
{
    int               val;    /* Value for SETVAL */
    struct semid_ds  *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short   *array;  /* Array for GETALL, SETALL */
    struct seminfo   *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
}semun;


class CSemHandler
{
public :

public :
    CSemHandler();
    virtual ~CSemHandler();

    /* dwValue 用于控制资源个数(0 : empty, 1 : lock, N : resource_num) */
    WORD32 Initialize(WORD32 dwKey, BOOL bMaster, WORD32 dwValue = 0);

    WORD32 Clean(WORD32 dwKey);

    WORD32 Post();  /* V操作, 即Val+1操作, 用于生产者通知消费者 */
    WORD32 Wait();  /* P操作, 即Val-1操作, 用于消费者等待资源 */

protected :
    WORD32         m_dwKey;
    SWORD32        m_iSemID;
    BOOL           m_bMaster;
    WORD32         m_dwResVal;
};


/* V操作, 即Val+1操作, 用于生产者通知消费者 */
inline WORD32 CSemHandler::Post()
{
    /* IPC_NOWAIT or SEM_UNDO */
    struct sembuf tSemBuf = {0, 1, SEM_UNDO};

    if (semop(m_iSemID, &tSemBuf, 1) < 0)
    {
        return FAIL;
    }

    return SUCCESS;
}


/* P操作, 即Val-1操作, 用于消费者等待资源 */
inline WORD32 CSemHandler::Wait()
{
    /* IPC_NOWAIT or SEM_UNDO */
    struct sembuf tSemBuf = {0, -1, SEM_UNDO};

    if (semop(m_iSemID, &tSemBuf, 1) < 0)
    {
        return FAIL;
    }

    return SUCCESS;
}


#endif


