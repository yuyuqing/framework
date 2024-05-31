

#include <string.h>

#include "pub_global_def.h"

#include "base_bitmap.h"
#include "base_log.h"


VOID CBaseBitMap02::Dump(const CHAR *pName)
{
    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "%s : "
               PRINTF_BINARY_16 "\n",
               pName,
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[1]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[0]));
}


VOID CBaseBitMap32::Dump(const CHAR *pName)
{
    LOG_VPRINT(E_BASE_FRAMEWORK, 0xFFFF, E_LOG_LEVEL_TRACE, TRUE,
               "%s : "
               PRINTF_BINARY_256 "\n",
               pName,
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[31]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[30]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[29]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[28]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[27]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[26]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[25]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[24]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[23]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[22]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[21]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[20]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[19]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[18]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[17]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[16]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[15]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[14]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[13]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[12]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[11]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[10]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[9]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[8]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[7]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[6]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[5]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[4]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[3]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[2]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[1]),
               PRINTF_BYTE_TO_BINARY(m_aucBitMap[0]));
}


