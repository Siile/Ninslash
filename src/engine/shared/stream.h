
#pragma once

#include <base/system.h>

class CInputStream
{
public:
	virtual ~CInputStream() {}

	virtual unsigned Read(void *pBuffer, unsigned Size) = 0;
};

class CIOStream : public CInputStream
{
	IOHANDLE m_File;

public:
	CIOStream(IOHANDLE File) { m_File = File; }
	virtual ~CIOStream() {}

	virtual unsigned Read(void *pBuffer, unsigned Size) { return io_read(m_File, pBuffer, Size); }
};

class CStringStream : public CInputStream
{
	const char *m_pStr;
	unsigned m_Cur;

public:
	CStringStream(const char *pStr);
	virtual ~CStringStream();

	virtual unsigned Read(void *pBuffer, unsigned Size);
};
