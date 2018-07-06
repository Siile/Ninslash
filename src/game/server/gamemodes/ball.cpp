#include <engine/shared/config.h>

#include <game/server/entities/character.h>
#include <game/server/entities/ball.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "ball.h"

#include <game/server/ai.h>
#include <game/server/ai/tdm_ai.h>


CGameControllerBall::CGameControllerBall(class CGameContext *pGameServer) : IGameController(pGameServer)
{
	m_pGameType = "BALL";
	m_GameFlags = GAMEFLAG_TEAMS;
	
	g_Config.m_SvDisablePVP = 1;
	
	m_GoalArea[TEAM_RED] = vec4(0, 0, 0, 0);
	m_GoalArea[TEAM_BLUE] = vec4(0, 0, 0, 0);
	
	m_GoalAreaSet[TEAM_RED] = false;
	m_GoalAreaSet[TEAM_BLUE] = false;
	
	m_RoundStartTick = 0;
	m_RoundEndTick = 0;
}

void CGameControllerBall::OnCharacterSpawn(CCharacter *pChr, bool RequestAI)
{
	IGameController::OnCharacterSpawn(pChr);
	
	// init AI
	//if (RequestAI)
	//	pChr->GetPlayer()->m_pAI = new CAItdm(GameServer(), pChr->GetPlayer());
}

int CGameControllerBall::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	IGameController::OnCharacterDeath(pVictim, pKiller, Weapon);

	return 0;
}



void CGameControllerBall::AddMapArea(int Team, vec2 Pos)
{
	if (Team != TEAM_RED && Team != TEAM_BLUE)
		return;
	
	if (!m_GoalAreaSet[Team])
	{
		m_GoalArea[Team] = vec4(Pos.x, Pos.y, Pos.x, Pos.y);
		m_GoalAreaSet[Team] = true;
	}
	else
	{
		m_GoalArea[Team].x = min(Pos.x, m_GoalArea[Team].x);
		m_GoalArea[Team].y = min(Pos.y, m_GoalArea[Team].y);
		m_GoalArea[Team].z = max(Pos.x, m_GoalArea[Team].z);
		m_GoalArea[Team].w = max(Pos.y, m_GoalArea[Team].w);
	}
}


bool CGameControllerBall::InMapArea(int Team, vec2 Pos)
{
	if (Team != TEAM_RED && Team != TEAM_BLUE)
		return false;
	
	if (!m_GoalAreaSet[Team])
		return false;
	
	if (Pos.x > m_GoalArea[Team].x && Pos.y > m_GoalArea[Team].y && Pos.x < m_GoalArea[Team].z && Pos.y < m_GoalArea[Team].w)
		return true;
	
	return false;
}


void CGameControllerBall::Snap(int SnappingClient)
{
	IGameController::Snap(SnappingClient);

	CNetObj_GameData *pGameDataObj = (CNetObj_GameData *)Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData));
	if(!pGameDataObj)
		return;

	pGameDataObj->m_TeamscoreRed = m_aTeamscore[TEAM_RED];
	pGameDataObj->m_TeamscoreBlue = m_aTeamscore[TEAM_BLUE];
}



void CGameControllerBall::Tick()
{
	IGameController::Tick();
	AutoBalance();
	GameServer()->UpdateAI();
	
	
	if (!GameServer()->m_pController->CountHumans())
	{
		
	}
	else
	{
		
		
	}
		
	if (m_RoundStartTick)
	{
		
	}
	
	if (!m_pBall)
		return;
	
	if (!m_RoundEndTick)
	{
		if (InMapArea(TEAM_RED, m_pBall->GetPosition()))
		{
			GameServer()->CreateSoundGlobal(SOUND_CTF_CAPTURE);
			GameServer()->SendBroadcast("Blue team scores!", -1);
			m_aTeamscore[TEAM_BLUE]++;
			m_RoundEndTick = Server()->Tick() + Server()->TickSpeed()*3;
		}
			
		if (InMapArea(TEAM_BLUE, m_pBall->GetPosition()))
		{
			GameServer()->CreateSoundGlobal(SOUND_CTF_CAPTURE);
			GameServer()->SendBroadcast("Red team scores!", -1);
			m_aTeamscore[TEAM_RED]++;
			m_RoundEndTick = Server()->Tick() + Server()->TickSpeed()*3;
		}
	}
	else if (m_RoundEndTick <= Server()->Tick())
	{
		GameServer()->SendBroadcast("Go!", -1);
		ResetBallRound();
		m_RoundEndTick = 0;
	}
}
