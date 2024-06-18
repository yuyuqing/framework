

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


/* VT : ��������Key */
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
        BYTE            ucNumEntry;  /* ����ڵ����� */

        VT              atKey[BTREE_MAX_NUM_ENTRY];      /* CRNTI or Other */
        
        tagT_TreeNode  *apChild[BTREE_MAX_NUM_ENTRY];    /* Leafʱ��Ч */
        T_TreeData     *apData[BTREE_MAX_NUM_ENTRY];     /* Leafʱ��Ч */
    }T_TreeNode;

    typedef CNumeralArray<POWER_NUM>                  CIDGenerator;
    typedef CBaseDataContainer<T_TreeNode, s_dwSize>  CTreeNodeList;

public :
    CBTreeTpl (PTreeCmpFunc<VT> pFunc = &BTreeCompare);
    virtual ~CBTreeTpl();

    WORD32 Initialize();

    /* ���������±�(UEID)��ѯʵ�� */
    T * FindByInstID(WORD32 dwInstID);

    /* ��������Key(CRNTI)ֵ��ѯʵ�� */
    T * Find(VT &rKey);

    /* ����ʵ��(�����ɹ�ʱ, ���ش���ʵ�������������±�) */
    T * Create(WORD32 &rdwInstID, VT &rKey);

    /* ���������±�(UEID)ɾ��ʵ�� */
    WORD32 DeleteByInstID(WORD32 dwInstID);

    /* ����Key(CRNTI)ɾ��ʵ�� */
    WORD32 Delete(VT &rKey);

    /* ����Key(CRNTI)ɾ��ʵ��, ����ȡɾ���Ľڵ�ID */
    WORD32 Delete(VT &rKey, WORD32 &dwInstID);

    VOID Dump(PTreeDump<T, VT> pFunc);

protected :
    WORD32 Clear(T_TreeNode *pNode);

    T_TreeNode * Malloc(BYTE ucType);
    VOID Free(T_TreeNode *pNode);

    /* ��Child��ʱ, ������з���; wPos : Parentָ��Childָ���λ���±�(0...) */
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


/* ���������±�(UEID)��ѯʵ�� */
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


/* ��������Key(CRNTI)ֵ��ѯʵ�� */
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

    /* �������ߴ��� */
    if (BTREE_MAX_NUM_ENTRY == m_pRoot->ucNumEntry)
    {
        T_TreeNode *pParent = Malloc(BTREE_INTERNAL_NODE);

        /* �ϼ��ڵ㱣���¼��ڵ�����Keyֵ */
        pParent->atKey[0]   = m_pRoot->atKey[m_pRoot->ucNumEntry - 1];
        pParent->apChild[0] = m_pRoot;
        pParent->ucNumEntry = 1;

        SplitChild(*pParent, 0, *m_pRoot);
        m_pRoot = pParent;
    }

    Add(m_pRoot, rKey, *pDNode);

    return (*pData);
}


/* ���������±�(UEID)ɾ��ʵ�� */
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


/* ����Key(CRNTI)ɾ��ʵ�� */
template<typename T, typename VT, WORD32 POWER_NUM>
inline WORD32 CBTreeTpl<T, VT, POWER_NUM>::Delete(VT &rKey)
{
    if (unlikely(NULL == m_pRoot))
    {
        return FAIL;
    }

    /* �������ߴ��� */
    if ( (2 == m_pRoot->ucNumEntry) 
      && (BTREE_INTERNAL_NODE == m_pRoot->ucType))
    {
        T_TreeNode *pChild1 = m_pRoot->apChild[0];
        T_TreeNode *pChild2 = m_pRoot->apChild[1];

        if ((pChild1->ucNumEntry + pChild2->ucNumEntry) < (BTREE_NODE_NUM_ENTRY - 1))
        {
            /* ������2�����нڵ�ƽ�ƺϲ�������1 */
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


/* ����Key(CRNTI)ɾ��ʵ��, ����ȡɾ���Ľڵ�ID */
template<typename T, typename VT, WORD32 POWER_NUM>
inline WORD32 CBTreeTpl<T, VT, POWER_NUM>::Delete(VT &rKey, WORD32 &rdwInstID)
{
    if (unlikely(NULL == m_pRoot))
    {
        return FAIL;
    }

    /* �������ߴ��� */
    if ( (2 == m_pRoot->ucNumEntry) 
      && (BTREE_INTERNAL_NODE == m_pRoot->ucType))
    {
        T_TreeNode *pChild1 = m_pRoot->apChild[0];
        T_TreeNode *pChild2 = m_pRoot->apChild[1];

        if ((pChild1->ucNumEntry + pChild2->ucNumEntry) < (BTREE_NODE_NUM_ENTRY - 1))
        {
            /* ������2�����нڵ�ƽ�ƺϲ�������1 */
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


/* �����Ȼ����ʧ��, �����ʧ��, ��ǿ������ */
template<typename T, typename VT, WORD32 POWER_NUM>
inline typename CBTreeTpl<T, VT, POWER_NUM>::T_TreeNode * 
CBTreeTpl<T, VT, POWER_NUM>::Malloc(BYTE ucType)
{
    T_TreeNode *pNode = m_cNodeList.Malloc();

    /* �����Ȼ����ʧ��, �����ʧ��, ��ǿ������ */
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


/* ��Child��ʱ, ������з���; wPos : Parentָ��Childָ���λ���±�(0...) */
template<typename T, typename VT, WORD32 POWER_NUM>
inline WORD32 CBTreeTpl<T, VT, POWER_NUM>::SplitChild(
    typename CBTreeTpl<T, VT, POWER_NUM>::T_TreeNode &rParent, WORD16 wPos, 
    typename CBTreeTpl<T, VT, POWER_NUM>::T_TreeNode &rChild)
{
    T_TreeNode *pNewChild = Malloc(rChild.ucType);

    pNewChild->ucNumEntry = BTREE_NODE_NUM_ENTRY - 1;

    /* �´���������, ���ڴ�ź�벿�������ڵ� */
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

    /* ԭ�������ض�, ������ǰ�벿�������ڵ� */
    rChild.ucNumEntry = BTREE_NODE_NUM_ENTRY;

    /* ���ڵ���ָ��������ڵ�����Ӧ��Keyֵ����Ų, ���ڿճ�λ�ô�Ž�С��Keyֵ */
    for (WORD32 dwIndex = rParent.ucNumEntry; dwIndex > wPos; dwIndex--)
    {
        rParent.atKey[dwIndex] = rParent.atKey[dwIndex - 1];

        if (dwIndex > (wPos + 1))
        {
            rParent.apChild[dwIndex] = rParent.apChild[dwIndex - 1];
        }
    }

    /* ��С��Keyֵ(��Ȼ�Ƕ�Ӧ���������Key)�����wPosλ����, �ϴ��������������wPos+1λ���� */
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

    /* ������2�����нڵ�ƽ�Ƹ�����1 */
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

    /* ���ڽ�wPos+1λ�ô�������2, �ϲ���wPosλ�ô�������1; ��Ҫ���ϲ���Ŀն�Ĩ�� */
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

    /* �ͷű��ϲ��Ľڵ� */
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
    
    if (BTREE_LEAF_NODE == pNode->ucType)    /* Ҷ�ӽڵ� */
    {
        swPos = pNode->ucNumEntry;
        
        while (swPos > 0)
        {
            swCompare = (*m_pCmpFunc)(rKey, pNode->atKey[swPos - 1]);
            if (swCompare < 0)
            {
                /* ����������, ����Ų */
                pNode->atKey[swPos]  = pNode->atKey[swPos - 1];
                pNode->apData[swPos] = pNode->apData[swPos - 1];
                swPos--;
            }
            else
            {
                /* �ҵ�����λ��, ����ѭ�� */
                break ;
            }
        }
    
        pNode->atKey[swPos]  = rKey;
        pNode->apData[swPos] = &rData;
        pNode->ucNumEntry++;
    
        return SUCCESS;
    }
    else    /* ��Ҷ�ӽڵ� */
    {
        swPos = pNode->ucNumEntry - 1;

        /* ������ԭ��Ѱ��Ӧ�ò���������ڵ� */
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
                /* ��Ҫ�޸������ڵ��Key�߽�, ��������ڵ�Key�ڸ���������� */
                pNode->atKey[swPos] = rKey;
            }            
        }

        /* Ѱ�ҵ������������Ѿ���, ��Ҫ���Ӹ���������� */
        if (BTREE_MAX_NUM_ENTRY == pNode->apChild[swPos]->ucNumEntry)
        {
            SplitChild(*pNode, swPos, *(pNode->apChild[swPos]));      
                        
            swCompare = (*m_pCmpFunc)(rKey, pNode->atKey[swPos]);
            if (swCompare > 0)
            {
                swPos++;
            }
        }

        /* �ݹ���������ڵ� */
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

                /* �ҵ���ɾ������, ���������޸ı�־λΪ�ͷ�״̬ */
                pData = (CTreeData *)(pNode->apData[swPos]->aucData);
                delete pData;
                pNode->apData[swPos]->bFree = TRUE;

                /* ����ɾ���ڵ������Ĩ�� */
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

        /* ��������, Ѱ�Ҵ�ɾ���ڵ��������� */
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

        /* ������������, û���ҵ���Ӧ���� */
        if (swPos == pNode->ucNumEntry)
        {
            return FAIL;
        }

        pChild1 = pNode->apChild[swPos];

        /* �ݹ�����Ѱ�Ҵ�ɾ���ڵ� */
        dwResult = Delete(pChild1, rKey);

        /* ɾ�����ݽڵ�֮��, ��Ҫ�����������й���ά�� */
        if ((swPos + 1) < pNode->ucNumEntry)
        {
            pChild2 = pNode->apChild[swPos + 1];

            /* �ж��Ƿ���Ҫ�ϲ���������(��Ա�ɾ���ڵ������) */
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
                    /* ����ɾ����pNode�����µĽڵ�, ���¸��������Keyֵ */
                    pNode->atKey[swPos] = pChild1->atKey[pChild1->ucNumEntry - 1];
                }

                return dwResult;
            }
        }
        else
        {
            /* ���һ����֧ */
            /* ���һ����֧�Ľڵ�����Ϊ0 */
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
                    /* ����ɾ����pNode�����µĽڵ�, ���¸��������Keyֵ */
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

                /* �ҵ���ɾ������, ���������޸ı�־λΪ�ͷ�״̬ */
                pData = (CTreeData *)(pNode->apData[swPos]->aucData);
                delete pData;
                pNode->apData[swPos]->bFree = TRUE;

                /* ����ɾ���ڵ������Ĩ�� */
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

        /* ��������, Ѱ�Ҵ�ɾ���ڵ��������� */
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

        /* ������������, û���ҵ���Ӧ���� */
        if (swPos == pNode->ucNumEntry)
        {
            return FAIL;
        }

        pChild1 = pNode->apChild[swPos];

        /* �ݹ�����Ѱ�Ҵ�ɾ���ڵ� */
        dwResult = Delete(pChild1, rKey, rdwInstID);

        /* ɾ�����ݽڵ�֮��, ��Ҫ�����������й���ά�� */
        if ((swPos + 1) < pNode->ucNumEntry)
        {
            pChild2 = pNode->apChild[swPos + 1];

            /* �ж��Ƿ���Ҫ�ϲ���������(��Ա�ɾ���ڵ������) */
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
                    /* ����ɾ����pNode�����µĽڵ�, ���¸��������Keyֵ */
                    pNode->atKey[swPos] = pChild1->atKey[pChild1->ucNumEntry - 1];
                }

                return dwResult;
            }
        }
        else
        {
            /* ���һ����֧ */
            /* ���һ����֧�Ľڵ�����Ϊ0 */
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
                    /* ����ɾ����pNode�����µĽڵ�, ���¸��������Keyֵ */
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


