
#include <base/math.h> // max

#include "stream.h"

CStringStream::CStringStream(const char *pStr)
{
	m_pStr = pStr;
	m_Cur = 0;
}

CStringStream::~CStringStream()
{
	m_pStr = 0x0;
}

unsigned CStringStream::Read(void *pBuffer, unsigned Size)
{
	dbg_assert(m_pStr != 0x0, "Missing string");
	dbg_assert(m_Cur >= 0, "Out of range");

	unsigned Len = str_length(m_pStr);

	if(m_Cur >= Len+1)
		return 0;

	unsigned DataSize = min(Size, Len+1-m_Cur);

	if(DataSize <= 0)
		return 0;

	mem_copy(pBuffer, m_pStr, DataSize);

	m_Cur += DataSize;
	return DataSize;
}
