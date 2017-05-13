


#include <engine/shared/config.h>

#include <game/mapitems.h>

#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "run.h"

#include <game/server/playerdata.h>
#include <game/server/ai.h>
#include <game/server/ai/robot1_ai.h>
#include <game/server/ai/robot2_ai.h>
#include <game/server/ai/alien1_ai.h>
#include <game/server/ai/alien2_ai.h>
#include <game/server/ai/bunny1_ai.h>
#include <game/server/ai/bunny2_ai.h>



CGameControllerCoop::CGameControllerCoop(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "INV";
	m_GameFlags = GAMEFLAG_COOP;
	m_GameState = STATE_STARTING;
	
	for (int i = 0; i < MAX_ENEMIES; i++)
		m_aEnemySpawnPos[i] = vec2(0, 0);
	
	m_RoundOverTick = 0;
	m_RoundWinTick = 0;
	
	m_RoundWin = false;
	
	// hordes of enemies
	bool Defend = (g_Config.m_SvMapGenLevel > 1 && g_Config.m_SvMapGenLevel%5 == 0);
	int e = 2 + log(float(1 + g_Config.m_SvMapGenLevel/3)) * 5;
	e += rand()%(1+g_Config.m_SvMapGenLevel/4);

	if (Defend)
		e *= 2;
	
	m_EnemyCount = 0;
	m_EnemiesLeft = e;
	m_Deaths = m_EnemiesLeft;
	m_NumEnemySpawnPos = 0;
	m_SpawnPosRotation = 0;
	
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
	
	//int i = rand()%m_NumEnemySpawnPos;
	*pOutPos = m_aEnemySpawnPos[m_SpawnPosRotation];
	return true;
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
			
			if (g_Config.m_SvMapGenLevel > 10 && frandom() < 0.35f)
				i = ENEMY_ROBOT1;
			
			if (g_Config.m_SvMapGenLevel > 20 && frandom() < 0.35f)
				i = ENEMY_ROBOT2;
			
			if (m_EnemyCount > 12)
			{
				i = ENEMY_ALIEN2;
				
				if (g_Config.m_SvMapGenLevel > 30 && frandom() < 0.35f)
						i = ENEMY_BUNNY1;
				if (g_Config.m_SvMapGenLevel > 35 && frandom() < 0.35f)
						i = ENEMY_BUNNY2;
			}
			
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
				
			case ENEMY_BUNNY1:
				pChr->GetPlayer()->m_pAI = new CAIbunny1(GameServer(), pChr->GetPlayer());
				break;
				
			case ENEMY_BUNNY2:
				pChr->GetPlayer()->m_pAI = new CAIbunny2(GameServer(), pChr->GetPlayer());
				break;
				
			default:
				pChr->GetPlayer()->m_pAI = new CAIalien1(GameServer(), pChr->GetPlayer());
				break;
			};
				
			m_EnemyCount++;
		}
		
		if (!Found)
		{
			pChr->GetPlayer()->m_pAI = new CAIalien1(GameServer(), pChr->GetPlayer());
			pChr->GetPlayer()->m_ToBeKicked = true;
		}
	}
	else
	{
		/*
		CPlayerData *pData = GameServer()->Server()->PlayerData(pChr->GetPlayer()->GetCID());
		pChr->GiveCustomWeapon(pData->m_Weapon);
		pChr->SetCustomWeapon(pData->m_Weapon);
		*/
	}
}


int CGameControllerCoop::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	IGameController::OnCharacterDeath(pVictim, pKiller, Weapon);

	//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "inv", "OnCharacterDeath");
		
	if (Weapon != WEAPON_GAME)
		GameServer()->Server()->PlayerData(pVictim->GetPlayer()->GetCID())->Die();
	else if (!pVictim->m_IsBot)
		pVictim->SaveData();
		
	if (pVictim->m_IsBot && !pVictim->GetPlayer()->m_ToBeKicked)
	{
		if (--m_Deaths <= 0 && CountPlayersAlive(-1, true) > 0)
			TriggerEscape();
				
		if (m_EnemiesLeft <= 0)
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
			for (int i = 0; i < m_EnemiesLeft && GameServer()->m_pController->CountBots() < 12; i++)
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
			g_Config.m_SvMapGenLevel++;
			g_Config.m_SvInvFails = 0;
			EndRound();
		}
	}
}
