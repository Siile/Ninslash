#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "landmine.h"

CLandmine::CLandmine(CGameWorld *pGameWorld, vec2 Pos, int Owner)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PICKUP)
{
	m_ProximityRadius = PickupPhysSize;
	m_Life = 4000;
	m_Pos = Pos;
	m_Owner = Owner;

	m_Flashing = false;
	m_FlashTimer = 0;
	
	GameWorld()->InsertEntity(this);
}

void CLandmine::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

void CLandmine::Tick()
{
	if (--m_Life <= 0)
	{
		GameServer()->m_World.DestroyEntity(this);
		return;
	}

	
	if (m_Life < 100)
		m_Flashing = true;
	
	// a small visual effect before disappearing
	if (m_Flashing)
	{
		m_FlashTimer--;
		if (m_FlashTimer <= 0)
			m_FlashTimer = 20;
	}
	
	
	// Check if a bot intersected us
	CCharacter *pChr = GameServer()->m_World.ClosestCharacter(m_Pos, 20.0f, 0);
	if(pChr && pChr->IsAlive() && pChr->GetPlayer()->m_pAI)
	{
		m_Life = 0;
		GameServer()->CreateExplosion(m_Pos+vec2(0, -16)+vec2(-24, -24), m_Owner, WEAPON_HAMMER, false);
		GameServer()->CreateExplosion(m_Pos+vec2(0, -16)+vec2(24, -24), m_Owner, WEAPON_HAMMER, false);
		GameServer()->CreateExplosion(m_Pos+vec2(0, -16)+vec2(24, 24), m_Owner, WEAPON_HAMMER, false);
		GameServer()->CreateExplosion(m_Pos+vec2(0, -16)+vec2(-24, 24), m_Owner, WEAPON_HAMMER, false);
	}
}

void CLandmine::TickPaused()
{
	++m_Life;
}

void CLandmine::Snap(int SnappingClient)
{
	if (m_FlashTimer > 10)
		return;
	
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_ID, sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = POWERUP_ARMOR;
	pP->m_Subtype = 2;
}
