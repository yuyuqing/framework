

#include "base_epoll.h"
#include "base_log.h"


CEpollEvent::CEpollEvent()
{
    m_dwEventID      = INVALID_DWORD;
    m_dwInstID       = INVALID_DWORD;
    m_pObj           = NULL;
    m_pHandleFunc    = NULL;
    m_pExceptionFunc = NULL;

    memset(&m_tEvent, 0x00, sizeof(m_tEvent));
}


CEpollEvent::CEpollEvent(WORD32     dwEventID,
                         WORD32     dwInstID,
                         WORD32     dwFlag,
                         CCBObject *pObj,
                         PCBFUNC    pHandleFunc,
                         PCBFUNC    pExceptionFunc)
{
    m_dwEventID      = dwEventID;
    m_dwInstID       = dwInstID;
    m_pObj           = pObj;
    m_pHandleFunc    = pHandleFunc;
    m_pExceptionFunc = pExceptionFunc;

    memset(&m_tEvent, 0x00, sizeof(m_tEvent));

    m_tEvent.data.fd  = dwEventID;
    m_tEvent.data.ptr = this;
    m_tEvent.events   = dwFlag;
}


CEpollEvent::~CEpollEvent()
{
    m_dwEventID      = INVALID_DWORD;
    m_dwInstID       = INVALID_DWORD;
    m_pObj           = NULL;
    m_pHandleFunc    = NULL;
    m_pExceptionFunc = NULL;

    memset(&m_tEvent, 0x00, sizeof(m_tEvent));
}


/* epoll_wait返回正常读取文件描述符 */
WORD32 CEpollEvent::Handle()
{
    if ( (unlikely(NULL == m_pObj))
      || (unlikely(NULL == m_pHandleFunc)))
    {
        return FAIL;
    }

    (m_pObj->*(m_pHandleFunc)) (&m_dwEventID, sizeof(m_dwEventID));

    return SUCCESS;
}


/* epoll_wait返回异常读取文件描述符 */
WORD32 CEpollEvent::Exception()
{
    if ( (unlikely(NULL == m_pObj))
      || (unlikely(NULL == m_pExceptionFunc)))
    {
        return FAIL;
    }

    (m_pObj->*(m_pExceptionFunc)) (&m_dwEventID, sizeof(m_dwEventID));

    return SUCCESS;
}


T_EpollEvent & CEpollEvent::GetEvent()
{
    return m_tEvent;
}


CBaseEpoll::CBaseEpoll()
{
    m_iFileID    = -1;
    m_dwEventNum = 0;
    memset(m_atEvent, 0x00, sizeof(m_atEvent));
}


CBaseEpoll::~CBaseEpoll()
{
    if (m_iFileID > 0)
    {
        close(m_iFileID);
    }

    m_iFileID    = -1;
    m_dwEventNum = 0;
    memset(m_atEvent, 0x00, sizeof(m_atEvent));
}


WORD32 CBaseEpoll::Initialize()
{
    SWORD32 iFileID = epoll_create(s_dwEventSize);
    if (iFileID < 0)
    {
        return FAIL;
    }

    m_iFileID = iFileID;
    m_cTree.Initialize();

    return SUCCESS;
}


/* 添加事件 */
WORD32 CBaseEpoll::AddEvent(SWORD32    iEventID,
                            CCBObject *pObj,
                            PCBFUNC    pHandleFunc,
                            PCBFUNC    pExceptionFunc)
{
    TRACE_STACK("CBaseEpoll::AddEvent()");

    if ( (m_iFileID < 0) 
      || (iEventID  < 0)
      || (NULL == pObj)
      || (NULL == pHandleFunc)
      || (NULL == pExceptionFunc))
    {
        return FAIL;
    }

    SWORD32                iResult   = 0;
    SWORD32                iFlag     = 0;
    WORD32                 dwInstID  = INVALID_DWORD;
    WORD32                 dwEventID = (WORD32)iEventID;
    CGuardLock<CSpinLock>  cGuard(m_cLock);

    iFlag = fcntl(iEventID, F_GETFL, 0);
    fcntl(iEventID, F_SETFL, iFlag | O_NONBLOCK);

    CEpollEvent *pEvent = m_cTree.Create(dwInstID, dwEventID);
    if (NULL == pEvent)
    {
        return FAIL;
    }

    /* EPOLLIN    : 文件描述符可读
     * EPOLLPRI   : 优先级数据可读
     * EPOLLRDHUP : 与EPOLLIN一起使用, 用于识别和处理对端关闭连接的情况(如TCP关闭或半关闭(shutdown)连接) 
     * EPOLLHUP   : 对端关闭连接或文件描述符不可用
     */
    new (pEvent) CEpollEvent(dwEventID,
                             dwInstID,
                             EPOLLIN | EPOLLPRI | EPOLLRDHUP | EPOLLHUP,
                             pObj,
                             pHandleFunc,
                             pExceptionFunc);

    T_EpollEvent &rtEvent = pEvent->GetEvent();

    iResult = epoll_ctl(m_iFileID, EPOLL_CTL_ADD, iEventID, &rtEvent);
    if (iResult < 0)
    {
        delete pEvent;
        m_cTree.DeleteByInstID(dwInstID);
        return FAIL;
    }

    m_dwEventNum++;

    return SUCCESS;
}


/* 移除事件 */
WORD32 CBaseEpoll::RmvEvent(SWORD32 iEventID)
{
    TRACE_STACK("CBaseEpoll::RmvEvent()");

    if ((m_iFileID < 0) || (iEventID < 0))
    {
        return FAIL;
    }

    SWORD32                iResult   = 0;
    WORD32                 dwEventID = (WORD32)iEventID;
    CGuardLock<CSpinLock>  cGuard(m_cLock);

    iResult = epoll_ctl(m_iFileID, EPOLL_CTL_DEL, iEventID, NULL);
    if (iResult < 0)
    {
        return FAIL;
    }

    m_cTree.Delete(dwEventID);
    m_dwEventNum--;

    return SUCCESS;
}


WORD32 CBaseEpoll::Wait(WORD32 dwTimeLen)
{
    WORD32 dwCount = epoll_wait(m_iFileID,
                                m_atEvent, 
                                s_dwEventSize,
                                dwTimeLen);
    if (FAIL == dwCount)
    {
        return 0;
    }

    if (0 == dwCount)
    {
        /* 超时处理 */
    }

    for (WORD32 dwIndex = 0; dwIndex < dwCount; dwIndex++)
    {
        if (dwIndex >= s_dwEventSize)
        {
            break ;
        }

        CEpollEvent *pEvent = (CEpollEvent *)(m_atEvent[dwIndex].data.ptr);
        if (unlikely(NULL == pEvent))
        {
            /* Error */
            RmvEvent(m_atEvent[dwIndex].data.fd);
            continue ;
        }

        /* 数据可读 */
        if (m_atEvent[dwIndex].events & EPOLLIN)
        {
            pEvent->Handle();
        }

        /* shutdown or error */
        if ( (m_atEvent[dwIndex].events & EPOLLRDHUP)
          || (m_atEvent[dwIndex].events & EPOLLERR)
          || (m_atEvent[dwIndex].events & EPOLLHUP))
        {
            pEvent->Exception();
            RmvEvent(m_atEvent[dwIndex].data.fd);
        }
    }

    return dwCount;
}


