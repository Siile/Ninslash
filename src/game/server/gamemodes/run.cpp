


#include <engine/shared/config.h>

#include <game/mapitems.h>

#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "run.h"

#include <game/server/ai.h>
#include <game/server/ai/robot1_ai.h>
#include <game/server/ai/robot2_ai.h>
#include <game/server/ai/alien1_ai.h>
#include <game/server/ai/alien2_ai.h>



CGameControllerCoop::CGameControllerCoop(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "INV";
	m_GameFlags = GAMEFLAG_COOP;
	m_GameState = STATE_STARTING;
	
	for (int i = 0; i < NUM_ENEMIES; i++)
	{
		m_Enemies[i] = 0;
		
		for (int j = 0; j < MAX_ROBOTS; j++)
			m_EnemySpawnPos[j + i*MAX_ROBOTS] = vec2(0, 0);
	}
	
	m_RoundOverTick = 0;
	m_RoundWinTick = 0;
	
	m_RoundWin = false;
	
	// force some settings
	g_Config.m_SvRandomWeapons = 0;
	g_Config.m_SvOneHitKill = 0;
	g_Config.m_SvForceWeapon = 0;
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
}


bool CGameControllerCoop::OnEntity(int Index, vec2 Pos)
{
	if (IGameController::OnEntity(Index, Pos))
		return true;
	
	int i = Index - ENTITY_ROBOT1;

	if (i >= 0 && i < NUM_ENEMIES)
	{
		if (m_Enemies[i] < MAX_ROBOTS)
		{
			m_EnemySpawnPos[m_Enemies[i] + i*MAX_ROBOTS] = Pos;
			m_Enemies[i]++;
			m_Deaths = EnemiesLeft();
		}
		
		return true;
	}
	
	return false;
}


bool CGameControllerCoop::GetSpawnPos(int Team, vec2 *pOutPos)
{
	if (!pOutPos)
		return false;

	for (int i = 0; i < NUM_ENEMIES; i++)
	{
		for (int j = 0; j < MAX_ROBOTS; j++)
		{
			if (m_EnemySpawnPos[j+i*MAX_ROBOTS].x != 0)
			{
				*pOutPos = m_EnemySpawnPos[j+i*MAX_ROBOTS];
				m_EnemySpawnPos[j+i*MAX_ROBOTS] = vec2(0, 0);
				return true;
			}
		}
	}
	
	return false;
}



void CGameControllerCoop::OnCharacterSpawn(CCharacter *pChr, bool RequestAI)
{
	IGameController::OnCharacterSpawn(pChr);
	
	// init AI
	if (RequestAI)
	{
		bool Found = false;
		
		for (int i = 0; i < NUM_ENEMIES; i++)
		{
			if (m_Enemies[i] > 0)
			{
				m_Enemies[i]--;
				Found = true;
			
				switch (i)
				{
				case ENEMY_ALIEN1:
					pChr->GetPlayer()->m_pAI = new CAIalien1(GameServer(), pChr->GetPlayer());
					break;
					
				case ENEMY_ALIEN2:
					pChr->GetPlayer()->m_pAI = new CAIalien2(GameServer(), pChr->GetPlayer());
					break;
					
				case ENEMY_ROBOT1:
					pChr->GetPlayer()->m_pAI = new CAIrobot1(GameServer(), pChr->GetPlayer());
					break;
					
				case ENEMY_ROBOT2:
					pChr->GetPlayer()->m_pAI = new CAIrobot2(GameServer(), pChr->GetPlayer());
					break;
				};
				
				if (Found)
					break;
			}
		}
		
		if (!Found)
		{
			pChr->GetPlayer()->m_pAI = new CAIalien1(GameServer(), pChr->GetPlayer());
			pChr->GetPlayer()->m_ToBeKicked = true;
		}
	}
}


int CGameControllerCoop::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	IGameController::OnCharacterDeath(pVictim, pKiller, Weapon);

	//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "inv", "OnCharacterDeath");
		
	if (pVictim->m_IsBot && !pVictim->GetPlayer()->m_ToBeKicked)
	{
		if (--m_Deaths <= 0)
			TriggerEscape();
				
		if (EnemiesLeft() <= 0)
			pVictim->GetPlayer()->m_ToBeKicked = true;
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
			
			m_GameState = STATE_GAME;
			for (int i = 0; i < EnemiesLeft() && GameServer()->m_pController->CountBots() < 12; i++)
				GameServer()->AddBot();
		}
		// reset to first map if there's no players for 60 seconds
		else if (Server()->Tick() > Server()->TickSpeed()*60.0f)
		{
			FirstMap();
		}
	}
	else
	{
		// lose => restart
		if (m_RoundOverTick && m_RoundOverTick < Server()->Tick() - Server()->TickSpeed()*2.0f)
			GameServer()->ReloadMap();
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
	
	if (m_RoundWin)
	{
		if (m_RoundWinTick < Server()->Tick())
		{
			m_RoundWin = false;
			m_RoundWinTick = 0;
			EndRound();
		}
	}
}
