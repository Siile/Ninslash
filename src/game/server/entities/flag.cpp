#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "flag.h"

CFlag::CFlag(CGameWorld *pGameWorld, int Team)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_FLAG)
{
	m_Team = Team;
	m_ProximityRadius = ms_PhysSize;
	m_pCarryingCharacter = NULL;
	m_GrabTick = 0;

	m_UseSnapping = true;
	
	Reset();
}

void CFlag::Reset()
{
	m_pCarryingCharacter = NULL;
	m_AtStand = 1;
	m_Pos = m_StandPos;
	m_Vel = vec2(0,0);
	m_GrabTick = 0;
	m_Hide = false;
	
	// for domination
	m_CaptureTeam = -1;
	m_CapturePoints = 0.0f;
	
	ResetDistanceInfo();
}

void CFlag::TickPaused()
{
	++m_DropTick;
	if(m_GrabTick)
		++m_GrabTick;
}

void CFlag::Snap(int SnappingClient)
{
	// return if this is not the closest flag to the character
	if (!m_ClosestFlagToCharacter[SnappingClient] && m_UseSnapping)
		return;
	
	if (m_Hide)
		return;

	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Flag *pFlag = (CNetObj_Flag *)Server()->SnapNewItem(NETOBJTYPE_FLAG, m_Team, sizeof(CNetObj_Flag));
	if(!pFlag)
		return;

	pFlag->m_X = (int)m_Pos.x;
	pFlag->m_Y = (int)m_Pos.y;
	
	if (str_comp(g_Config.m_SvGametype, "dom") == 0)
	{
		if (m_CaptureTeam == -1)
		{
			if (GameServer()->m_apPlayers[SnappingClient])
			{
				if (GameServer()->m_apPlayers[SnappingClient]->GetTeam() == TEAM_RED)
					pFlag->m_Team = TEAM_BLUE;
				else
					pFlag->m_Team = TEAM_RED;
			}
		}
		else
			pFlag->m_Team = m_CaptureTeam;
	}
	else
		pFlag->m_Team = m_Team;
}
