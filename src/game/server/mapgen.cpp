#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>
#include <engine/shared/config.h>

#include "mapgen.h"
#include <game/server/mapgen/gen_layer.h>
#include <game/server/mapgen/room.h>
#include <game/server/gamecontext.h>
#include <game/layers.h>
#include <game/mapitems.h>

CMapGen::CMapGen()
{
	m_pLayers = 0x0;
	m_pCollision = 0x0;
}
CMapGen::~CMapGen()
{
	
}

void CMapGen::Init(CLayers *pLayers, CCollision *pCollision)
{
	m_pLayers = pLayers;
	m_pCollision = pCollision;
}

void CMapGen::FillMap(int Seed)
{
	dbg_msg("mapgen", "started map generation with seed=%d", Seed);

	int64 ProcessTime = 0;
	int64 TotalTime = time_get();

	int MineTeeLayerSize = m_pLayers->GameLayer()->m_Width*m_pLayers->GameLayer()->m_Height;

	// clear map, but keep background, envelopes etc
	ProcessTime = time_get();
	for(int i = 0; i < MineTeeLayerSize; i++)
	{
		int x = i%m_pLayers->GameLayer()->m_Width;
		ivec2 TilePos(x, (i-x)/m_pLayers->GameLayer()->m_Width);
		
		// clear the different layers
		ModifTile(TilePos, m_pLayers->GetGameLayerIndex(), 0);
		ModifTile(TilePos, m_pLayers->GetBackgroundLayerIndex(), 0);
		ModifTile(TilePos, m_pLayers->GetForegroundLayerIndex(), 0);
	}
	dbg_msg("mapgen", "map normalized in %.5fs", (float)(time_get()-ProcessTime)/time_freq());


	ProcessTime = time_get();
	GenerateRooms();
	
	dbg_msg("mapgen", "map successfully generated in %.5fs", (float)(time_get()-TotalTime)/time_freq());
}


void CMapGen::GenerateStart(CGenLayer *pTiles)
{
	int w = pTiles->Width();
	int h = pTiles->Height();
	
	int Spawns = 0;
	
	// find a platform
	for(int x = 3; x < w-3; x++)
		for(int y = 3; y < h-3; y++)
		{
			if (!pTiles->Get(x-2, y) && !pTiles->Get(x-1, y) && !pTiles->Get(x, y) && !pTiles->Get(x+1, y) && !pTiles->Get(x+2, y) && 
				pTiles->Get(x-2, y+1) && pTiles->Get(x-1, y+1) && pTiles->Get(x, y+1) && pTiles->Get(x+1, y+1) && pTiles->Get(x+2, y+1))
			{
				ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SPAWN);
				
				pTiles->Set(-1, x-2, y);
				pTiles->Set(-1, x-1, y);
				pTiles->Set(-1, x, y);
				pTiles->Set(-1, x+1, y);
				pTiles->Set(-1, x+2, y);
				
				if (++Spawns >= 4)
					return;
				
				x++;
			}
		}
}


void CMapGen::GenerateEnd(CGenLayer *pTiles)
{
	int w = pTiles->Width();
	int h = pTiles->Height();
	
	// find a platform
	for(int x = w-3; x > 3; x--)
		for(int y = 3; y < h-3; y++)
		{
			if (!pTiles->Get(x-2, y) && !pTiles->Get(x-1, y) && !pTiles->Get(x, y) && !pTiles->Get(x+1, y) && !pTiles->Get(x+2, y) && 
				pTiles->Get(x-2, y+1) && pTiles->Get(x-1, y+1) && pTiles->Get(x, y+1) && pTiles->Get(x+1, y+1) && pTiles->Get(x+2, y+1) &&
				!pTiles->Get(x, y-2) && !pTiles->Get(x, y-3) && !pTiles->Get(x, y-4) && !pTiles->Get(x, y-5))
			{
				ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_DOOR1);
				
				pTiles->Set(-1, x-2, y);
				pTiles->Set(-1, x-1, y);
				pTiles->Set(-1, x, y);
				pTiles->Set(-1, x+1, y);
				pTiles->Set(-1, x+2, y);
				
				return;
			}
		}
}


void CMapGen::GenerateSawblade(CGenLayer *pTiles)
{
	ivec2 p = ivec2(0, 0);
	
	if (frandom() < 0.4f)
		p = pTiles->GetSharpCorner();
	else if (frandom() < 0.4f)
	{
		p = pTiles->GetCeiling();
		p.y -= 1;
	}
	else if (frandom() < 0.4f)
	{
		p = pTiles->GetWall();
		
		if (p.x == 0)
			return;
		
		if (pTiles->Get(p.x-1, p.y))
			p.x -= 1;
		else
			p.x += 1;
	}
	else
	{
		p = pTiles->GetPlatform();
		p.y += 1;
	}
	
	if (p.x == 0)
		return;
	
	pTiles->Use(p.x, p.y);
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SAWBLADE);
}


void CMapGen::GenerateWeapon(CGenLayer *pTiles, int Weapon)
{
	ivec2 p = ivec2(0, 0);
	
	if (frandom() < 0.4f)
	{
		p = pTiles->GetTopCorner();
		
		if (pTiles->Get(p.x-1, p.y))
			p.x += 1;
		else
			p.x -= 1;
		
		p.y += 1;
	}
	else
		p = pTiles->GetPlatform();
	
	if (p.x == 0)
		return;
	
	pTiles->Use(p.x, p.y);
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+Weapon);
}

void CMapGen::GenerateBarrel(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetPlatform();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_BARREL);
}

void CMapGen::GenerateAlien(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetPlatform();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ROBOT1);
}

void CMapGen::GenerateFiretrap(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetWall();
	
	if (p.x == 0)
		return;
	
	if (pTiles->Get(p.x-1, p.y))
		ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_FLAMETRAP_RIGHT);
	else
		ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_FLAMETRAP_LEFT);
}

void CMapGen::GenerateDeathray(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetCeiling();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_LAZER);
}


void CMapGen::GenerateHearts(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetTopCorner();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_HEALTH_1);
	ModifTile(p+ivec2(0, 1), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_HEALTH_1);
	ModifTile(p+ivec2(0, 2), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_HEALTH_1);
}


void CMapGen::GenerateAmmo(CGenLayer *pTiles)
{
	if (frandom() < 0.4f)
	{
		ivec2 p = pTiles->GetTopCorner();
		
		if (p.x == 0)
			return;
		
		ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ARMOR_1);
		ModifTile(p+ivec2(0, 1), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ARMOR_1);
		ModifTile(p+ivec2(0, 2), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ARMOR_1);
	}
	else if (frandom() < 0.5f)
	{
		ivec2 p = pTiles->GetWall();
		
		if (p.x == 0)
			return;
		
		ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ARMOR_1);
		ModifTile(p+ivec2(0, 1), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ARMOR_1);
	}
	else
	{
		ivec2 p = pTiles->GetPlatform();
		
		if (p.x == 0)
			return;
		
		ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ARMOR_1);
		ModifTile(p+ivec2(1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ARMOR_1);
		ModifTile(p+ivec2(-1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ARMOR_1);
	}
}


void CMapGen::GenerateAcid(CGenLayer *pTiles)
{
	ivec4 p = pTiles->GetPit();
	
	if (p.x == 0)
		return;
	
	for (int x = p.x; x < p.z; x++)
		for (int y = p.y; y < p.w; y++)
		{
			ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), TILE_DAMAGEFLUID);
			pTiles->Use(x, y);
		}
}


void CMapGen::GenerateRooms()
{
	int w = m_pLayers->GameLayer()->m_Width;
	int h = m_pLayers->GameLayer()->m_Height;

	if (w < 10 || h < 10)
		return;
	
	CGenLayer *pTiles = new CGenLayer(w, h);
	
	// generate room structure
	CRoom *pRoom = new CRoom(3, 3, w-6, h-6);
	
	
	// linear sine wave
	/*
	{
		int y = h / 2;
		
		float a = frandom()*pi*2;
		float c = frandom()*0.1f;
		float s = 2.5f + frandom()*2.0f;
		
		for (int x = 8; x < w-8; x++)
		{
			pRoom->Open(x-1, y-1+sin(a+x*c)*(1.0f*h/s));
			pRoom->Open(x, y+sin(a+x*c)*(1.0f*h/s));
			pRoom->Open(x+1, y+1+sin(a+x*c)*(1.0f*h/s));
		}
	}
	*/
	
	// linear line
	/*
	{
		int y = h / 2;
		
		for (int x = 8; x < w-8; x++)
		{
			pRoom->Open(x-1, y-1);
			pRoom->Open(x, y);
			pRoom->Open(x+1, y+1);
		}
	}
	*/
	
	// box
	{
		int y = 13;
		for (int x = 8+rand()%10; x < w-8-rand()%10; x++)
		{
			pRoom->Open(x-1, y-1);
			pRoom->Open(x, y);
			pRoom->Open(x+1, y+1);
		}
		
		y = h-13;
		for (int x = 8+rand()%10; x < w-8-rand()%10; x++)
		{
			pRoom->Open(x-1, y-1);
			pRoom->Open(x, y);
			pRoom->Open(x+1, y+1);
		}
		
		int x = 18;
		for (y = 4; y < h-4; y++)
		{
			pRoom->Open(x-1, y-1);
			pRoom->Open(x, y);
			pRoom->Open(x+1, y+1);
		}
		
		x = w-18;
		for (y = 4; y < h-4; y++)
		{
			pRoom->Open(x-1, y-1);
			pRoom->Open(x, y);
			pRoom->Open(x+1, y+1);
		}
	}
	
	// #
	/*
	{
		int y = 13;
		for (int x = 8+rand()%10; x < w-8-rand()%10; x++)
		{
			pRoom->Open(x-1, y-1);
			pRoom->Open(x, y);
			pRoom->Open(x+1, y+1);
		}
		
		y = h-13;
		for (int x = 8+rand()%10; x < w-8-rand()%10; x++)
		{
			pRoom->Open(x-1, y-1);
			pRoom->Open(x, y);
			pRoom->Open(x+1, y+1);
		}
		
		int n = 1 + rand()%6;
		
		for (int i = 0; i < n; i++)
		{
			int x = w * (0.1f + frandom()*0.8f);
			for (y = 4; y < h-4; y++)
			{
				pRoom->Open(x-1, y-1);
				pRoom->Open(x, y);
				pRoom->Open(x+1, y+1);
			}
		}
	}
	*/
	
	// dual route
	/*
	{
		int y = h / 2;
		
		for (int x = 8; x < w-8; x++)
		{
			y = h/2 + (w/2-abs(x-w/2))*(1.0f*h/w)*0.8f;
			
			pRoom->Open(x-1, y-1);
			pRoom->Open(x, y);
			pRoom->Open(x+1, y+1);
		}
		
		for (int x = 8; x < w-8; x++)
		{
			y = h/2 - (w/2-abs(x-w/2))*(1.0f*h/w)*0.8f;
			
			pRoom->Open(x-1, y-1);
			pRoom->Open(x, y);
			pRoom->Open(x+1, y+1);
		}
	}
	*/
	
	// double sine wave
	/*
	{
		int y = h / 2;
		
		float a = frandom()*pi*2;
		float c = 0.1f;
		float s = 2.5f + frandom()*2.0f;
		
		for (int x = 8; x < w-8; x++)
		{
			pRoom->Open(x-1, y-1+sin(a+x*c)*(1.0f*h/s));
			pRoom->Open(x, y+sin(a+x*c)*(1.0f*h/s));
			pRoom->Open(x+1, y+1+sin(a+x*c)*(1.0f*h/s));
		}
		
		for (int x = 8; x < w-8; x++)
		{
			pRoom->Open(x-1, y-1-sin(a+x*c)*(1.0f*h/s));
			pRoom->Open(x, y-sin(a+x*c)*(1.0f*h/s));
			pRoom->Open(x+1, y+1-sin(a+x*c)*(1.0f*h/s));
		}
	}
	*/
	
	
	// sine + linear
	/*
	{
		int y = h / 2;
		
		float a = frandom()*pi*2;
		float c = 0.1f;
		float s = 2.5f + frandom()*2.0f;
		
		for (int x = 8; x < w-8; x++)
		{
			pRoom->Open(x-1, y-1+sin(a+x*c)*(1.0f*h/s));
			pRoom->Open(x, y+sin(a+x*c)*(1.0f*h/s));
			pRoom->Open(x+1, y+1+sin(a+x*c)*(1.0f*h/s));
		}
		
		y = h * (0.1f + frandom()*0.8f);
		
		for (int x = 8; x < w-8; x++)
		{
			pRoom->Open(x-1, y-1);
			pRoom->Open(x, y);
			pRoom->Open(x+1, y+1);
		}
	}
	*/
	

	pRoom->Generate(pTiles);
	
	// check for too tight corridors
	{
		for(int y = 3; y < h-4; y++)
			for(int x = 3; x < w-4; x++)
			{
				if (!pTiles->Get(x-1, y) && pTiles->Get(x, y) && pTiles->Get(x+1, y) && !pTiles->Get(x+2, y))
					pRoom->Fill(pTiles, 0, x, y, 2, 1);
				
				if (!pTiles->Get(x, y-1) && pTiles->Get(x, y) && pTiles->Get(x, y+1) && !pTiles->Get(x, y+2))
					pRoom->Fill(pTiles, 0, x, y, 1, 2);
			}
	}
	
	// write to layers
	for(int x = 0; x < w; x++)
		for(int y = 0; y < h; y++)
		{
			if (pTiles->Get(x, y))
			{
				ModifTile(ivec2(x, y), m_pLayers->GetForegroundLayerIndex(), 1);
				ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), 1);
			}
		}
	
	// start pos
	GenerateStart(pTiles);
	
	// finish pos
	GenerateEnd(pTiles);
	
	// find platforms, corners etc.
	pTiles->Scan();
	
	// fill level with stuff
	for (int i = 0; i < 99; i++)
		GenerateAcid(pTiles);
	
	for (int i = 0; i < pTiles->NumPlatforms() / 5; i++)
		GenerateBarrel(pTiles);
	
	for (int i = 0; i < 5; i++)
		GenerateAlien(pTiles);
	
	for (int i = 0; i < 3; i++)
		GenerateHearts(pTiles);
	
	for (int i = 0; i < 9; i++)
		GenerateAmmo(pTiles);
	
	for (int i = 0; i < 2; i++)
		GenerateSawblade(pTiles);
	
	for (int i = 0; i < 2; i++)
		GenerateFiretrap(pTiles);
	
	for (int i = 0; i < 2; i++)
		GenerateDeathray(pTiles);
	
	for (int i = 0; i < 8; i++)
		GenerateWeapon(pTiles, ENTITY_WEAPON_CHAINSAW+i);
	
	if (pRoom)
		delete pRoom;
	
	if (pTiles)
		delete pTiles;
}



inline void CMapGen::ModifTile(ivec2 Pos, int Layer, int Tile)
{
	m_pCollision->ModifTile(Pos, m_pLayers->GetGameGroupIndex(), Layer, Tile, 0, 0);
}
