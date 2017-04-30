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

	for (int i = 0; i < g_Config.m_SvMapGenLevel; i++)
		rand();
	
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
	GenerateLevel();
	
	dbg_msg("mapgen", "map successfully generated in %.5fs", (float)(time_get()-TotalTime)/time_freq());
}


void CMapGen::GenerateStart(CGenLayer *pTiles)
{
	int w = pTiles->Width();
	int h = pTiles->Height();
	
	int Spawns = 0;
	
	/*
	for (int i = 0; i < 4; i++)
	{
		ivec2 p = pTiles->GetPlayerSpawn();
		ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SPAWN);
	}
	*/
	
	// find a platform
	/*
	for(int x = 3; x < w-3; x++)
		for(int y = 3; y < h-3; y++)
		{
			if (!pTiles->Get(x-2, y) && !pTiles->Get(x-1, y) && !pTiles->Get(x, y) && !pTiles->Get(x+1, y) && !pTiles->Get(x+2, y) && 
				pTiles->Get(x-2, y+1) && pTiles->Get(x-1, y+1) && pTiles->Get(x, y+1) && pTiles->Get(x+1, y+1) && pTiles->Get(x+2, y+1))
			{
				ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SPAWN);
				
				for (int xx = -3; xx < 3; xx++)
					for (int yy = 1; yy < 40; yy++)
						pTiles->Use(x+xx, y-yy);
					
					
				
				pTiles->Set(-1, x-2, y);
				pTiles->Set(-1, x-1, y);
				pTiles->Set(-1, x, y);
				pTiles->Set(-1, x+1, y);
				pTiles->Set(-1, x+2, y);
				
				
				for (int xx = -2; xx < 2; xx++)
					for (int yy = -3; yy < 0; yy++)
						pTiles->Use(x+xx, y+yy);
				
				if (++Spawns >= 4)
					return;
				
				x++;
			}
		}
	*/
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
				
				for (int xx = -2; xx < 2; xx++)
					for (int yy = -4; yy < 0; yy++)
						pTiles->Set(-1, x+xx, y+yy);
				
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
	
	for (int x = -2; x < 2; x++)
		for (int y = -2; y < 2; y++)
			pTiles->Use(p.x+x, p.y+y);
		
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

void CMapGen::GenerateMine(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetPlatform();
	
	if (p.x == 0)
		return;
	
	if (frandom() < 0.5f)
		ModifTile(p+ivec2(1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_MINE1);
	else
		ModifTile(p+ivec2(-1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_MINE2);
}

void CMapGen::GenerateWalker(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetPlatform();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_MONSTER1);
}

void CMapGen::GenerateSwitch(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetPlatform();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SWITCH);
}

void CMapGen::GenerateTurretStand(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetPlatform();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_STAND);
}

void CMapGen::GeneratePowerupper(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetPlatform();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_POWERUPPER);
}

void CMapGen::GenerateAlien(CGenLayer *pTiles, int Type)
{
	ivec2 p = pTiles->GetPlatform();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ROBOT1+Type);
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
	
	for (int x = -1; x < 1; x++)
		for (int y = -1; y < 1; y++)
			pTiles->Use(p.x+x, p.y+y);
}

void CMapGen::GenerateDeathray(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetCeiling();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_LAZER);
	
	for (int x = -1; x < 1; x++)
		for (int y = -1; y < 1; y++)
			pTiles->Use(p.x+x, p.y+y);
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


void CMapGen::GenerateRooms(CRoom *pRoom, int Type, int w, int h)
{
	// vertical line
	if (Type == -1)
	{
		int x = w * (0.2f + frandom()*0.6f);
		for (int y = 4; y < h-4; y++)
		{
			pRoom->Open(x-1, y-1);
			pRoom->Open(x, y);
			pRoom->Open(x+1, y+1);
		}

		return;
	}
	
	
	// linear
	if (Type == 0)
	{
		int y = h / 2;
		
		for (int x = 8; x < w-8; x++)
		{
			pRoom->Open(x-1, y-1);
			pRoom->Open(x, y);
			pRoom->Open(x+1, y+1);
		}
		
		return;
	}
	
	// sine wave
	if (Type == 1)
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
		
		return;
	}
	
	// double route
	if (Type == 2)
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
		
		return;
	}
	
	// line + sine
	if (Type == 3)
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
		
		return;
	}
	
	// double sine wave
	if (Type == 4)
	{
		int y = h / 2;
		
		float a = frandom()*pi*2;
		float c = 0.1f + frandom()*0.05f;
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
		
		return;
	}
	
	// #
	if (Type == 5)
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
		
		return;
	}
	
	// box
	if (Type == 6)
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
		
		return;
	}
	
	// box + sine wave
	if (Type == 7)
	{
		GenerateRooms(pRoom, 6, w, h);
		GenerateRooms(pRoom, 1, w, h);
		
		return;
	}
	
	// if all else fails, try random
	GenerateRooms(pRoom, rand()%10, w, h);
}



void CMapGen::GenerateLevel()
{
	int w = m_pLayers->GameLayer()->m_Width;
	int h = m_pLayers->GameLayer()->m_Height;

	if (w < 10 || h < 10)
		return;
	
	CGenLayer *pTiles = new CGenLayer(w, h);
	
	// generate room structure
	CRoom *pRoom = new CRoom(3, 3, w-6, h-6);
	
	int Level = g_Config.m_SvMapGenLevel;

	
	if (Level <= 5)
		GenerateRooms(pRoom, 0, w, h);
	else if (Level <= 10)
		GenerateRooms(pRoom, 1, w, h);
	else if (Level <= 20)
		GenerateRooms(pRoom, 2+rand()%3, w, h);
	else if (Level <= 30)
		GenerateRooms(pRoom, 5+rand()%3, w, h);
	else
		GenerateRooms(pRoom, 0+rand()%8, w, h);
	
	// vertical line(s) to some levels sometimes
	for (int i = 0; i < 2; i++)
		if (Level > 1 && frandom() < 0.25f)
			GenerateRooms(pRoom, -1, w, h);

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
	
	dbg_msg("mapgen", "rooms generated, map size: %d", pTiles->Size());
	
	pTiles->GenerateAirPlatforms(pTiles->Size()/500);
	
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
	//GenerateStart(pTiles);
	
	// finish pos
	GenerateEnd(pTiles);
	
	// find platforms, corners etc.
	pTiles->Scan();
	
	// start pos
	for (int i = 0; i < 4; i++)
	{
		ivec2 p = pTiles->GetPlayerSpawn();
		ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SPAWN);
	}
	
	// acid pools
	for (int i = 0; i < Level-1; i++)
		GenerateAcid(pTiles);
	
	// barrels
	int b = max(4, 15 - Level/3)+rand()%3;
	
	for (int i = 0; i < pTiles->NumPlatforms() / b; i++)
		GenerateBarrel(pTiles);
	
	bool Defend = false;
	
	if (Level > 1 && Level%5 == 0)
		Defend = true;
	
	int e = 2 + log(float(1 + Level/4)) * 5;
	
	if (Defend)
		e *= 2;
	
	for (int i = 0; i < e; i++)
	{
		int t = 0;
		
		if (Level > 10 && frandom() < 0.2f)
			t = 1;
		else if (Level > 20 && frandom() < 0.2f)
			t = 2;
		
		if (i <= 12)
			GenerateAlien(pTiles, t);
	}
	
	if (Defend)
	{
		int t = rand()%(e/3+1)+2;
		
		for (int i = 0; i < t; i++)
			GenerateTurretStand(pTiles);
	}
	
	// pickups
	for (int i = 0; i < (pTiles->Size()-Level*5)/700; i++)
		GenerateWeapon(pTiles, ENTITY_WEAPON_CHAINSAW+rand()%8);
	
	for (int i = 0; i < (pTiles->Size()-Level*5)/1000; i++)
		GenerateHearts(pTiles);
	
	for (int i = 0; i < (pTiles->Size()-Level*5)/1000; i++)
		GenerateAmmo(pTiles);
	
	// power upper(s)
	if (Level > 15 && frandom() < 0.15f)
		GeneratePowerupper(pTiles);
	if (Level > 20 && frandom() < 0.1f)
		GeneratePowerupper(pTiles);
	
	if (Level > 10 && frandom() < 0.1f)
		GenerateSwitch(pTiles);
	if (Level > 20 && frandom() < 0.1f)
		GenerateSwitch(pTiles);
	
	// walkers
	if (Level%3 == 0 || Level%7 == 0 || Level%13 == 0 || Level%17 == 0)
	{
		int w = 1+rand()%(1+min(Level/3, 4));
		
		for (int i = 0; i < w; i++)
			GenerateWalker(pTiles);
	}
	
	
	// obstacles
	int Obs = Level/3 - 4;
	
	if (Level > 10 && frandom() < 0.3f)
		Obs += Level;
	
	if (Defend)
		Obs /= 5;
	
	if (Obs > 1)
		Obs = Obs/3 + (rand()%Obs)/2;
	
	while (Obs-- > 0)
	{
		switch (rand()%6)
		{
		case 0:
		case 1:
		case 2: GenerateSawblade(pTiles); break;
		case 3: GenerateMine(pTiles); break;
		case 4: GenerateFiretrap(pTiles); break;
		case 5: GenerateDeathray(pTiles); break;
		}
	}

	
	for (int i = 0; i < e; i++)
	{
		if (i > 12)
			GenerateAlien(pTiles, 3);
	}
	
	
	if (pRoom)
		delete pRoom;
	
	if (pTiles)
		delete pTiles;
}



inline void CMapGen::ModifTile(ivec2 Pos, int Layer, int Tile)
{
	m_pCollision->ModifTile(Pos, m_pLayers->GetGameGroupIndex(), Layer, Tile, 0, 0);
}
