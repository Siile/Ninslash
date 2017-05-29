#include <random>

#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>
#include <engine/shared/config.h>
#include <engine/shared/linereader.h>

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
	
	Load("warehouse_main");
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

	char aBuf[256];

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

	//str_format(aBuf, sizeof(aBuf),"loaded %s", aPath);
	//m_pEditor->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "editor", aBuf);

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
		ModifTile(TilePos, m_pLayers->GetForegroundLayerIndex(), 0);
	}
	dbg_msg("mapgen", "map normalized in %.5fs", (float)(time_get()-ProcessTime)/time_freq());


	ProcessTime = time_get();
	GenerateLevel();
	
	dbg_msg("mapgen", "map successfully generated in %.5fs", (float)(time_get()-TotalTime)/time_freq());
}



void CMapGen::GenerateEnd(CGenLayer *pTiles)
{
	int w = pTiles->Width();
	int h = pTiles->Height();
	
	// find a platform
	for(int x = w-3; x > 3; x--)
		for(int y = 3; y < h-3; y++)
		{
			if (!pTiles->Get(x-2, y) && !pTiles->Get(x-1, y) && !pTiles->Get(x, y) && !pTiles->Get(x+1, y) && !pTiles->Get(x+2, y) && !pTiles->Get(x+3, y) && 
				pTiles->Get(x-2, y+1) && pTiles->Get(x-1, y+1) && pTiles->Get(x, y+1) && pTiles->Get(x+1, y+1) && pTiles->Get(x+2, y+1) && pTiles->Get(x+3, y+1) &&
				!pTiles->Get(x, y-2) && !pTiles->Get(x, y-3) && !pTiles->Get(x, y-4) && !pTiles->Get(x, y-5))
			{
				ModifTile(ivec2(x, y), m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_DOOR1);
				
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
					for (int yy = -7; yy < 4000; yy++)
						pTiles->Set(1, x+xx, y+yy, 0, CGenLayer::BACKGROUND);
				
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
	
	if (frandom() < 0.3f && g_Config.m_SvMapGenLevel > 15)
		ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_POWERBARREL);
	else
		ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_BARREL);
}

void CMapGen::GenerateLightningWall(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetPlatform();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_LIGHTNINGWALL);
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
		ModifTile(ivec2(x, p.y), m_pLayers->GetForegroundLayerIndex(), 241, 0);
	}
	
	ModifTile(ivec2(p.x, p.y), m_pLayers->GetForegroundLayerIndex(), 240, 0);
	ModifTile(ivec2(p.z, p.y), m_pLayers->GetForegroundLayerIndex(), 242, 0);
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

void CMapGen::GenerateEnemySpawn(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetPlatform();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_ENEMYSPAWN);
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


void CMapGen::GenerateSpeaker(CGenLayer *pTiles)
{
	ivec2 p = pTiles->GetTopCorner();
	
	if (p.x == 0)
		return;
	
	ModifTile(p, m_pLayers->GetGameLayerIndex(), ENTITY_OFFSET+ENTITY_SPEAKER);
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
	
	// horizontal line
	if (Type == -2)
	{
		int y = h * (0.2f + frandom()*0.6f);
		for (int x = 8; x < w-8; x++)
		{
			pRoom->Open(x-1, y-1);
			pRoom->Open(x, y);
			pRoom->Open(x+1, y+1);
		}

		return;
	}
	
	// ***********************************************
	
	if (h > w)
	{
		// top
		{
			int y = h * 0.1f;
				
			for (int x = 8; x < w-8; x++)
			{
				pRoom->Open(x-1, y-1);
				pRoom->Open(x, y);
				pRoom->Open(x+1, y+1);
			}
		}
			
		// bot
		{
			int y = h * 0.9f;
				
			for (int x = 8; x < w-8; x++)
			{
				pRoom->Open(x-1, y-1);
				pRoom->Open(x, y);
				pRoom->Open(x+1, y+1);
			}
		}
		
		// random horizontals
		for (int i = 0; i < 4; i++)
			if (frandom() < 0.25f)
				GenerateRooms(pRoom, -2, w, h);
		
		Type = rand()%4;
		
		// sine wave
		if (Type == 0)
		{
			int x = w / 2;
			
			float a = frandom()*pi*2;
			float c = frandom()*0.1f;
			float s = 2.5f + frandom()*2.0f;
			
			for (int y = 8; y < h-8; y++)
			{
				pRoom->Open(x-1+sin(a+y*c)*(1.0f*w/s), y-1);
				pRoom->Open(x+sin(a+y*c)*(1.0f*w/s), y);
				pRoom->Open(x+1+sin(a+y*c)*(1.0f*w/s), y+1);
			}
			
			if (frandom() < 0.5f)
				GenerateRooms(pRoom, -1, w, h);
			
			return;
		}
		
		// mix 1
		if (Type == 1)
		{
			// sine
			{
				int x = w / 2;
				
				float a = frandom()*pi*2;
				float c = 0.1f;
				float s = 2.5f + frandom()*2.0f;
				
				for (int y = 8; y < h-8; y++)
				{
					pRoom->Open(x-1+sin(a+y*c)*(1.0f*w/s), y-1);
					pRoom->Open(x+sin(a+y*c)*(1.0f*w/s), y);
					pRoom->Open(x+1+sin(a+y*c)*(1.0f*w/s), y+1);
				}
			}
			
			// line
			{
				int x = w * (0.2f + frandom()*0.6f);
				for (int y = 4; y < h-4; y++)
				{
					pRoom->Open(x-1, y-1);
					pRoom->Open(x, y);
					pRoom->Open(x+1, y+1);
				}
			}
			
			return;
		}
		
		// dual line
		if (Type == 2)
		{
			// line
			{
				int x = w * 0.15f;
				for (int y = 4; y < h-4; y++)
				{
					pRoom->Open(x-1, y-1);
					pRoom->Open(x, y);
					pRoom->Open(x+1, y+1);
				}
			}
			
			// line
			{
				int x = w * 0.85f;
				for (int y = 4; y < h-4; y++)
				{
					pRoom->Open(x-1, y-1);
					pRoom->Open(x, y);
					pRoom->Open(x+1, y+1);
				}
			}
			
			return;
		}
		
		// mix 2
		{
			// sine
			for (int i = 0; i < 2; i++)
			{
				int x = w / 2;
				
				float a = frandom()*pi*2;
				float c = 0.1f;
				float s = 2.5f + frandom()*2.0f;
				
				for (int y = 8; y < h-8; y++)
				{
					pRoom->Open(x-1+sin(a+y*c)*(1.0f*w/s), y-1);
					pRoom->Open(x+sin(a+y*c)*(1.0f*w/s), y);
					pRoom->Open(x+1+sin(a+y*c)*(1.0f*w/s), y+1);
				}
			}
			return;
		}
		
		return;
	}
	
	
	// *********************************************
	
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
	
	int n = pTiles->Size()/500;
	
	if (n > 1)
		pTiles->GenerateAirPlatforms(n/2 + rand()%(n/2));
	else
		pTiles->GenerateAirPlatforms(n);
	
	// finish pos
	GenerateEnd(pTiles);
	pTiles->GenerateBackground();

	Proceed(pTiles, 0);
	
	// write to layers; foreground
	for(int x = 0; x < w; x++)
		for(int y = 0; y < h; y++)
		{
			int i = pTiles->Get(x, y);
			
			if (i > 0)
			{
				ModifTile(ivec2(x, y), m_pLayers->GetForegroundLayerIndex(), i, pTiles->GetFlags(x, y));
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
	
	// enemy spawn positions
	for (int i = 0; i < 12; i++)
		GenerateEnemySpawn(pTiles);
	
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
	
	
	for (int i = 0; i < min(Level/3, 4) ; i++)
		GenerateSpeaker(pTiles);
	
	
	// pickups
	//for (int i = 0; i < (pTiles->Size()-Level*5)/700; i++)
	
	w = 2 + rand()%3 + (Level > 15 ? 1 : 0);
	
	for (int i = 0; i < w; i++)
		GenerateWeapon(pTiles, ENTITY_WEAPON_CHAINSAW+rand()%8);

	GenerateWeapon(pTiles, ENTITY_KIT);
	
	for (int i = 0; i < (pTiles->Size()-Level*5)/1200; i++)
		GenerateHearts(pTiles);
	
	
	
	for (int i = 0; i < (pTiles->Size()-Level*3)/900; i++)
		GenerateHearts(pTiles);
	
	// power upper(s)
	if (Level > 15 && frandom() < 0.15f)
		GeneratePowerupper(pTiles);
	if (Level > 5 && frandom() < 0.4f)
		GeneratePowerupper(pTiles);
	
	if (Level%3 == 0 || Level%5 == 0)
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
		case 3: GenerateMine(pTiles); break;
		case 4: GenerateFiretrap(pTiles); break;
		case 5: GenerateDeathray(pTiles); break;
		}
	}

	// more enemy spawn positions
	for (int i = 0; i < 12; i++)
		GenerateEnemySpawn(pTiles);
	
	if (pRoom)
		delete pRoom;
	
	if (pTiles)
		delete pTiles;
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
	for (int l = 0; l < 2; l++)
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
