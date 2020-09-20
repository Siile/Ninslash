#ifndef GAME_SERVER_BLOCKENTITIES_H
#define GAME_SERVER_BLOCKENTITIES_H

class CStoredEntity
{
	CStoredEntity *m_pNext;
	int m_aStats[5];
	
public:
	CStoredEntity(int ObjType, int Type, int Subtype, int x, int y);
	~CStoredEntity();
	
	void Add(int ObjType, int Type, int Subtype, int x, int y);
	void Restore(class CGameContext *pGameServer);
};

class CBlockEntities
{
	bool m_EntitiesCreated;
	int m_X;
	int m_SizeX;
	int m_NumChunks;
	
	vec2 m_aSpawn[9];
	int m_NumSpawns;
	
	CBlockEntities *m_pPrev;
	CBlockEntities *m_pNext;
	
	CStoredEntity *m_pStoredEntities;
	
public:
	CBlockEntities(class CGameContext *pGameServer, int X, int SizeX, CBlockEntities *pPrev, bool Activate = true);
	~CBlockEntities();
	bool GetSpawn(vec2 *Pos);
	bool AddSpawn(vec2 Pos);
	
	CBlockEntities *GetBlockEntities(class CGameContext *pGameServer, int X, bool Activate = false);
	int GetSize() const { return m_SizeX; }
	
	void StoreEntity(int ObjType, int Type, int Subtype, int x, int y);
};

#endif
