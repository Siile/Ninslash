#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "electrowall.h"

CElectroWall::CElectroWall(CGameWorld *pGameWorld, vec2 Pos1, vec2 Pos2)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_LASERFAIL)
{
	m_Pos = Pos1;
	m_Pos2 = Pos2;
	
	GameWorld()->InsertEntity(this);
	m_StartTick = Server()->Tick();
	m_DestructionTick = Server()->Tick()+Server()->TickSpeed()*8.0f;
}



void CElectroWall::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}


void CElectroWall::Tick()
{
	if(Server()->Tick() > m_DestructionTick)
		GameServer()->m_World.DestroyEntity(this);

	if (Server()->Tick() < m_DestructionTick-Server()->TickSpeed()*0.1f)
	{
		m_StartTick = Server()->Tick();
		HitCharacter();
	}
}


bool CElectroWall::HitCharacter()
{
	vec2 From = m_Pos-normalize(m_Pos-m_Pos2)*12.0f;
	vec2 To = m_Pos2+normalize(m_Pos-m_Pos2)*12.0f;
	CCharacter *pHit = GameServer()->m_World.IntersectCharacter(From, To, 4.0f, To, NULL);
	if(!pHit)
		return false;
	
	pHit->TakeDamage(-1, GetStaticWeapon(SW_ELECTROWALL), 0, vec2(0, 0), To);
	
	/*
	if (m_PowerLevel == 1)
		pHit->TakeDamage(normalize(To-From)*0.1f, m_Damage, m_Owner, WEAPON_LASER, At, DAMAGETYPE_ELECTRIC, m_OwnerBuilding ? true : false);
	else
		pHit->TakeDamage(normalize(To-From)*0.1f, m_Damage, m_Owner, WEAPON_LASER, At, DAMAGETYPE_NORMAL, m_OwnerBuilding ? true : false);
	*/
	
	//if (m_PowerLevel > 1)
	//	pHit->Deathray(false);
	
	return true;
}


void CElectroWall::TickPaused()
{
	++m_StartTick;
}


void CElectroWall::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_LaserFail *pObj = static_cast<CNetObj_LaserFail *>(Server()->SnapNewItem(NETOBJTYPE_LASERFAIL, m_ID, sizeof(CNetObj_LaserFail)));
	if(!pObj)
		return;

	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_FromX = (int)m_Pos2.x;
	pObj->m_FromY = (int)m_Pos2.y;
	pObj->m_PowerLevel = 100;
	pObj->m_StartTick = m_StartTick;
}
