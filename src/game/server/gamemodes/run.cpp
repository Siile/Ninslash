


#include <engine/shared/config.h>

#include <game/mapitems.h>

#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "run.h"

#include <game/server/ai.h>
#include <game/server/ai/robot1_ai.h>
#include <game/server/ai/robot2_ai.h>



CGameControllerCoop::CGameControllerCoop(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "coop";
	m_GameFlags = GAMEFLAG_COOP;
	m_Robots1 = 0;
	m_Robots2 = 0;
	m_GameState = STATE_STARTING;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
		m_Robot1SpawnPos[i] = vec2(0, 0);
}


bool CGameControllerCoop::OnEntity(int Index, vec2 Pos)
{
	if(IGameController::OnEntity(Index, Pos))
		return true;
	
	if(Index == ENTITY_ROBOT1)
	{
		if (m_Robots1 < MAX_ROBOTS)
			m_Robot1SpawnPos[m_Robots1] = Pos;
		
		m_Robots1++;
		return true;
	}
	else if(Index == ENTITY_ROBOT2)
	{
		if (m_Robots2 < MAX_ROBOTS)
			m_Robot2SpawnPos[m_Robots2] = Pos;
		
		m_Robots2++;
		return true;
	}
	
	return false;
}


bool CGameControllerCoop::GetSpawnPos(int Team, vec2 *pOutPos)
{
	if (!pOutPos)
		return false;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_Robot1SpawnPos[i].x != 0)
		{
			*pOutPos = m_Robot1SpawnPos[i];
			m_Robot1SpawnPos[i] = vec2(0, 0);
			return true;
		}
	}
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_Robot2SpawnPos[i].x != 0)
		{
			*pOutPos = m_Robot2SpawnPos[i];
			m_Robot2SpawnPos[i] = vec2(0, 0);
			return true;
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
		if (m_Robots1-- > 0)
			pChr->GetPlayer()->m_pAI = new CAIrobot1(GameServer(), pChr->GetPlayer());
		else if (m_Robots2-- > 0)
			pChr->GetPlayer()->m_pAI = new CAIrobot2(GameServer(), pChr->GetPlayer());
		else
			pChr->GetPlayer()->m_pAI = new CAIrobot1(GameServer(), pChr->GetPlayer());
	}
}



void CGameControllerCoop::Tick()
{
	IGameController::Tick();
	//AutoBalance();
	
	// 
	if (m_GameState == STATE_STARTING && GameServer()->m_pController->CountPlayers(0) > 0)
	{
		m_GameState = STATE_GAME;
		for (int i = 0; i < m_Robots1+m_Robots2 && GameServer()->m_pController->CountBots() <= 12; i++)
			GameServer()->AddBot();
	}
	
	
	GameServer()->UpdateAI();
}
