#include <engine/shared/config.h>

#include <game/mapitems.h>

#include <game/server/entities/character.h>
#include <game/server/entities/radar.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "dm.h"

#include <game/server/ai.h>
#include <game/server/ai/dm_ai.h>



CGameControllerDM::CGameControllerDM(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "DM";
	
	g_Config.m_SvDisablePVP = 0;

	if (g_Config.m_SvSurvivalMode)
		m_GameFlags |= GAMEFLAG_SURVIVAL;
	
	if (g_Config.m_SvSurvivalMode && g_Config.m_SvSurvivalTime && g_Config.m_SvSurvivalAcid)
		m_GameFlags |= GAMEFLAG_ACID;
	
	if (g_Config.m_SvEnableBuilding)
		m_GameFlags |= GAMEFLAG_BUILD;
	
	//for (int i = 0; i < MAX_CLIENTS; i++)
	//	new CRadar(&GameServer()->m_World, RADAR_CHARACTER, i);
}

void CGameControllerDM::OnCharacterSpawn(CCharacter *pChr, bool RequestAI)
{
	IGameController::OnCharacterSpawn(pChr);
	
	// init AI
	if (RequestAI)
	{
		if (!pChr->GetPlayer()->m_AISkin.m_Valid)
			GameServer()->GetAISkin(&pChr->GetPlayer()->m_AISkin, true);
		pChr->GetPlayer()->SetAISkin();
		pChr->GetPlayer()->m_pAI = new CAIdm(GameServer(), pChr->GetPlayer());
	}
}



void CGameControllerDM::Tick()
{
	IGameController::Tick();
	AutoBalance();
	GameServer()->UpdateAI();
}
