

#ifndef _BASE_EPOLL_H_
#define _BASE_EPOLL_H_


#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "base_call_back.h"
#include "base_lock.h"
#include "base_tree.h"


typedef struct epoll_event T_EpollEvent;


class CEpollEvent : public CBaseData
{
public :
    CEpollEvent();

    CEpollEvent(WORD32     dwEventID,
                WORD32     dwInstID,
                WORD32     dwFlag,
                CCBObject *pObj,
                PCBFUNC    pHandleFunc,
                PCBFUNC    pExceptionFunc);

    virtual ~CEpollEvent();

    /* epoll_wait����������ȡ�ļ������� */
    WORD32 Handle();

    /* epoll_wait�����쳣��ȡ�ļ������� */
    WORD32 Exception();

    T_EpollEvent & GetEvent();

protected :
    WORD32          m_dwEventID;
    WORD32          m_dwInstID;

    CCBObject      *m_pObj;
    PCBFUNC         m_pHandleFunc;     /* ������ȡ�¼�(Socket/File)�ص����� */
    PCBFUNC         m_pExceptionFunc;  /* �쳣����ص����� */

    T_EpollEvent    m_tEvent;
};


class CBaseEpoll
{
public :
    enum { EPOLL_EVENT_POWER_NUM =    8 };  /* epoll_event����¼����� */
    enum { EPOLL_MAX_WAIT_TIME   = 1000 };  /* epoll_wait���ʱʱ��(��λ:����) */

    static const WORD32 s_dwEventSize = (1 << EPOLL_EVENT_POWER_NUM);

    typedef CBTreeTpl<CEpollEvent, WORD32, EPOLL_EVENT_POWER_NUM>  CEpollEventTree;

public :
    CBaseEpoll();
    virtual ~CBaseEpoll();

    WORD32 Initialize();

    /* ����¼� */
    WORD32 AddEvent(SWORD32    iEventID,
                    CCBObject *pObj,
                    PCBFUNC    pHandleFunc,
                    PCBFUNC    pExceptionFunc);

    /* �Ƴ��¼� */
    WORD32 RmvEvent(SWORD32 iEventID);

    WORD32 Wait(WORD32 dwTimeLen = EPOLL_MAX_WAIT_TIME);

protected :
    SWORD32          m_iFileID;                 /* epoll_create�������ļ�ID */
    WORD32           m_dwEventNum;              /* �¼����� */

    T_EpollEvent     m_atEvent[s_dwEventSize];  /* ���汻�������¼� */

    CSpinLock        m_cLock;
    CEpollEventTree  m_cTree;
};


#endif


