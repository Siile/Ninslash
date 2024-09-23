#include <generated/protocol.h>
#include <game/server/gamecontext.h>
#include "lightning.h"

CLightning::CLightning(CGameWorld *pGameWorld, vec2 Pos, vec2 From)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_Pos = Pos;
	m_From = From;
	m_EvalTick = 0;
	m_EvalTick = Server()->Tick();
	GameWorld()->InsertEntity(this);
}



void CLightning::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

void CLightning::Tick()
{
	if(Server()->Tick() > m_EvalTick+(Server()->TickSpeed()*GameServer()->Tuning()->m_LaserBounceDelay)/1000.0f)
		GameServer()->m_World.DestroyEntity(this);
}

void CLightning::TickPaused()
{
	++m_EvalTick;
}

void CLightning::Snap(int SnappingClient)
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
	pObj->m_StartTick = m_EvalTick;
	pObj->m_Charge = -1;
}
