#include <engine/shared/config.h>

#include <game/server/entities/building.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "base.h"

#include <game/server/entities/droid_crawler.h>
#include <game/server/entities/droid_bosscrawler.h>

#include <game/server/playerdata.h>
#include <game/server/ai.h>
#include <game/server/ai/base_ai.h>
#include <game/server/ai/def/def_alien1_ai.h>
#include <game/server/ai/def/def_alien2_ai.h>
#include <game/server/ai/def/def_robot1_ai.h>
#include <game/server/ai/def/def_robot2_ai.h>
#include <game/server/ai/def/def_robot3_ai.h>
#include <game/server/ai/def/def_bunny1_ai.h>
#include <game/server/ai/def/def_bunny2_ai.h>
#include <game/server/ai/def/def_pyro1_ai.h>
#include <game/server/ai/def/def_pyro2_ai.h>


CGameControllerBase::CGameControllerBase(class CGameContext *pGameServer) : IGameController(pGameServer)
{
	m_pGameType = "DEF";
	m_GameFlags = GAMEFLAG_COOP;
	m_GameState = STATE_STARTING;
	
	for (int i = 0; i < MAX_ENEMIES; i++)
		m_aEnemySpawnPos[i] = vec2(0, 0);
	
	m_RoundOverTick = 0;
	m_RoundWinTick = 0;
	m_NoPlayersTick = 0;
	m_Crawlers = 0;
	
	m_GameOverBroadcast = false;
	m_RoundWin = false;

	m_WaveStartTick = 0;
	m_Wave = 0;
	m_EnemyCount = 0;
	m_EnemiesLeft = 0;
	m_Deaths = m_EnemiesLeft;
	m_NumEnemySpawnPos = 0;
	m_SpawnPosRotation = 0;
	m_Bosses = 0;
	
	// force some settings
	g_Config.m_SvOneHitKill = 0;
	g_Config.m_SvWarmup = 0;
	g_Config.m_SvTimelimit = 0;
	g_Config.m_SvScorelimit = 0;
	g_Config.m_SvSurvivalTime = 0;
	g_Config.m_SvEnableBuilding = 1;
	g_Config.m_SvDisablePVP = 1;
	g_Config.m_SvMapGen = 0;
	g_Config.m_SvMapGenLevel = 0;

	
	if (g_Config.m_SvEnableBuilding)
		m_GameFlags |= GAMEFLAG_BUILD;
	
	if (g_Config.m_SvSurvivalMode)
		m_GameFlags |= GAMEFLAG_SURVIVAL;
}


bool CGameControllerBase::OnEntity(int Index, vec2 Pos)
{
	if (Index == ENTITY_REACTOR)
	{
		new CBuilding(&GameServer()->m_World, Pos, BUILDING_REACTOR, 0);
		return true;
	}
	
	if (IGameController::OnEntity(Index, Pos))
		return true;

	if (Index == ENTITY_ENEMYSPAWN && m_NumEnemySpawnPos < MAX_ENEMIES)
	{
		m_aEnemySpawnPos[m_NumEnemySpawnPos++] = Pos;
		return true;
	}
	
	return false;
}


bool CGameControllerBase::GetSpawnPos(int Team, vec2 *pOutPos)
{
	if (!pOutPos || !m_NumEnemySpawnPos)
		return false;

	m_SpawnPosRotation++;
	m_SpawnPosRotation = m_SpawnPosRotation%m_NumEnemySpawnPos;
	
	//int i = rand()%m_NumEnemySpawnPos;
	*pOutPos = m_aEnemySpawnPos[m_SpawnPosRotation];
	return true;
}


void CGameControllerBase::OnCharacterSpawn(CCharacter *pChr, bool RequestAI)
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
			
			// not in use
			if (m_Bosses > 0)
				m_Bosses--;
			
			GameServer()->GetAISkin(&pChr->GetPlayer()->m_AISkin, false, 1+(m_Wave-1)/1.5f);
			//GameServer()->GetAISkin(&pChr->GetPlayer()->m_AISkin, false, 1+frandom()*7);
			pChr->GetPlayer()->SetAISkin();
			pChr->GetPlayer()->m_pAI = new CAIbase(GameServer(), pChr->GetPlayer());
			pChr->m_IsBot = true;
		
			
			int level = pChr->GetPlayer()->m_AISkin.m_Level;

			pChr->SetHealth(50+min(level*10.0f, 200.0f));
			pChr->SetArmor(10+min(level*10.0f, 300.0f));
			switch (level)
			{
				case 1:
					if (frandom() < 0.7f) pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_GUN1)));
					else				  pChr->GiveWeapon(GameServer()->NewWeapon(GetModularWeapon(1, 1)));
					break;
				
				case 2:
					if (frandom() < 0.5f) pChr->GiveWeapon(GameServer()->NewWeapon(GetModularWeapon(3, 1)));
					else
					{
						pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_GUN2)));
						pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_GRENADE2)));
					}
					break;
				
				case 3:
					if (frandom() < 0.5f) pChr->GiveWeapon(GameServer()->NewWeapon(GetModularWeapon(5, 6)));
					else				  pChr->GiveWeapon(GameServer()->NewWeapon(GetModularWeapon(5, 7)));
					break;
					
				case 4:
					if (frandom() < 0.5f) pChr->GiveWeapon(GameServer()->NewWeapon(GetModularWeapon(1, 1)));
					else if (frandom() < 0.5f) pChr->GiveWeapon(GameServer()->NewWeapon(GetModularWeapon(3, 4)));
					else				  pChr->GiveWeapon(GameServer()->NewWeapon(GetModularWeapon(3, 2)));
					break;
					
				case 5:
					if (frandom() < 0.5f) pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_CHAINSAW)));
					else if (frandom() < 0.5f) pChr->GiveWeapon(GameServer()->NewWeapon(GetModularWeapon(1, 2)));
					else				  pChr->GiveWeapon(GameServer()->NewWeapon(GetModularWeapon(1, 4)));
					if (frandom() < 0.2f) pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_GRENADE2)));
					break;
					
				case 6:
					if (frandom() < 0.5f) pChr->GiveWeapon(GameServer()->NewWeapon(GetChargedWeapon(GetStaticWeapon(SW_CLUSTER), 1)));
					else if (frandom() < 0.5f) pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_BOUNCER)));
					else				  pChr->GiveWeapon(GameServer()->NewWeapon(GetChargedWeapon(GetModularWeapon(1, 1), 2)));
					if (frandom() < 0.3f) pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_GRENADE1)));
					if (frandom() < 0.3f) pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_SHIELD)));
					break;
					
				case 7:
					if (frandom() < 0.3f) pChr->GiveWeapon(GameServer()->NewWeapon(GetChargedWeapon(GetModularWeapon(4, 3), 3)));
					if (frandom() < 0.3f) pChr->GiveWeapon(GameServer()->NewWeapon(GetModularWeapon(6, 7)));
					else if (frandom() < 0.3f) pChr->GiveWeapon(GameServer()->NewWeapon(GetChargedWeapon(GetModularWeapon(4, 4), 3)));
					else if (frandom() < 0.3f) pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_FLAMER)));
					else				  pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_BAZOOKA)));
					if (frandom() < 0.3f) pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_GRENADE1)));
					if (frandom() < 0.4f) pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_SHIELD)));
					break;
					
				default:
					;
			};
			
			/*
			pChr->GiveWeapon(GameServer()->NewWeapon(GetModularWeapon(1, 1)));
			pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_GRENADE1)));
			pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_GRENADE1)));
			*/
			
			m_EnemyCount++;
		}
		
		if (!Found)
		{
			GameServer()->GetAISkin(&pChr->GetPlayer()->m_AISkin, false, 1+(m_Wave-1)/1.5f);
			pChr->GetPlayer()->SetAISkin();
			pChr->GetPlayer()->m_pAI = new CAIbase(GameServer(), pChr->GetPlayer());
			pChr->GetPlayer()->m_ToBeKicked = true;
		}
	}
	else
	{
		if (!pChr->GetPlayer()->m_Welcomed)
		{
			pChr->GetPlayer()->m_Welcomed = true;
			GameServer()->SendBroadcast("Reactor defense", pChr->GetPlayer()->GetCID());
		}
	}
}



int CGameControllerBase::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	IGameController::OnCharacterDeath(pVictim, pKiller, Weapon);
	

	if (pVictim->m_IsBot && !pVictim->GetPlayer()->m_ToBeKicked)
	{
		if (--m_Deaths <= 0 && CountPlayersAlive(-1, true) > 0 && !m_WaveStartTick && !m_RoundOverTick)
		{
			// next wave
			if (m_Wave >= 10)
			{
				GameServer()->SendBroadcast("Stage completed", -1);
				m_RoundOverTick = Server()->Tick();
				m_Wave++;
			}
			else
			{
				m_WaveStartTick = Server()->Tick() + Server()->TickSpeed() * 15.0f;
				GameServer()->SendBroadcast("Wave cleared", -1);
			}
		}
				
		if (m_EnemiesLeft <= 0)
			pVictim->GetPlayer()->m_ToBeKicked = true;
	}
	
	if (g_Config.m_SvSurvivalMode && !pVictim->m_IsBot && CountPlayersAlive(-1, true) <= 1)
	{
		DeathMessage();
		m_RoundOverTick = Server()->Tick();
	}
	
	if (!pVictim->m_IsBot)
		pVictim->GetPlayer()->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()*g_Config.m_SvRespawnDelay;

	return 0;
}


int CGameControllerBase::ReactorsLeft()
{
	CBuilding *apEnts[9999];
	int Num = GameServer()->m_World.FindEntities(vec2(0, 0), 0, (CEntity**)apEnts, 9999, CGameWorld::ENTTYPE_BUILDING);

	int Reactors = 0;
	
	for (int i = 0; i < Num; ++i)
	{
		CBuilding *pBuilding = apEnts[i];

		if (pBuilding->m_Type == BUILDING_REACTOR)
			Reactors++;
	}
	
	return Reactors;
}



void CGameControllerBase::NextWave()
{
	m_WaveStartTick = 0;
	m_Bosses = 0;
	m_Wave++;
	
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "Wave %d", m_Wave);
	GameServer()->SendBroadcast(aBuf, -1);

	m_EnemiesLeft = 5 + m_Wave * 6;
	m_Deaths = m_EnemiesLeft;
	
	if (m_Wave > 2)
		for (int i = 0; i < min(int(1+m_Wave/3), 3); i++)
		{
			vec2 p;
			GetSpawnPos(0, &p);
			new CCrawler(&GameServer()->m_World, p+vec2(0, -100));
		}
		
	if (m_Wave > 7)
	{
			vec2 p;
			GetSpawnPos(0, &p);
			new CBossCrawler(&GameServer()->m_World, p+vec2(0, -100));
	}
	
	// add bots
	for (int i = 0; i < m_EnemiesLeft && GameServer()->m_pController->CountBots() < 12; i++)
		GameServer()->AddBot();
}



void CGameControllerBase::Tick()
{
	IGameController::Tick();
	
	if (m_Wave > 0 && !m_NoPlayersTick && CountHumans() <= 0)
	{
		m_NoPlayersTick = Server()->Tick() + Server()->TickSpeed() * 10.0f;
	}
	
	if (m_NoPlayersTick && m_NoPlayersTick < Server()->Tick())
	{
		m_NoPlayersTick = 0;
		m_Wave = 0;
		EndRound();
		//CycleMap();
	}
	
	// 
	if (m_GameState == STATE_STARTING)
	{
		if (GameServer()->m_pController->CountPlayers(0) > 0 && !m_WaveStartTick)
		{
			/*
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "start round, enemies: '%u'", m_Deaths);
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "inv", aBuf);
			
			m_GameState = STATE_GAME;
			for (int i = 0; i < m_EnemiesLeft && GameServer()->m_pController->CountBots() < 12; i++)
				GameServer()->AddBot();
			*/
			
			m_GameState = STATE_GAME;
			m_WaveStartTick = Server()->Tick() + Server()->TickSpeed() * 10.0f;
			m_Wave = 0;
		}
		/*
		// reset to first map if there's no players for 60 seconds
		else if (Server()->Tick() > Server()->TickSpeed()*60.0f)
		{
			CycleMap();
		}
		*/
	}
	else
	{
		if (!m_RoundOverTick && m_WaveStartTick && m_WaveStartTick < Server()->Tick())
			NextWave();
		
		// wave win
		if (!m_RoundOverTick && m_Bosses <= 0 && m_Deaths <= 4 && !CountBotsAlive() && CountPlayersAlive(-1, true) > 0 && !m_WaveStartTick)
		{
			// next wave
			m_WaveStartTick = Server()->Tick() + Server()->TickSpeed() * 15.0f;
			GameServer()->SendBroadcast("Wave cleared", -1);
		}
		
		// lose condition
		if (!m_RoundOverTick && Server()->Tick()%20 == 1 && !ReactorsLeft())
		{
			m_RoundOverTick = Server()->Tick();
			GameServer()->SendBroadcast("All reactors lost", -1);
			m_GameOverBroadcast = 0;
		}
		
		// lose => restart
		if (m_RoundOverTick && !m_GameOverBroadcast && m_RoundOverTick < Server()->Tick() - Server()->TickSpeed()*3.0f && m_Wave <= 10)
		{
			m_GameOverBroadcast = true;
			
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "Game over on wave %d", m_Wave);
			GameServer()->SendBroadcast(aBuf, -1);
		}
		
		if (m_RoundOverTick && m_RoundOverTick < Server()->Tick() - Server()->TickSpeed()*7.0f)
		{
			m_RoundOverTick = 0;
			EndRound();
			//CycleMap();
		}
	}
	
	
	GameServer()->UpdateAI();
	
	// kick unwanted bots
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
			
		if (pPlayer->m_IsBot && pPlayer->m_ToBeKicked)
			GameServer()->KickBot(pPlayer->GetCID());
	}
}
