#include <random>

#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>
#include <engine/shared/config.h>
#include <engine/shared/linereader.h>

#include "mapgen.h"
#include <game/server/mapgen/gen_layer.h>
#include <game/server/mapgen/room.h>
#include <game/server/mapgen/maze.h>
#include <game/server/gamecontext.h>
#include <game/layers.h>
#include <game/mapitems.h>

CMapGen::CMapGen()
{
	m_pLayers = 0x0;
	m_pCollision = 0x0;
	m_pStorage = 0x0;
	m_FileLoaded = false;
}
CMapGen::~CMapGen()
{
	
}

void CMapGen::Init(CLayers *pLayers, CCollision *pCollision, IStorage *pStorage)
{
	m_pLayers = pLayers;
	m_pCollision = pCollision;
	m_pStorage = pStorage;
	
	Load("metal_main");
}



void CMapGen::Load(const char* pTileName)
{
	char aPath[256];
	str_format(aPath, sizeof(aPath), "editor/%s.rules", pTileName);
	IOHANDLE RulesFile = Storage()->OpenFile(aPath, IOFLAG_READ, IStorage::TYPE_ALL);
	if(!RulesFile)
		return;

	CLineReader LineReader;
	LineReader.Init(RulesFile);

	CConfiguration *pCurrentConf = 0;
	CIndexRule *pCurrentIndex = 0;

	// read each line
	while(char *pLine = LineReader.Get())
	{
		// skip blank/empty lines as well as comments
		if(str_length(pLine) > 0 && pLine[0] != '#' && pLine[0] != '\n' && pLine[0] != '\r'
			&& pLine[0] != '\t' && pLine[0] != '\v' && pLine[0] != ' ')
		{
			if(pLine[0]== '[')
			{
				// new configuration, get the name
				pLine++;

				CConfiguration NewConf;
				int ID = m_lConfigs.add(NewConf);
				pCurrentConf = &m_lConfigs[ID];

				str_copy(pCurrentConf->m_aName, pLine, str_length(pLine));
			}
			else
			{
				if(!str_comp_num(pLine, "Index", 5))
				{
					// new index
					int ID = 0;
					char aFlip[128] = "";

					sscanf(pLine, "Index %d %127s", &ID, aFlip);

					CIndexRule NewIndexRule;
					NewIndexRule.m_ID = ID;
					NewIndexRule.m_Flag = 0;
					NewIndexRule.m_RandomValue = 0;
					NewIndexRule.m_YDivisor = 0;
					NewIndexRule.m_YRemainder = 0;
					NewIndexRule.m_BaseTile = false;

					if(str_length(aFlip) > 0)
					{
						if(!str_comp(aFlip, "XFLIP"))
							NewIndexRule.m_Flag = TILEFLAG_VFLIP;
						else if(!str_comp(aFlip, "YFLIP"))
							NewIndexRule.m_Flag = TILEFLAG_HFLIP;
						else if(!str_comp(aFlip, "XYFLIP"))
							NewIndexRule.m_Flag = TILEFLAG_VFLIP+TILEFLAG_HFLIP;
						else if(!str_comp(aFlip, "ROTATE"))
							NewIndexRule.m_Flag = TILEFLAG_ROTATE;
						else if(!str_comp(aFlip, "XFLIP_ROTATE"))
							NewIndexRule.m_Flag = TILEFLAG_ROTATE+TILEFLAG_VFLIP;
						else if(!str_comp(aFlip, "YFLIP_ROTATE"))
							NewIndexRule.m_Flag = TILEFLAG_ROTATE+TILEFLAG_HFLIP;
						else if(!str_comp(aFlip, "XYFLIP_ROTATE"))
							NewIndexRule.m_Flag = TILEFLAG_ROTATE+TILEFLAG_VFLIP+TILEFLAG_HFLIP;
					}

					// add the index rule object and make it current
					int ArrayID = pCurrentConf->m_aIndexRules.add(NewIndexRule);
					pCurrentIndex = &pCurrentConf->m_aIndexRules[ArrayID];
				}
				else if(!str_comp_num(pLine, "BaseTile", 8) && pCurrentIndex)
				{
					pCurrentIndex->m_BaseTile = true;
				}
				else if(!str_comp_num(pLine, "Pos", 3) && pCurrentIndex)
				{
					int x = 0, y = 0;
					char aValue[128];
					int Value = CPosRule::EMPTY;
					bool IndexValue = false;

					sscanf(pLine, "Pos %d %d %127s", &x, &y, aValue);

					if(!str_comp(aValue, "FULL"))
						Value = CPosRule::FULL;
					else if(!str_comp_num(aValue, "INDEX", 5))
					{
						sscanf(pLine, "Pos %*d %*d INDEX %d", &Value);
						IndexValue = true;
					}

					CPosRule NewPosRule = {x, y, Value, IndexValue};
					pCurrentIndex->m_aRules.add(NewPosRule);
				}
				else if(!str_comp_num(pLine, "Random", 6) && pCurrentIndex)
				{
					sscanf(pLine, "Random %d", &pCurrentIndex->m_RandomValue);
				}
				else if(!str_comp_num(pLine, "YRemainder", 10) && pCurrentIndex)
				{
					sscanf(pLine, "YRemainder %d %d", &pCurrentIndex->m_YDivisor, &pCurrentIndex->m_YRemainder);
				}
			}
		}
	}

	io_close(RulesFile);

	m_FileLoaded = true;
}

const char* CMapGen::GetConfigName(int Index)
{
	if(Index < 0 || Index >= m_lConfigs.size())
		return "";

	return m_lConfigs[Index].m_aName;
}



void CMapGen::FillMap()
{
	dbg_msg("mapgen", "started map generation");

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
		ModifTile(TilePos, m_pLayers->GetDoodadsLayerIndex(), 0);
		ModifTile(TilePos, m_pLayers->GetForegroundLayerIndex(), 0);
	}
	dbg_msg("mapgen", "map normalized in %.5fs", (float)(time_get()-ProcessTime)/time_freq());


	ProcessTime = time_get();
	
	if (str_comp(g_Config.m_SvGametype, "coop") == 0)
		GenerateLevel();
	else
		GeneratePVPLevel();
	
	dbg_msg("mapgen", "map successfully generated in %.5fs", (float)(time_get()-TotalTime)/time_freq());
}



void CMapGen::GenerateEnd(CGenLayer *pTiles)
{
	int w = pTiles->Width();
	int h = pTiles->Height();
	
	// find a platform
	if (g_Config.m_SvMapGenLevel%10 == 9)
	{
		for(int y = 3; y < h-3; y++)
			for(int x = w-3; x > 3; x--)
			{
				if (!pTiles->Get(x-2, y) && !pTiles->Get(x-1, y) && !pTiles->Get(x, y) && !pTiles->Get(x+1, y) && !pTiles->Get(x+2, y) && !pTiles->Get(x+3, y) && 
					pTiles->Get(x-3, y+1) && pTiles->Get(x-2, y+1) && pTiles->Get(x-1, y+1) && pTiles->Get(x, y+1) && pTiles->Get(x+1, y+1) && pTiles->Get(x+2, y+1) && pTiles->Get(x+3, y+1) &&
					!pTiles->Get(x, y-2) && !pTiles->Get(x, y-3) && !pTiles->Get(x, y-4) && !pTiles->Get(x, y-5))
				{
					ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_DOOR1);
					
					pTiles->m_EndPos = ivec2(x, y);
					
					pTiles->Set(-1, x-2, y);
					pTiles->Set(-1, x-1, y);
					pTiles->Set(-1, x, y);
					pTiles->Set(-1, x+1, y);
					pTiles->Set(-1, x+2, y);
					
					// clear
					for (int xx = -2; xx < 3; xx++)
						for (int yy = -4; yy < 0; yy++)
							pTiles->Set(-1, x+xx, y+yy);
						
					// background
					for (int xx = -5; xx < 6; xx++)
						for (int yy = -7; yy < 500; yy++)
							pTiles->Set(1, x+xx, y+yy, 0, CGenLayer::BACKGROUND);
					
					return;
				}
			}
	}
	else
	{
		for(int x = w-3; x > 3; x--)
			for(int y = 3; y < h-3; y++)
			{
				if (!pTiles->Get(x-2, y) && !pTiles->Get(x-1, y) && !pTiles->Get(x, y) && !pTiles->Get(x+1, y) && !pTiles->Get(x+2, y) && !pTiles->Get(x+3, y) && 
					pTiles->Get(x-3, y+1) && pTiles->Get(x-2, y+1) && pTiles->Get(x-1, y+1) && pTiles->Get(x, y+1) && pTiles->Get(x+1, y+1) && pTiles->Get(x+2, y+1) && pTiles->Get(x+3, y+1) &&
					!pTiles->Get(x, y-2) && !pTiles->Get(x, y-3) && !pTiles->Get(x, y-4) && !pTiles->Get(x, y-5))
				{
					ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_DOOR1);
					
					pTiles->m_EndPos = ivec2(x, y);
					
					pTiles->Set(-1, x-2, y);
					pTiles->Set(-1, x-1, y);
					pTiles->Set(-1, x, y);
					pTiles->Set(-1, x+1, y);
					pTiles->Set(-1, x+2, y);
					
					// clear
					for (int xx = -2; xx < 3; xx++)
						for (int yy = -4; yy < 0; yy++)
							pTiles->Set(-1, x+xx, y+yy);
						
					// background
					for (int xx = -5; xx < 6; xx++)
						for (int yy = -7; yy < 500; yy++)
							pTiles->Set(1, x+xx, y+yy, 0, CGenLayer::BACKGROUND);
					
					return;
				}
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
	
	p = pTiles->GetTopCorner();
		
	if (p.x != 0)
	{
		if (pTiles->Get(p.x-1, p.y))
			p.x += 1;
		else
			p.x -= 1;
		
		p.y += 1;
		
		pTiles->Use(p.x, p.y);
		ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+Weapon);
	}
	else
	{
		p = pTiles->GetCeiling();
		
		if (p.x != 0)
		{
			p.y += 1;
			
			pTiles->Use(p.x, p.y);
			ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+Weapon);
		}
		else
		{
			p = pTiles->GetPlatform();
		
			if (p.x == 0)
				return;
			
			pTiles->Use(p.x, p.y);
			ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+Weapon);
		}
	}
}

void CMapGen::GenerateBarrel(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetPlatform();
	
	if (p.x == 0)
		return;
	
	if (str_comp(g_Config.m_SvGametype, "coop") == 0)
	{
		if (frandom() < 0.3f && g_Config.m_SvMapGenLevel > 15)
			ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_POWERBARREL);
		else
			ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_BARREL);
	}
	else
	{
		if (frandom() < 0.3f)
			ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_POWERBARREL);
		else
			ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_BARREL);
	}
}

void CMapGen::GenerateLightningWall(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetPlatform();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_LIGHTNINGWALL);
	pTiles->Use(p.x, p.y);
}

void CMapGen::GenerateConveyorBelt(CGenLayer *pTiles)
{
	ivec3 p = pTiles->GetLongPlatform();
	
	if (p.x == 0)
		return;
	
	int i = TILE_MOVELEFT;
	
	if (frandom() < 0.5f)
		i = TILE_MOVERIGHT;
	
	for (int x = p.x; x <= p.z; x++)
		ModifTile(ivec2(x, p.y), m_pLayers->GetGameLayerIndex(), i);
}

void CMapGen::GenerateHangables(CGenLayer *pTiles)
{
	ivec3 p = pTiles->GetLongCeiling();
	
	if (p.x == 0)
		return;
	
	p.y++;

	for (int x = p.x; x <= p.z; x++)
	{
		ModifTile(ivec2(x, p.y), m_pLayers->GetGameLayerIndex(), TILE_HANG);
		if (frandom() < 0.11f)
			ModifTile(ivec2(x, p.y), m_pLayers->GetForegroundLayerIndex(), 91, 0);
		else
			ModifTile(ivec2(x, p.y), m_pLayers->GetForegroundLayerIndex(), 90, 0);
	}
	
	if (pTiles->Get(p.x-1, p.y))
		ModifTile(ivec2(p.x, p.y), m_pLayers->GetForegroundLayerIndex(), 89, 0);
	else
	{
		ModifTile(ivec2(p.x, p.y), m_pLayers->GetForegroundLayerIndex(), 92, TILEFLAG_VFLIP);
		ModifTile(ivec2(p.x+1, p.y), m_pLayers->GetForegroundLayerIndex(), 91, 0);
	}
	
	if (pTiles->Get(p.z+1, p.y))
		ModifTile(ivec2(p.z, p.y), m_pLayers->GetForegroundLayerIndex(), 89, TILEFLAG_VFLIP);
	else
	{
		ModifTile(ivec2(p.z, p.y), m_pLayers->GetForegroundLayerIndex(), 92, 0);
		ModifTile(ivec2(p.z-1, p.y), m_pLayers->GetForegroundLayerIndex(), 91, 0);
	}
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
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_DROID_WALKER);
	pTiles->Use(p.x, p.y);
}

void CMapGen::GenerateStarDroid(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetOpenArea();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_DROID_STAR);
}

void CMapGen::GenerateCrawlerDroid(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetOpenArea();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_DROID_CRAWLER);
	pTiles->Use(p.x, p.y);
}

void CMapGen::GenerateSwitch(CGenLayer *pTiles)
{
	ivec2 p = ivec2(0, 0);
	
	if (g_Config.m_SvMapGenLevel%10 == 9)
		p = pTiles->GetBotPlatform();
	else
		p = pTiles->GetPlatform();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SWITCH);
	pTiles->Use(p.x, p.y);
}

void CMapGen::GenerateTurretStand(CGenLayer *pTiles)
{
	
	if (frandom() < 0.4f)
	{
		ivec2 p = ivec2(0, 0);
		
		if (frandom() < 0.6f)
			p = pTiles->GetLeftCeiling();
		else
			p = pTiles->GetCeiling();
		
		if (p.x != 0)
		{
			ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_STAND);
			pTiles->Use(p.x, p.y);
			return;
		}
	}
	
	ivec2 p = pTiles->GetLeftPlatform();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_STAND);
	pTiles->Use(p.x, p.y);
}

void CMapGen::GenerateTurret(CGenLayer *pTiles)
{
	
	if (frandom() < 0.4f)
	{
		ivec2 p = pTiles->GetRightCeiling();
		
		if (p.x != 0)
		{
			ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_TURRET);
			pTiles->Use(p.x, p.y);
			return;
		}
	}
	
	ivec2 p = pTiles->GetRightPlatform();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_TURRET);
	pTiles->Use(p.x, p.y);
}

void CMapGen::GenerateTeslacoil(CGenLayer *pTiles)
{
	
	if (frandom() < 0.4f)
	{
		ivec2 p = pTiles->GetRightCeiling();
		
		if (p.x != 0)
		{
			ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_TESLACOIL);
			pTiles->Use(p.x, p.y);
			return;
		}
	}
	
	ivec2 p = pTiles->GetRightPlatform();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_TESLACOIL);
	pTiles->Use(p.x, p.y);
}

void CMapGen::GeneratePowerupper(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetPlatform();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_POWERUPPER);
	pTiles->Use(p.x, p.y);
}

void CMapGen::GenerateEnemySpawn(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetPlatform();
	
	if (p.x == 0)
		p = pTiles->GetOpenArea();
	
	if (p.x == 0)
		p = pTiles->GetCeiling();
	
	if (p.x == 0)
		return;
	
	ModifTile(p+ivec2(-1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ENEMYSPAWN);
	ModifTile(p+ivec2(+1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ENEMYSPAWN);
	pTiles->Use(p.x, p.y);
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
	
	bool Valid = false;
	
	for (int y = 1; y < 22; y++)
	{
		if (pTiles->Get(p.x, p.y+y))
			Valid = true;
	}
	
	if (Valid)
		ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_LAZER);
	else
		ModifTile(p+ivec2(0, -1), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SAWBLADE);
	
	for (int x = -1; x < 1; x++)
		for (int y = -1; y < 1; y++)
			pTiles->Use(p.x+x, p.y+y);
}


void CMapGen::GenerateScreen(CGenLayer *pTiles)
{
	ivec3 p = pTiles->GetLongPlatform();
	
	if (p.x == 0)
		return;
	int x = (p.x+p.z)/2;
	
	for (int y = 1; y < 6; y++)
		if (pTiles->Get(x, p.y-y))
			return;
	
	
	if (frandom() < 0.7f)
		ModifTile(ivec2(x, p.y-1), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SCREEN);
	else
		ModifTile(ivec2(x, p.y-1), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_REACTOR);
	
	pTiles->Use(x, p.y-1);
}

void CMapGen::GenerateSpeaker(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetTopCorner();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SPEAKER);
	pTiles->Use(p.x, p.y);
}

void CMapGen::GenerateHearts(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetTopCorner();
	
	if (p.x != 0)
	{
		ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_HEALTH_1);
		ModifTile(p+ivec2(0, 1), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_HEALTH_1);
		ModifTile(p+ivec2(0, 2), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_HEALTH_1);
	}
	else
	{
		p = pTiles->GetCeiling();
		
		if (p.x != 0)
		{
			ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_HEALTH_1);
			ModifTile(p+ivec2(-1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_HEALTH_1);
			ModifTile(p+ivec2(1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_HEALTH_1);
		}
		else
		{
			p = pTiles->GetWall();
		
			if (p.x != 0)
			{
				ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_HEALTH_1);
				ModifTile(p+ivec2(0, -1), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_HEALTH_1);
			}
			else
			{
				ivec2 p = pTiles->GetPlatform();
				
				if (p.x == 0)
					return;
				
				ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_HEALTH_1);
				ModifTile(p+ivec2(1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_HEALTH_1);
				ModifTile(p+ivec2(-1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_HEALTH_1);
			}
		}
	}
}


void CMapGen::GenerateAmmo(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetTopCorner();
	
	if (p.x != 0)
	{
		ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_AMMO_1);
		ModifTile(p+ivec2(0, 1), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_AMMO_1);
		ModifTile(p+ivec2(0, 2), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_AMMO_1);
	}
	else
	{
		p = pTiles->GetCeiling();
		
		if (p.x != 0)
		{
			ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_AMMO_1);
			ModifTile(p+ivec2(-1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_AMMO_1);
			ModifTile(p+ivec2(1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_AMMO_1);
		}
		else
		{
			p = pTiles->GetWall();
		
			if (p.x != 0)
			{
				ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_AMMO_1);
				ModifTile(p+ivec2(0, -1), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_AMMO_1);
			}
			else
			{
				ivec2 p = pTiles->GetPlatform();
				
				if (p.x == 0)
					return;
				
				ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_AMMO_1);
				ModifTile(p+ivec2(1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_AMMO_1);
				ModifTile(p+ivec2(-1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_AMMO_1);
			}
		}
	}
}



void CMapGen::GenerateArmor(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetTopCorner();
	
	if (p.x != 0)
	{
		ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ARMOR_1);
		ModifTile(p+ivec2(0, 1), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ARMOR_1);
		ModifTile(p+ivec2(0, 2), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ARMOR_1);
	}
	else
	{
		p = pTiles->GetCeiling();
		
		if (p.x != 0)
		{
			ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ARMOR_1);
			ModifTile(p+ivec2(-1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ARMOR_1);
			ModifTile(p+ivec2(1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ARMOR_1);
		}
		else
		{
			p = pTiles->GetWall();
		
			if (p.x != 0)
			{
				ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ARMOR_1);
				ModifTile(p+ivec2(0, -1), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ARMOR_1);
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



void CMapGen::GenerateLevel()
{
	int w = m_pLayers->GameLayer()->m_Width;
	int h = m_pLayers->GameLayer()->m_Height;

	if (w < 10 || h < 10)
		return;
	
	CGenLayer *pTiles = new CGenLayer(w, h);
	
	// generate room structure
	CRoom *pRoom = new CRoom(3, 3, w-6, h-6);
	CMaze *pMaze = new CMaze(w, h);
	
	int Level = g_Config.m_SvMapGenLevel;

	pMaze->OpenRooms(pRoom);

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
	
	pTiles->GenerateSlopes();
	pTiles->RemoveSingles();
	
	dbg_msg("mapgen", "rooms generated, map size: %d", pTiles->Size());
	
	int n = pTiles->Size()/500;
	
	GenerateEnd(pTiles);
	pTiles->GenerateBackground();
	pTiles->GenerateMoreBackground();
	
	if (n > 1)
		pTiles->GenerateAirPlatforms(n/2 + rand()%(n/2));
	else
		pTiles->GenerateAirPlatforms(n);

	dbg_msg("mapgen", "Proceed tiles");
	Proceed(pTiles, 0);
	
	pTiles->GenerateBoxes();
	
	pTiles->GenerateFences();
	
	// write to layers; foreground
	for(int x = 0; x < w; x++)
		for(int y = 0; y < h; y++)
		{
			int i = pTiles->Get(x, y);
			
			if (i > 0)
			{
				int f = pTiles->GetFlags(x, y);
				ModifTile(ivec2(x, y), m_pLayers->GetForegroundLayerIndex(), i, f);
				
				// slopes
				if (i == 20 && f == TILEFLAG_VFLIP)
					ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), TILE_RAMP_RIGHT);
				else if (i == 20 && f == 0)
					ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), TILE_RAMP_LEFT);
				else if (i == 20 && f == TILEFLAG_HFLIP+TILEFLAG_VFLIP)
					ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), TILE_ROOFSLOPE_RIGHT);
				else if (i == 20 && f == TILEFLAG_HFLIP)
					ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), TILE_ROOFSLOPE_LEFT);
				else
					ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), 1);
			}
		}
		
	// write to layers; FGOBJECTS to foreground
	for(int x = 0; x < w; x++)
		for(int y = 0; y < h; y++)
		{
			int i = pTiles->Get(x, y, CGenLayer::FGOBJECTS);
			
			if (i > 0)
			{
				int f = pTiles->GetFlags(x, y, CGenLayer::FGOBJECTS);
				ModifTile(ivec2(x, y), m_pLayers->GetForegroundLayerIndex(), i, f);
				ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), 1);
			}
		}
		
	// background
	for(int x = 0; x < w; x++)
		for(int y = 0; y < h; y++)
		{
			int i = pTiles->Get(x, y, CGenLayer::BACKGROUND);
			
			if (i > 0)
				ModifTile(ivec2(x, y), m_pLayers->GetBackgroundLayerIndex(), i, pTiles->GetFlags(x, y, CGenLayer::BACKGROUND));
		}
	
	// doodads
	for(int x = 0; x < w; x++)
		for(int y = 0; y < h; y++)
		{
			int i = pTiles->Get(x, y, CGenLayer::DOODADS);
			
			if (i > 0)
				ModifTile(ivec2(x, y), m_pLayers->GetDoodadsLayerIndex(), i, pTiles->GetFlags(x, y, CGenLayer::DOODADS));
		}
	
	
	// find platforms, corners etc.
	dbg_msg("mapgen", "Scanning level");
	pTiles->Scan();
	
	// start pos
	for (int i = 0; i < 4; i++)
	{
		ivec2 p = pTiles->GetPlayerSpawn();
		ModifTile(p+ivec2(-1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SPAWN);
		ModifTile(p+ivec2(+1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SPAWN);
	}
	
	// acid pools
	for (int i = 0; i < Level+1; i++)
		GenerateAcid(pTiles);

	// conveyor belts
	if (Level > 10)
	{
		int c = rand()%(min(10, Level/5));
		for (int i = 0; i < c; i++)
			GenerateConveyorBelt(pTiles);
	}
	
	// hangables
	if (Level > 5)
	{
		int c = 1+rand()%(min(11, Level/4));
		for (int i = 0; i < c; i++)
			GenerateHangables(pTiles);
	}
	
	for (int i = 0; i < min(4, 1 + int(Level * 0.05f)); i++)
		GeneratePowerupper(pTiles);
	
	// enemy spawn positions
	for (int i = 0; i < min(5+Level, 14) ; i++)
		GenerateEnemySpawn(pTiles);
	
	
	for (int i = 0; i < 4; i++)
		GenerateScreen(pTiles);
	
	
	// barrels
	int b = max(4, 15 - Level/3)+rand()%3;
	
	for (int i = 0; i < pTiles->NumPlatforms() / b; i++)
		GenerateBarrel(pTiles);
	
	// lightning walls
	if (Level > 1 + rand()%15)
	{
		int l = 1 + rand()%min(9, 1 + Level/5);
		for (int i = 0; i < l; i++)
			GenerateLightningWall(pTiles);
	}
		
	bool Defend = (Level > 1 && Level%5 == 0);
	int e = 2 + log(float(1 + Level/4)) * 5;

	if (Defend)
		e *= 2;
	
	if (Defend)
	{
		int t = rand()%(e/3+3)+3;
		
		for (int i = 0; i < t; i++)
			GenerateTurretStand(pTiles);
	}
	
	/*
	for (int i = 0; i < 3; i++)
		GenerateTurretStand(pTiles);
	*/
	
	
	for (int i = 0; i < min(Level/3, 4) ; i++)
		GenerateSpeaker(pTiles);
	
	
	// pickups
	//for (int i = 0; i < (pTiles->Size()-Level*5)/700; i++)
	
	w = 2 + rand()%3 + (Level > 15 ? 1 : 0);
	
	for (int i = 0; i < w; i++)
		GenerateWeapon(pTiles, ENTITY_WEAPON_CHAINSAW+rand()%8);

	GenerateWeapon(pTiles, ENTITY_KIT);
	
	if (Level > 5)
		GenerateWeapon(pTiles, ENTITY_KIT);
	
	if (Level > 15)
		GenerateWeapon(pTiles, ENTITY_KIT);
	
	if (Level%5 == 4 || Level%7 == 6 || Level%11 == 9)
	{
		for (int i = 0; i < 2 + (0.3f + frandom())*min(10.0f, Level * 0.2f); i++)
			GenerateTurret(pTiles);
		
		if (Level > 20 && frandom() < 0.7f)
			GenerateTeslacoil(pTiles);
	}
	else
	{
		if (frandom() < 0.5f && Level > 3)
			GenerateTurret(pTiles);
		
		if (frandom() < 0.5f && Level > 9)
			GenerateTurret(pTiles);
	}
	
	for (int i = 0; i < (pTiles->Size())/1100; i++)
		GenerateHearts(pTiles);
	
	for (int i = 0; i < (pTiles->Size())/1100; i++)
		GenerateAmmo(pTiles);
	
	for (int i = 0; i < (pTiles->Size())/1400; i++)
		GenerateArmor(pTiles);
		
	if (Level%5 == 4)
		GenerateSwitch(pTiles);
	
	// walkers
	if (Level%3 == 0 || Level%7 == 0 || Level%13 == 0 || Level%17 == 0)
	{
		int w = 1+rand()%(1+min(Level/4, 4));
		
		for (int i = 0; i < w; i++)
			GenerateWalker(pTiles);
	}

	
	// star droids
	if (Level > 5)
		if (Level%4 == 0 || Level%7 == 0 || Level%11 == 0 || Level%17 == 0)
		{
			int w = 1+rand()%(1+min(Level/4, 4));
			
			for (int i = 0; i < w; i++)
				GenerateStarDroid(pTiles);
		}
		
	/*
	for (int i = 0; i < 2; i++)
		GenerateCrawlerDroid(pTiles);
	*/
	
	// obstacles
	int Obs = Level/3 - 4;
	
	if (Level > 10 && frandom() < 0.3f)
		Obs += Level/2;
	
	if (Defend)
		Obs /= 5;
	
	if (Obs > 1)
		Obs = Obs/3 + (rand()%Obs)/2;
	
	while (Obs-- > 0)
	{
		switch (1+rand()%5)
		{
		case 0:
		case 1:
		case 2: GenerateSawblade(pTiles); break;
		case 3: 
		case 4: GenerateFiretrap(pTiles); break;
		case 5: GenerateDeathray(pTiles); break;
		}
	}

	// more enemy spawn positions
	for (int i = 0; i < min(Level, 20); i++)
		GenerateEnemySpawn(pTiles);
	
	if (pRoom)
		delete pRoom;
	
	if (pTiles)
		delete pTiles;
}




void CMapGen::Mirror(CGenLayer *pTiles)
{
	int w = pTiles->Width();
	int h = pTiles->Height();
	
	for (int x = 0; x < w/2; x++)
		for (int y = 0; y < h; y++)
		{
			pTiles->Set(pTiles->Get(w/2-x, y), w/2+x, y);
		}
	
}



void CMapGen::GeneratePVPLevel()
{
	int w = m_pLayers->GameLayer()->m_Width;
	int h = m_pLayers->GameLayer()->m_Height;

	if (w < 10 || h < 10)
		return;
	
	CGenLayer *pTiles = new CGenLayer(w, h);
	
	// generate room structure
	CRoom *pRoom = new CRoom(3, 3, w-6, h-6);
	CMaze *pMaze = new CMaze(w, h);
	
	pMaze->OpenRooms(pRoom);

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
	
	pTiles->GenerateSlopes();
	Mirror(pTiles);
	
	pTiles->RemoveSingles();
	
	dbg_msg("mapgen", "rooms generated, map size: %d", pTiles->Size());
	
	int n = pTiles->Size()/500;

	pTiles->GenerateBackground();
	pTiles->GenerateMoreBackground();
	
	if (n > 1)
		pTiles->GenerateAirPlatforms(n/2 + rand()%(n/2));
	else
		pTiles->GenerateAirPlatforms(n);

	dbg_msg("mapgen", "Proceed tiles");
	Proceed(pTiles, 0);
	
	pTiles->GenerateBoxes();
	
	pTiles->GenerateFences();
	
	WriteLayers(pTiles);
	WriteBackground(pTiles);
	
	// find platforms, corners etc.
	dbg_msg("mapgen", "Scanning level");
	pTiles->Scan();
	
	// flags to ctf
	if (str_comp(g_Config.m_SvGametype, "ctf") == 0)
	{
		// left & rightmost tiles as spawns
		
		// red team spawn pos
		/*
		for (int i = 0; i < 2; i++)
		{
			ivec2 p = pTiles->GetLeftPlatform();
			
			if (p.x != 0)
			{
				pTiles->Use(p.x, p.y);
				ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SPAWN_RED);
			}
		}
		
		// blue team spawn pos
		for (int i = 0; i < 2; i++)
		{
			ivec2 p = pTiles->GetRightPlatform();
			
			if (p.x != 0)
			{
				pTiles->Use(p.x, p.y);
				ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SPAWN_BLUE);
			}
		}
		*/
		
		{
			ivec2 p = pTiles->GetLeftPlatform();
			
			if (p.x != 0)
			{
				pTiles->Use(p.x, p.y);
				ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_FLAGSTAND_RED);
				WriteBase(pTiles, 0, p, 6);
			}
			else
				dbg_msg("mapgen", "Can't set red flag");
		}
		{
			ivec2 p = pTiles->GetRightPlatform();
			
			if (p.x != 0)
			{
				pTiles->Use(p.x, p.y);
				ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_FLAGSTAND_BLUE);
				WriteBase(pTiles, 1, p, 6);
			}
			else
				dbg_msg("mapgen", "Can't set blue flag");
		}
	}
	
	// dm spawn pos
	if (str_comp(g_Config.m_SvGametype, "dm") == 0)
	{
		for (int i = 0; i < 16; i++)
		{
			ivec2 p = pTiles->GetPlatform();
			
			if (p.x == 0)
				p = pTiles->GetWall();
			if (p.x == 0)
				p = pTiles->GetCeiling();
			
			if (p.x != 0)
				ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SPAWN);
		}
	}
	else
	// tdm & ctf
	{
		for (int i = 0; i < 6; i++)
		{
			// red team spawn pos
			{
				ivec2 p = pTiles->GetLeftPlatform();
				
				if (p.x != 0)
				{
					pTiles->Use(p.x, p.y);
					ModifTile(p+ivec2(-1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SPAWN_RED);
					ModifTile(p+ivec2(+1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SPAWN_RED);
				}
			}
			
			// blue team spawn pos
			{
				ivec2 p = pTiles->GetRightPlatform();
				
				if (p.x != 0)
				{
					pTiles->Use(p.x, p.y);
					ModifTile(p+ivec2(-1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SPAWN_BLUE);
					ModifTile(p+ivec2(+1, 0), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SPAWN_BLUE);
				}
			}
		}
	}
	
	
	// acid pools
	for (int i = 0; i < 40; i++)
		GenerateAcid(pTiles);

	// conveyor belts
	{
		int c = rand()%10;
		for (int i = 0; i < c; i++)
			GenerateConveyorBelt(pTiles);
	}
	
	// hangables
	int c = 1+rand()%4;
	for (int i = 0; i < c; i++)
		GenerateHangables(pTiles);
		
	for (int i = 0; i < 4; i++)
		GenerateScreen(pTiles);
	

	//for (int i = 0; i < 2; i++)
	//	GenerateCrawlerDroid(pTiles);
	
	int Obs = 1 + pTiles->NumPlatforms() / 4.0f;
	
	GeneratePowerupper(pTiles);
	GeneratePowerupper(pTiles);
	
	// barrels
	int b = 5 + rand()%3;
	
	for (int i = 0; i < pTiles->NumPlatforms() / b; i++)
		GenerateBarrel(pTiles);

	
	for (int i = 0; i < 4 ; i++)
		GenerateSpeaker(pTiles);
	
	
	w = 1 + pTiles->NumPlatforms() / 5.0f;
	
	for (int i = 0; i < w; i++)
		GenerateWeapon(pTiles, ENTITY_WEAPON_CHAINSAW);
	
	for (int i = 0; i < 3; i++)
		GenerateWeapon(pTiles, ENTITY_KIT);
	
	for (int i = 0; i < (pTiles->Size())/800; i++)
		GenerateHearts(pTiles);
	
	for (int i = 0; i < (pTiles->Size())/800; i++)
		GenerateAmmo(pTiles);
	
	for (int i = 0; i < (pTiles->Size())/1200; i++)
		GenerateArmor(pTiles);


	// obstacles
	
	while (Obs-- > 0)
	{
		switch (rand()%6)
		{
		case 0:
		case 1:
		case 2: GenerateSawblade(pTiles); break;
		case 3:
		case 4: GenerateFiretrap(pTiles); break;
		case 5: GenerateDeathray(pTiles); break;
		}
	}

	if (pRoom)
		delete pRoom;
	
	if (pTiles)
		delete pTiles;
}

void CMapGen::WriteBase(class CGenLayer *pTiles, int BaseNum, ivec2 Pos, float Size)
{
	int w = m_pLayers->GameLayer()->m_Width;
	int h = m_pLayers->GameLayer()->m_Height;
	
	CGenLayer *pBaseTiles = new CGenLayer(w, h);
	pBaseTiles->CleanTiles();
	
	// copy tiles & check distance to base pos
	for(int x = 1; x < w-1; x++)
		for(int y = 1; y < h-1; y++)
		{
			int i = pTiles->Get(x, y);
			
			if (i > 0 && distance(vec2(Pos.x, Pos.y), vec2(x, y)) < Size)
				pBaseTiles->Set(1, x, y);
		}
	
	// auto map
	pBaseTiles->RemoveSingles();
	pBaseTiles->BaseCleanup();
	Proceed(pBaseTiles, 0);
	
	// write to layer
	int LayerIndex = 0;
	
	if (BaseNum == 0)
		LayerIndex = m_pLayers->GetBase1LayerIndex();
	else if (BaseNum == 1)
		LayerIndex = m_pLayers->GetBase2LayerIndex();
	
	
	for(int x = 0; x < w; x++)
		for(int y = 0; y < h; y++)
		{
			int i = pBaseTiles->Get(x, y);
			
			if (i > 0)
			{
				int f = pBaseTiles->GetFlags(x, y);
				ModifTile(ivec2(x, y), LayerIndex, i, f);
			}
		}
		
	delete pBaseTiles;
}


void CMapGen::WriteLayers(CGenLayer *pTiles)
{
	int w = m_pLayers->GameLayer()->m_Width;
	int h = m_pLayers->GameLayer()->m_Height;
	
	// write to layers; foreground
	for(int x = 0; x < w; x++)
		for(int y = 0; y < h; y++)
		{
			int i = pTiles->Get(x, y);
			
			if (i > 0)
			{
				int f = pTiles->GetFlags(x, y);
				ModifTile(ivec2(x, y), m_pLayers->GetForegroundLayerIndex(), i, f);
				
				// slopes
				if (i == 20 && f == TILEFLAG_VFLIP)
					ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), TILE_RAMP_RIGHT);
				else if (i == 20 && f == 0)
					ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), TILE_RAMP_LEFT);
				else if (i == 20 && f == TILEFLAG_HFLIP+TILEFLAG_VFLIP)
					ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), TILE_ROOFSLOPE_RIGHT);
				else if (i == 20 && f == TILEFLAG_HFLIP)
					ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), TILE_ROOFSLOPE_LEFT);
				else
					ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), 1);
			}
		}
		
	// write to layers; FGOBJECTS to foreground
	for(int x = 0; x < w; x++)
		for(int y = 0; y < h; y++)
		{
			int i = pTiles->Get(x, y, CGenLayer::FGOBJECTS);
			
			if (i > 0)
			{
				int f = pTiles->GetFlags(x, y, CGenLayer::FGOBJECTS);
				ModifTile(ivec2(x, y), m_pLayers->GetForegroundLayerIndex(), i, f);
				ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), 1);
			}
		}
		
	/*
	// background
	for(int x = 0; x < w; x++)
		for(int y = 0; y < h; y++)
		{
			int i = pTiles->Get(x, y, CGenLayer::BACKGROUND);
			
			if (i > 0)
				ModifTile(ivec2(x, y), m_pLayers->GetBackgroundLayerIndex(), i, pTiles->GetFlags(x, y, CGenLayer::BACKGROUND));
		}
	*/
	
	// doodads
	for(int x = 0; x < w; x++)
		for(int y = 0; y < h; y++)
		{
			int i = pTiles->Get(x, y, CGenLayer::DOODADS);
			
			if (i > 0)
				ModifTile(ivec2(x, y), m_pLayers->GetDoodadsLayerIndex(), i, pTiles->GetFlags(x, y, CGenLayer::DOODADS));
		}
}

void CMapGen::WriteBackground(CGenLayer *pTiles)
{
	int w = m_pLayers->GameLayer()->m_Width;
	int h = m_pLayers->GameLayer()->m_Height;
	
	// background
	for(int x = 0; x < w; x++)
		for(int y = 0; y < h; y++)
		{
			int i = pTiles->Get(x, y, CGenLayer::BACKGROUND);
			
			if (i > 0)
				ModifTile(ivec2(x, y), m_pLayers->GetBackgroundLayerIndex(), i, pTiles->GetFlags(x, y, CGenLayer::BACKGROUND));
		}
}



inline void CMapGen::ModifTile(ivec2 Pos, int Layer, int Tile, int Flags)
{
	m_pCollision->ModifTile(Pos, m_pLayers->GetGameGroupIndex(), Layer, Tile, Flags, 0);
}




void CMapGen::Proceed(CGenLayer *pTiles, int ConfigID)
{
	if(!m_FileLoaded || ConfigID < 0 || ConfigID >= m_lConfigs.size())
		return;

	CConfiguration *pConf = &m_lConfigs[ConfigID];

	if(!pConf->m_aIndexRules.size())
		return;

	int BaseTile = 1;

	// find base tile if there is one
	for(int i = 0; i < pConf->m_aIndexRules.size(); ++i)
	{
		if(pConf->m_aIndexRules[i].m_BaseTile)
		{
			BaseTile = pConf->m_aIndexRules[i].m_ID;
			break;
		}
	}
	
	int Width = m_pLayers->GameLayer()->m_Width;
	int Height = m_pLayers->GameLayer()->m_Height;
	
	// auto map !
	int MaxIndex = Width*Height;
	for (int l = 0; l < 3; l++)
		for (int y = 0; y < Height; y++)
			for (int x = 0; x < Width; x++)
			{
				if (pTiles->Get(x, y, l) == 0)
					continue;
				
				pTiles->Set(BaseTile, x, y, 0, l);

				if (y == 0 || y == Height-1 || x == 0 || x == Width-1)
					continue;

				for (int i = 0; i < pConf->m_aIndexRules.size(); ++i)
				{
					if (pConf->m_aIndexRules[i].m_BaseTile)
						continue;

					bool RespectRules = true;
					for (int j = 0; j < pConf->m_aIndexRules[i].m_aRules.size() && RespectRules; ++j)
					{
						CPosRule *pRule = &pConf->m_aIndexRules[i].m_aRules[j];
						int CheckIndex = (y+pRule->m_Y)*Width+(x+pRule->m_X);

						if (CheckIndex < 0 || CheckIndex >= MaxIndex)
							RespectRules = false;
						else
						{
							if (pRule->m_IndexValue)
							{
								if (pTiles->GetByIndex(CheckIndex, l) != pRule->m_Value)
									RespectRules = false;
							}
							else
							{
								if(pTiles->GetByIndex(CheckIndex, l) > 0 && pRule->m_Value == CPosRule::EMPTY)
									RespectRules = false;

								if(pTiles->GetByIndex(CheckIndex, l) == 0 && pRule->m_Value == CPosRule::FULL)
									RespectRules = false;
							}
						}
					}

					if (RespectRules &&
						(pConf->m_aIndexRules[i].m_YDivisor < 2 || y%pConf->m_aIndexRules[i].m_YDivisor == pConf->m_aIndexRules[i].m_YRemainder) &&
						(pConf->m_aIndexRules[i].m_RandomValue <= 1 || (int)((float)rand() / ((float)RAND_MAX + 1) * pConf->m_aIndexRules[i].m_RandomValue) == 1))
					{
						pTiles->Set(pConf->m_aIndexRules[i].m_ID, x, y, pConf->m_aIndexRules[i].m_Flag, l);
					}
				}
			}
}
