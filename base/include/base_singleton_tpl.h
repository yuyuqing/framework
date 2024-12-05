

#ifndef _BASE_SINGLETON_TPL_H_
#define _BASE_SINGLETON_TPL_H_


#include "pub_typedef.h"


template <typename T>
class CSingleton 
{
public :
    static T * GetInstance()
    { 
        if (NULL != s_pInstance)
        {
            return s_pInstance;
        }

        s_pInstance = new T();

        return s_pInstance;
    }

    static T * GetInstance(BYTE *pMem)
    {
        if (NULL != s_pInstance)
        {
            return s_pInstance;
        }

        s_pInstance = new (pMem) T();

        return s_pInstance;
    }

    static VOID Destroy()
    {
        if (NULL != s_pInstance)
        {
            delete s_pInstance;
        }

        s_pInstance = NULL;
    }

protected :
    CSingleton () {}
    virtual ~CSingleton() {}

protected :
    CSingleton (CSingleton&) = delete;
    CSingleton (const CSingleton&) = delete;
    CSingleton& operator= (const CSingleton&) = delete;

private :
    static T *s_pInstance;
};


template <typename T>
T * CSingleton<T>::s_pInstance = NULL;


#endif


