#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "character.h"
#include "building.h"
#include "deathray.h"

CDeathray::CDeathray(CGameWorld *pGameWorld, vec2 Pos)
: CBuilding(pGameWorld, Pos, BUILDING_LAZER, TEAM_NEUTRAL)
{
	m_ProximityRadius = TurretPhysSize;
	m_Life = 100;
	m_AttackTick = Server()->Tick() - Server()->TickSpeed()*3.4f + int(m_Pos.x/4)%int(Server()->TickSpeed()*3.4f);
	m_Loading = true;
	m_Height = 0;
}



void CDeathray::Tick()
{
	if (m_SnapTick && m_SnapTick < Server()->Tick()-Server()->TickSpeed()*5.0f)
	{
		if (GameServer()->StoreEntity(m_ObjType, m_Type, 0, m_Pos.x, m_Pos.y))
		{
			GameServer()->m_World.DestroyEntity(this);
			return;
		}
	}
	
	if (m_Loading)
	{
		if (m_AttackTick + Server()->TickSpeed()*3.4f < GameServer()->Server()->Tick())
		{
			m_Loading = false;
			m_AttackTick = GameServer()->Server()->Tick();
			m_Height = GameServer()->CreateDeathray(m_Pos+vec2(0, 8));
		}
	}
	else
	{
		if (m_AttackTick + Server()->TickSpeed()*3 < GameServer()->Server()->Tick())
		{
			m_Loading = true;
			GameServer()->CreateEffect(FX_LAZERLOAD, m_Pos+vec2(0, 32));
		}
	}
	
	
	// death ray on characters & buildings
	if (m_AttackTick > GameServer()->Server()->Tick() - Server()->TickSpeed()*0.2f)
	{
		vec2 At;
		CCharacter *pHit = GameServer()->m_World.IntersectCharacter(m_Pos+vec2(0, 32), m_Pos+vec2(0, m_Height), 4.0f, At);
		
		if(pHit)
			pHit->Deathray();
		
		CBuilding *pBHit = GameServer()->m_World.IntersectBuilding(m_Pos+vec2(0, 48), m_Pos+vec2(0, m_Height), 4.0f, At, -666);
		
		if(pBHit && pBHit != this)
			pBHit->TakeDamage(1000, -1, -1);
	}
}
