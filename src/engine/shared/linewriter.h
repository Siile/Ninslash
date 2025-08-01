#ifndef ENGINE_SHARED_LINEWRITER_H
#define ENGINE_SHARED_LINEWRITER_H
#include <base/system.h>

class CLineWriter
{
	IOHANDLE m_IO;

public:
	CLineWriter(IOHANDLE IO);
	~CLineWriter();

	void Shutdown();

	void Write(const char *pStr);
	void WriteNewLine() { io_write_newline(m_IO); }
};
#endif
