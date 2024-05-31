

#ifndef _BASE_INIT_COMPONENT_H_
#define _BASE_INIT_COMPONENT_H_


#include "base_singleton_tpl.h"
#include "base_log_wrapper.h"
#include "base_mem_mgr.h"
#include "base_time.h"


typedef WORD32 (*PINITFUNC)(WORD32 dwProcID, VOID *pArg);
typedef WORD32 (*PEXITFUNC)(VOID *pArg);


typedef struct tagT_InitFunc
{
    WORD32            dwLevel;
    CHAR             *pName;
    PINITFUNC         pInitFunc;    
    PEXITFUNC         pExitFunc;
    CCentralMemPool  *pMemInterface;   /* ��ҳ�ڴ����ӿ� */
}T_InitFunc;


class CInitList : public CSingleton<CInitList>
{
public :    
    enum { MAX_INIT_FUNC_NUM = 64 };

    static WORD32 CreateFunc(const T_InitFunc *ptFunc);

public :
    CInitList ();
    virtual ~CInitList ();

    const T_InitFunc * Create(const T_InitFunc *ptFunc);
    const T_InitFunc * Find(CHAR * pName);

    virtual WORD32 InitComponent(WORD32 dwProcID, CB_RegistMemPool pFunc = NULL);

    VOID Dump();

protected :
    WORD32 InitGlobalData(WORD32 dwProcID);
    WORD32 DestroyGlobalData();

public :
    CB_RegistMemPool  m_pRegistMemPoolFunc;  /* ��NGP�ڴ��ע���߳���Ϣ */

protected :
    WORD32            m_dwNum;
    T_InitFunc        m_atFunc[MAX_INIT_FUNC_NUM];

    CMemMgr          *m_pMemMgr;       /* �ڴ����ӿ� */

    CGlobalClock     *m_pClock;        /* ȫ��ϵͳʱ�� */
    CMemPools        *m_pMemPools;     /* ���õ�Block/Trunk�ڴ��, ��ҵ������/�ͷ��ڴ� */
};


#define INIT_EXPORT(FUNC, Level)          \
    CHAR __init_##FUNC##_name[] = #FUNC;  \
    const T_InitFunc __pInit_##FUNC##_ =  \
    {                                     \
        Level,                            \
        __init_##FUNC##_name,             \
        (PINITFUNC) &FUNC,                \
        (PEXITFUNC) NULL,                 \
    };                                    \
    WORD32 __dwInit_##FUNC##_ = CInitList::CreateFunc(&(__pInit_##FUNC##_));


#endif


