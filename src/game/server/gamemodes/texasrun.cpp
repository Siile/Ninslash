

#include <engine/shared/config.h>

#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "texasrun.h"

#include <game/server/ai.h>
#include <game/server/ai/texas_ai.h>


CGameControllerTexasRun::CGameControllerTexasRun(class CGameContext *pGameServer) : IGameController(pGameServer)
{
	m_pGameType = "INF";
	m_GameFlags = GAMEFLAG_TEAMS|GAMEFLAG_INFECTION;
	g_Config.m_SvWarmup = 10;
	m_GameState = TEXAS_STARTING;
	m_EndTick = 0;
	m_GameStateLockTick = 0;
}

void CGameControllerTexasRun::OnCharacterSpawn(CCharacter *pChr, bool RequestAI)
{
	IGameController::OnCharacterSpawn(pChr);
	
	// init AI
	if (RequestAI)
		pChr->GetPlayer()->m_pAI = new CAItexas(GameServer(), pChr->GetPlayer());
	
	if (pChr->GetPlayer()->GetTeam() == TEAM_BLUE && CountPlayers(TEAM_BLUE) < 3)
		pChr->GiveRandomBuff();
}

int CGameControllerTexasRun::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	IGameController::OnCharacterDeath(pVictim, pKiller, Weapon);
	
	if (CountPlayers() > 1 && m_GameState == TEXAS_STARTED)
		m_GameState = TEXAS_FIRSTDEATH;
	
	if(pKiller && Weapon != WEAPON_GAME)
	{
		/*
		// do team scoring
		if(pKiller == pVictim->GetPlayer() || pKiller->GetTeam() == pVictim->GetPlayer()->GetTeam())
			m_aTeamscore[pKiller->GetTeam()&1]--; // klant arschel
		else
			m_aTeamscore[pKiller->GetTeam()&1]++; // good shit
		*/
	}

	pVictim->GetPlayer()->m_RespawnTick = max(pVictim->GetPlayer()->m_RespawnTick, Server()->Tick()+Server()->TickSpeed()*g_Config.m_SvRespawnDelayTDM);

	return 0;
}

void CGameControllerTexasRun::Snap(int SnappingClient)
{
	IGameController::Snap(SnappingClient);

	CNetObj_GameData *pGameDataObj = (CNetObj_GameData *)Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData));
	if(!pGameDataObj)
		return;

	pGameDataObj->m_TeamscoreRed = m_aTeamscore[TEAM_RED];
	pGameDataObj->m_TeamscoreBlue = m_aTeamscore[TEAM_BLUE];

	pGameDataObj->m_FlagCarrierRed = 0;
	pGameDataObj->m_FlagCarrierBlue = 0;
}



void CGameControllerTexasRun::MovePlayersToRed()
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
			
		if (pPlayer->GetTeam() == TEAM_BLUE)
			pPlayer->SetTeam(TEAM_RED, false);
	}
}


void CGameControllerTexasRun::SelectStartingDead()
{
	if (CountPlayers() < 1)
		return;

	//char aBuf[256];
	//str_format(aBuf, sizeof(aBuf), "Blue team size: '%d'", CountPlayers(TEAM_BLUE));
	//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
	
	int i = rand()%MAX_CLIENTS;

	CPlayer *pPlayer = GameServer()->m_apPlayers[i];
	if(!pPlayer)
		return;
		
	if (pPlayer->GetTeam() != TEAM_SPECTATORS)
	{
		pPlayer->SetTeam(TEAM_BLUE, false);
	}
}

void CGameControllerTexasRun::Tick()
{
	IGameController::Tick();
	
	if (m_GameStateLockTick > GameServer()->Server()->Tick())
		return;
	
	if (m_Warmup)
		m_GameState = TEXAS_STARTING;
	else
	{
		if (m_GameState == TEXAS_STARTING && CountPlayers() > 1)
		{
			m_GameState = TEXAS_STARTED;
			GameServer()->SendBroadcast("", -1);
		}
	}
	
	if (m_GameState == TEXAS_STARTED)
	{
		// set random player to blue team
		if (CountPlayers(TEAM_BLUE) == 0)
			SelectStartingDead();
	}
	
	if (m_GameState == TEXAS_FIRSTDEATH)
	{
		if (CountPlayers(TEAM_RED) == 0)
		{
			//EndRound();
			switch (rand()%5)
			{
				case 0:
					GameServer()->SendBroadcast("All hope is lost", -1); break;
				case 1:
					GameServer()->SendBroadcast("Slaughter", -1); break;
				case 2:
					GameServer()->SendBroadcast("Ocean of blood", -1); break;
				default:
					GameServer()->SendBroadcast("Everybody dies", -1); break;
			};
			m_GameState = TEXAS_ENDING;
			m_GameStateLockTick = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed() * 5;
			//MovePlayersToRed();
		}
		
	}
	else if (m_GameState == TEXAS_ENDING)
	{
		m_GameState = TEXAS_STARTING;
		MovePlayersToRed();
	}
	
	AutoBalance();
	GameServer()->UpdateAI();
}
