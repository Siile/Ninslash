#include <engine/shared/config.h>

#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "base.h"

#include <game/server/ai.h>
#include <game/server/ai/tdm_ai.h>


CGameControllerBase::CGameControllerBase(class CGameContext *pGameServer) : IGameController(pGameServer)
{
	m_pGameType = "base";
	m_GameFlags = GAMEFLAG_TEAMS;
}

void CGameControllerBase::OnCharacterSpawn(CCharacter *pChr, bool RequestAI)
{
	IGameController::OnCharacterSpawn(pChr);
	
	// init AI
	if (RequestAI)
		pChr->GetPlayer()->m_pAI = new CAItdm(GameServer(), pChr->GetPlayer());
}

int CGameControllerBase::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	IGameController::OnCharacterDeath(pVictim, pKiller, Weapon);

	if(pKiller && Weapon != WEAPON_GAME)
	{
		// do team scoring
		if(pKiller == pVictim->GetPlayer() || pKiller->GetTeam() == pVictim->GetPlayer()->GetTeam())
			m_aTeamscore[pKiller->GetTeam()&1]--; // klant arschel
		else
			m_aTeamscore[pKiller->GetTeam()&1]++; // good shit
	}

	pVictim->GetPlayer()->m_RespawnTick = max(pVictim->GetPlayer()->m_RespawnTick, Server()->Tick()+Server()->TickSpeed()*g_Config.m_SvRespawnDelayTDM);

	return 0;
}

void CGameControllerBase::Snap(int SnappingClient)
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



void CGameControllerBase::Tick()
{
	IGameController::Tick();
	AutoBalance();
	GameServer()->UpdateAI();
}
