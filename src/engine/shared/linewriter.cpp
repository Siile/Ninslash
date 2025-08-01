#include "linewriter.h"

CLineWriter::CLineWriter(IOHANDLE IO)
{
	m_IO = IO;
}

CLineWriter::~CLineWriter()
{
	Shutdown();
}

void CLineWriter::Shutdown()
{
	io_write_newline(m_IO);
	io_close(m_IO);
}

void CLineWriter::Write(const char *pStr)
{
	io_write(m_IO, pStr, str_length(pStr));
}