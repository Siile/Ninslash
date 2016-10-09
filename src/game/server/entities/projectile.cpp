#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "projectile.h"
#include "building.h"
#include "monster.h"
#include "electro.h"
#include "superexplosion.h"
#include "smokescreen.h"


CProjectile::CProjectile(CGameWorld *pGameWorld, int Weapon, int Owner, vec2 Pos, vec2 Dir, int Span,
		int Damage, int Explosive, float Force, int SoundImpact)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PROJECTILE)
{
	m_Weapon = Weapon;
	m_Pos = Pos;
	m_Direction = Dir;
	m_LifeSpan = Span;
	m_Owner = Owner;
	m_Force = Force;
	m_Damage = Damage;
	m_SoundImpact = SoundImpact;
	m_StartTick = Server()->Tick();
	m_Explosive = Explosive;

	m_ElectroTimer = 0;
	
	m_OwnerBuilding = NULL;
	
	GameWorld()->InsertEntity(this);
}

void CProjectile::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

vec2 CProjectile::GetPos(float Time)
{
	float Curvature = 0;
	float Speed = 0;

	switch(m_Weapon)
	{
		case WEAPON_GRENADE:
			Curvature = GameServer()->Tuning()->m_GrenadeCurvature;
			Speed = GameServer()->Tuning()->m_GrenadeSpeed;
			break;
			
		case WEAPON_ELECTRIC:
			Curvature = GameServer()->Tuning()->m_ElectricCurvature;
			Speed = GameServer()->Tuning()->m_ElectricSpeed;
			break;

		case WEAPON_FLAMER:
			Curvature = GameServer()->Tuning()->m_FlamerCurvature;
			Speed = GameServer()->Tuning()->m_FlamerSpeed;
			break;

		case WEAPON_SHOTGUN:
			Curvature = GameServer()->Tuning()->m_ShotgunCurvature;
			Speed = GameServer()->Tuning()->m_ShotgunSpeed;
			break;

		case WEAPON_RIFLE:
		case WEAPON_GUN:
			Curvature = GameServer()->Tuning()->m_GunCurvature;
			Speed = GameServer()->Tuning()->m_GunSpeed;
			break;
	}

	return CalcPos(m_Pos, m_Direction, Curvature, Speed, Time);
}


void CProjectile::Tick()
{
	float Pt = (Server()->Tick()-m_StartTick-1)/(float)Server()->TickSpeed();
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	vec2 PrevPos = GetPos(Pt);
	vec2 CurPos = GetPos(Ct);
	int Collide = GameServer()->Collision()->IntersectLine(PrevPos, CurPos, &CurPos, 0);
	CCharacter *OwnerChar = GameServer()->GetPlayerChar(m_Owner);
	CCharacter *TargetChr = GameServer()->m_World.IntersectCharacter(PrevPos, CurPos, 6.0f, CurPos, OwnerChar);
	
	int Team = -1;
	if (m_Owner == RED_BASE)
		Team = TEAM_RED;
	else if (m_Owner == BLUE_BASE)
		Team = TEAM_BLUE;
	else if (OwnerChar)
		Team = OwnerChar->GetPlayer()->GetTeam();
	
	CBuilding *TargetBuilding = GameServer()->m_World.IntersectBuilding(PrevPos, CurPos, 6.0f, CurPos, Team);
	
	if (m_OwnerBuilding == TargetBuilding)
		TargetBuilding = NULL;
	
	CMonster *TargetMonster = GameServer()->m_World.IntersectMonster(PrevPos, CurPos, 6.0f, CurPos);
	
	m_LifeSpan--;

	if(TargetMonster || TargetBuilding || TargetChr || Collide || m_LifeSpan < 0 || GameLayerClipped(CurPos))
	{
		if(m_LifeSpan >= 0 || m_Weapon == WEAPON_GRENADE || m_Weapon == WEAPON_FLAMER || m_Weapon == WEAPON_ELECTRIC)
			GameServer()->CreateSound(CurPos, m_SoundImpact);

		if(m_Explosive == EXPLOSION_FLAME)
		{		
			GameServer()->CreateFlameExplosion(CurPos, m_Owner, m_Weapon, false);
		}
		
		else if(m_Explosive == EXPLOSION_ELECTRIC)
		{		
			GameServer()->CreateElectricExplosion(CurPos, m_Owner, m_Weapon, false);
		}
	
		else if(m_Explosive == EXPLOSION_EXPLOSION)
		{
			GameServer()->CreateExplosion(CurPos, m_Owner, m_Weapon, false);
		}

		else if(TargetChr)
		{
			TargetChr->TakeDamage(m_Direction * max(0.001f, m_Force), m_Damage, m_Owner, m_Weapon, CurPos);
		}
		
		else if(TargetBuilding)
		{
			TargetBuilding->TakeDamage(m_Damage, m_Owner, m_Weapon);
			GameServer()->CreateBuildingHit(CurPos);
			//GameServer()->m_World.DestroyEntity(this);
		}
		
		else if(TargetMonster)
		{
			TargetMonster->TakeDamage(m_Direction * max(0.001f, m_Force), m_Damage, m_Owner, CurPos);
			//GameServer()->CreateMonsterHit(CurPos);
			GameServer()->m_World.DestroyEntity(this);
		}

		GameServer()->m_World.DestroyEntity(this);
	}
	
	// fluid kills the projectile
	if (GameServer()->Collision()->GetCollisionAt(PrevPos.x, PrevPos.y) == CCollision::COLFLAG_DAMAGEFLUID)
	{
		GameServer()->m_World.DestroyEntity(this);
	}
}

void CProjectile::TickPaused()
{
	++m_StartTick;
}

void CProjectile::FillInfo(CNetObj_Projectile *pProj)
{
	pProj->m_X = (int)m_Pos.x;
	pProj->m_Y = (int)m_Pos.y;
	pProj->m_VelX = (int)(m_Direction.x*100.0f);
	pProj->m_VelY = (int)(m_Direction.y*100.0f);
	pProj->m_StartTick = m_StartTick;
	pProj->m_Type = m_Weapon;
}

void CProjectile::Snap(int SnappingClient)
{
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();

	if(NetworkClipped(SnappingClient, GetPos(Ct)))
		return;

	CNetObj_Projectile *pProj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_ID, sizeof(CNetObj_Projectile)));
	if(pProj)
		FillInfo(pProj);
}
