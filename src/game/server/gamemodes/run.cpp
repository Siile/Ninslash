


#include <engine/shared/config.h>

#include <game/mapitems.h>

#include <game/server/entities/character.h>
#include <game/server/entities/radar.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "run.h"

#include <game/server/playerdata.h>
#include <game/server/ai.h>
#include <game/server/ai/inv/robot1_ai.h>
#include <game/server/ai/inv/robot2_ai.h>
#include <game/server/ai/inv/alien1_ai.h>
#include <game/server/ai/inv/alien2_ai.h>
#include <game/server/ai/inv/bunny1_ai.h>
#include <game/server/ai/inv/bunny2_ai.h>
#include <game/server/ai/inv/pyro1_ai.h>
#include <game/server/ai/inv/pyro2_ai.h>



CGameControllerCoop::CGameControllerCoop(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "INV";
	m_GameFlags = GAMEFLAG_COOP;
	m_GameState = STATE_STARTING;
	
	m_BotSpawnTick = 0;
	
	if (g_Config.m_SvMapGenRandSeed)
	{
		g_Config.m_SvMapGenSeed = rand()%32767;
		g_Config.m_SvMapGenRandSeed = 0;
	}
	
	srand(g_Config.m_SvMapGenLevel + g_Config.m_SvMapGenSeed);
	
	for (int i = 0; i < MAX_ENEMIES; i++)
		m_aEnemySpawnPos[i] = vec2(0, 0);
	
	m_RoundOverTick = 0;
	m_RoundWinTick = 0;
	
	m_RoundWin = false;
	
	
	int Level = g_Config.m_SvMapGenLevel;
	
	m_GroupsLeft = 2 + min(3, Level/4) + (Level%5)/3;
	
	m_TriggerLevel = 0;
	m_Group = 0;
	m_GroupSpawnPos = vec2(0, 0);
	SpawnNewGroup(false);
		
	m_AutoRestart = false;
	
	m_NumEnemySpawnPos = 0;
	m_SpawnPosRotation = 0;
	m_TriggerTick = 0;
	
	// force some settings
	g_Config.m_SvRandomWeapons = 0;
	g_Config.m_SvOneHitKill = 0;
	g_Config.m_SvWarmup = 0;
	g_Config.m_SvTimelimit = 0;
	g_Config.m_SvScorelimit = 0;
	g_Config.m_SvSurvivalTime = 0;
	g_Config.m_SvEnableBuilding = 1;
	g_Config.m_SvDisablePVP = 1;
	
	if (g_Config.m_SvEnableBuilding)
		m_GameFlags |= GAMEFLAG_BUILD;
	
	if (g_Config.m_SvSurvivalMode)
		m_GameFlags |= GAMEFLAG_SURVIVAL;
	
	m_GameFlags |= GAMEFLAG_ACID;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
		new CRadar(&GameServer()->m_World, RADAR_HUMAN, i);
	
	m_pDoor = new CRadar(&GameServer()->m_World, RADAR_DOOR);
	m_pEnemySpawn = new CRadar(&GameServer()->m_World, RADAR_ENEMY);
}


bool CGameControllerCoop::OnEntity(int Index, vec2 Pos)
{
	if (IGameController::OnEntity(Index, Pos))
		return true;

	if (Index == ENTITY_ENEMYSPAWN && m_NumEnemySpawnPos < MAX_ENEMIES)
	{
		m_aEnemySpawnPos[m_NumEnemySpawnPos++] = Pos;
		return true;
	}
	
	return false;
}


bool CGameControllerCoop::GetSpawnPos(int Team, vec2 *pOutPos)
{
	if (!pOutPos || !m_NumEnemySpawnPos)
		return false;

	m_SpawnPosRotation++;
	m_SpawnPosRotation = m_SpawnPosRotation%m_NumEnemySpawnPos;
	
	*pOutPos = m_aEnemySpawnPos[m_SpawnPosRotation];
	return true;
}

vec2 CGameControllerCoop::GetBotSpawnPos()
{
	if (m_GroupSpawnPos.x < 1.0f)
	{
		vec2 p;
		GetSpawnPos(0, &p);
		return p;
	}
	
	vec2 Pos = m_GroupSpawnPos;
	
	for (int i = 0; i < 99; i++)
	{
		Pos = m_GroupSpawnPos + vec2(frandom()-frandom(), frandom()-frandom()) * 400;
		if (!GameServer()->Collision()->TestBox(Pos, vec2(28.0f, 64.0f)))
			return Pos;
	}
	
	/*
	if (GameServer()->Collision()->IsTileSolid(Pos.x, Pos.y - 48) && !GameServer()->Collision()->IsTileSolid(Pos.x, Pos.y + 32))
		Pos.y += 32;
	else if (!GameServer()->Collision()->IsTileSolid(Pos.x, Pos.y - 48) && GameServer()->Collision()->IsTileSolid(Pos.x, Pos.y + 32))
		Pos.y -= 32;
	
	for (int i = 0; i < 9; i++)
	{
		vec2 p = Pos + vec2(0, -32);
		vec2 p2 = Pos;
		
		vec2 r = vec2(frandom()-frandom(), frandom()-frandom())*300;
		
		vec2 To = p + r + vec2(0, -32);
		vec2 To2 = p + r + vec2(0, 0);
		
		if (!GameServer()->Collision()->IntersectLine(p, To, 0x0, &To) && !GameServer()->Collision()->IntersectLine(p2, To2, 0x0, &To2))
			return mix(p2, To2, frandom());
	}
	*/

	return m_GroupSpawnPos;
}

void CGameControllerCoop::RandomGroupSpawnPos()
{
	m_GroupSpawnPos =  m_aEnemySpawnPos[rand()%m_NumEnemySpawnPos];
	m_pEnemySpawn->Activate(m_GroupSpawnPos, Server()->Tick() + Server()->TickSpeed()*5);
}



bool CGameControllerCoop::CanSpawn(int Team, vec2 *pOutPos, bool IsBot)
{
	CSpawnEval Eval;

	// spectators can't spawn
	if(Team == TEAM_SPECTATORS)
		return false;

	if (IsBot)
	{
		if (m_BotSpawnTick > Server()->Tick())
			return false;
		
		if (m_GroupSpawnPos.x < 1.0f && GetSpawnPos(1, pOutPos))
			return true;
	
		vec2 Pos = GetBotSpawnPos();
		*pOutPos = Pos;
		
		m_BotSpawnTick = Server()->Tick() + Server()->TickSpeed() * 0.25f;
		
		return true;
	}
	else
		EvaluateSpawnType(&Eval, 0);

	*pOutPos = Eval.m_Pos;
	return Eval.m_Got;
}


void CGameControllerCoop::OnCharacterSpawn(CCharacter *pChr, bool RequestAI)
{
	IGameController::OnCharacterSpawn(pChr);
	
	// init AI
	if (RequestAI)
	{
		bool Found = false;
		
		if (m_EnemiesLeft > 0)
		{
			m_EnemiesLeft--;
			Found = true;
		
			int i = ENEMY_ALIEN1;
			
			int Level = 0;
			
			for (int i = 0; i < 9; i++)
				if (m_EnemiesLeft < 1-i*3 + g_Config.m_SvMapGenLevel/2 - m_Group/3)
					Level++;
			
			switch (m_GroupType)
			{
				case GROUP_ALIENS: 
					pChr->GetPlayer()->m_pAI = new CAIalien1(GameServer(), pChr->GetPlayer(), Level);
					break;
					
				case GROUP_BUNNIES: 
					pChr->GetPlayer()->m_pAI = new CAIbunny1(GameServer(), pChr->GetPlayer(), Level);
					break;
					
				case GROUP_ROBOTS: 
					pChr->GetPlayer()->m_pAI = new CAIrobot1(GameServer(), pChr->GetPlayer(), Level);
					break;
					
				case GROUP_PYROS: 
					pChr->GetPlayer()->m_pAI = new CAIpyro1(GameServer(), pChr->GetPlayer(), Level);
					break;
					
				case GROUP_SKELETONS: 
					pChr->GetPlayer()->m_pAI = new CAIpyro1(GameServer(), pChr->GetPlayer(), Level);
					break;
			};
			
			m_EnemyCount++;
			pChr->m_SkipPickups = 999;
			Trigger(false);
		}
		
		if (!Found)
		{
			pChr->GetPlayer()->m_pAI = new CAIalien1(GameServer(), pChr->GetPlayer(), g_Config.m_SvMapGenLevel);
			pChr->GetPlayer()->m_ToBeKicked = true;
			Trigger(false);
		}
	}
}

void CGameControllerCoop::Trigger(bool IncreaseLevel)
{
	if (IncreaseLevel)
		m_TriggerLevel++;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if (!pPlayer)
			continue;
		
		if (pPlayer->m_pAI)
			pPlayer->m_pAI->Trigger(m_TriggerLevel);
	}
}

void CGameControllerCoop::SpawnNewGroup(bool AddBots)
{
	m_EnemyCount = 0;
	m_EnemiesLeft = 20;
	m_GroupSpawnTick = 0;
	
	
	g_Config.m_SvInvBosses = 1;
	
	int Level = g_Config.m_SvMapGenLevel;
	
	m_GroupType = GROUP_ALIENS+((Level/3)%3+m_Group/2)%4;
	
	m_EnemiesLeft = max(3, 3 + min(12, Level) + m_Group - m_GroupType*2);
	
	if (AddBots)
	{
		RandomGroupSpawnPos();
		
		for (int i = 0; i < m_EnemiesLeft && GameServer()->m_pController->CountBots() < 24; i++)
			GameServer()->AddBot();
	}
	
	m_Deaths = m_EnemiesLeft;
	m_Group++;
	m_GroupsLeft--;
}


void CGameControllerCoop::DisplayExit(vec2 Pos)
{
	m_pDoor->Activate(Pos);	
}


int CGameControllerCoop::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	IGameController::OnCharacterDeath(pVictim, pKiller, Weapon);

	if (pVictim->m_IsBot && !pVictim->GetPlayer()->m_ToBeKicked)
	{
		if (--m_Deaths <= 0 && CountPlayersAlive(-1, true) > 0)
		{
			if (m_GroupsLeft <= 0)
				TriggerEscape();
			else if (!m_GroupSpawnTick)
				m_GroupSpawnTick = Server()->Tick() + Server()->TickSpeed()*5;
		}
			
		if (m_EnemiesLeft <= 0)
			pVictim->GetPlayer()->m_ToBeKicked = true;
		
		if (pKiller)
		{
			Trigger(true);
			
			if (frandom() < 0.011f)
				GameServer()->m_pController->DropWeapon(pVictim->m_Pos, vec2(frandom()*6.0-frandom()*6.0, 0-frandom()*14.0), GameServer()->NewWeapon(GetStaticWeapon(SW_UPGRADE)));
			else if (frandom() < 0.011f)
				GameServer()->m_pController->DropWeapon(pVictim->m_Pos, vec2(frandom()*6.0-frandom()*6.0, 0-frandom()*14.0), GameServer()->NewWeapon(GetStaticWeapon(SW_RESPAWNER)));
		}
	}
	
	if (g_Config.m_SvSurvivalMode && !pVictim->m_IsBot && CountPlayersAlive(-1, true) <= 1)
	{
		DeathMessage();
		m_RoundOverTick = Server()->Tick();
	}

	return 0;
}



void CGameControllerCoop::NextLevel(int CID)
{
	//
	if (!m_RoundWin)
	{
		m_RoundWin = true;
		m_RoundWinTick = Server()->Tick() + Server()->TickSpeed()*CountHumans()*1;
		
		if (CountHumans() > 1)
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "%s reached the door", Server()->ClientName(CID));
			GameServer()->SendBroadcast(aBuf, -1);
		}
	}
	
	
	CPlayer *pPlayer = GameServer()->m_apPlayers[CID];
	if(pPlayer && pPlayer->GetCharacter() && !pPlayer->GetCharacter()->IgnoreCollision())
		pPlayer->GetCharacter()->Warp();
}

void CGameControllerCoop::Tick()
{
	IGameController::Tick();
	
	// 
	if (m_GameState == STATE_STARTING)
	{
		if (GameServer()->m_pController->CountPlayers(0) > 0)
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "start round, enemies: '%u'", m_Deaths);
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "inv", aBuf);
			
			m_TriggerTick = 0;
			m_AutoRestart = true;
			
			m_GameState = STATE_GAME;
			for (int i = 0; i < m_EnemiesLeft && GameServer()->m_pController->CountBots() < 32; i++)
				GameServer()->AddBot();
		}
		// reset to first map if there's no players for 60 seconds
		else if ((m_AutoRestart || g_Config.m_SvMapGenLevel > 1) && Server()->Tick() > Server()->TickSpeed()*60.0f)
		{
			m_AutoRestart = false;
			
			if (g_Config.m_SvMapGenRandSeed)
				g_Config.m_SvMapGenSeed = rand()%32767;
			
			FirstMap();
		}
	}
	else
	{
		if (g_Config.m_SvMapGenLevel > 1)
			m_AutoRestart = true;
			
		// lose => restart
		if (m_RoundOverTick && m_RoundOverTick < Server()->Tick() - Server()->TickSpeed()*2.0f)
		{
			if (++g_Config.m_SvInvFails >= 3)
			{
				g_Config.m_SvInvFails = 0;
				
				if (--g_Config.m_SvMapGenLevel < 1)
					g_Config.m_SvMapGenLevel = 1;
				
				int l = g_Config.m_SvMapGenLevel;
				FirstMap();
				g_Config.m_SvMapGenLevel = l;
			}
			else
				GameServer()->ReloadMap();
		}
		
		
		if (m_GroupSpawnTick && m_GroupSpawnTick <= Server()->Tick())
			SpawnNewGroup();
	}
	
	GameServer()->UpdateAI();
	
	if (m_TriggerTick < Server()->Tick())
	{
		Trigger(true);
		m_TriggerTick = Server()->Tick() + Server()->TickSpeed()*4;
	}
	
	// kick unwanted bots
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
			
		if (pPlayer->m_IsBot && pPlayer->m_ToBeKicked)
			GameServer()->KickBot(pPlayer->GetCID());
	}
	
	if (m_RoundWin)
	{
		if (m_RoundWinTick < Server()->Tick())
		{
			m_RoundWin = false;
			m_RoundWinTick = 0;
			g_Config.m_SvMapGenLevel++;
			g_Config.m_SvInvFails = 0;
			EndRound();
		}
	}
}
