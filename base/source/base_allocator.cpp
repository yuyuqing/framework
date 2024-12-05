

#include "base_allocator.h"


BYTE * CAllocInterface::Malloc(WORD32 dwSize)
{
    return NULL;
}


BYTE * CAllocInterface::Malloc(WORD32 dwSize, WORD32 dwIndex)
{
    return this->Malloc(dwSize);
}


BYTE * CAllocInterface::Malloc(WORD32 dwSize, WORD32 dwPoolID, WORD32 dwPoint)
{
    return this->Malloc(dwSize);
}


WORD32 CAllocInterface::Free(VOID *pAddr)
{
    return FAIL;
}


