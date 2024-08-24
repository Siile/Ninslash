#include <engine/shared/config.h>

#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include <game/server/entities/building.h>
#include <game/server/entities/radar.h>

#include "cs.h"

#include <game/server/ai.h>
#include <game/server/ai/def_ai.h>


CGameControllerCS::CGameControllerCS(class CGameContext *pGameServer) : IGameController(pGameServer)
{
	m_pGameType = "CS";
	m_GameFlags = GAMEFLAG_TEAMS;
	

	m_GameFlags |= GAMEFLAG_SURVIVAL;
	m_GameFlags |= GAMEFLAG_BUILD;
	
	g_Config.m_SvWarmup = 0;
	g_Config.m_SvTimelimit = 0;
	g_Config.m_SvSurvivalMode = 1;
	g_Config.m_SvDisablePVP = 0;
	
	m_pBombRadar = NULL;
	
	if (g_Config.m_SvSurvivalMode && g_Config.m_SvSurvivalTime && g_Config.m_SvSurvivalAcid)
		m_GameFlags |= GAMEFLAG_ACID;
	
	m_RoundWinner = -1;
	
	m_GameState = 0;
	m_Bomb = false;
	
	m_AreaCount = 0;
	
	for (int i = 0; i < 9; i++)
		m_aArea[i] = vec4(0, 0, 0, 0);
	
	for (int i = 0; i < (int)MAX_CLIENTS*(int)NUM_SLOTS; i++)
		m_aPlayerWeapon[i] = 0;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
		m_aPlayerArmor[i] = 0;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
		m_aPlayerKits[i] = 0;
}

void CGameControllerCS::OnCharacterSpawn(CCharacter *pChr, bool RequestAI)
{
	IGameController::OnCharacterSpawn(pChr);

	// init AI
	if (RequestAI)
	{
		if (!pChr->GetPlayer()->m_AISkin.m_Valid)
			GameServer()->GetAISkin(&pChr->GetPlayer()->m_AISkin, true);
		pChr->GetPlayer()->SetAISkin();
		pChr->GetPlayer()->m_pAI = new CAIdef(GameServer(), pChr->GetPlayer());
	}
	
	int c = pChr->GetPlayer()->GetCID();
	
	for (int i = 0; i < NUM_SLOTS; i++)
	{
		if (m_aPlayerWeapon[c*NUM_SLOTS + i])
		{
			pChr->GiveWeapon(GameServer()->NewWeapon(m_aPlayerWeapon[c*NUM_SLOTS + i]));
			m_aPlayerWeapon[c*NUM_SLOTS + i] = 0;
		}
	}
	
	pChr->IncreaseArmor(m_aPlayerArmor[c]);
	pChr->AddKits(m_aPlayerKits[c]);
	m_aPlayerArmor[c] = 0;
	m_aPlayerKits[c] = 0;
}

int CGameControllerCS::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	IGameController::OnCharacterDeath(pVictim, pKiller, Weapon);

	if(pKiller && Weapon != WEAPON_GAME)
	{
		// do team scoring
		if(pKiller == pVictim->GetPlayer() || pKiller->GetTeam() == pVictim->GetPlayer()->GetTeam())
		{
			if (!g_Config.m_SvSelfKillPenalty)
				m_aTeamscore[pKiller->GetTeam()&1]--;
		}
		else
			m_aTeamscore[pKiller->GetTeam()&1]++;
	}

	return 0;
}

vec2 CGameControllerCS::GetAttackPos()
{
	if (!m_AreaCount)
		return vec2(0.0f, 0.0f);
	
	int i = rand()%m_AreaCount;
	
	return vec2((m_aArea[i].x+m_aArea[i].z)/2, (m_aArea[i].y+m_aArea[i].w)/2);
}

bool CGameControllerCS::InBombArea(vec2 Pos)
{
	for (int i = 0; i < m_AreaCount; i++)
	{
		if (Pos.x > m_aArea[i].x && Pos.x < m_aArea[i].z && Pos.y > m_aArea[i].y && Pos.y < m_aArea[i].w)
			return true;
	}
	
	return false;
}

void CGameControllerCS::StartRound()
{
	FindReactors();
}


void CGameControllerCS::NewSurvivalRound()
{
	// save weapons if character alive
	for (int c = 0; c < MAX_CLIENTS; c++)
	{
		CCharacter *pChar = GameServer()->GetPlayerChar(c);
		
		if (pChar)
		{
			for (int i = 0; i < NUM_SLOTS; i++)
				if (GetStaticType(pChar->GetWeaponType(i)) != SW_BOMB)
					m_aPlayerWeapon[c*NUM_SLOTS + i] = pChar->GetWeaponType(i);
				
				
			m_aPlayerArmor[c] = pChar->GetArmor();
			m_aPlayerKits[c] = pChar->m_Kits;
		}
		else
		{
			for (int i = 0; i < NUM_SLOTS; i++)
			{
				m_aPlayerWeapon[c*NUM_SLOTS + i] = 0;
				m_aPlayerArmor[c] = 0;
				m_aPlayerKits[c] = 0;
			}
		}
		
		if (GameServer()->m_apPlayers[c])
			GameServer()->m_apPlayers[c]->IncreaseGold(15);
	}
	
	m_Bomb = false;
	GameServer()->SendBroadcast(-1, false, _("Go!"));
}


void CGameControllerCS::TriggerBomb()
{
	// time limit display
	m_SurvivalStartTick = Server()->Tick() - Server()->TickSpeed()*(g_Config.m_SvSurvivalTime-20.1f);
	GameServer()->SendBroadcast(-1, false, _("Bomb armed"));
	m_SurvivalDeathReset = false;
}


void CGameControllerCS::OnSurvivalTimeOut()
{
	if (!m_SurvivalResetTick)
	{
		m_SurvivalResetTick = Server()->Tick() + Server()->TickSpeed()*4.0f;
		m_RoundWinner = TEAM_BLUE;
		GameServer()->SendBroadcast(-1, false, _("Time out - Blue team wins"));
		m_aTeamscore[TEAM_BLUE] += g_Config.m_SvSurvivalReward; 
	}
}

	
void CGameControllerCS::DisarmBomb()
{
	if (!m_SurvivalResetTick)
	{
		m_SurvivalResetTick = Server()->Tick() + Server()->TickSpeed()*4.0f;
		m_RoundWinner = TEAM_BLUE;
		GameServer()->SendBroadcast(-1, false, _("Bomb disarmed - Blue team wins"));
		m_aTeamscore[TEAM_BLUE] += g_Config.m_SvSurvivalReward;
	}
}

void CGameControllerCS::ReactorDestroyed()
{
	if (!m_SurvivalResetTick)
	{
		m_SurvivalResetTick = Server()->Tick() + Server()->TickSpeed()*4.0f;
		m_RoundWinner = TEAM_RED;
		GameServer()->SendBroadcast(-1, false, _("Reactor destroyed - Red team wins"));
		m_aTeamscore[TEAM_RED] += g_Config.m_SvSurvivalReward;
	}
}



void CGameControllerCS::AddToArea(vec2 Pos)
{
	int Size = 200;
	
	// no bomb areas
	if (!m_AreaCount)
	{
		m_aArea[m_AreaCount++] = vec4(Pos.x-Size, Pos.y-Size, Pos.x+Size, Pos.y+Size);
		return;
	}
	
	// expand existing area
	for (int i = 0; i < m_AreaCount; i++)
	{
		if (Pos.x > m_aArea[i].x && Pos.x < m_aArea[i].z && Pos.y > m_aArea[i].y && Pos.y < m_aArea[i].w)
		{
			m_aArea[i].x = min(Pos.x-Size, m_aArea[i].x);
			m_aArea[i].y = min(Pos.y-Size, m_aArea[i].y);
			m_aArea[i].z = max(Pos.x+Size, m_aArea[i].z);
			m_aArea[i].w = max(Pos.y+Size, m_aArea[i].w);
			return;
		}
	}
	
	if (m_AreaCount >= 9)
		return;
	
	// ...or new area
	m_aArea[m_AreaCount++] = vec4(Pos.x-Size, Pos.y-Size, Pos.x+Size, Pos.y+Size);
}


void CGameControllerCS::FindReactors()
{
	CBuilding *apEnts[9999];
	int Num = GameServer()->m_World.FindEntities(vec2(0, 0), 0, (CEntity**)apEnts, 9999, CGameWorld::ENTTYPE_BUILDING);

	for (int i = 0; i < Num; ++i)
	{
		CBuilding *pBuilding = apEnts[i];

		if (pBuilding->m_Type == BUILDING_REACTOR)
			AddToArea(pBuilding->m_Pos);
	}
	
	for (int i = 0; i < m_AreaCount; i++)
	{
		CRadar *pRadar = new CRadar(&GameServer()->m_World, RADAR_REACTOR);
		pRadar->Activate(vec2((m_aArea[i].x+m_aArea[i].z)/2, (m_aArea[i].y+m_aArea[i].w)/2));
	}
}
	
	
void CGameControllerCS::Snap(int SnappingClient)
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



void CGameControllerCS::Tick()
{
	IGameController::Tick();
	AutoBalance();
	GameServer()->UpdateAI();
	
	if (!m_Bomb)
	{
		int i = frandom()*(int)MAX_CLIENTS;
		
		if (GameServer()->GetPlayerChar(i) && GameServer()->GetPlayerChar(i)->GiveBomb())
			m_Bomb = true;
	}
	
	if (!m_pBombRadar)
		m_pBombRadar = new CRadar(&GameServer()->m_World, RADAR_BOMB);
	else
		m_pBombRadar->Activate(GameServer()->m_pController->m_BombPos);
	
	if (!m_GameState)
	{
		m_GameState++;
		StartRound();
	}
}




