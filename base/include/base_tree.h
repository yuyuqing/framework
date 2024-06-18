

#ifndef _BASE_TREE_H_
#define _BASE_TREE_H_


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"


#include "base_util.h"
#include "base_sort.h"
#include "base_numeral_generate.h"


#define BTREE_LEAF_NODE         ((BYTE)(0))
#define BTREE_INTERNAL_NODE     ((BYTE)(1))

#define BTREE_NODE_NUM_ENTRY    ((BYTE)(4))
#define BTREE_MAX_NUM_ENTRY     ((BYTE)((2 * BTREE_NODE_NUM_ENTRY) - 1))

#define BTREE_NODE_NUM_ALIGN    ((WORD32)(128))


/* all compare */
template <typename VT>
using PTreeCmpFunc = SWORD32 (*)(const VT &rLeft, const VT &rRight);


template <typename T, typename VT>
using PTreeDump = VOID (*)(VT *pKey, T *pData);


template <class T>
inline SWORD32 BTreeCompare(T &rLeft, T &rRight)
{
    if (rLeft > rRight)
    {
        return 1;
    }
    else if (rLeft < rRight)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}


/* VT : 构建树的Key */
template<typename T, typename VT, WORD32 POWER_NUM>
class CBTreeTpl
{
public :
    static const WORD32 s_dwNodeNum = (1 << POWER_NUM);
    static const WORD32 s_dwSize    = ROUND_UP((s_dwNodeNum / BTREE_NODE_NUM_ENTRY), BTREE_NODE_NUM_ALIGN);

    class CTreeData : public CBaseData
    {
    public :
        T    m_tData;

        CTreeData () {}
        ~CTreeData() {}

        operator T& ()
        {
            return m_tData;
        }
        
        operator T* ()
        {
            return &m_tData;
        }
    };

    typedef struct tagT_TreeData
    {
        WORD32      dwInstID;
        BOOL        bFree;
        VT          tKey;
        BYTE        aucData[sizeof(CTreeData)];
    }T_TreeData;
    
    typedef struct tagT_TreeNode
    {
        BYTE            ucType;      /* 0 : Leaf; 1 : Internal */
        BYTE            ucNumEntry;  /* 本层节点数量 */

        VT              atKey[BTREE_MAX_NUM_ENTRY];      /* CRNTI or Other */
        
        tagT_TreeNode  *apChild[BTREE_MAX_NUM_ENTRY];    /* Leaf时无效 */
        T_TreeData     *apData[BTREE_MAX_NUM_ENTRY];     /* Leaf时有效 */
    }T_TreeNode;

    typedef CNumeralArray<POWER_NUM>                  CIDGenerator;
    typedef CBaseDataContainer<T_TreeNode, s_dwSize>  CTreeNodeList;

public :
    CBTreeTpl (PTreeCmpFunc<VT> pFunc = &BTreeCompare);
    virtual ~CBTreeTpl();

    WORD32 Initialize();

    /* 根据数组下标(UEID)查询实例 */
    T * FindByInstID(WORD32 dwInstID);

    /* 根据树的Key(CRNTI)值查询实例 */
    T * Find(VT &rKey);

    /* 创建实例(创建成功时, 返回创建实例所处的数组下标) */
    T * Create(WORD32 &rdwInstID, VT &rKey);

    /* 根据数组下标(UEID)删除实例 */
    WORD32 DeleteByInstID(WORD32 dwInstID);

    /* 根据Key(CRNTI)删除实例 */
    WORD32 Delete(VT &rKey);

    /* 根据Key(CRNTI)删除实例, 并获取删除的节点ID */
    WORD32 Delete(VT &rKey, WORD32 &dwInstID);

    VOID Dump(PTreeDump<T, VT> pFunc);

protected :
    WORD32 Clear(T_TreeNode *pNode);

    T_TreeNode * Malloc(BYTE ucType);
    VOID Free(T_TreeNode *pNode);

    /* 当Child满时, 将其进行分裂; wPos : Parent指向Child指针的位置下标(0...) */
    WORD32 SplitChild(T_TreeNode &rParent, WORD16 wPos, T_TreeNode &rChild);

    VOID MergeChild(T_TreeNode &rParent, 
                    WORD16      wPos, 
                    T_TreeNode *pChild1,
                    T_TreeNode *pChild2);

    WORD32 Add(T_TreeNode *pNode, const VT &rKey, T_TreeData &rData);

    WORD32 Delete(T_TreeNode *pNode, const VT &rKey);

    WORD32 Delete(T_TreeNode *pNode, const VT &rKey, WORD32 &dwInstID);

    T * Search(T_TreeNode *pNode, const VT &rKey);
    
    VOID LevelDump(T_TreeNode *pNode, PTreeDump<T, VT> pFunc);

protected :
    PTreeCmpFunc<VT>    m_pCmpFunc;

    CIDGenerator        m_cIDGenerator;
    T_TreeNode         *m_pRoot;
    CTreeNodeList       m_cNodeList;
    T_TreeData          m_atData[s_dwNodeNum];
};


template<typename T, typename VT, WORD32 POWER_NUM>
CBTreeTpl<T, VT, POWER_NUM>::CBTreeTpl (PTreeCmpFunc<VT> pFunc)
{
    m_pCmpFunc = pFunc;
    m_pRoot    = NULL;

    memset(m_atData, 0x00, sizeof(m_atData));
}


template<typename T, typename VT, WORD32 POWER_NUM>
CBTreeTpl<T, VT, POWER_NUM>::~CBTreeTpl()
{
    Clear(m_pRoot);

    m_pCmpFunc = NULL;
    m_pRoot    = NULL;
}


template<typename T, typename VT, WORD32 POWER_NUM>
WORD32 CBTreeTpl<T, VT, POWER_NUM>::Initialize()
{
    m_cIDGenerator.Initialize();
    m_cNodeList.Initialize();

    for (WORD32 dwIndex = 0; dwIndex < s_dwNodeNum; dwIndex++)
    {
        m_atData[dwIndex].dwInstID = dwIndex;
        m_atData[dwIndex].bFree    = TRUE;
    }

    return SUCCESS;
}


/* 根据数组下标(UEID)查询实例 */
template<typename T, typename VT, WORD32 POWER_NUM>
inline T * CBTreeTpl<T, VT, POWER_NUM>::FindByInstID(WORD32 dwInstID)
{
    if (unlikely(dwInstID >= s_dwNodeNum))
    {
        return NULL;
    }

    if (TRUE == m_atData[dwInstID].bFree)
    {
        return NULL;
    }

    CTreeData *pNode = (CTreeData *)(m_atData[dwInstID].aucData);

    return (*pNode);
}


/* 根据树的Key(CRNTI)值查询实例 */
template<typename T, typename VT, WORD32 POWER_NUM>
inline T * CBTreeTpl<T, VT, POWER_NUM>::Find(VT &rKey)
{
    if (unlikely(NULL == m_pRoot))
    {
        return NULL;
    }
    
    return Search(m_pRoot, rKey);
}


template<typename T, typename VT, WORD32 POWER_NUM>
inline T * CBTreeTpl<T, VT, POWER_NUM>::Create(WORD32 &rdwInstID, VT &rKey)
{
    rdwInstID = m_cIDGenerator.Generate();
    if (INVALID_DWORD == rdwInstID)
    {
        return NULL;
    }

    CTreeData  *pData  = NULL;
    T_TreeData *pDNode = &(m_atData[rdwInstID]);
    if (TRUE != pDNode->bFree)
    {
        m_cIDGenerator.Retrieve(rdwInstID);
        return NULL;
    }

    pDNode->dwInstID = rdwInstID;
    pDNode->tKey     = rKey;
    pData            = new (pDNode->aucData) CTreeData();

    if (NULL == m_pRoot)
    {
        m_pRoot = Malloc(BTREE_LEAF_NODE);

        m_pRoot->atKey[0]   = rKey;
        m_pRoot->apData[0]  = pDNode;
        m_pRoot->ucNumEntry = 1;

        return (*pData);
    }

    /* 增加树高处理 */
    if (BTREE_MAX_NUM_ENTRY == m_pRoot->ucNumEntry)
    {
        T_TreeNode *pParent = Malloc(BTREE_INTERNAL_NODE);

        /* 上级节点保存下级节点最大的Key值 */
        pParent->atKey[0]   = m_pRoot->atKey[m_pRoot->ucNumEntry - 1];
        pParent->apChild[0] = m_pRoot;
        pParent->ucNumEntry = 1;

        SplitChild(*pParent, 0, *m_pRoot);
        m_pRoot = pParent;
    }

    Add(m_pRoot, rKey, *pDNode);

    return (*pData);
}


/* 根据数组下标(UEID)删除实例 */
template<typename T, typename VT, WORD32 POWER_NUM>
inline WORD32 CBTreeTpl<T, VT, POWER_NUM>::DeleteByInstID(WORD32 dwInstID)
{
    if (unlikely(dwInstID >= s_dwNodeNum))
    {
        return FAIL;
    }

    T_TreeData *pDNode = &(m_atData[dwInstID]);

    if (TRUE == pDNode->bFree)
    {
        return FAIL;
    }

    return Delete(pDNode->tKey);
}


/* 根据Key(CRNTI)删除实例 */
template<typename T, typename VT, WORD32 POWER_NUM>
inline WORD32 CBTreeTpl<T, VT, POWER_NUM>::Delete(VT &rKey)
{
    if (unlikely(NULL == m_pRoot))
    {
        return FAIL;
    }

    /* 降低树高处理 */
    if ( (2 == m_pRoot->ucNumEntry) 
      && (BTREE_INTERNAL_NODE == m_pRoot->ucType))
    {
        T_TreeNode *pChild1 = m_pRoot->apChild[0];
        T_TreeNode *pChild2 = m_pRoot->apChild[1];

        if ((pChild1->ucNumEntry + pChild2->ucNumEntry) < (BTREE_NODE_NUM_ENTRY - 1))
        {
            /* 将子树2的所有节点平移合并至子树1 */
            MergeChild(*m_pRoot, 0, pChild1, pChild2);
        }
    }

    WORD32 dwResult = Delete(m_pRoot, rKey);

    if (0 == m_pRoot->ucNumEntry)
    {
        Free(m_pRoot);
        m_pRoot = NULL;
    }

    return dwResult;
}


/* 根据Key(CRNTI)删除实例, 并获取删除的节点ID */
template<typename T, typename VT, WORD32 POWER_NUM>
inline WORD32 CBTreeTpl<T, VT, POWER_NUM>::Delete(VT &rKey, WORD32 &rdwInstID)
{
    if (unlikely(NULL == m_pRoot))
    {
        return FAIL;
    }

    /* 降低树高处理 */
    if ( (2 == m_pRoot->ucNumEntry) 
      && (BTREE_INTERNAL_NODE == m_pRoot->ucType))
    {
        T_TreeNode *pChild1 = m_pRoot->apChild[0];
        T_TreeNode *pChild2 = m_pRoot->apChild[1];

        if ((pChild1->ucNumEntry + pChild2->ucNumEntry) < (BTREE_NODE_NUM_ENTRY - 1))
        {
            /* 将子树2的所有节点平移合并至子树1 */
            MergeChild(*m_pRoot, 0, pChild1, pChild2);
        }
    }

    WORD32 dwResult = Delete(m_pRoot, rKey, rdwInstID);

    if (0 == m_pRoot->ucNumEntry)
    {
        Free(m_pRoot);
        m_pRoot = NULL;
    }

    return dwResult;
}


template<typename T, typename VT, WORD32 POWER_NUM>
inline VOID CBTreeTpl<T, VT, POWER_NUM>::Dump(PTreeDump<T, VT> pFunc)
{
    if (unlikely(NULL == m_pRoot))
    {
        return ;
    }

    LevelDump(m_pRoot, pFunc);
}


template<typename T, typename VT, WORD32 POWER_NUM>
WORD32 CBTreeTpl<T, VT, POWER_NUM>::Clear(
    typename CBTreeTpl<T, VT, POWER_NUM>::T_TreeNode *pNode)
{
    if (NULL == pNode)
    {
        return SUCCESS;
    }

    if (BTREE_INTERNAL_NODE == pNode->ucType)
    {
        for (WORD32 dwIndex = 0; dwIndex < pNode->ucNumEntry; dwIndex++)
        {
            Clear(pNode->apChild[dwIndex]);
        }
    }

    Free(pNode);
    
    return SUCCESS;
}


/* 分配必然不会失败, 如出现失败, 则强制跑死 */
template<typename T, typename VT, WORD32 POWER_NUM>
inline typename CBTreeTpl<T, VT, POWER_NUM>::T_TreeNode * 
CBTreeTpl<T, VT, POWER_NUM>::Malloc(BYTE ucType)
{
    T_TreeNode *pNode = m_cNodeList.Malloc();

    /* 分配必然不会失败, 如出现失败, 则强制跑死 */
    memset(pNode, 0x00, sizeof(T_TreeNode));
    
    pNode->ucType = ucType;

    return pNode;
}


template<typename T, typename VT, WORD32 POWER_NUM>
inline VOID CBTreeTpl<T, VT, POWER_NUM>::Free(
    typename CBTreeTpl<T, VT, POWER_NUM>::T_TreeNode *pNode)
{
    m_cNodeList.Free(pNode);
}


/* 当Child满时, 将其进行分裂; wPos : Parent指向Child指针的位置下标(0...) */
template<typename T, typename VT, WORD32 POWER_NUM>
inline WORD32 CBTreeTpl<T, VT, POWER_NUM>::SplitChild(
    typename CBTreeTpl<T, VT, POWER_NUM>::T_TreeNode &rParent, WORD16 wPos, 
    typename CBTreeTpl<T, VT, POWER_NUM>::T_TreeNode &rChild)
{
    T_TreeNode *pNewChild = Malloc(rChild.ucType);

    pNewChild->ucNumEntry = BTREE_NODE_NUM_ENTRY - 1;

    /* 新创建的子树, 用于存放后半部分子树节点 */
    for (WORD32 dwIndex = 0; dwIndex < pNewChild->ucNumEntry; dwIndex++)
    {
        pNewChild->atKey[dwIndex] = rChild.atKey[dwIndex + BTREE_NODE_NUM_ENTRY];

        if (BTREE_LEAF_NODE == rChild.ucType)
        {
            pNewChild->apData[dwIndex] = rChild.apData[dwIndex + BTREE_NODE_NUM_ENTRY];
        }
        else
        {
            pNewChild->apChild[dwIndex] = rChild.apChild[dwIndex + BTREE_NODE_NUM_ENTRY];
        }
    }

    /* 原子树被截断, 仅保留前半部分子树节点 */
    rChild.ucNumEntry = BTREE_NODE_NUM_ENTRY;

    /* 父节点所指向的子树节点所对应的Key值往后挪, 用于空出位置存放较小的Key值 */
    for (WORD32 dwIndex = rParent.ucNumEntry; dwIndex > wPos; dwIndex--)
    {
        rParent.atKey[dwIndex] = rParent.atKey[dwIndex - 1];

        if (dwIndex > (wPos + 1))
        {
            rParent.apChild[dwIndex] = rParent.apChild[dwIndex - 1];
        }
    }

    /* 较小的Key值(仍然是对应子树的最大Key)存放在wPos位置上, 较大的新增子树挂载wPos+1位置上 */
    rParent.atKey[wPos]       = rChild.atKey[rChild.ucNumEntry - 1];
    rParent.apChild[wPos + 1] = pNewChild;
    rParent.ucNumEntry++;

    return SUCCESS;
}


template<typename T, typename VT, WORD32 POWER_NUM>
VOID CBTreeTpl<T, VT, POWER_NUM>::MergeChild(
    typename CBTreeTpl<T, VT, POWER_NUM>::T_TreeNode &rParent, 
    WORD16      wPos, 
    typename CBTreeTpl<T, VT, POWER_NUM>::T_TreeNode *pChild1,
    typename CBTreeTpl<T, VT, POWER_NUM>::T_TreeNode *pChild2)
{
    WORD32 dwPos = pChild1->ucNumEntry;

    /* 将子树2的所有节点平移给子树1 */
    pChild1->ucNumEntry += pChild2->ucNumEntry;

    for (WORD32 dwIndex = dwPos; 
         dwIndex < pChild1->ucNumEntry;
         dwIndex++)
    {
        pChild1->atKey[dwIndex] = pChild2->atKey[dwIndex - dwPos];

        if (BTREE_LEAF_NODE == pChild1->ucType)
        {
            pChild1->apData[dwIndex] = pChild2->apData[dwIndex - dwPos];
        }
        else
        {
            pChild1->apChild[dwIndex] = pChild2->apChild[dwIndex - dwPos];
        }
    }

    /* 由于将wPos+1位置处的子树2, 合并至wPos位置处的子树1; 需要将合并后的空洞抹除 */
    for (WORD32 dwIndex = (wPos + 1); 
         dwIndex < rParent.ucNumEntry; 
         dwIndex++)
    {
        rParent.atKey[dwIndex - 1] = rParent.atKey[dwIndex];

        if ((dwIndex + 1) < rParent.ucNumEntry)
        {
            rParent.apChild[dwIndex]  = rParent.apChild[dwIndex + 1];
        }
    }

    rParent.apChild[rParent.ucNumEntry - 1] = NULL;
    rParent.ucNumEntry--;

    /* 释放被合并的节点 */
    Free(pChild2);
}


template<typename T, typename VT, WORD32 POWER_NUM>
WORD32 CBTreeTpl<T, VT, POWER_NUM>::Add(
    typename CBTreeTpl<T, VT, POWER_NUM>::T_TreeNode *pNode, 
    const VT &rKey, 
    typename CBTreeTpl<T, VT, POWER_NUM>::T_TreeData &rData)
{
    SWORD16 swPos     = 0;
    SWORD32 swCompare = 0;
    
    if (BTREE_LEAF_NODE == pNode->ucType)    /* 叶子节点 */
    {
        swPos = pNode->ucNumEntry;
        
        while (swPos > 0)
        {
            swCompare = (*m_pCmpFunc)(rKey, pNode->atKey[swPos - 1]);
            if (swCompare < 0)
            {
                /* 按升序排列, 往后挪 */
                pNode->atKey[swPos]  = pNode->atKey[swPos - 1];
                pNode->apData[swPos] = pNode->apData[swPos - 1];
                swPos--;
            }
            else
            {
                /* 找到插入位置, 跳出循环 */
                break ;
            }
        }
    
        pNode->atKey[swPos]  = rKey;
        pNode->apData[swPos] = &rData;
        pNode->ucNumEntry++;
    
        return SUCCESS;
    }
    else    /* 非叶子节点 */
    {
        swPos = pNode->ucNumEntry - 1;

        /* 按升序原则寻找应该插入的子树节点 */
        while (swPos >= 0)
        {
            swCompare = (*m_pCmpFunc)(rKey, pNode->atKey[swPos]);
            if ((swCompare < 0) && (swPos > 0))
            {
                swPos--;
            }
            else
            {
                break ;
            }
        }

        if (swCompare > 0)
        {
            if (swPos < (pNode->ucNumEntry - 1))
            {
                swPos++;
            }
            else
            {
                /* 需要修改子树节点的Key边界, 即待插入节点Key在该子树下最大 */
                pNode->atKey[swPos] = rKey;
            }            
        }

        /* 寻找到待插入子树已经满, 需要增加该子树的深度 */
        if (BTREE_MAX_NUM_ENTRY == pNode->apChild[swPos]->ucNumEntry)
        {
            SplitChild(*pNode, swPos, *(pNode->apChild[swPos]));      
                        
            swCompare = (*m_pCmpFunc)(rKey, pNode->atKey[swPos]);
            if (swCompare > 0)
            {
                swPos++;
            }
        }

        /* 递归子树插入节点 */
        return Add(pNode->apChild[swPos], rKey, rData);
    }        
}


template<typename T, typename VT, WORD32 POWER_NUM>
WORD32 CBTreeTpl<T, VT, POWER_NUM>::Delete(
    typename CBTreeTpl<T, VT, POWER_NUM>::T_TreeNode *pNode, const VT &rKey)
{
    SWORD16 swPos     = 0;
    SWORD32 swCompare = 0;
    WORD32  dwResult  = INVALID_DWORD;

    CTreeData *pData  = NULL;

    if (BTREE_LEAF_NODE == pNode->ucType)
    {
        while (swPos < pNode->ucNumEntry)
        {
            swCompare = (*m_pCmpFunc)(rKey, pNode->atKey[swPos]);
            if (swCompare > 0)
            {
                swPos++;
            }
            else if (swCompare == 0)
            {
                m_cIDGenerator.Retrieve(pNode->apData[swPos]->dwInstID);

                /* 找到待删除数据, 析构对象并修改标志位为释放状态 */
                pData = (CTreeData *)(pNode->apData[swPos]->aucData);
                delete pData;
                pNode->apData[swPos]->bFree = TRUE;

                /* 将待删除节点从树中抹除 */
                for (WORD32 dwIndex = (swPos + 1); 
                     dwIndex < pNode->ucNumEntry;
                     dwIndex++)
                {
                    pNode->atKey[dwIndex - 1]  = pNode->atKey[dwIndex];
                    pNode->apData[dwIndex - 1] = pNode->apData[dwIndex];
                }

                pNode->ucNumEntry--;

                dwResult = SUCCESS;
            }
            else
            {
                break ;
            }
        }

        return dwResult;
    }
    else
    {
        T_TreeNode *pChild1 = NULL;
        T_TreeNode *pChild2 = NULL;

        /* 遍历子树, 寻找待删除节点所在子树 */
        while (swPos < pNode->ucNumEntry)
        {
            swCompare = (*m_pCmpFunc)(rKey, pNode->atKey[swPos]);
            if (swCompare > 0)
            {
                swPos++;
            }
            else
            {
                break ;
            }
        }

        /* 遍历所有子树, 没有找到相应子树 */
        if (swPos == pNode->ucNumEntry)
        {
            return FAIL;
        }

        pChild1 = pNode->apChild[swPos];

        /* 递归子树寻找待删除节点 */
        dwResult = Delete(pChild1, rKey);

        /* 删除数据节点之后, 需要对整个树进行管理维护 */
        if ((swPos + 1) < pNode->ucNumEntry)
        {
            pChild2 = pNode->apChild[swPos + 1];

            /* 判断是否需要合并部分子树(针对被删除节点的子树) */
            if ( (0 == pChild1->ucNumEntry)
              || ( (swCompare == 0)
                && ((BTREE_MAX_NUM_ENTRY - 1) > (pChild1->ucNumEntry + pChild2->ucNumEntry))))
            {
                MergeChild(*pNode, swPos, pChild1, pChild2);
                return dwResult;
            }
            else
            {
                if (swCompare == 0)
                {
                    /* 由于删除了pNode子树下的节点, 更新该子树最大Key值 */
                    pNode->atKey[swPos] = pChild1->atKey[pChild1->ucNumEntry - 1];
                }

                return dwResult;
            }
        }
        else
        {
            /* 最后一个分支 */
            /* 最后一个分支的节点数量为0 */
            if (0 == pChild1->ucNumEntry)
            {
                pNode->ucNumEntry--;
                pNode->apChild[pNode->ucNumEntry] = NULL;

                Free(pChild1);

                return dwResult;
            }
            else
            {
                if (swCompare == 0)
                {
                    /* 由于删除了pNode子树下的节点, 更新该子树最大Key值 */
                    pNode->atKey[swPos] = pChild1->atKey[pChild1->ucNumEntry - 1];
                }

                return dwResult;
            }                
        }
    }
}


template<typename T, typename VT, WORD32 POWER_NUM>
WORD32 CBTreeTpl<T, VT, POWER_NUM>::Delete(
    typename CBTreeTpl<T, VT, POWER_NUM>::T_TreeNode *pNode,
    const VT &rKey,
    WORD32   &rdwInstID)
{
    SWORD16 swPos     = 0;
    SWORD32 swCompare = 0;
    WORD32  dwResult  = INVALID_DWORD;

    CTreeData *pData  = NULL;

    if (BTREE_LEAF_NODE == pNode->ucType)
    {
        while (swPos < pNode->ucNumEntry)
        {
            swCompare = (*m_pCmpFunc)(rKey, pNode->atKey[swPos]);
            if (swCompare > 0)
            {
                swPos++;
            }
            else if (swCompare == 0)
            {
                rdwInstID = pNode->apData[swPos]->dwInstID;
                m_cIDGenerator.Retrieve(rdwInstID);

                /* 找到待删除数据, 析构对象并修改标志位为释放状态 */
                pData = (CTreeData *)(pNode->apData[swPos]->aucData);
                delete pData;
                pNode->apData[swPos]->bFree = TRUE;

                /* 将待删除节点从树中抹除 */
                for (WORD32 dwIndex = (swPos + 1); 
                     dwIndex < pNode->ucNumEntry;
                     dwIndex++)
                {
                    pNode->atKey[dwIndex - 1]  = pNode->atKey[dwIndex];
                    pNode->apData[dwIndex - 1] = pNode->apData[dwIndex];
                }

                pNode->ucNumEntry--;

                dwResult = SUCCESS;
            }
            else
            {
                break ;
            }
        }

        return dwResult;
    }
    else
    {
        T_TreeNode *pChild1 = NULL;
        T_TreeNode *pChild2 = NULL;

        /* 遍历子树, 寻找待删除节点所在子树 */
        while (swPos < pNode->ucNumEntry)
        {
            swCompare = (*m_pCmpFunc)(rKey, pNode->atKey[swPos]);
            if (swCompare > 0)
            {
                swPos++;
            }
            else
            {
                break ;
            }
        }

        /* 遍历所有子树, 没有找到相应子树 */
        if (swPos == pNode->ucNumEntry)
        {
            return FAIL;
        }

        pChild1 = pNode->apChild[swPos];

        /* 递归子树寻找待删除节点 */
        dwResult = Delete(pChild1, rKey, rdwInstID);

        /* 删除数据节点之后, 需要对整个树进行管理维护 */
        if ((swPos + 1) < pNode->ucNumEntry)
        {
            pChild2 = pNode->apChild[swPos + 1];

            /* 判断是否需要合并部分子树(针对被删除节点的子树) */
            if ( (0 == pChild1->ucNumEntry)
              || ( (swCompare == 0)
                && ((BTREE_MAX_NUM_ENTRY - 1) > (pChild1->ucNumEntry + pChild2->ucNumEntry))))
            {
                MergeChild(*pNode, swPos, pChild1, pChild2);
                return dwResult;
            }
            else
            {
                if (swCompare == 0)
                {
                    /* 由于删除了pNode子树下的节点, 更新该子树最大Key值 */
                    pNode->atKey[swPos] = pChild1->atKey[pChild1->ucNumEntry - 1];
                }

                return dwResult;
            }
        }
        else
        {
            /* 最后一个分支 */
            /* 最后一个分支的节点数量为0 */
            if (0 == pChild1->ucNumEntry)
            {
                pNode->ucNumEntry--;
                pNode->apChild[pNode->ucNumEntry] = NULL;

                Free(pChild1);

                return dwResult;
            }
            else
            {
                if (swCompare == 0)
                {
                    /* 由于删除了pNode子树下的节点, 更新该子树最大Key值 */
                    pNode->atKey[swPos] = pChild1->atKey[pChild1->ucNumEntry - 1];
                }

                return dwResult;
            }                
        }
    }
}


template<typename T, typename VT, WORD32 POWER_NUM>
T * CBTreeTpl<T, VT, POWER_NUM>::Search(
    typename CBTreeTpl<T, VT, POWER_NUM>::T_TreeNode *pNode, const VT &rKey)
{
    SWORD16 swPos     = 0;
    SWORD32 swCompare = 0;

    if (BTREE_LEAF_NODE == pNode->ucType)
    {
        while (swPos < pNode->ucNumEntry)
        {
            swCompare = (*m_pCmpFunc)(rKey, pNode->atKey[swPos]);
            if (swCompare == 0)
            {
                return *((CTreeData *)(pNode->apData[swPos]->aucData));
            }
            else if (swCompare < 0)
            {
                break ;
            }
            else
            {
                swPos++;
            }
        }

        return NULL;
    }
    else
    {
        while (swPos < pNode->ucNumEntry)
        {
            swCompare = (*m_pCmpFunc)(rKey, pNode->atKey[swPos]);
            if (swCompare <= 0)
            {
                return Search(pNode->apChild[swPos], rKey);
            }

            swPos++;
        }

        return NULL;
    }    
}


template<typename T, typename VT, WORD32 POWER_NUM>
VOID CBTreeTpl<T, VT, POWER_NUM>::LevelDump(
    typename CBTreeTpl<T, VT, POWER_NUM>::T_TreeNode *pNode, 
    PTreeDump<T, VT> pFunc)
{
    if (BTREE_LEAF_NODE == pNode->ucType)
    {
        for (WORD32 dwIndex = 0; dwIndex < pNode->ucNumEntry; dwIndex++)
        {
            (*pFunc)(&(pNode->atKey[dwIndex]), 
                     (T *)(*((CTreeData *)(pNode->apData[dwIndex]->aucData))));
        }
    }
    else
    {
        for (WORD32 dwIndex = 0; dwIndex < pNode->ucNumEntry; dwIndex++)
        {
            LevelDump(pNode->apChild[dwIndex], pFunc);
        }
    }
}


#pragma GCC diagnostic pop


#endif


