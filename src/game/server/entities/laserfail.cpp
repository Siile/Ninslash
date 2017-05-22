#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "laserfail.h"

CLaserFail::CLaserFail(CGameWorld *pGameWorld, vec2 From, vec2 To, int PowerLevel)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_LASERFAIL)
{
	m_From = From;
	m_Pos = To;
	m_PowerLevel = PowerLevel;
	
	m_EvalTick = 0;
	GameWorld()->InsertEntity(this);
	m_EvalTick = Server()->Tick();
}



void CLaserFail::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

void CLaserFail::Tick()
{
	if(Server()->Tick() > m_EvalTick+(Server()->TickSpeed()*GameServer()->Tuning()->m_LaserBounceDelay)/1000.0f)
		GameServer()->m_World.DestroyEntity(this);


	//if(Server()->Tick() > m_EvalTick+(Server()->TickSpeed()*GameServer()->Tuning()->m_LaserBounceDelay)/1000.0f)
	//	m_EvalTick = Server()->Tick();
}

void CLaserFail::TickPaused()
{
	++m_EvalTick;
}

void CLaserFail::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_LaserFail *pObj = static_cast<CNetObj_LaserFail *>(Server()->SnapNewItem(NETOBJTYPE_LASERFAIL, m_ID, sizeof(CNetObj_LaserFail)));
	//CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID, sizeof(CNetObj_Laser)));
	if(!pObj)
		return;

	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_FromX = (int)m_From.x;
	pObj->m_FromY = (int)m_From.y;
	pObj->m_PowerLevel = m_PowerLevel;
	pObj->m_StartTick = m_EvalTick;
}
