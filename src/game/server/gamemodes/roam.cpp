#include <engine/shared/config.h>

#include <game/mapitems.h>

#include <game/server/entities/character.h>
#include <game/server/entities/radar.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include <game/server/aiskin.h>

#include "roam.h"

#include <game/server/ai.h>
#include <game/server/ai/roam_ai.h>



CGameControllerRoam::CGameControllerRoam(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "Roam";
	m_aBotSpawn[0] = vec2(0, 0);
	m_BotSpawnNum = 0;
	g_Config.m_SvDisablePVP = true;
	m_GameFlags = GAMEFLAG_COOP;

	if (g_Config.m_SvSurvivalMode)
		m_GameFlags |= GAMEFLAG_SURVIVAL;
	
	//if (g_Config.m_SvSurvivalMode && g_Config.m_SvSurvivalTime && g_Config.m_SvSurvivalAcid)
	//	m_GameFlags |= GAMEFLAG_ACID;
	
	//if (g_Config.m_SvEnableBuilding)
	//	m_GameFlags |= GAMEFLAG_BUILD;
	
	//for (int i = 0; i < MAX_CLIENTS; i++)
	//	new CRadar(&GameServer()->m_World, RADAR_CHARACTER, i);
}

void CGameControllerRoam::OnCharacterSpawn(CCharacter *pChr, bool RequestAI)
{
	IGameController::OnCharacterSpawn(pChr);
	
	// init AI
	if (RequestAI)
	{
		GameServer()->GetAISkin(&pChr->GetPlayer()->m_AISkin, false);
		
		//pChr->GetPlayer()->m_AISkin = GameServer()->GetAISkin(false);
		pChr->GetPlayer()->m_pAI = new CAIroam(GameServer(), pChr->GetPlayer(), 0);
		pChr->GetPlayer()->SetAISkin();
		
		//m_Skin = SKIN_ALIEN1;
		//Player()->SetCustomSkin(m_Skin);
	}
}


int CGameControllerRoam::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	IGameController::OnCharacterDeath(pVictim, pKiller, Weapon);

	if (pVictim->m_IsBot)
		pVictim->GetPlayer()->m_ToBeKicked = true;
	
	/*
	if (g_Config.m_SvSurvivalMode && !pVictim->m_IsBot && CountPlayersAlive(-1, true) <= 1)
	{
		DeathMessage();
		m_RoundOverTick = Server()->Tick();
	}
	*/

	return 0;
}


void CGameControllerRoam::AddEnemy(vec2 Pos)
{
	if (GameServer()->m_pController->CountBots() < 32)
	{
		GameServer()->AddBot();
		m_aBotSpawn[m_BotSpawnNum] = Pos;
		if (m_BotSpawnNum < 99)
			m_BotSpawnNum++;
	}
}

bool CGameControllerRoam::CanSpawn(int Team, vec2 *pOutPos, bool IsBot)
{
	if (IsBot)
	{
		if (m_BotSpawnNum > 0)
			m_BotSpawnNum--;
		*pOutPos = m_aBotSpawn[m_BotSpawnNum];
	}
	else
		GameServer()->GetRoamSpawnPos(pOutPos);
	
	return true;
}

void CGameControllerRoam::Tick()
{
	IGameController::Tick();
	//AutoBalance();
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
