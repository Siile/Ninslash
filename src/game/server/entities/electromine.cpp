#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include <game/server/entities/lightning.h>

#include "electromine.h"

#define RAD 0.017453292519943295769236907684886f

CElectromine::CElectromine(CGameWorld *pGameWorld, vec2 Pos, int Owner)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PICKUP)
{
	m_ProximityRadius = 14;
	m_Life = 4000;
	m_Pos = Pos;
	m_Owner = Owner;
	
	m_Flashing = false;
	m_FlashTimer = 0;
	
	m_ElectroTimer = 0;
	
	GameWorld()->InsertEntity(this);
}

void CElectromine::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

void CElectromine::Tick()
{
	if (m_Life <= 0)
	{
		GameServer()->m_World.DestroyEntity(this);
		return;
	}

	
	if (m_ElectroTimer == 0)
	{
		if (m_Life-- < 100)
			m_Flashing = true;
		
		// a small visual effect before disappearing
		if (m_Flashing)
		{
			m_FlashTimer--;
			if (m_FlashTimer <= 0)
				m_FlashTimer = 25;
		}
		
		// Check if a bot intersected us
		CCharacter *pChr = GameServer()->m_World.ClosestCharacter(m_Pos, 20.0f, 0);
		if(pChr && pChr->IsAlive() && pChr->GetPlayer()->m_pAI)
		{
			m_ElectroTimer++;
		}
	}
	else
	{
		m_FlashTimer = 0;
		if (m_ElectroTimer%5 == 1)
		{
			float Angle = frandom()*1 - frandom()*1;
			Angle = (-90 + frandom()*25 - frandom()*25) * RAD;
			new CLightning(GameWorld(), m_Pos+vec2(0, -12), vec2(cosf(Angle), sinf(Angle)), 50, 50, m_Owner, 1, 1);
			
			Angle = frandom()*360 * RAD;
			new CLightning(GameWorld(), m_Pos+vec2(frandom()*20-frandom()*20, -16 - frandom()*20), vec2(cosf(Angle), sinf(Angle)), 50, 50, m_Owner, 1, 1);
		}
		
		if (m_ElectroTimer++ > 30)
			m_Life = 0;
	}
}

void CElectromine::TickPaused()
{
	++m_Life;
}

void CElectromine::Snap(int SnappingClient)
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
	pP->m_Subtype = 1;
}
