

#ifndef _BASE_TREE_ARRAY_H_
#define _BASE_TREE_ARRAY_H_


#include "base_tree.h"
#include "base_numeral_generate.h"


template<typename T, typename VT, WORD32 POWER_NUM>
class CBTreeArray
{
public :
    static const WORD32 s_dwNodeNum = (1 << POWER_NUM);

    typedef CNumeralArray<POWER_NUM>         CIDGenerator;
    typedef CBTreeTpl<T, VT, s_dwNodeNum>    CBTree;

public :
    CBTreeArray ();
    virtual ~CBTreeArray();

    WORD32 Initialize();

    T * FindByInstID(WORD32 dwID);

    T * Find(VT &rKey);

    T * Create(VT &rKey, WORD32 &rdwID);

    WORD32 RemoveByInstID(WORD32 dwID);

    WORD32 Remove(VT &rKey);

    VOID Dump(PTreeDump<T, VT> pFunc);

protected :
    CIDGenerator  m_cIDGenerator;
    CBTree        m_cTree;
};


template<typename T, typename VT, WORD32 POWER_NUM>
CBTreeArray<T, VT, POWER_NUM>::CBTreeArray ()
{
}


template<typename T, typename VT, WORD32 POWER_NUM>
CBTreeArray<T, VT, POWER_NUM>::~CBTreeArray()
{
}


template<typename T, typename VT, WORD32 POWER_NUM>
WORD32 CBTreeArray<T, VT, POWER_NUM>::Initialize()
{
    m_cIDGenerator.Initialize();
    m_cTree.Initialize();

    return SUCCESS;
}


template<typename T, typename VT, WORD32 POWER_NUM>
inline T * CBTreeArray<T, VT, POWER_NUM>::FindByInstID(WORD32 dwID)
{
    return m_cTree.FindByInstID(dwID);
}


template<typename T, typename VT, WORD32 POWER_NUM>
inline T * CBTreeArray<T, VT, POWER_NUM>::Find(VT &rKey)
{
    return m_cTree.Find(rKey);
}


template<typename T, typename VT, WORD32 POWER_NUM>
inline T * CBTreeArray<T, VT, POWER_NUM>::Create(VT &rKey, WORD32 &rdwID)
{
    WORD32 dwID = m_cIDGenerator.Generate();
    if (INVALID_DWORD == dwID)
    {
        return NULL;
    }

    T *pNode = m_cTree.Create(dwID, rKey);
    if (NULL == pNode)
    {
        m_cIDGenerator.Retrieve(dwID);
        return NULL;
    }

    rdwID = dwID;

    return pNode;
}


template<typename T, typename VT, WORD32 POWER_NUM>
inline WORD32 CBTreeArray<T, VT, POWER_NUM>::RemoveByInstID(WORD32 dwID)
{
    WORD32 dwResult = m_cTree.DeleteByInstID(dwID);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    dwResult = m_cIDGenerator.Retrieve(dwID);
    if (SUCCESS != dwResult)
    {
        assert(0);
    }

    return SUCCESS;
}


template<typename T, typename VT, WORD32 POWER_NUM>
inline WORD32 CBTreeArray<T, VT, POWER_NUM>::Remove(VT &rKey)
{
    WORD32 dwID     = INVALID_DWORD;
    WORD32 dwResult = m_cTree.Delete(rKey, dwID);
    if (SUCCESS != dwResult)
    {
        return FAIL;
    }

    dwResult = m_cIDGenerator.Retrieve(dwID);
    if (SUCCESS != dwResult)
    {
        assert(0);
    }

    return SUCCESS;
}


template<typename T, typename VT, WORD32 POWER_NUM>
inline VOID CBTreeArray<T, VT, POWER_NUM>::Dump(PTreeDump<T, VT> pFunc)
{
    m_cTree.Dump(pFunc);
}


#endif


