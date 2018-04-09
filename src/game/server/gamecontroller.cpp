

#include <engine/shared/config.h>
#include <game/mapitems.h>

#include <game/generated/protocol.h>

#include "entities/bomb.h"
#include "entities/flag.h"
#include "entities/pickup.h"
#include "entities/weapon.h"
#include "entities/character.h"
#include "entities/turret.h"
#include "entities/teslacoil.h"
#include "entities/building.h"
#include "entities/deathray.h"
#include "entities/powerupper.h"
#include "entities/droid_walker.h"
#include "entities/droid_star.h"
#include "entities/droid_crawler.h"
#include "gamecontroller.h"
#include "gamecontext.h"


IGameController::IGameController(class CGameContext *pGameServer)
{
	m_pGameServer = pGameServer;
	m_pServer = m_pGameServer->Server();
	m_pGameType = "unknown";

	//
	DoWarmup(g_Config.m_SvWarmup);
	m_GameOverTick = -1;
	m_SuddenDeath = 0;
	m_RoundStartTick = Server()->Tick();
	m_RoundCount = 0;
	m_GameFlags = 0;
	m_aTeamscore[TEAM_RED] = 0;
	m_aTeamscore[TEAM_BLUE] = 0;
	m_aMapWish[0] = 0;

	m_UnbalancedTick = -1;
	m_ForceBalanced = false;

	m_RoundTimeLimit = 0;
	m_ResetTime = false;
	
	m_aNumSpawnPoints[0] = 0;
	m_aNumSpawnPoints[1] = 0;
	m_aNumSpawnPoints[2] = 0;
	
	m_SurvivalStatus = 0;
	m_SurvivalStartTick = Server()->Tick();
	m_SurvivalDeathTick = 0;
	m_ClearBroadcastTick = 0;
	
	// custom
	for (int i = 0; i < MAX_PICKUPS; i++)
		m_apPickup[i] = NULL;
	
	m_PickupCount = 0;
	m_PickupDropCount = 0;
	m_DroppablesCreated = false;
	
	GameServer()->Collision()->GenerateWaypoints();
	
	char aBuf[128]; str_format(aBuf, sizeof(aBuf), "%d waypoints generated, %d connections created", GameServer()->Collision()->WaypointCount(), GameServer()->Collision()->ConnectionCount());
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "ai", aBuf);
}

IGameController::~IGameController()
{
}


void IGameController::DropPickup(vec2 Pos, int PickupType, vec2 Force, int PickupSubtype, float Ammo, int PowerLevel)
{
	if (!g_Config.m_SvEnableBuilding && PickupType == POWERUP_KIT)
		PickupType = POWERUP_HEALTH;
	
	for (int i = 0; i < m_PickupCount; i++)
	{
		if (m_apPickup[i] && m_apPickup[i]->m_Dropable && m_apPickup[i]->m_Life <= 0 && m_apPickup[i]->GetType() == PickupType)
		{
			m_apPickup[i]->m_Pos = Pos;
			m_apPickup[i]->RespawnDropable();
			//if (m_apPickup[i]->GetType() == POWERUP_WEAPON)
				m_apPickup[i]->SetSubtype(PickupSubtype);
			
			m_apPickup[i]->m_Vel = Force;
			
			if (Ammo >= 0.0f)
				m_apPickup[i]->m_Ammo = Ammo;
			return;
		}
	}
}

void IGameController::DropWeapon(vec2 Pos, vec2 Force, CWeapon *pWeapon)
{
	if (!pWeapon)
		return;
	
	for (int i = 0; i < m_PickupCount; i++)
	{
		if (m_apPickup[i] && m_apPickup[i]->m_Dropable && m_apPickup[i]->m_Life <= 0 && m_apPickup[i]->GetType() == POWERUP_WEAPON)
		{
			m_apPickup[i]->m_Pos = Pos;
			m_apPickup[i]->RespawnDropable();
			m_apPickup[i]->m_pWeapon = pWeapon;
			m_apPickup[i]->SetSubtype(pWeapon->GetWeaponType());
			pWeapon->m_Disabled = true;
			
			m_apPickup[i]->m_Vel = Force;
			return;
		}
	}
}


void IGameController::SetPickup(vec2 Pos, int PickupType, int PickupSubtype, int Amount)
{
	vec2 P = Pos;
	int a = 1;
	for (int i = 0; i < m_PickupCount; i++)
	{
		if (m_apPickup[i] && m_apPickup[i]->m_Dropable && m_apPickup[i]->m_Life <= 0 && m_apPickup[i]->GetType() == PickupType)
		{
			m_apPickup[i]->m_Pos = P;
			m_apPickup[i]->RespawnTreasure();
			m_apPickup[i]->SetSubtype(PickupSubtype);
			if (a++ >= Amount)
				return;
			
			int d = 32;
			
			if (a == 2)
			{
				if (!GameServer()->Collision()->IsTileSolid(Pos.x - d, Pos.y))
					P = Pos + vec2(-d, 0);
				else if (!GameServer()->Collision()->IsTileSolid(Pos.x + d, Pos.y))
					P = Pos + vec2(+d, 0);
				else if (!GameServer()->Collision()->IsTileSolid(Pos.x , Pos.y - d))
					P = Pos + vec2(0, -d);
				else if (!GameServer()->Collision()->IsTileSolid(Pos.x , Pos.y + d))
					P = Pos + vec2(0, +d);
			}
			
			if (a == 3)
			{
				if (!GameServer()->Collision()->IsTileSolid(Pos.x , Pos.y + d))
					P = Pos + vec2(0, +d);
				else if (!GameServer()->Collision()->IsTileSolid(Pos.x , Pos.y - d))
					P = Pos + vec2(0, -d);
				else if (!GameServer()->Collision()->IsTileSolid(Pos.x + d, Pos.y))
					P = Pos + vec2(+d, 0);
				else if (!GameServer()->Collision()->IsTileSolid(Pos.x - d, Pos.y))
					P = Pos + vec2(-d, 0);
			}
		}
	}
}

int IGameController::GetRandomWeapon()
{
	return GetRandomWeaponType();
}

int IGameController::GetRandomModularWeapon()
{
	if (frandom() < 0.2f)
		return GetModularWeapon(5, 6+rand()%2);
	
	return GetModularWeapon(1+rand()%4, 1+rand()%4);
}

void IGameController::TriggerWeapon(class CWeapon *pWeapon)
{
	if (!pWeapon)
		return;
	
	CCharacter *p = GameServer()->GetPlayerChar(pWeapon->GetOwner());
	
	if (p)
		p->TriggerWeapon(pWeapon);
}

void IGameController::ReleaseWeapon(class CWeapon *pWeapon)
{
	if (!pWeapon)
		return;
	
	CCharacter *p = GameServer()->GetPlayerChar(pWeapon->GetOwner());
	
	if (p)
		p->ReleaseWeapon(pWeapon);
}

void IGameController::ClearPickups()
{
	for (int i = 0; i < m_PickupCount; i++)
	{
		if (m_apPickup[i])
			m_apPickup[i]->Hide();
	}
}

void IGameController::RespawnPickups()
{
	for (int i = 0; i < m_PickupCount; i++)
	{
		if (m_apPickup[i])
			m_apPickup[i]->Respawn();
	}
}

void IGameController::FlashPickups()
{
	for (int i = 0; i < m_PickupCount; i++)
	{
		if (m_apPickup[i] && !m_apPickup[i]->m_Dropable && m_apPickup[i]->m_SpawnTick <= 0)
			m_apPickup[i]->m_Flashing = true;
	}
}



float IGameController::EvaluateSpawnPos(CSpawnEval *pEval, vec2 Pos)
{
	float Score = 0.0f;
	CCharacter *pC = static_cast<CCharacter *>(GameServer()->m_World.FindFirst(CGameWorld::ENTTYPE_CHARACTER));
	for(; pC; pC = (CCharacter *)pC->TypeNext())
	{
		// team mates are not as dangerous as enemies
		float Scoremod = 1.0f;
		if(pEval->m_FriendlyTeam != -1 && pC->GetPlayer()->GetTeam() == pEval->m_FriendlyTeam)
			Scoremod = 0.5f;

		float d = distance(Pos, pC->m_Pos);
		Score += Scoremod * (d == 0 ? 1000000000.0f : 1.0f/d);
	}

	return Score;
}

void IGameController::EvaluateSpawnType(CSpawnEval *pEval, int Type)
{
	// get random spawn point
	
	//for(int i = 0; i < m_aNumSpawnPoints[Type]; i++)
	
	// let's start with a random instead
	int i = frandom()*m_aNumSpawnPoints[Type];
	for (int c = 0; c < m_aNumSpawnPoints[Type]; c++)
	{	
		i++;
		if (i >= m_aNumSpawnPoints[Type])
			i = 0;
	
		// check if the position is occupado
		CCharacter *aEnts[MAX_CLIENTS];
		int Num = GameServer()->m_World.FindEntities(m_aaSpawnPoints[Type][i], 64, (CEntity**)aEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
		vec2 Positions[5] = { vec2(0.0f, 0.0f), vec2(-32.0f, 0.0f), vec2(0.0f, -32.0f), vec2(32.0f, 0.0f), vec2(0.0f, 32.0f) };	// start, left, up, right, down
		int Result = -1;
		for(int Index = 0; Index < 5 && Result == -1; ++Index)
		{
			Result = Index;
			for(int c = 0; c < Num; ++c)
				if(GameServer()->Collision()->CheckPoint(m_aaSpawnPoints[Type][i]+Positions[Index]) ||
					distance(aEnts[c]->m_Pos, m_aaSpawnPoints[Type][i]+Positions[Index]) <= aEnts[c]->m_ProximityRadius)
				{
					Result = -1;
					break;
				}
		}
		if(Result == -1)
			continue;	// try next spawn point

		vec2 P = m_aaSpawnPoints[Type][i]+Positions[Result];
		float S = EvaluateSpawnPos(pEval, P);
		if(!pEval->m_Got || pEval->m_Score > S)
		{
			pEval->m_Got = true;
			pEval->m_Score = S;
			pEval->m_Pos = P;
			
			// let's be happy with the first one popping up
			break;
		}
	}
}

bool IGameController::CanSpawn(int Team, vec2 *pOutPos, bool IsBot)
{
	CSpawnEval Eval;

	// spectators can't spawn
	if(Team == TEAM_SPECTATORS)
		return false;

	if (IsTeamplay())
	{
		Eval.m_FriendlyTeam = Team;

		// first try own team spawn, then normal spawn and then enemy
		EvaluateSpawnType(&Eval, 1+(Team&1));
		if(!Eval.m_Got)
		{
			EvaluateSpawnType(&Eval, 0);
			if(!Eval.m_Got)
				EvaluateSpawnType(&Eval, 1+((Team+1)&1));
		}
	}
	else if (IsCoop())
	{
		if (IsBot)
		{
			//EvaluateSpawnType(&Eval, 1);
			if (GetSpawnPos(1, pOutPos))
				return true;
			
			return false;
		}
		else
			EvaluateSpawnType(&Eval, 0);
	}
	else
	{
		// pick random spawn point in dm, from any of the different types
		int i = rand()%3;
		
		for (int c = 0; c < 3; c++)
		{
			if (++i > 2)
				i = 0;
			
			EvaluateSpawnType(&Eval, i);
		}
		
		/*
		EvaluateSpawnType(&Eval, 0);
		EvaluateSpawnType(&Eval, 1);
		EvaluateSpawnType(&Eval, 2);
		*/
	}

	*pOutPos = Eval.m_Pos;
	return Eval.m_Got;
}




	
void IGameController::AutoBalance()
{
	// no bots
	if (g_Config.m_SvPreferredTeamSize == 0)
	{
		int Bots = 0;
		
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			CPlayer *pPlayer = GameServer()->m_apPlayers[i];
			if(!pPlayer)
				continue;
			
			if (pPlayer->m_IsBot)
				Bots++;
		}
		
		if (Bots > 0)
			GameServer()->KickBots();
		
		return;
	}
	
	if (!IsTeamplay() || IsInfection())
	{
		int Players = 0, Bots = 0, Spectators = 0;
		int BotID = -1;
		
		// count players
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			CPlayer *pPlayer = GameServer()->m_apPlayers[i];
			if(!pPlayer)
				continue;
			
			if (pPlayer->GetTeam() != TEAM_SPECTATORS)
			{
				if (!pPlayer->m_IsBot)
					Players++;
				else
				{
					BotID = i;
					Bots++;
				}
			}
			else
				Spectators++;
		}
		
		// kick bots if there's no players
		if (Players == 0 && Spectators == 0)
		{
			if (Bots > 0)
				GameServer()->KickBot(BotID);
			
			return;
		}
		

		// add bots
		if (Players + Bots < g_Config.m_SvPreferredTeamSize)
		{
			GameServer()->AddBot();
		}
		
		// kick bots
		if (Players + Bots > g_Config.m_SvPreferredTeamSize && Bots > 0)
		{
			GameServer()->KickBot(BotID);
		}
	}
	else
	// team play
	{
		int Red = 0, Blue = 0;
		int RedBots = 0, BlueBots = 0;
		
		int Spectators = 0;
		
		int RedBotID = -1;
		int BlueBotID = -1;
		
		
		// count players
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			CPlayer *pPlayer = GameServer()->m_apPlayers[i];
			if(!pPlayer)
				continue;

			if (pPlayer->GetTeam() == TEAM_RED)
			{
				if (!pPlayer->m_IsBot)
					Red++;
				else
				{
					RedBotID = i;
					RedBots++;
				}
			}
			
			if (pPlayer->GetTeam() == TEAM_BLUE)
			{
				if (!pPlayer->m_IsBot)
					Blue++;
				else
				{
					BlueBotID = i;
					BlueBots++;
				}
			}
			
			if (pPlayer->GetTeam() == TEAM_SPECTATORS)
				Spectators++;
		}
		
		
		// kick bots if there's no players
		if (Red + Blue + Spectators == 0)
		{
			if (RedBots + BlueBots > 0)
				GameServer()->KickBots();
			
			return;
		}

		// not enough players
		if ((Red+RedBots) < g_Config.m_SvPreferredTeamSize || (Blue+BlueBots) < g_Config.m_SvPreferredTeamSize)
			GameServer()->AddBot();

		
		// unbalanced teams
		if (Red+RedBots > Blue+BlueBots && Red+RedBots > g_Config.m_SvPreferredTeamSize && RedBots > 0)
			GameServer()->KickBot(RedBotID);
		if (Red+RedBots < Blue+BlueBots && Blue+BlueBots > g_Config.m_SvPreferredTeamSize && BlueBots > 0)
			GameServer()->KickBot(BlueBotID);
		
		if (Red+RedBots == Blue+BlueBots && Red+RedBots > g_Config.m_SvPreferredTeamSize && RedBots > 0 && BlueBots > 0)
		{
			GameServer()->KickBot(RedBotID);
			GameServer()->KickBot(BlueBotID);
		}
	}
}





bool IGameController::OnNonPickupEntity(int Index, vec2 Pos)
{
	if(Index == ENTITY_SPAWN)
		m_aaSpawnPoints[0][m_aNumSpawnPoints[0]++] = Pos;
	else if(Index == ENTITY_SPAWN_RED)
		m_aaSpawnPoints[1][m_aNumSpawnPoints[1]++] = Pos;
	else if(Index == ENTITY_SPAWN_BLUE)
		m_aaSpawnPoints[2][m_aNumSpawnPoints[2]++] = Pos;

	return false;
}


void IGameController::CreateDroppables()
{
	for (int i = 0; i < MAX_DROPPABLES; i++)
	{
		// hearts
		m_apPickup[m_PickupCount] = new CPickup(&GameServer()->m_World, POWERUP_HEALTH, 0);
		m_apPickup[m_PickupCount]->m_Pos = vec2(0, 0);
		m_apPickup[m_PickupCount]->m_Dropable = true;
		m_PickupCount++;

		// armors
		m_apPickup[m_PickupCount] = new CPickup(&GameServer()->m_World, POWERUP_AMMO, 0);
		m_apPickup[m_PickupCount]->m_Pos = vec2(0, 0);
		m_apPickup[m_PickupCount]->m_Dropable = true;
		m_PickupCount++;
		
		// mines
		m_apPickup[m_PickupCount] = new CPickup(&GameServer()->m_World, POWERUP_ARMOR, 0);
		m_apPickup[m_PickupCount]->m_Pos = vec2(0, 0);
		m_apPickup[m_PickupCount]->m_Dropable = true;
		m_PickupCount++;
		
		// kits
		m_apPickup[m_PickupCount] = new CPickup(&GameServer()->m_World, POWERUP_KIT, 0);
		m_apPickup[m_PickupCount]->m_Pos = vec2(0, 0);
		m_apPickup[m_PickupCount]->m_Dropable = true;
		m_PickupCount++;
		
		// weapons
		m_apPickup[m_PickupCount] = new CPickup(&GameServer()->m_World, POWERUP_WEAPON, 0);
		m_apPickup[m_PickupCount]->m_Pos = vec2(0, 0);
		m_apPickup[m_PickupCount]->m_Dropable = true;
		m_PickupCount++;
	}
	
	m_DroppablesCreated = true;
}



void IGameController::DeathMessage()
{
	switch (rand()%5)
	{
		case 0:
			GameServer()->SendBroadcast("All hope is lost", -1); break;
		case 1:
			GameServer()->SendBroadcast("Slaughter", -1); break;
		case 2:
			GameServer()->SendBroadcast("Ocean of blood", -1); break;
		case 3:
			GameServer()->SendBroadcast("Death takes all", -1); break;
		default:
			GameServer()->SendBroadcast("Everybody dies", -1); break;
	};
}
	
	
void IGameController::TriggerSwitch(vec2 Pos)
{
	TriggerEscape();
	
	if (str_comp(g_Config.m_SvGametype, "coop") == 0 && g_Config.m_SvMapGenLevel%10 == 9)
	{
		m_SurvivalStartTick = Server()->Tick();
		g_Config.m_SvSurvivalTime = 10;
	}
		
	
	/*
	float Radius = 1000;
	
	CBuilding *apEnts[99];
	int Num = GameServer()->m_World.FindEntities(Pos, Radius, (CEntity**)apEnts,
													99, CGameWorld::ENTTYPE_BUILDING);
	for (int i = 0; i < Num; ++i)
	{
		CBuilding *pTarget = apEnts[i];
		pTarget->Trigger();
	}
	*/
}


void IGameController::TriggerEscape()
{
	float Radius = 1000000;
	
	CBuilding *apEnts[999];
	int Num = GameServer()->m_World.FindEntities(vec2(4000, 4000), Radius, (CEntity**)apEnts,
													999, CGameWorld::ENTTYPE_BUILDING);
	for (int i = 0; i < Num; ++i)
	{
		CBuilding *pTarget = apEnts[i];
		if (pTarget->m_Type == BUILDING_DOOR1)
			pTarget->Trigger();
	}
}


void IGameController::NextLevel(int CID)
{
	//
}



bool IGameController::OnEntity(int Index, vec2 Pos)
{
	int Type = -1;
	int SubType = 0;
	
	
	//if(IGameController::OnNonPickupEntity(Index, Pos))
	//	return true;

	if (!m_DroppablesCreated)
		CreateDroppables();
	
	// buildings
	if (Index == ENTITY_SAWBLADE)
	{
		new CBuilding(&GameServer()->m_World, Pos, BUILDING_SAWBLADE, TEAM_NEUTRAL);
		return true;
	}
	else if (Index == ENTITY_MINE1)
	{
		new CBuilding(&GameServer()->m_World, Pos+vec2(0,6), BUILDING_MINE1, TEAM_NEUTRAL);
		return true;
	}
	else if (Index == ENTITY_MINE2)
	{
		new CBuilding(&GameServer()->m_World, Pos+vec2(0,6), BUILDING_MINE2, TEAM_NEUTRAL);
		return true;
	}
	else if (Index == ENTITY_BARREL)
	{
		new CBuilding(&GameServer()->m_World, Pos+vec2(0,-12), BUILDING_BARREL, TEAM_NEUTRAL);
		return true;
	}
	else if (Index == ENTITY_POWERBARREL)
	{
		new CBuilding(&GameServer()->m_World, Pos+vec2(0,-12), BUILDING_POWERBARREL, TEAM_NEUTRAL);
		return true;
	}
	else if (Index == ENTITY_LIGHTNINGWALL)
	{
		new CBuilding(&GameServer()->m_World, Pos, BUILDING_LIGHTNINGWALL, TEAM_NEUTRAL);
		return true;
	}
	else if (Index == ENTITY_REACTOR)
	{
		new CBuilding(&GameServer()->m_World, Pos, BUILDING_REACTOR, TEAM_NEUTRAL);
		return true;
	}
	else if (Index == ENTITY_TESLACOIL)
	{
		new CTeslacoil(&GameServer()->m_World, Pos, TEAM_NEUTRAL);
		return true;
	}
	else if (Index == ENTITY_SCREEN)
	{
		new CBuilding(&GameServer()->m_World, Pos, BUILDING_SCREEN, TEAM_NEUTRAL);
		return true;
	}
	else if (Index == ENTITY_LAZER)
	{
		new CDeathray(&GameServer()->m_World, Pos+vec2(0, -20));
		return true;
	}
	else if (Index == ENTITY_POWERUPPER)
	{
		new CPowerupper(&GameServer()->m_World, Pos+vec2(0, 12));
		return true;
	}
	else if (Index == ENTITY_STAND)
	{
		if (g_Config.m_SvEnableBuilding)
			new CBuilding(&GameServer()->m_World, Pos+vec2(0, -10), BUILDING_STAND, TEAM_NEUTRAL);
		return true;
	}
	else if (Index == ENTITY_TURRET)
	{
		new CTurret(&GameServer()->m_World, Pos+vec2(0, -10), TEAM_NEUTRAL, GameServer()->NewWeapon(GetModularWeapon(1, 1)));
		return true;
	}
	else if (Index == ENTITY_TESLACOIL)
	{
		new CTeslacoil(&GameServer()->m_World, Pos+vec2(0, -10), TEAM_NEUTRAL);
		return true;
	}
	else if (Index == ENTITY_SWITCH)
	{
		new CBuilding(&GameServer()->m_World, Pos+vec2(0, -10), BUILDING_SWITCH, TEAM_NEUTRAL);
		return true;
	}
	else if (Index == ENTITY_DOOR1)
	{
		new CBuilding(&GameServer()->m_World, Pos, BUILDING_DOOR1, TEAM_NEUTRAL);
		return true;
	}
	else if (Index == ENTITY_SPEAKER)
	{
		new CBuilding(&GameServer()->m_World, Pos, BUILDING_SPEAKER, TEAM_NEUTRAL);
		return true;
	}
	else if (Index == ENTITY_JUMPPAD)
	{
		new CBuilding(&GameServer()->m_World, Pos, BUILDING_JUMPPAD, TEAM_NEUTRAL);
		return true;
	}
	else if (Index == ENTITY_FLAMETRAP_RIGHT || Index == ENTITY_FLAMETRAP_LEFT)
	{
		CBuilding *pFlametrap = new CBuilding(&GameServer()->m_World, Pos, BUILDING_FLAMETRAP, TEAM_NEUTRAL);
		
		if (Index == ENTITY_FLAMETRAP_LEFT)
			pFlametrap->m_Mirror = true;
		return true;
	}
	else if (Index == ENTITY_DROID_WALKER)
	{
		new CWalker(&GameServer()->m_World, Pos+vec2(0, 16));
		return true;
	}
	else if (Index == ENTITY_DROID_STAR)
	{
		new CStar(&GameServer()->m_World, Pos+vec2(0, -80));
		return true;
	}
	else if (Index == ENTITY_DROID_CRAWLER)
	{
		new CCrawler(&GameServer()->m_World, Pos+vec2(0, -40));
		return true;
	}
	
	if(Index == ENTITY_SPAWN)
		m_aaSpawnPoints[0][m_aNumSpawnPoints[0]++] = Pos;
	else if(Index == ENTITY_SPAWN_RED)
		m_aaSpawnPoints[1][m_aNumSpawnPoints[1]++] = Pos;
	else if(Index == ENTITY_SPAWN_BLUE)
		m_aaSpawnPoints[2][m_aNumSpawnPoints[2]++] = Pos;
	else
	if (g_Config.m_SvVanillaPickups)
	{
		if(Index == ENTITY_AMMO_1)
			Type = POWERUP_AMMO;
		else if(Index == ENTITY_HEALTH_1)
			Type = POWERUP_HEALTH;
		else if(Index == ENTITY_ARMOR_1)
			Type = POWERUP_ARMOR;
		else if(Index == ENTITY_KIT)
		{
			if (g_Config.m_SvEnableBuilding)
				Type = POWERUP_KIT;
			else
				Type = POWERUP_AMMO;
		}
		else if(Index == ENTITY_WEAPON_CHAINSAW)
		{
			Type = POWERUP_WEAPON;
			SubType = WEAPON_CHAINSAW;
		}
		else if(Index == ENTITY_WEAPON_SHOTGUN)
		{
			Type = POWERUP_WEAPON;
			SubType = WEAPON_SHOTGUN;
		}
		else if(Index == ENTITY_WEAPON_GRENADE)
		{
			Type = POWERUP_WEAPON;
			SubType = WEAPON_GRENADE;
		}
		else if(Index == ENTITY_WEAPON_RIFLE)
		{
			Type = POWERUP_WEAPON;
			SubType = WEAPON_RIFLE;
		}
		else if(Index == ENTITY_WEAPON_ELECTRIC)
		{
			Type = POWERUP_WEAPON;
			SubType = WEAPON_ELECTRIC;
		}
		else if(Index == ENTITY_WEAPON_LASER)
		{
			Type = POWERUP_WEAPON;
			SubType = WEAPON_LASER;
		}
		else if(Index == ENTITY_WEAPON_FLAMER)
		{
			Type = POWERUP_WEAPON;
			SubType = WEAPON_FLAMER;
		}
		else if(Index == ENTITY_WEAPON_SCYTHE)
		{
			Type = POWERUP_WEAPON;
			SubType = WEAPON_SCYTHE;
		}
	}

	if(Type != -1)
	{
		/*
		if (g_Config.m_SvForceWeapon)
		{
			if (Type == POWERUP_WEAPON || Type == POWERUP_AMMO)
				return true;
		}
		*/
		
		if (Type == POWERUP_WEAPON)
			SubType = GetRandomWeapon();
		
		CPickup *pPickup = new CPickup(&GameServer()->m_World, Type, SubType);
		pPickup->m_Pos = Pos;
		
		return true;
	}

	return false;
}


int IGameController::CountHumans()
{
	int Num = 0;
		
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		if (!pPlayer->m_IsBot)
			Num++;
	}
	
	return Num;
}


int IGameController::CountPlayers(int Team)
{
	int Num = 0;
		
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		if (pPlayer->GetTeam() != TEAM_SPECTATORS)
		{
			if (pPlayer->GetTeam() == Team || Team == -1)
				Num++;
		}
	}
	
	return Num;
}


int IGameController::CountPlayersAlive(int Team, bool IgnoreBots)
{
	int Num = 0;
		
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		if (pPlayer->GetTeam() != TEAM_SPECTATORS)
		{
			if (pPlayer->GetTeam() == Team || Team == -1)
			{
				if (pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive() && (!IgnoreBots || !pPlayer->m_IsBot))
					Num++;
			}
		}
	}
	
	return Num;
}


int IGameController::GetAliveCID(int Team)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		if (pPlayer->GetTeam() != TEAM_SPECTATORS)
		{
			if (pPlayer->GetTeam() == Team || Team == -1)
			{
				if (pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())
					return i;
			}
		}
	}
	
	return -1;
}


int IGameController::CountBots()
{
	int Num = 0;
		
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		if (pPlayer->m_IsBot)
			Num++;
	}
	
	return Num;
}

int IGameController::CountBotsAlive()
{
	int Num = 0;
		
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		if (pPlayer->m_IsBot && pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())
			Num++;
	}
	
	return Num;
}


void IGameController::EndRound()
{
	if(m_Warmup) // game can't end when we are running warmup
		return;

	GameServer()->m_World.m_Paused = true;
	m_GameOverTick = Server()->Tick();
	m_SuddenDeath = 0;
}

void IGameController::ResetGame()
{
	GameServer()->m_World.m_ResetRequested = true;
}

int IGameController::GetLockedWeapon(CCharacter *pCharacter)
{
	if(IsInfection() && pCharacter->GetPlayer()->GetTeam() == TEAM_BLUE)
		return WEAPON_CHAINSAW;
	return -1;
}

const char *IGameController::GetTeamName(int Team)
{
	if(IsTeamplay())
	{
		if (IsInfection())
		{
			if(Team == TEAM_RED)
				return "the living";
			else if(Team == TEAM_BLUE)
				return "the dead";
		}
		else
		{
			if(Team == TEAM_RED)
				return "red team";
			else if(Team == TEAM_BLUE)
				return "blue team";
		}
	}
	else
	{
		if(Team == 0)
			return "game";
	}

	return "spectators";
}

static bool IsSeparator(char c) { return c == ';' || c == ' ' || c == ',' || c == '\t'; }

void IGameController::StartRound()
{
	ResetGame();

	m_SurvivalStartTick = Server()->Tick();
	m_RoundStartTick = Server()->Tick();
	m_SuddenDeath = 0;
	m_GameOverTick = -1;
	GameServer()->m_World.m_Paused = false;
	m_aTeamscore[TEAM_RED] = 0;
	m_aTeamscore[TEAM_BLUE] = 0;
	m_ForceBalanced = false;
	Server()->DemoRecorder_HandleAutoStart();
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "start round type='%s' teamplay='%d'", m_pGameType, m_GameFlags&GAMEFLAG_TEAMS);
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
}

void IGameController::ChangeMap(const char *pToMap)
{
	str_copy(m_aMapWish, pToMap, sizeof(m_aMapWish));
	EndRound();
}

void IGameController::CycleMap()
{
	if(m_aMapWish[0] != 0)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "rotating map to %s", m_aMapWish);
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
		str_copy(g_Config.m_SvMap, m_aMapWish, sizeof(g_Config.m_SvMap));
		m_aMapWish[0] = 0;
		m_RoundCount = 0;
		return;
	}
	if(!str_length(g_Config.m_SvMaprotation))
		return;

	if(m_RoundCount < g_Config.m_SvRoundsPerMap-1)
	{
		if(g_Config.m_SvRoundSwap)
			GameServer()->SwapTeams();
		return;
	}

	// handle maprotation
	const char *pMapRotation = g_Config.m_SvMaprotation;
	const char *pCurrentMap = g_Config.m_SvMap;
	
	if (strcmp(g_Config.m_SvMap, "generated") == 0)
		pCurrentMap = g_Config.m_SvInvMap;
	
	int CurrentMapLen = str_length(pCurrentMap);
	const char *pNextMap = pMapRotation;
	while(*pNextMap)
	{
		int WordLen = 0;
		while(pNextMap[WordLen] && !IsSeparator(pNextMap[WordLen]))
			WordLen++;

		if(WordLen == CurrentMapLen && str_comp_num(pNextMap, pCurrentMap, CurrentMapLen) == 0)
		{
			// map found
			pNextMap += CurrentMapLen;
			while(*pNextMap && IsSeparator(*pNextMap))
				pNextMap++;

			break;
		}

		pNextMap++;
	}

	// restart rotation
	if(pNextMap[0] == 0)
		pNextMap = pMapRotation;

	// cut out the next map
	char aBuf[512] = {0};
	for(int i = 0; i < 511; i++)
	{
		aBuf[i] = pNextMap[i];
		if(IsSeparator(pNextMap[i]) || pNextMap[i] == 0)
		{
			aBuf[i] = 0;
			break;
		}
	}

	// skip spaces
	int i = 0;
	while(IsSeparator(aBuf[i]))
		i++;

	m_RoundCount = 0;

	char aBufMsg[256];
	str_format(aBufMsg, sizeof(aBufMsg), "rotating map to %s", &aBuf[i]);
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
	str_copy(g_Config.m_SvMap, &aBuf[i], sizeof(g_Config.m_SvMap));
}


void IGameController::FirstMap()
{
	g_Config.m_SvMapGenLevel = 1;
	g_Config.m_SvInvFails = 0;
	
	//Server()->ResetPlayerData();
	
	if(m_aMapWish[0] != 0)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "rotating map to %s", m_aMapWish);
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
		str_copy(g_Config.m_SvMap, m_aMapWish, sizeof(g_Config.m_SvMap));
		m_aMapWish[0] = 0;
		m_RoundCount = 0;
		return;
	}
	if(!str_length(g_Config.m_SvMaprotation))
		return;

	if(m_RoundCount < g_Config.m_SvRoundsPerMap-1)
	{
		if(g_Config.m_SvRoundSwap)
			GameServer()->SwapTeams();
		return;
	}

	// handle maprotation
	const char *pMapRotation = g_Config.m_SvMaprotation;
	const char *pCurrentMap = g_Config.m_SvMap;

	int CurrentMapLen = str_length(pCurrentMap);
	const char *pNextMap = pMapRotation;
	while(*pNextMap)
	{
		int WordLen = 0;
		while(pNextMap[WordLen] && !IsSeparator(pNextMap[WordLen]))
			WordLen++;

		if(WordLen == CurrentMapLen && str_comp_num(pNextMap, pCurrentMap, CurrentMapLen) == 0)
		{
			// map found
			pNextMap += CurrentMapLen;
			while(*pNextMap && IsSeparator(*pNextMap))
				pNextMap++;

			break;
		}

		pNextMap++;
	}

	// restart rotation
	//if(pNextMap[0] == 0)
		pNextMap = pMapRotation;

	// cut out the next map
	char aBuf[512] = {0};
	for(int i = 0; i < 511; i++)
	{
		aBuf[i] = pNextMap[i];
		if(IsSeparator(pNextMap[i]) || pNextMap[i] == 0)
		{
			aBuf[i] = 0;
			break;
		}
	}

	// skip spaces
	int i = 0;
	while(IsSeparator(aBuf[i]))
		i++;

	m_RoundCount = 0;

	char aBufMsg[256];
	str_format(aBufMsg, sizeof(aBufMsg), "restarting map rotating to %s", &aBuf[i]);
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
	str_copy(g_Config.m_SvMap, &aBuf[i], sizeof(g_Config.m_SvMap));
}


void IGameController::PostReset()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			GameServer()->m_apPlayers[i]->Respawn();
			GameServer()->m_apPlayers[i]->m_Score = 0;
			GameServer()->m_apPlayers[i]->m_ScoreStartTick = Server()->Tick();
			GameServer()->m_apPlayers[i]->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()/2;
		}
	}
}

void IGameController::OnPlayerInfoChange(class CPlayer *pP)
{
	const int aTeamColors[2] = {2555648, 8912640};
	const int aTeamFeetColors[2] = {65280, 10354432};
	
	if(IsTeamplay())
	{
		if (IsInfection())
		{
			if (pP->GetTeam() == TEAM_BLUE)
			{
				//pP->m_TeeInfos.m_ColorBody = aInfectedColor;
				//pP->m_TeeInfos.m_ColorFeet = aInfectedColor;
			}
		}
		else if(pP->GetTeam() >= TEAM_RED && pP->GetTeam() <= TEAM_BLUE)
		{
			pP->m_TeeInfos.m_ColorBody = aTeamColors[pP->GetTeam()];
			pP->m_TeeInfos.m_ColorFeet = aTeamFeetColors[pP->GetTeam()];
		}
		else
		{
			pP->m_TeeInfos.m_ColorBody = 12895054;
			pP->m_TeeInfos.m_ColorFeet = 12895054;
		}
	}
}



int IGameController::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	if (g_Config.m_SvSurvivalMode)
	{
		// update spectator modes
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->m_SpectatorID == pVictim->GetPlayer()->GetCID())
			{
				GameServer()->m_apPlayers[i]->m_LastSetSpectatorMode = Server()->Tick() - Server()->TickSpeed()*(g_Config.m_SvSpectatorUpdateTime-1);
				GameServer()->m_apPlayers[i]->m_SpectatorID = SPEC_FREEVIEW;
			}
		}
	}
	
	if (g_Config.m_SvSurvivalMode && Weapon != WEAPON_GAME)
	{
		if (!IsCoop() || !pVictim->m_IsBot)
			m_SurvivalStatus = SURVIVAL_NOCANDO;
		
		// wait a second before ending the round if it's going to end
		m_SurvivalDeathTick = Server()->Tick() + Server()->TickSpeed()*1.0f;
	}
	
	// pickup drops
	if (g_Config.m_SvPickupDrops && Weapon != WEAPON_GAME)
	{
		// drop stuff on death
		DropPickup(pVictim->m_Pos, POWERUP_HEALTH, pVictim->m_LatestHitVel+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);

		if (pVictim->m_Kits > 0)
			DropPickup(pVictim->m_Pos, POWERUP_KIT, pVictim->m_LatestHitVel+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);

		if (pVictim->HasAmmo())
			DropPickup(pVictim->m_Pos, POWERUP_AMMO, pVictim->m_LatestHitVel+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);

		if (frandom() < 0.5f)
			DropPickup(pVictim->m_Pos, POWERUP_AMMO, pVictim->m_LatestHitVel+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);
			
		if (pVictim->GetArmor() > 0)
			DropPickup(pVictim->m_Pos, POWERUP_ARMOR, pVictim->m_LatestHitVel+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);

	}
	
	if (pVictim->GetWeapon())
		pVictim->GetWeapon()->OnOwnerDeath(true);

	// weapon drops
	if (g_Config.m_SvWeaponDrops)
		pVictim->DropWeapon();
	
	pVictim->ReleaseWeapons();
	
	// for active spectator mode
	if(pKiller && (pKiller->GetTeam() != pVictim->GetPlayer()->GetTeam() || !IsTeamplay()))
	{
		//pKiller->m_Score++;
		pKiller->m_InterestPoints += 60;
	}
	
	// give or take scores
	if(!pKiller || Weapon == WEAPON_GAME)
		return 0;
	if(pKiller == pVictim->GetPlayer())
	{
		if (!(IsInfection() && pVictim->GetPlayer()->GetTeam() == TEAM_BLUE) && g_Config.m_SvSelfKillPenalty)
			pVictim->GetPlayer()->m_Score--; // suicide
	}
	else
	{
		if(IsTeamplay() && pVictim->GetPlayer()->GetTeam() == pKiller->GetTeam())
		{
			if (g_Config.m_SvSelfKillPenalty)
				pKiller->m_Score--; // teamkill
		}
		else
			pKiller->m_Score++; // normal kill
	}
	
	return 0;
}


bool IGameController::GetSpawnPos(int Team, vec2 *pOutPos)
{
	//
	return false;
}

void IGameController::OnCharacterSpawn(class CCharacter *pChr, bool RequestAI)
{	
	// default health
	pChr->SetHealth(100);

	if (pChr->GetPlayer()->m_pAI)
		pChr->GetPlayer()->m_pAI->Reset();
}





vec2 IGameController::GetFlagPos(int Team)
{
	return vec2(0, 0);
}

int IGameController::GetFlagState(int Team)
{
	return -1;
}



// CSTT & CSBB
int IGameController::GetDefendingTeam()
{
	return -1;
}

CBomb *IGameController::GetBomb()
{
	return NULL;
}

CFlag *IGameController::GetClosestBase(vec2 Pos, int Team)
{
	return NULL;
}

CFlag *IGameController::GetUndefendedBase(int Team)
{
	return NULL;
}

int IGameController::Defenders(CFlag *Base)
{
	return 0;
}

int IGameController::CountBases(int Team)
{
	return 0;
}

CFlag *IGameController::GetRandomBase(int NotThisTeam)
{
	return NULL;
}



bool IGameController::CanCharacterSpawn(int ClientID)
{
	if (g_Config.m_SvSurvivalMode)
	{
		if (m_SurvivalStatus == SURVIVAL_CANJOIN)
			return true;
		
		if (IsCoop())
		{
			CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
				
			if(pPlayer && pPlayer->m_IsBot)
				return true;
		}
		
		return false;
	}
	
	return true;
}

void IGameController::DoWarmup(int Seconds)
{
	if(Seconds < 0)
		m_Warmup = 0;
	else
		m_Warmup = Seconds*Server()->TickSpeed();
}

bool IGameController::IsFriendlyFire(int ClientID1, int ClientID2)
{
	if(ClientID1 == ClientID2)
		return false;

	if (ClientID2 < 0)
	{
		if(!GameServer()->m_apPlayers[ClientID1])
			return false;
		
		if (IsTeamplay())
		{
			if(GameServer()->m_apPlayers[ClientID1]->GetTeam() == TEAM_RED && ClientID2 == RED_BASE)
				return true;
			
			if(GameServer()->m_apPlayers[ClientID1]->GetTeam() == TEAM_BLUE && ClientID2 == BLUE_BASE)
				return true;
		}
		
		return false;
	}

	if(IsTeamplay() || g_Config.m_SvDisablePVP)
	{	
		if(!GameServer()->m_apPlayers[ClientID1] || !GameServer()->m_apPlayers[ClientID2])
			return false;

		if (g_Config.m_SvDisablePVP && !GameServer()->IsBot(ClientID1) && !GameServer()->IsBot(ClientID2))
			return true;
		
		if (IsTeamplay())
		{
			if(GameServer()->m_apPlayers[ClientID1]->GetTeam() == GameServer()->m_apPlayers[ClientID2]->GetTeam())
				return true;
		}
	}
	
	return false;
}

bool IGameController::IsForceBalanced()
{
	if(m_ForceBalanced)
	{
		m_ForceBalanced = false;
		return true;
	}
	else
		return false;
}

bool IGameController::CanBeMovedOnBalance(int ClientID)
{
	return true;
}

void IGameController::NewSurvivalRound()
{
	
}
	
	
void IGameController::ResetSurvivalRound()
{
	KillEveryone();
	NewSurvivalRound();
	m_ClearBroadcastTick = Server()->Tick() + Server()->TickSpeed()*2;
	m_SurvivalStartTick = Server()->Tick();
	m_SurvivalStatus = 0;
	
	// reset pickups
	CPickup *apEnts[4000];
	int Num = GameServer()->m_World.FindEntities(vec2(0, 0), 0.0f, (CEntity**)apEnts, 4000, CGameWorld::ENTTYPE_PICKUP);

	for (int i = 0; i < Num; ++i)
		apEnts[i]->SurvivalReset();
}

void IGameController::KillEveryone()
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		if (pPlayer->GetTeam() != TEAM_SPECTATORS)
		{
			if (pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())
				pPlayer->GetCharacter()->Die(-1, WEAPON_GAME);
		}
	}
}


void IGameController::Tick()
{
	// do warmup
	if(m_Warmup)
	{
		m_Warmup--;
		if(!m_Warmup)
			StartRound();
	}
	
	GameServer()->UpdateSpectators();

	if(m_GameOverTick != -1)
	{
		// game over.. wait for restart
		if(Server()->Tick() > m_GameOverTick+Server()->TickSpeed()*(IsCoop() ? 1 : 5))
		{
			GameServer()->KickBots();
			CycleMap();
			GameServer()->SwapTeams();
			StartRound();
			m_RoundCount++;
		}
	}
	
	// clear / interrupt broadcast
	if (m_ClearBroadcastTick && m_ClearBroadcastTick < Server()->Tick())
	{
		m_ClearBroadcastTick = 0;
		GameServer()->SendBroadcast("", -1);
	}

	
	// survival mode
	
	// force survival mode off in some gamemodes
	if (g_Config.m_SvSurvivalMode && (IsInfection()))
		g_Config.m_SvSurvivalMode = 0;
	
	// check for round time ending
	if (!g_Config.m_SvSurvivalAcid && g_Config.m_SvSurvivalTime && g_Config.m_SvSurvivalMode && m_SurvivalStartTick < Server()->Tick() - Server()->TickSpeed() * g_Config.m_SvSurvivalTime)
	{
		GameServer()->SendBroadcast("Draw", -1);
		ResetSurvivalRound();
	}
	
	// global acid level
	if (g_Config.m_SvSurvivalAcid && g_Config.m_SvSurvivalMode && g_Config.m_SvSurvivalTime && !m_Warmup)
	{
		GameServer()->Collision()->m_GlobalAcid = true;
		GameServer()->Collision()->m_Time = g_Config.m_SvSurvivalTime*Server()->TickSpeed() - ((Server()->Tick()-m_SurvivalStartTick));
	}
	else 
		GameServer()->Collision()->m_GlobalAcid = false;
				
	// check for winning conditions
	if (!IsCoop() && g_Config.m_SvSurvivalMode && m_SurvivalStatus == SURVIVAL_NOCANDO && m_SurvivalDeathTick < Server()->Tick())
	{
		// check if only the last player (or the team) alive
		if (IsTeamplay())
		{
			// draw
			if (!CountPlayersAlive(TEAM_BLUE) && !CountPlayersAlive(TEAM_RED))
			{
				GameServer()->SendBroadcast("Draw", -1);
				ResetSurvivalRound();
			}
			// red team wins
			else if (!CountPlayersAlive(TEAM_BLUE) && CountPlayersAlive(TEAM_RED))
			{
				GameServer()->SendBroadcast("Red team wins", -1);
				m_aTeamscore[TEAM_RED] += g_Config.m_SvSurvivalReward;
				ResetSurvivalRound();
			}
			// blue team wins
			else if (CountPlayersAlive(TEAM_BLUE) && !CountPlayersAlive(TEAM_RED))
			{
				GameServer()->SendBroadcast("Blue team wins", -1);
				m_aTeamscore[TEAM_BLUE] += g_Config.m_SvSurvivalReward;
				ResetSurvivalRound();
			}
			
		}
		else
		{
			// no one wins
			if (!CountPlayersAlive())
			{
				GameServer()->SendBroadcast("Draw", -1);
				ResetSurvivalRound();
			}
				
			// a winner!
			if (CountPlayersAlive() == 1)
			{
				int Winner = GetAliveCID();
				
				if (Winner >= 0)
				{
					char aBuf[64];
					str_format(aBuf, sizeof(aBuf), "%s survives", Server()->ClientName(Winner));
					GameServer()->SendBroadcast(aBuf, -1);
					
					GameServer()->m_apPlayers[Winner]->m_Score += g_Config.m_SvSurvivalReward;
				}
				
				ResetSurvivalRound();
			}
		}
	}
	
	// game is Paused
	if(GameServer()->m_World.m_Paused)
		++m_RoundStartTick;


	// check for inactive players
	if(g_Config.m_SvInactiveKickTime > 0)
	{
		// dont kick bots
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
		#ifdef CONF_DEBUG
			if(g_Config.m_DbgDummies)
			{
				if(i >= MAX_CLIENTS-g_Config.m_DbgDummies)
					break;
			}
		#endif
		
			if(GameServer()->m_apPlayers[i] && !GameServer()->IsBot(i) && GameServer()->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS && !Server()->IsAuthed(i))
			{
				if(Server()->Tick() > GameServer()->m_apPlayers[i]->m_LastActionTick+g_Config.m_SvInactiveKickTime*Server()->TickSpeed()*60)
				{
					switch(g_Config.m_SvInactiveKick)
					{
					case 0:
						{
							// move player to spectator
							GameServer()->m_apPlayers[i]->SetTeam(TEAM_SPECTATORS);
						}
						break;
					case 1:
						{
							// move player to spectator if the reserved slots aren't filled yet, kick him otherwise
							int Spectators = 0;
							for(int j = 0; j < MAX_CLIENTS; ++j)
								if(GameServer()->m_apPlayers[j] && GameServer()->m_apPlayers[j]->GetTeam() == TEAM_SPECTATORS)
									++Spectators;
							if(Spectators >= g_Config.m_SvSpectatorSlots)
								Server()->Kick(i, "Kicked for inactivity");
							else
								GameServer()->m_apPlayers[i]->SetTeam(TEAM_SPECTATORS);
						}
						break;
					case 2:
						{
							// kick the player
							Server()->Kick(i, "Kicked for inactivity");
						}
					}
				}
			}
		}
	}
	
	DoWincheck();
}


bool IGameController::IsCoop() const
{
	return m_GameFlags&GAMEFLAG_COOP;
}

bool IGameController::IsTeamplay() const
{
	return m_GameFlags&GAMEFLAG_TEAMS;
}

bool IGameController::IsInfection() const
{
	return m_GameFlags&GAMEFLAG_INFECTION;
}

int IGameController::GetTimeLeft()
{
	if (m_TimeLimit == 0)
		return 0;
	
	int Time = m_TimeLimit*60 - (Server()->Tick() - m_RoundStartTick)/Server()->TickSpeed();
	
	return Time;
}



void IGameController::Snap(int SnappingClient)
{
	CNetObj_GameInfo *pGameInfoObj = (CNetObj_GameInfo *)Server()->SnapNewItem(NETOBJTYPE_GAMEINFO, 0, sizeof(CNetObj_GameInfo));
	if(!pGameInfoObj)
		return;

	if (m_ResetTime)
	{
		m_ResetTime = false;
		if (m_RoundTimeLimit > 0)
			m_RoundStartTick = Server()->Tick() - Server()->TickSpeed()*(60-m_RoundTimeLimit%60);
		else
			m_RoundStartTick = Server()->Tick();
	}
	
	pGameInfoObj->m_GameFlags = m_GameFlags;
	pGameInfoObj->m_GameStateFlags = 0;
	if(m_GameOverTick != -1)
		pGameInfoObj->m_GameStateFlags |= GAMESTATEFLAG_GAMEOVER;
	if(m_SuddenDeath)
		pGameInfoObj->m_GameStateFlags |= GAMESTATEFLAG_SUDDENDEATH;
	if(GameServer()->m_World.m_Paused)
		pGameInfoObj->m_GameStateFlags |= GAMESTATEFLAG_PAUSED;
	pGameInfoObj->m_RoundStartTick = m_RoundStartTick;
	pGameInfoObj->m_WarmupTimer = m_Warmup;

	pGameInfoObj->m_ScoreLimit = g_Config.m_SvScorelimit;
	
	
	if (g_Config.m_SvSurvivalMode && g_Config.m_SvSurvivalTime)
	{
		pGameInfoObj->m_TimeLimit = g_Config.m_SvSurvivalTime/60+1;
		pGameInfoObj->m_RoundStartTick = m_SurvivalStartTick - Server()->TickSpeed()*(60-g_Config.m_SvSurvivalTime%60);
	}
	else
	{		
		if (m_RoundTimeLimit > 0)
			pGameInfoObj->m_TimeLimit = m_RoundTimeLimit/60+1;
		else
			pGameInfoObj->m_TimeLimit = 0;
	}
	
	m_TimeLimit = pGameInfoObj->m_TimeLimit;
			
	pGameInfoObj->m_RoundNum = (str_length(g_Config.m_SvMaprotation) && g_Config.m_SvRoundsPerMap) ? g_Config.m_SvRoundsPerMap : 0;
	pGameInfoObj->m_RoundCurrent = m_RoundCount+1;
}

int IGameController::GetAutoTeam(int NotThisID)
{
	// this will force the auto balancer to work overtime aswell
	if(g_Config.m_DbgStress)
		return 0;
	
	if (IsInfection() && m_Warmup)
		return TEAM_RED;
		
	int aNumplayers[2] = {0,0};
	int aNumbots[2] = {0,0};
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i] && i != NotThisID)
		{
			if(GameServer()->m_apPlayers[i]->GetTeam() >= TEAM_RED && GameServer()->m_apPlayers[i]->GetTeam() <= TEAM_BLUE)
			{
				aNumplayers[GameServer()->m_apPlayers[i]->GetTeam()]++;
				if (GameServer()->m_apPlayers[i]->m_IsBot)
					aNumbots[GameServer()->m_apPlayers[i]->GetTeam()]++;
			}
		}
	}

	if (IsInfection())
	{
		//if (aNumplayers[TEAM_BLUE] > 0)
		//	return TEAM_BLUE;
		//else
			return TEAM_RED;
	}
	
	int Team = 0;
	if(IsTeamplay())
	{
		if (aNumplayers[TEAM_RED] == aNumplayers[TEAM_BLUE])
		{
			if (aNumbots[TEAM_RED] == aNumbots[TEAM_BLUE])
				Team = rand()%2 == 0 ? TEAM_BLUE : TEAM_RED;
			else
				Team = aNumbots[TEAM_RED] < aNumbots[TEAM_BLUE] ? TEAM_BLUE : TEAM_RED;
			
		}
		else
			Team = aNumplayers[TEAM_RED] > aNumplayers[TEAM_BLUE] ? TEAM_BLUE : TEAM_RED;
	}

	if(CanJoinTeam(Team, NotThisID))
		return Team;
	return -1;
}

bool IGameController::CanJoinTeam(int Team, int NotThisID)
{
	return true;
	
	if(Team == TEAM_SPECTATORS || (GameServer()->m_apPlayers[NotThisID] && GameServer()->m_apPlayers[NotThisID]->GetTeam() != TEAM_SPECTATORS))
		return true;

	int aNumplayers[2] = {0,0};
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i] && i != NotThisID)
		{
			if(GameServer()->m_apPlayers[i]->GetTeam() >= TEAM_RED && GameServer()->m_apPlayers[i]->GetTeam() <= TEAM_BLUE)
				aNumplayers[GameServer()->m_apPlayers[i]->GetTeam()]++;
		}
	}

	return (aNumplayers[0] + aNumplayers[1]) < Server()->MaxClients()-g_Config.m_SvSpectatorSlots;
}

bool IGameController::CheckTeamBalance()
{
	return true;
	
	if(!IsTeamplay() || !g_Config.m_SvTeambalanceTime)
		return true;

	int aT[2] = {0, 0};
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pP = GameServer()->m_apPlayers[i];
		if(pP && pP->GetTeam() != TEAM_SPECTATORS)
			aT[pP->GetTeam()]++;
	}

	char aBuf[256];
	if(absolute(aT[0]-aT[1]) >= 2)
	{
		str_format(aBuf, sizeof(aBuf), "Teams are NOT balanced (red=%d blue=%d)", aT[0], aT[1]);
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
		if(GameServer()->m_pController->m_UnbalancedTick == -1)
			GameServer()->m_pController->m_UnbalancedTick = Server()->Tick();
		return false;
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "Teams are balanced (red=%d blue=%d)", aT[0], aT[1]);
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
		GameServer()->m_pController->m_UnbalancedTick = -1;
		return true;
	}
}

bool IGameController::CanChangeTeam(CPlayer *pPlayer, int JoinTeam)
{
	int aT[2] = {0, 0};

	if (JoinTeam == TEAM_SPECTATORS)
	{
		//GameServer()->SendChatTarget(pPlayer->GetCID(), "Spectator mode disabled");
		return true;
	}
	
	if (IsInfection())
		return false;
	
	if (!IsTeamplay() || JoinTeam == TEAM_SPECTATORS || !g_Config.m_SvTeambalanceTime)
		return true;

	return true;
	
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pP = GameServer()->m_apPlayers[i];
		if(pP && pP->GetTeam() != TEAM_SPECTATORS)
			aT[pP->GetTeam()]++;
	}
	
	// simulate what would happen if changed team
	aT[JoinTeam]++;
	if (pPlayer->GetTeam() != TEAM_SPECTATORS)
		aT[JoinTeam^1]--;

	// there is a player-difference of at least 2
	if(absolute(aT[0]-aT[1]) >= 2)
	{
		// player wants to join team with less players
		if ((aT[0] < aT[1] && JoinTeam == TEAM_RED) || (aT[0] > aT[1] && JoinTeam == TEAM_BLUE))
			return true;
		else
			return false;
	}
	else
		return true;
}


void IGameController::DoWincheck()
{
	if(m_GameOverTick == -1 && !m_Warmup && !GameServer()->m_World.m_ResetRequested)
	{
		if(IsTeamplay())
		{
			// check score win condition
			if (g_Config.m_SvSurvivalMode && !g_Config.m_SvSurvivalAcid)
			{
				if((g_Config.m_SvScorelimit > 0 && (m_aTeamscore[TEAM_RED] >= g_Config.m_SvScorelimit || m_aTeamscore[TEAM_BLUE] >= g_Config.m_SvScorelimit)))
				{
					if(m_aTeamscore[TEAM_RED] != m_aTeamscore[TEAM_BLUE])
						EndRound();
					else
						m_SuddenDeath = 1;
				}
			}
			else
			{
				if((g_Config.m_SvScorelimit > 0 && (m_aTeamscore[TEAM_RED] >= g_Config.m_SvScorelimit || m_aTeamscore[TEAM_BLUE] >= g_Config.m_SvScorelimit)) ||
					(g_Config.m_SvTimelimit > 0 && (Server()->Tick()-m_RoundStartTick) >= g_Config.m_SvTimelimit*Server()->TickSpeed()*60))
				{
					if(m_aTeamscore[TEAM_RED] != m_aTeamscore[TEAM_BLUE])
						EndRound();
					else
						m_SuddenDeath = 1;
				}
			}
		}
		else
		{
			// gather some stats
			int Topscore = 0;
			int TopscoreCount = 0;
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(GameServer()->m_apPlayers[i])
				{
					if(GameServer()->m_apPlayers[i]->m_Score > Topscore)
					{
						Topscore = GameServer()->m_apPlayers[i]->m_Score;
						TopscoreCount = 1;
					}
					else if(GameServer()->m_apPlayers[i]->m_Score == Topscore)
						TopscoreCount++;
				}
			}

			// check score win condition
			if (g_Config.m_SvSurvivalMode && !g_Config.m_SvSurvivalAcid)
			{
				if (g_Config.m_SvScorelimit > 0 && Topscore >= g_Config.m_SvScorelimit)
				{
					if(TopscoreCount == 1)
						EndRound();
					else
						m_SuddenDeath = 1;
				}
			}
			else
			{
				if((g_Config.m_SvScorelimit > 0 && Topscore >= g_Config.m_SvScorelimit) ||
					(g_Config.m_SvTimelimit > 0 && (Server()->Tick()-m_RoundStartTick) >= g_Config.m_SvTimelimit*Server()->TickSpeed()*60))
				{
					if(TopscoreCount == 1)
						EndRound();
					else
						m_SuddenDeath = 1;
				}
			}
		}
	}
}

int IGameController::ClampTeam(int Team)
{
	if(Team < 0)
		return TEAM_SPECTATORS;
	if(IsTeamplay())
		return Team&1;
	return 0;
}


bool IGameController::CanSeePickup(int CID, int Type, int Subtype)
{
	return true;
}

bool IGameController::CanDropWeapon(CCharacter *pCharacter)
{
	return pCharacter != NULL;
}
