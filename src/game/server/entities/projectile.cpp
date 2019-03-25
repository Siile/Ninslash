#include <game/generated/protocol.h>
#include <game/collision.h>
#include <game/server/gamecontext.h>
#include <game/weapons.h>
#include "projectile.h"
#include "ball.h"
#include "building.h"
#include "droid.h"
#include "electro.h"
#include "superexplosion.h"

inline vec2 RandomDir() { return normalize(vec2(frandom()-0.5f, frandom()-0.5f)); }

CProjectile::CProjectile(CGameWorld *pGameWorld, int Weapon, int Owner, vec2 Pos, vec2 Dir, vec2 Vel, int Span,
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
	m_Bounces = 0;
	m_Vel2 = Vel*30.0f;

	m_ElectroTimer = 0;
	
	m_OwnerBuilding = NULL;
	BounceTick = 0;
	
	UpdateStats();
	
	GameWorld()->InsertEntity(this);
}

void CProjectile::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}


void CProjectile::UpdateStats()
{
	m_Part1 = GetPart(m_Weapon, 0);
	m_Part2 = GetPart(m_Weapon, 1);
	m_Speed = GetProjectileSpeed(m_Weapon);
	m_Curvature = GetProjectileCurvature(m_Weapon);
	m_Bounces = IsProjectileBouncy(m_Weapon);
}


vec2 CProjectile::GetPos(float Time)
{
	if (WeaponProjectilePosType(m_Weapon) == 1)
		return CalcLogPos(m_Pos, m_Direction, m_Vel2, m_Curvature, m_Speed, Time);
	
	if (WeaponProjectilePosType(m_Weapon) == 2)
		return CalcRocketPos(m_Pos, m_Direction, m_Vel2, m_Curvature, m_Speed, Time);
	
	return CalcPos(m_Pos, m_Direction, m_Vel2, m_Curvature, m_Speed, Time);
}




// todo: fix broken bouncing
bool CProjectile::Bounce(vec2 Pos, int Collision)
{
	if (m_Bounces-- > 0)
	{
		BounceTick = Server()->Tick();
	
	/*
		if (Collision == CCollision::COLFLAG_RAMP_LEFT)
			m_Direction = Reflect(m_Direction, normalize(vec2(1, -1)));
		else if (Collision == CCollision::COLFLAG_RAMP_RIGHT)
			m_Direction = Reflect(m_Direction, normalize(vec2(-1, -1)));
		else if (Collision == CCollision::COLFLAG_ROOFSLOPE_LEFT)
			m_Direction = Reflect(m_Direction, normalize(vec2(1, 1)));
		else if (Collision == CCollision::COLFLAG_ROOFSLOPE_RIGHT)
			m_Direction = Reflect(m_Direction, normalize(vec2(-1, 1)));
		else
		{
			if (!GameServer()->Collision()->GetCollisionAt(Pos.x, Pos.y-8) || !GameServer()->Collision()->GetCollisionAt(Pos.x, Pos.y+8))
				m_Direction = Reflect(m_Direction, normalize(vec2(0, -1)));
			else
				m_Direction = Reflect(m_Direction, normalize(vec2(-1, 0)));
		}
		*/
		
		m_Direction = GameServer()->Collision()->WallReflect(Pos, m_Direction, Collision);
		
		if (GetStaticType(m_Weapon) == SW_CLUSTER)
			GameServer()->CreateSound(Pos, SOUND_SFX_BOUNCE1);
		else
			GameServer()->CreateSound(Pos, SOUND_BOUNCER_BOUNCE);
	
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
	
	float r = 6.0f * GetProjectileSize(m_Weapon);
	
	ReflectChr = GameServer()->m_World.IntersectReflect(PrevPos, CurPos, r*0.8f, CurPos, OwnerChar);
	
	if (!ReflectChr)
		TargetChr = GameServer()->m_World.IntersectCharacter(PrevPos, CurPos, r, CurPos, OwnerChar);
	
	int Team = m_Owner;
	
	if (OwnerChar && GameServer()->m_pController->IsTeamplay())
		Team = OwnerChar->GetPlayer()->GetTeam();
	
	CBuilding *TargetBuilding = NULL;
	
	TargetBuilding = GameServer()->m_World.IntersectBuilding(PrevPos, CurPos, r, CurPos, Team);
	
	CBall *Ball = NULL;
	Ball = GameServer()->m_World.IntersectBall(PrevPos, CurPos, r, CurPos);
	
	bool Shielded = GameServer()->m_World.IsShielded(PrevPos, CurPos, r, Team);
	
	if (m_OwnerBuilding == TargetBuilding)
		TargetBuilding = NULL;
	
	CDroid *TargetMonster = NULL;
	
	TargetMonster = GameServer()->m_World.IntersectWalker(PrevPos, CurPos, r, CurPos);
	
	if (m_Owner == NEUTRAL_BASE)
		TargetMonster = NULL;
	
	m_LifeSpan--;

	if (Collide && Bounce(CurPos, Collide))
	{
		m_StartTick = Server()->Tick()-1;
		m_Pos = CurPos;
		Collide = false;
	}
	
	if (ReflectChr)
	{
		m_StartTick = Server()->Tick()-1;
		m_Pos = CurPos;
		
		//m_Direction.y *= -1;
		//m_Direction.x *= -1;
		
		//vec2 d = (ReflectChr->m_Pos+vec2(0, -24))-PrevPos;
		//d += vec2(frandom()-frandom(), frandom()-frandom()) * length(d) * 0.4f;
		//m_Direction = -normalize(d);
		
		vec2 d = (ReflectChr->m_Pos+vec2(0, -24)) - PrevPos;
		m_Direction = GameServer()->Collision()->Reflect(m_Direction, normalize(d));
		GameServer()->CreateBuildingHit(CurPos);
	}
	
	if (Collide)
	{
		if (GameServer()->Collision()->CheckBlocks(CurPos))
			GameServer()->DamageBlocks(CurPos, m_Damage, 1);
		else if (GameServer()->Collision()->CheckBlocks(CurPos+vec2(-4, -4)))
			GameServer()->DamageBlocks(CurPos+vec2(-4, -4), m_Damage, 1);
		else if (GameServer()->Collision()->CheckBlocks(CurPos+vec2(4, -4)))
			GameServer()->DamageBlocks(CurPos+vec2(4, -4), m_Damage, 1);
		else if (GameServer()->Collision()->CheckBlocks(CurPos+vec2(-4, 4)))
			GameServer()->DamageBlocks(CurPos+vec2(-4, 4), m_Damage, 1);
		else if (GameServer()->Collision()->CheckBlocks(CurPos+vec2(4, 4)))
			GameServer()->DamageBlocks(CurPos+vec2(4, 4), m_Damage, 1);
	}
	
	if(Ball || TargetMonster || TargetBuilding || TargetChr || Collide || m_LifeSpan < 0 || Shielded || GameLayerClipped(CurPos))
	{
		if(TargetChr)
		{
			vec2 Force = m_Direction * max(0.001f, m_Force);
			TargetChr->TakeDamage(m_Owner, m_Weapon, m_Damage, Force, CurPos);
			
			GameServer()->CreateEffect(FX_BLOOD2, (CurPos+TargetChr->m_Pos)/2.0f + vec2(0, -4));
		}
		
		if (Shielded)
			GameServer()->CreateEffect(FX_SHIELDHIT, CurPos);

		if(TargetBuilding)
		{
			vec2 Force = m_Direction * max(0.001f, m_Force);
			
			if (TargetBuilding->m_Type == BUILDING_GENERATOR)
			{
				TargetBuilding->m_DamagePos = CurPos;
				
				if (distance(TargetBuilding->m_Pos, CurPos) > TargetBuilding->m_ProximityRadius)
				{
					GameServer()->CreateEffect(FX_SHIELDHIT, CurPos);
					TargetBuilding->TakeDamage(m_Damage/3, m_Owner, m_Weapon, Force);
				}
				else
				{
					GameServer()->CreateBuildingHit(CurPos);
					TargetBuilding->TakeDamage(m_Damage, m_Owner, m_Weapon, Force);
				}
			}
			else
			{
				GameServer()->CreateBuildingHit(CurPos);
				TargetBuilding->TakeDamage(m_Damage, m_Owner, m_Weapon, Force);
			}
		}
		
		if (TargetMonster)
		{
			TargetMonster->TakeDamage(m_Direction * max(0.001f, m_Force), m_Damage, m_Owner, CurPos, m_Weapon);
		}
		
		if (Ball)
		{
			vec2 Force = m_Direction * max(0.001f, m_Force);
			Ball->AddForce(Force);
			GameServer()->m_pController->m_LastBallToucher = m_Owner;
		}
		
		// cluster grenades
		if (IsStaticWeapon(m_Weapon) && GetStaticType(m_Weapon) == SW_CLUSTER && GetWeaponCharge(m_Weapon) < 15)
		{
			for (int i = 0; i < 1+GetWeaponLevelCharge(m_Weapon)*2.0f; i++)
			{
				GameServer()->CreateProjectile(m_Owner, GetChargedWeapon(GetStaticWeapon(SW_CLUSTER), 15), 0, PrevPos, normalize(RandomDir()), PrevPos);
			}
		}
		
		if (m_LifeSpan < 0)
			GameServer()->CreateExplosion(PrevPos, m_Owner, m_Weapon);
		else if (IsExplosiveProjectile(m_Weapon))
			GameServer()->CreateExplosion(CurPos, m_Owner, m_Weapon);
		
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
	pProj->m_Vel2X = (int)(m_Vel2.x*10.0f);
	pProj->m_Vel2Y = (int)(m_Vel2.y*10.0f);
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
