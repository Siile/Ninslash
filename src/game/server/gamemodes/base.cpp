#include <engine/shared/config.h>

#include <game/server/entities/building.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "base.h"

#include <game/server/playerdata.h>
#include <game/server/ai.h>
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
			
			int e = ENEMY_ALIEN1;
			
			if (frandom() < (m_Wave-1) * 0.2f)
				e = ENEMY_ALIEN2;
			
			if (m_Wave > 5)
				e = ENEMY_ROBOT1;
				
			if (frandom() < (m_Wave-5) * 0.2f)
				e = ENEMY_ROBOT2;
			
			if (m_Wave > 10)
				e = ENEMY_BUNNY1;
				
			if (m_Wave > 10 && frandom() < 0.5f)
				e = ENEMY_BUNNY2;
			
			if (m_Wave > 15)
				e = ENEMY_PYRO1;
			
			if (m_Wave > 15 && frandom() < 0.4f)
				e = ENEMY_ROBOT2;
			
			if (m_Wave > 20 && frandom() < 0.06f)
				e = ENEMY_ROBOT3;
			
			if (m_Bosses > 0)
			{
				if (m_Wave > 15)
					e = ENEMY_PYRO2;
				else
					e = ENEMY_ROBOT3;
				m_Bosses--;
			}
			
			switch (e)
			{
			case ENEMY_ALIEN1:
				pChr->GetPlayer()->m_pAI = new CAIdefalien1(GameServer(), pChr->GetPlayer()); break;
			case ENEMY_ALIEN2:
				pChr->GetPlayer()->m_pAI = new CAIdefalien2(GameServer(), pChr->GetPlayer()); break;
			case ENEMY_ROBOT1:
				pChr->GetPlayer()->m_pAI = new CAIdefrobot1(GameServer(), pChr->GetPlayer()); break;
			case ENEMY_ROBOT2:
				pChr->GetPlayer()->m_pAI = new CAIdefrobot2(GameServer(), pChr->GetPlayer()); break;
			case ENEMY_ROBOT3:
				pChr->GetPlayer()->m_pAI = new CAIdefrobot3(GameServer(), pChr->GetPlayer()); break;
			case ENEMY_BUNNY1:
				pChr->GetPlayer()->m_pAI = new CAIdefbunny1(GameServer(), pChr->GetPlayer()); break;
			case ENEMY_BUNNY2:
				pChr->GetPlayer()->m_pAI = new CAIdefbunny2(GameServer(), pChr->GetPlayer()); break;
			case ENEMY_PYRO1:
				pChr->GetPlayer()->m_pAI = new CAIdefpyro1(GameServer(), pChr->GetPlayer()); break;
			case ENEMY_PYRO2:
				pChr->GetPlayer()->m_pAI = new CAIdefpyro2(GameServer(), pChr->GetPlayer()); break;
			default:
				pChr->GetPlayer()->m_pAI = new CAIdefalien1(GameServer(), pChr->GetPlayer()); break;
			}
			
			if (m_Wave == 12 && m_EnemiesLeft > 20)
				pChr->GiveBuff(PLAYERITEM_INVISIBILITY);
			
			if ((m_Wave == 8 || m_Wave > 14) && frandom() < 0.3f)
				pChr->GiveBuff(PLAYERITEM_RAGE);
	
			if (m_Wave > 12 && frandom() < 0.3f)
				pChr->GiveBuff(PLAYERITEM_SHIELD);
			
			m_EnemyCount++;
		}
		
		if (!Found)
		{
			pChr->GetPlayer()->m_pAI = new CAIdefalien1(GameServer(), pChr->GetPlayer());
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
			m_WaveStartTick = Server()->Tick() + Server()->TickSpeed() * 15.0f;
			GameServer()->SendBroadcast("Wave cleared", -1);
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
		pVictim->GetPlayer()->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()*2;

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
	
	if (m_Wave >= 15)
		m_Bosses++;
	
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "Wave %d", m_Wave);
	GameServer()->SendBroadcast(aBuf, -1);

	m_EnemiesLeft = 5 + m_Wave * 5;
	m_EnemiesLeft -= int(m_Wave/5) * 15;
	
	if (m_EnemiesLeft > 60)
		m_EnemiesLeft = 60;
	
	if (m_Wave == 10)
	{
		m_EnemiesLeft = 5;
		m_Bosses = 5;
	}
	
	if (m_Wave == 20)
	{
		m_EnemiesLeft = 10;
		m_Bosses = 10;
	}
	
	if (m_Wave > 20)
		m_Bosses = (m_Wave-20)*2;
	
	m_Deaths = m_EnemiesLeft;
	
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
		EndRound();
		CycleMap();
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
		if (m_RoundOverTick && !m_GameOverBroadcast && m_RoundOverTick < Server()->Tick() - Server()->TickSpeed()*3.0f)
		{
			m_GameOverBroadcast = true;
			
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "Game over on wave %d", m_Wave);
			GameServer()->SendBroadcast(aBuf, -1);
		}
		
		if (m_RoundOverTick && m_RoundOverTick < Server()->Tick() - Server()->TickSpeed()*7.0f)
		{
			EndRound();
			CycleMap();
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
