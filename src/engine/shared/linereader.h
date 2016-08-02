

#ifndef ENGINE_SHARED_LINEREADER_H
#define ENGINE_SHARED_LINEREADER_H
#include <base/system.h>

#include "stream.h"

// buffered stream for reading lines, should perhaps be something smaller
class CLineReader
{
	char m_aBuffer[4*1024];
	unsigned m_BufferPos;
	unsigned m_BufferSize;
	unsigned m_BufferMaxSize;
	
	CInputStream *m_pStream;

public:
	CLineReader();
	~CLineReader();
	void Init(IOHANDLE IoHandle);
	void InitString(const char *pString);
	char *Get();

	void Shutdown();
};
#endif
