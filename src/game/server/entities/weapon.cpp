#include <game/server/gamecontext.h>
#include <game/weapons.h>
#include "laser.h"
#include "weapon.h"

inline vec2 RandomDir() { return normalize(vec2(frandom()-0.5f, frandom()-0.5f)); }

CWeapon::CWeapon(CGameWorld *pGameWorld, int Type)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_WEAPON)
{
	m_ProximityRadius = ms_PhysSize;
	m_WeaponType = Type;

	if (WeaponMaxLevel(m_WeaponType) > 0)
		m_PowerLevel = GetWeaponCharge(m_WeaponType);
	else
		m_PowerLevel = 0;
	
	Reset();
	
	GameWorld()->InsertEntity(this);
}

void CWeapon::Reset()
{
	m_MaxLevel = WeaponMaxLevel(m_WeaponType);
	m_Disabled = false;
	m_IsTurret = false;
	m_ChargeSoundTimer = 0;
	m_TriggerTick = 0;
	m_ReloadTimer = 0;
	m_BurstReloadTimer = 0;
	m_Pos = vec2(0, 0);
	m_Direction = vec2(0, 0);
	m_Owner = TEAM_NEUTRAL;
	m_ChargeLocked = false;
	m_Charge = 0;
	m_LastNoAmmoSound = -1;
	m_Vel = vec2(0, 0);
	m_Released = false;
	m_DestructionTick = 0;
	m_AttackTick = 0;
	m_AngleForce = 0.0f;
	m_Angle = 0.0f;
	m_TriggerCount = 0;
	m_BurstCount = 0;
	m_BurstMax = 0;
	m_Stuck = false;
	
	UpdateStats();
	m_Ammo = m_MaxAmmo;
}
	
void CWeapon::SetCharge(int Charge)
{
	if (!m_ChargeLocked)
		m_Charge = Charge;
}

void CWeapon::SetOwner(int CID)
{
	m_Owner = CID;
	m_Disabled = false;
}

void CWeapon::OnOwnerDeath(bool IsActive)
{
	m_Owner = TEAM_NEUTRAL;
	Deactivate();
	
	if (IsActive)
	{
		if (Drop())
		{
			m_TriggerTick = 0;
			m_TriggerCount = 0;
		}
	}
	else
	{
		GameServer()->m_World.RemoveEntity(this);
	}
}

void CWeapon::Deactivate()
{
	if (GetWeaponFiringType(m_WeaponType) == WFT_CHARGE)
		m_Charge = 0;
}

void CWeapon::Clear()
{
	GameServer()->m_World.RemoveEntity(this);
}	

void CWeapon::SetPos(vec2 Pos, vec2 Vel, vec2 Direction, float Radius)
{
	m_Pos = Pos;
	m_Vel = Vel;
	m_Direction = Direction;
	m_ProximityRadius = Radius;
	m_Disabled = false;
}

void CWeapon::OnPlayerPick()
{
	m_AttackTick = 0;
}


void CWeapon::SetTurret(bool TurretBit)
{
	if (!TurretBit && IsBuilding(m_WeaponType))
		m_WeaponType ^= BIT_TURRET;
	
	if (TurretBit && !IsBuilding(m_WeaponType))
		m_WeaponType ^= BIT_TURRET;
	
	m_IsTurret = TurretBit;
}

void CWeapon::UpdateStats()
{
	m_CanFire = true;
	m_MaxLevel = WeaponMaxLevel(m_WeaponType);

	if (m_MaxLevel > 0)
		m_WeaponType = GetChargedWeapon(m_WeaponType, m_PowerLevel);
	
	m_FireRate = GetWeaponFireRate(m_WeaponType);
	m_KnockBack =GetWeaponKnockback(m_WeaponType);
	m_FireSound = GetWeaponFireSound(m_WeaponType);
	m_FireSound2 = GetWeaponFireSound2(m_WeaponType);
	m_FullAuto = GetWeaponFullAuto(m_WeaponType);
	m_MaxAmmo = GetWeaponMaxAmmo(m_WeaponType);
	m_UseAmmo = WeaponUseAmmo(m_WeaponType);
	m_BurstMax = WeaponBurstCount(m_WeaponType);
}


bool CWeapon::Activate()
{
	if (IsStaticWeapon(m_WeaponType))
	{
		switch (GetStaticType(m_WeaponType))
		{
			case SW_INVIS: case SW_SHIELD:
			{
				GameServer()->m_pController->TriggerWeapon(this);
				m_DestructionTick = 1;
			} break;
			
			default: break;
		}
	}
	
	
	return true;
}



bool CWeapon::Fire(float *pKnockback)
{
	m_Disabled = false;
	
	if (!m_CanFire)
		return false;
	
	// check if reloading
	if(m_ReloadTimer > 0)
		return false;
	
	int WFT = GetWeaponFiringType(m_WeaponType);
	
	if (WFT == WFT_NONE)
		return false;
	
	if (WFT == WFT_ACTIVATE)
	{
		return Activate();
	}
	
	// check for ammo
	if (m_IsTurret)
		m_Ammo = m_MaxAmmo;
	
	if (m_UseAmmo && m_MaxAmmo > 0 && m_Ammo <= 0)
	{
		if(m_LastNoAmmoSound+Server()->TickSpeed() <= Server()->Tick())
		{
			GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO);
			m_LastNoAmmoSound = Server()->Tick();
		}
		return false;
	}
	
	UpdateStats();
	
	if (m_BurstMax > 1 && ++m_BurstCount >= m_BurstMax)
		m_BurstCount = 0;
	
	if (m_BurstCount > 0)
	{
		m_ReloadTimer = m_FireRate * WeaponBurstReload(m_WeaponType) * Server()->TickSpeed()/1000;
		m_BurstReloadTimer = m_FireRate * Server()->TickSpeed()/1000;
	}
	else
	{
		m_ReloadTimer = m_FireRate * Server()->TickSpeed()/1000;
		m_BurstReloadTimer = 0;
	}
	
	// longer reload for turrets
	if (m_IsTurret)
		m_ReloadTimer *= 1.5f;
		
	if (WFT == WFT_PROJECTILE)
	{
		// create the projectile & effects
		CreateProjectile();
		
		// set knockback
		if (pKnockback)
			*pKnockback = m_KnockBack;
		
		// reduce ammo
		if (m_Ammo > 0)
			m_Ammo--;
		
		return true;
	}
	
	if (WFT == WFT_MELEE)
	{
		// create the projectile & effects
		CreateProjectile();
		
		// set knockback
		if (pKnockback)
			*pKnockback = m_KnockBack;
		
		// reduce ammo
		if (m_Ammo > 0)
			m_Ammo--;
		
		return true;
	}
	
	if (WFT == WFT_HOLD)
	{
		if (GetStaticType(m_WeaponType) == SW_FLAMER)
			m_TriggerTick = Server()->Tick() + m_FireRate * 2 * Server()->TickSpeed()/1000;
		else
			m_TriggerTick = Server()->Tick() + m_FireRate * Server()->TickSpeed()/1000;
		
		// set knockback
		if (pKnockback)
			*pKnockback = m_KnockBack;
		
		// reduce ammo
		if (m_Ammo > 0)
			m_Ammo--;
		
		if (m_FireSound >= 0)
			GameServer()->CreateSound(m_Pos, m_FireSound);
	
		return true;
	}
	
	return false;
}


int CWeapon::GetCharge()
{
	return m_Charge;
}


bool CWeapon::Charge()
{
	if (!m_CanFire)
		return false;
	
	// check if reloading
	if(m_ReloadTimer > 0)
		return false;
	
	// trigger grenade destruction timer on charge start
	if (!m_DestructionTick && IsStaticWeapon(m_WeaponType))
	{
		switch (GetStaticType(m_WeaponType))
		{
			case SW_GRENADE1:
				m_AttackTick = Server()->Tick();
				m_DestructionTick = Server()->Tick() + 2.0f * Server()->TickSpeed();
				break;
			case SW_GRENADE2:
				m_AttackTick = Server()->Tick();
				m_TriggerTick = Server()->Tick() + 2.0f * Server()->TickSpeed();
				m_DestructionTick = Server()->Tick() + 4.0f * Server()->TickSpeed();
				break;
			default: break;
		}
	}
	
	if (GetWeaponFiringType(m_WeaponType) == WFT_THROW)
		m_Charge = min(m_Charge+3, 100);
	else
		m_Charge = min(m_Charge+1, 100);

	
	return true;
}


bool CWeapon::ReleaseCharge(float *pKnockback)
{
	if (!m_CanFire)
	{
		m_Charge = 0;
		return false;
	}
	
	if (m_Charge > 0)
	{
		if (GetWeaponFiringType(m_WeaponType) == WFT_CHARGE)
		{
			if (IsModularWeapon(m_WeaponType) && GetPart(m_WeaponType, 0) == 1)
			{
				m_TriggerCount = GetWeaponCharge(m_WeaponType);
				
				if (m_TriggerCount)
					m_TriggerTick = Server()->Tick() + m_FireRate * 0.5f * Server()->TickSpeed() / 1000;
			}
			
			CreateProjectile();
			m_Charge = 0;
		}
		//else if (GetWeaponFiringType(m_WeaponType) == WFT_THROW)
		//	Throw();
		
		m_ReloadTimer = m_FireRate * Server()->TickSpeed() / 1000;
		
		return true;
	}
	
	m_Charge = 0;
	return false;
}

bool CWeapon::Throw()
{
	if (m_Released)
		return false;
	
	vec2 v = vec2(0, 0);
	
	v.x = sin(m_Direction.x)*(m_Direction.x > 0.0f ? 1 : -1)*m_Vel.x;
	v.y = sin(m_Direction.y)*(m_Direction.y > 0.0f ? 1 : -1)*m_Vel.y;
	
	m_Vel = v*1.0f + m_Direction * m_Charge * 0.24f * WeaponThrowForce(m_WeaponType);

	m_Angle = 0.0f;
	m_AngleForce = m_Vel.x * 0.3f;
	
	if (GetStaticType(m_WeaponType) == SW_SHURIKEN)
	{
		m_AngleForce = m_Charge*0.1f * (m_Direction.x < 0 ? -1.0f : 1.0f);
		m_AttackTick = Server()->Tick();
	}
	
	m_Charge = 0;
	m_Released = true;
	return true;
}



void CWeapon::CreateProjectile()
{
	vec2 offs = GetProjectileOffset(m_WeaponType);
	vec2 ProjStartPos = m_Pos+m_Direction*offs.x + vec2(0, offs.y);
	
	if (GetWeaponFiringType(m_WeaponType) == WFT_PROJECTILE)
		GameServer()->Collision()->IntersectLine(m_Pos, ProjStartPos, 0x0, &ProjStartPos);
	
	GameServer()->CreateProjectile(m_Owner, m_WeaponType, m_Charge, ProjStartPos, m_Direction);
	
	if (m_FireSound >= 0 && GetWeaponFiringType(m_WeaponType) != WFT_HOLD)
	{
		GameServer()->CreateSound(m_Pos, m_FireSound);
		
		if (m_FireSound2 >= 0)
			GameServer()->CreateSound(m_Pos, m_FireSound2);
	}
}


void CWeapon::ReduceAmmo(int Amount)
{
	m_Ammo = max(0, m_Ammo - Amount);
}


void CWeapon::IncreaseAmmo(int Amount)
{
	m_Ammo = min(m_MaxAmmo, m_Ammo + Amount);
}
	
	
bool CWeapon::CanSwitch()
{
	if (m_Charge > 0 || m_ReloadTimer > 0)
		return false;
		
	return true;
}

bool CWeapon::Drop()
{
	if (m_Charge > 0 && GetWeaponFiringType(m_WeaponType) == WFT_THROW)
		return false;
	
	m_Owner = TEAM_NEUTRAL;
	
	m_TriggerTick = 0;
	m_TriggerCount = 0;
			
	return true;
}

bool CWeapon::AddClip()
{
	if (!m_UseAmmo || !m_MaxAmmo)
		return false;
	
	if (m_Ammo < m_MaxAmmo)
	{
		m_Ammo = min(m_Ammo+m_MaxAmmo/3, m_MaxAmmo);
		return true;
	}
	
	return false;
}

bool CWeapon::Upgrade()
{
	if (m_PowerLevel < m_MaxLevel)
	{
		m_PowerLevel++;
		UpdateStats();
		return true;
	}
	
	return false;
}

void CWeapon::Tick()
{
	if (m_Disabled)
		return;
	
	if (m_MaxLevel > 0)
		m_WeaponType = GetChargedWeapon(m_WeaponType, m_PowerLevel);
	
	// pick me up!
	if (m_Stuck && m_Owner < 0)
	{
		CCharacter *pChr = GameServer()->m_World.ClosestCharacter(m_Pos, 18.0f, 0);
		if(pChr && pChr->IsAlive())
		{
			if (pChr->PickWeapon(this))
			{
				Reset();
				// pickup sound
				
				if (pChr->GetPlayer())
					GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), pChr->m_PickedWeaponSlot);
			}
		}
	}
	
	 // shuriken flying hit
	if (!m_Stuck && IsStaticWeapon(m_WeaponType) && GetStaticType(m_WeaponType) == SW_SHURIKEN)
	{
		if (length(m_Vel) > 20.0f)
			CreateProjectile();
	}
	
	if (m_BurstCount > 0)
		m_FullAuto = true;
	
	//if (IsModularWeapon(m_WeaponType) && !m_TriggerCount)
	//	m_WeaponType = GetChargedWeapon(m_WeaponType, m_Charge / 20);
	
	if(m_ReloadTimer > 0)
		m_ReloadTimer--;
	
	if(m_BurstReloadTimer > 0)
	{
		if (--m_BurstReloadTimer <= 0)
			m_BurstCount = 0;
	}
	
	if (m_Released)
		Move();
	
	
	if (GetWeaponFiringType(m_WeaponType) == WFT_HOLD)
	{
		if (m_TriggerTick && m_TriggerTick > Server()->Tick())
			Trigger();
	}
	else if (m_TriggerTick && m_TriggerTick <= Server()->Tick())
		Trigger();
	
	// charge sound

	if (m_DestructionTick && m_ChargeSoundTimer-- <= 0)
	{
		float d = min(14.0f, (Server()->Tick() - m_AttackTick) * 0.1f);
		m_ChargeSoundTimer = 18 - d;
		GameServer()->CreateSound(m_Pos, SOUND_WEAPON_CHARGE1_1+min(8, int(d)));
	}
	
	
	if (m_DestructionTick && m_DestructionTick <= Server()->Tick())
		SelfDestruct();
}

void CWeapon::Trigger()
{
	if (GetWeaponFiringType(m_WeaponType) == WFT_HOLD)
	{
		CreateProjectile();
		return;
	}
	
	if (IsStaticWeapon(m_WeaponType))
	{
		switch (GetStaticType(m_WeaponType))
		{
			case SW_GRENADE2:
				m_TriggerTick = Server()->Tick() + 0.05f * Server()->TickSpeed();
			
				new CLaser(&GameServer()->m_World, m_Pos, RandomDir(), 160.0f, m_Owner, m_WeaponType, 5, -2);
				new CLaser(&GameServer()->m_World, m_Pos, RandomDir(), 160.0f, m_Owner, m_WeaponType, 5, -2);
				new CLaser(&GameServer()->m_World, m_Pos, RandomDir(), 160.0f, m_Owner, m_WeaponType, 5, -2);
				break;
			default: return;
		}
	}
	
	/*
	if (IsModularWeapon(m_WeaponType))
	{
		m_WeaponType = GetChargedWeapon(m_WeaponType, max(0, GetWeaponCharge(m_WeaponType)-1));
		m_TriggerCount = GetWeaponCharge(m_WeaponType);
		
		if (m_TriggerCount)
		{
			GameServer()->m_pController->TriggerWeapon(this);
			m_TriggerTick = Server()->Tick() + m_FireRate * 0.5f * Server()->TickSpeed() / 1000;
			CreateProjectile();
		}
		else
			m_TriggerTick = 0;
	}
	*/
}

void CWeapon::SelfDestruct()
{
	if (!m_Released)
		GameServer()->m_pController->ReleaseWeapon(this);
	
	if (IsStaticWeapon(m_WeaponType))
	{
		switch (GetStaticType(m_WeaponType))
		{
		case SW_GRENADE1: case SW_GRENADE2: GameServer()->CreateExplosion(m_Pos, m_Owner, m_WeaponType); break;
		default: break;
		}
	}
	
	GameServer()->m_World.RemoveEntity(this);
}


void CWeapon::Move()
{
	if (m_Stuck)
		return;
	
	m_Vel.y += 0.5f;
	
	//m_Vel.y = min(m_Vel.y, 25.0f);
	
	bool Grounded = false;
	if(GameServer()->Collision()->CheckPoint(m_Pos.x+12, m_Pos.y+12+5))
		Grounded = true;
	if(GameServer()->Collision()->CheckPoint(m_Pos.x-12, m_Pos.y+12+5))
		Grounded = true;
		
	int OnForceTile = GameServer()->Collision()->IsForceTile(m_Pos.x-12, m_Pos.y+12+5);
	if (OnForceTile == 0)
		OnForceTile = GameServer()->Collision()->IsForceTile(m_Pos.x+12, m_Pos.y+12+5);
		
	if (Grounded)
	{
		m_Vel.x = (m_Vel.x + OnForceTile*0.7f) * 0.925f;
		//m_Vel.x *= 0.8f;
		m_AngleForce += (m_Vel.x - m_AngleForce) / 2.0f;
	}
	else
	{
		m_Vel.x *= 0.99f;
		m_Vel.y *= 0.99f;
	}
	
	vec2 OldVel = m_Vel;
	GameServer()->Collision()->MoveBox(&m_Pos, &m_Vel, vec2(18.0f, 18.0f), 0.5f);
	
	if ((((OldVel.x < 0 && m_Vel.x > 0) || (OldVel.x > 0 && m_Vel.x < 0)) && abs(m_Vel.x) > 3.0f) ||
		(((OldVel.y < 0 && m_Vel.y > 0) || (OldVel.y > 0 && m_Vel.y < 0)) && abs(m_Vel.y) > 3.0f))
		GameServer()->CreateSound(m_Pos, SOUND_SFX_BOUNCE1);
		
	if (GetStaticType(m_WeaponType) == SW_SHURIKEN)
	{
		if ((((OldVel.x < 0 && m_Vel.x > 0) || (OldVel.x > 0 && m_Vel.x < 0)) && abs(m_Vel.x) > 5.0f) ||
			(((OldVel.y < 0 && m_Vel.y > 0) || (OldVel.y > 0 && m_Vel.y < 0)) && abs(m_Vel.y) > 5.0f))
		{
			m_Pos += OldVel*0.3f;
			GameServer()->CreateExplosion(m_Pos, m_Owner, m_WeaponType);
			GameServer()->CreateSound(m_Pos, SOUND_SFX_BOUNCE1);
			m_Stuck = true;
			m_Owner = -1;
			
			// todo: correct sound & effect
		}
		
		if (abs(m_Vel.x) < 0.1f && abs(m_Vel.y) < 1.0f && GameServer()->Collision()->IsTileSolid(m_Pos.x, m_Pos.y+10.0f))
		{
			m_Stuck = true;
			m_Owner = -1;
		}
	}
	
	if (GetStaticType(m_WeaponType) != SW_SHURIKEN)
	{
		m_AngleForce *= 0.98f;
	}
	
	m_Angle += clamp(m_AngleForce*0.04f, -0.6f, 0.6f);
}
	
void CWeapon::TickPaused()
{
	
}

void CWeapon::Snap(int SnappingClient)
{
	if (m_Disabled)
		return;
	
	if(!m_Released || NetworkClipped(SnappingClient))
		return;
	
	CNetObj_Weapon *pW = static_cast<CNetObj_Weapon *>(Server()->SnapNewItem(NETOBJTYPE_WEAPON, m_ID, sizeof(CNetObj_Weapon)));
	if(!pW)
		return;

	pW->m_X = (int)m_Pos.x;
	pW->m_Y = (int)m_Pos.y;
	pW->m_Angle = (int)(m_Angle*256.0f);
	pW->m_WeaponType = m_WeaponType;
	pW->m_AttackTick = m_AttackTick;
}
