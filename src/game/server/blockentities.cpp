#include <math.h>
#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>

#include <game/server/gamecontext.h>
#include "blockentities.h"


	
CStoredEntity::CStoredEntity(int ObjType, int Type, int Subtype, int x, int y)
{
	m_pNext = NULL;
	m_aStats[0] = ObjType;
	m_aStats[1] = Type;
	m_aStats[2] = Subtype;
	m_aStats[3] = x;
	m_aStats[4] = y;
}

CStoredEntity::~CStoredEntity()
{
	if (m_pNext)
		delete m_pNext;
}

void CStoredEntity::Add(int ObjType, int Type, int Subtype, int x, int y)
{
	if (m_pNext)
		m_pNext->Add(ObjType, Type, Subtype, x, y);
	else
		m_pNext = new CStoredEntity(ObjType, Type, Subtype, x, y);
}

void CStoredEntity::Restore(CGameContext *pGameServer)
{
	pGameServer->RestoreEntity(m_aStats[0], m_aStats[1], m_aStats[2], m_aStats[3], m_aStats[4]);
	
	if (m_pNext)
		m_pNext->Restore(pGameServer);
}




CBlockEntities::~CBlockEntities()
{
	if (m_pStoredEntities)
		delete m_pStoredEntities;
	
	if (m_pNext)
		delete m_pNext;
}
	
	
bool CBlockEntities::AddSpawn(vec2 Pos)
{
	if (Pos.x < m_X)
	{
		if (!m_pPrev)
			return false;
		
		return m_pPrev->AddSpawn(Pos);
	}
	
	if (Pos.x >= m_X + m_SizeX)
	{
		if (!m_pNext)
			return false;
		
		return m_pNext->AddSpawn(Pos);
	}
	
	
	if (m_NumSpawns >= 9)
		return false;
	
	m_aSpawn[m_NumSpawns++] = Pos;
	return true;
}

bool CBlockEntities::GetSpawn(vec2 *Pos)
{
	//if (!Pos)
	//	return false;

		
	if (!m_NumSpawns)
	{
		if (m_pPrev)
			return m_pPrev->GetSpawn(Pos);
		
		return false;
	}
	
	if (Pos->x > m_X+m_SizeX || !m_pPrev)
	{
		vec2 p = m_aSpawn[rand()%m_NumSpawns]*32;
		*Pos = p;
		return true;
	}
	else if (m_pPrev)
		return m_pPrev->GetSpawn(Pos);
	
	return false;
}


CBlockEntities::CBlockEntities(CGameContext *pGameServer, int X, int SizeX, CBlockEntities *pPrev, bool Activate)
{
	m_X = X;
	m_SizeX = SizeX;
	m_EntitiesCreated = false;
	
	m_pPrev = pPrev;
	m_pNext = NULL;
	
	//if (Activate)
	//	pGameServer->CreateEntitiesForBlock(m_X/m_SizeX);
	
	m_pStoredEntities = NULL;
	m_NumSpawns = 0;
}


void CBlockEntities::StoreEntity(int ObjType, int Type, int Subtype, int x, int y)
{
	if (m_pStoredEntities)
		m_pStoredEntities->Add(ObjType, Type, Subtype, x, y);
	else
		m_pStoredEntities = new CStoredEntity(ObjType, Type, Subtype, x, y);
}


CBlockEntities *CBlockEntities::GetBlockEntities(CGameContext *pGameServer, int X, bool Activate)
{
	if (Activate)
	{
		if (!m_EntitiesCreated)
		{
			pGameServer->CreateEntitiesForBlock(m_X/m_SizeX);
			m_EntitiesCreated = true;
		}
		
		if (m_pStoredEntities)
		{
			m_pStoredEntities->Restore(pGameServer);
			delete m_pStoredEntities;
			m_pStoredEntities = NULL;
			
			//pGameServer->CreateEnemiesForBlock(m_X/m_SizeX);
		}
		
	}
	
	if (X < m_X)
	{
		if (m_pPrev)
			return m_pPrev->GetBlockEntities(pGameServer, X);
		else
			return this;
	}
	
	if (X >= m_X + m_SizeX)
	{
		if (!m_pNext)
			m_pNext = new CBlockEntities(pGameServer, m_X+m_SizeX, m_SizeX, this, Activate);
		
		return m_pNext->GetBlockEntities(pGameServer, X, Activate);
	}
	
	if (Activate)
	{
		/*
		if (m_pStoredEntities)
		{
			m_pStoredEntities->Restore(pGameServer);
			delete m_pStoredEntities;
			m_pStoredEntities = NULL;
		}
		*/
	}
	
	return this;
}