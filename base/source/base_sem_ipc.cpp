

#include <errno.h>

#include "base_sem_ipc.h"


CSemHandler::CSemHandler()
{
    m_dwKey    = INVALID_DWORD;
    m_iSemID   = -1;
    m_bMaster  = FALSE;
    m_dwResVal = 0;
}


CSemHandler::~CSemHandler()
{
    if ((m_iSemID > 0) && (m_bMaster))
    {
        semctl(m_iSemID, 0, IPC_RMID);
    }

    m_dwKey    = INVALID_DWORD;
    m_iSemID   = -1;
    m_bMaster  = FALSE;
    m_dwResVal = 0;
}


WORD32 CSemHandler::Initialize(WORD32 dwKey, BOOL bMaster, WORD32 dwValue)
{
    key_t    tSemKey     = static_cast<key_t>(dwKey);
    SWORD32  iSemgetFlag = 0666;
    SWORD32  iSemID      = -1;
    SWORD32  iResult     = 0;
    semun    tSemValue;

    iSemID = semget(tSemKey, 1, iSemgetFlag);
    if ((iSemID < 0) && (ENOENT == errno))
    {
        iSemgetFlag = 0666 | IPC_CREAT | IPC_EXCL;

        iSemID = semget(tSemKey, 1, iSemgetFlag);
        if (iSemID < 0)
        {
            return FAIL;
        }
    }

    if (bMaster)
    {
        tSemValue.val = dwValue;

        if (semctl(iSemID, 0, SETVAL, tSemValue) < 0)
        {
            return FAIL;
        }
    }
    else
    {
        do
        {
            iResult = semctl(iSemID, 0, GETVAL, tSemValue);
            if (iResult < 0)
            {
                return FAIL;
            }

            if (((WORD32)iResult) == dwValue)
            {
                break ;
            }

            usleep(100);
        } while(TRUE);
    }

    m_dwKey    = dwKey;
    m_iSemID   = iSemID;
    m_bMaster  = bMaster;
    m_dwResVal = dwValue;

    return SUCCESS;
}


WORD32 CSemHandler::Clean(WORD32 dwKey)
{
    key_t    tSemKey     = static_cast<key_t>(dwKey);
    SWORD32  iSemgetFlag = 0666;
    SWORD32  iSemID      = -1;

    iSemID = semget(tSemKey, 1, iSemgetFlag);
    if (iSemID >= 0)
    {
        semctl(iSemID, 0, IPC_RMID);
    }

    return SUCCESS;
}


