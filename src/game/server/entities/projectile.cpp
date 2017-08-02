#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "projectile.h"
#include "building.h"
#include "droid.h"
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
	m_Bouncy = false;
	
	m_PowerLevel = 0;

	m_ElectroTimer = 0;
	
	m_OwnerBuilding = NULL;
	BounceTick = 0;
	
	if (Weapon == W_DROID_STAR)
		m_Explosive = EXPLOSION_GREEN;
	
	GameWorld()->InsertEntity(this);
}

void CProjectile::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}


void CProjectile::SetPowerLevel(int PowerLevel)
{
	m_PowerLevel = PowerLevel;
	
	if (PowerLevel > 0)
	{
		if (m_Weapon == WEAPON_RIFLE)
		{
			m_Damage += PowerLevel*3;
			m_Bouncy = true;
			m_LifeSpan *= 1.1f;
		}
		
		if (m_Weapon == WEAPON_SHOTGUN)
		{
			m_LifeSpan *= 1.2f;
			m_Damage += 1;
		}
	}
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
			//Curvature = GameServer()->Tuning()->m_GunCurvature;
			//Speed = GameServer()->Tuning()->m_GunSpeed;
			//break;
			
		case W_DROID_WALKER:
			Curvature = GameServer()->Tuning()->m_WalkerCurvature;
			Speed = GameServer()->Tuning()->m_WalkerSpeed;
			break;
			
		case W_DROID_STAR:
			Curvature = GameServer()->Tuning()->m_StarDroidCurvature;
			Speed = GameServer()->Tuning()->m_StarDroidSpeed;
			break;
	}

	if (m_Weapon == W_DROID_STAR)
		return CalcLogPos(m_Pos, m_Direction, Curvature, Speed, Time);
	
	return CalcPos(m_Pos, m_Direction, Curvature, Speed, Time);
}


bool CProjectile::Bounce(vec2 Pos)
{
	if (m_Bouncy)
	{
		BounceTick = Server()->Tick();
	
		int top = GameServer()->Collision()->GetCollisionAt(Pos.x, Pos.y-8);
		int bot = GameServer()->Collision()->GetCollisionAt(Pos.x, Pos.y+8);
		int left = GameServer()->Collision()->GetCollisionAt(Pos.x-8, Pos.y);
		int right = GameServer()->Collision()->GetCollisionAt(Pos.x+8, Pos.y);
		
		int c = (top > 0) + (bot > 0) + (left > 0) + (right > 0);
		
		if (c == 4)
		{
			m_Direction.y *= -1;
			m_Direction.x *= -1;
		}
		else
		{
			if(!top && bot)
				m_Direction.y *= -1;
			if(!bot && top)
				m_Direction.y *= -1;
			if(!left && right)
				m_Direction.x *= -1;
			if(!right && left)
				m_Direction.x *= -1;
		}
		
		return true;
	}
	
	return false;
}


void CProjectile::Tick()
{
	float Pt = (Server()->Tick()-m_StartTick-1)/(float)Server()->TickSpeed();
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	vec2 PrevPos = GetPos(Pt);
	vec2 CurPos = GetPos(Ct);
	int Collide = 0;
	CCharacter *OwnerChar = GameServer()->GetPlayerChar(m_Owner);
	CCharacter *TargetChr = NULL;
	CCharacter *ReflectChr = NULL;
	
	Collide = GameServer()->Collision()->IntersectLine(PrevPos, CurPos, &CurPos, 0);
		
	if (m_Weapon != W_DROID_STAR)
	{
		TargetChr = GameServer()->m_World.IntersectCharacter(PrevPos, CurPos, 6.0f, CurPos, OwnerChar);
		ReflectChr = GameServer()->m_World.IntersectScythe(PrevPos, CurPos, 50.0f, CurPos, OwnerChar);
	}
	
	int Team = -1;
	if (m_Owner == RED_BASE)
		Team = TEAM_RED;
	else if (m_Owner == BLUE_BASE)
		Team = TEAM_BLUE;
	else if (OwnerChar)
		Team = OwnerChar->GetPlayer()->GetTeam();
	
	CBuilding *TargetBuilding = NULL;
	
	if (m_Weapon != W_DROID_STAR)
		TargetBuilding = GameServer()->m_World.IntersectBuilding(PrevPos, CurPos, 6.0f, CurPos, Team);
	
	
	
	if (m_OwnerBuilding == TargetBuilding)
		TargetBuilding = NULL;
	
	CDroid *TargetMonster = NULL;
	
	if (m_Weapon != W_DROID_STAR)
		TargetMonster = GameServer()->m_World.IntersectWalker(PrevPos, CurPos, 6.0f, CurPos);
	
	if (m_Owner == NEUTRAL_BASE)
		TargetMonster = NULL;
	
	m_LifeSpan--;

	if (Collide && Bounce(CurPos))
	{
		m_StartTick = Server()->Tick()-1;
		//m_Direction.y *= -1;
		m_Pos = CurPos;
		Collide = false;
	}
	
	if (ReflectChr)
	{
		m_StartTick = Server()->Tick()-1;
		m_Pos = CurPos;
		
		m_Direction.y *= -1;
		m_Direction.x *= -1;
		
		vec2 d = ReflectChr->m_Pos-m_Pos;
		d += vec2(frandom()-frandom(), frandom()-frandom()) * length(d) * 0.4f;
		m_Direction = -normalize(d);
		GameServer()->CreateBuildingHit(CurPos);
	}
	
	if(TargetMonster || TargetBuilding || TargetChr || Collide || m_LifeSpan < 0 || GameLayerClipped(CurPos))
	{
		if(m_LifeSpan >= 0 || m_Weapon == WEAPON_GRENADE || m_Weapon == WEAPON_FLAMER || m_Weapon == WEAPON_ELECTRIC)
			GameServer()->CreateSound(CurPos, m_SoundImpact);

		if(m_Explosive == EXPLOSION_FLAME)
		{		
			GameServer()->CreateFlameExplosion(CurPos, m_Owner, m_Weapon, false, m_OwnerBuilding ? true : false);
		}
		
		else if(m_Explosive == EXPLOSION_ELECTRIC)
		{		
			GameServer()->CreateElectricExplosion(CurPos, m_Owner, m_Weapon, m_PowerLevel, false, m_OwnerBuilding ? true : false);
		}
	
		else if(m_Explosive == EXPLOSION_GREEN)
		{
			GameServer()->CreateExplosion(CurPos, m_Owner, m_Weapon, -1, false, m_OwnerBuilding ? true : false);
			GameServer()->CreateSound(CurPos, SOUND_GREEN_EXPLOSION);
		}
		else if(m_Explosive == EXPLOSION_EXPLOSION)
		{
			GameServer()->CreateExplosion(CurPos, m_Owner, m_Weapon, m_PowerLevel, false, m_OwnerBuilding ? true : false);
		}

		else if(TargetChr)
		{
			vec2 Force = m_Direction * max(0.001f, m_Force);
			
			if (m_Weapon == WEAPON_RIFLE && m_PowerLevel > 1)
			{
				TargetChr->Electrocute(0.15f);
				Force *= 2.0f;
			}
			
			TargetChr->TakeDamage(Force, m_Damage, m_Owner, m_Weapon, CurPos, DAMAGETYPE_NORMAL, m_OwnerBuilding ? true : false);
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
	if (GameServer()->Collision()->IsInFluid(PrevPos.x, PrevPos.y))
		GameServer()->m_World.DestroyEntity(this);
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
	pProj->m_PowerLevel = m_PowerLevel;
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
