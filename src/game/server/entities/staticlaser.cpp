

#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "staticlaser.h"

CStaticlaser::CStaticlaser(CGameWorld *pGameWorld, vec2 From, vec2 To, int Life)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_From = From;
	m_Pos = To;
	m_Life = Life;
	
	GameWorld()->InsertEntity(this);
}




void CStaticlaser::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}


void CStaticlaser::Tick()
{
	if (m_Life-- <= 0)
		GameServer()->m_World.DestroyEntity(this);
}


void CStaticlaser::TickPaused()
{
}

void CStaticlaser::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID, sizeof(CNetObj_Laser)));
	if(!pObj)
		return;

	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_FromX = (int)m_From.x;
	pObj->m_FromY = (int)m_From.y;
	pObj->m_StartTick = Server()->Tick();
	pObj->m_Charge = m_Life;
}
