#include <math.h>
#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>

#include "mapchunk.h"

inline int GetCalculated(float x, float y, int Max){ return min(Max, abs(int(sin((x+6531.38593f)*337.123f*cos((y+8641.5123f)*6173.147f))*Max))); }


CMapChunk::CMapChunk(int X, int SizeX, int NumChunks, int *apGenerationRules, CMapChunk *pPrev)
{
	m_X = X;
	m_SizeX = SizeX;
	
	m_pPrev = pPrev;
	m_pNext = NULL;
	
	m_NumChunks = NumChunks;
	m_apGenerationRules = apGenerationRules;
	
	if (m_pPrev)
	{
		int i = GetCalculated(X*3.1467f, X/3, 64)%4;
		
		int w = 0;
		while (!apGenerationRules[m_pPrev->GetIndex()*4+i] || ++w < 3)
			if (++i >= 4)
				i = 0;
		
		m_ChunkIndex = apGenerationRules[m_pPrev->GetIndex()*4+i];
	}
	else
		m_ChunkIndex = 0;
}



CMapChunk *CMapChunk::GetMapChunk(int X)
{
	if (X < m_X)
	{
		if (m_pPrev)
			return m_pPrev->GetMapChunk(X);
		else
			return this;
	}
	
	if (X >= m_X + m_SizeX)
	{
		if (!m_pNext)
			m_pNext = new CMapChunk(m_X+m_SizeX, m_SizeX, m_NumChunks, m_apGenerationRules, this);
		
		return m_pNext->GetMapChunk(X);
	}
	
	return this;
}