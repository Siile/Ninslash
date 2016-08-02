
/* If you are missing that file, acquire a complete release at ninslash.com.                */
#include <game/server/gamecontext.h>
#include "bomb.h"

CBomb::CBomb(CGameWorld *pGameWorld)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_FLAG)
{
	m_ProximityRadius = ms_PhysSize;
	m_pCarryingCharacter = NULL;
	m_GrabTick = 0;

	m_Owner = -1;
	
	Reset();
}

void CBomb::Reset()
{
	m_pCarryingCharacter = NULL;
	m_Pos = vec2(0,0);
	m_Vel = vec2(0,0);
	m_GrabTick = 0;
	m_Hide = true;
	
	m_Status = BOMB_IDLE;
	m_Timer = 0;
	m_Team = TEAM_RED;
}

void CBomb::TickPaused()
{
	++m_DropTick;
	if(m_GrabTick)
		++m_GrabTick;
}

void CBomb::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient) || m_Hide)
		return;

	CNetObj_Flag *pFlag = (CNetObj_Flag *)Server()->SnapNewItem(NETOBJTYPE_FLAG, m_Team, sizeof(CNetObj_Flag));
	if(!pFlag)
		return;

	pFlag->m_X = (int)m_Pos.x;
	pFlag->m_Y = (int)m_Pos.y;
	pFlag->m_Team = m_Team;
}
