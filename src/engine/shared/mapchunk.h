#ifndef ENGINE_MAPCHUNK_H
#define ENGINE_MAPCHUNK_H


class CMapChunk
{
	int m_X;
	int m_SizeX;
	int m_ChunkIndex;
	int m_NumChunks;
	int *m_apGenerationRules;
	
	CMapChunk *m_pPrev;
	CMapChunk *m_pNext;
	
public:
	CMapChunk(int X, int SizeX, int NumChunks, int *apGenerationRules, CMapChunk *pPrev);
	
	CMapChunk *GetMapChunk(int X);
	int GetIndex() const { return m_ChunkIndex; }
	int GetSize() const { return m_SizeX; }
};

#endif
