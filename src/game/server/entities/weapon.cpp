#include <game/server/gamecontext.h>
#include <game/weapons.h>
#include "weapon.h"

CWeapon::CWeapon(CGameWorld *pGameWorld, int Type)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_WEAPON)
{
	m_ProximityRadius = ms_PhysSize;
	m_Type = Type;
	m_PowerLevel = 0;
	Reset();
	
	GameWorld()->InsertEntity(this);
}

void CWeapon::Reset()
{
	m_FullAuto = aCustomWeapon[m_Type].m_FullAuto;
	m_MaxAmmo = aCustomWeapon[m_Type].m_MaxAmmo;
	m_Ammo = m_MaxAmmo;
	m_ReloadTimer = 0;
	m_Pos = vec2(0, 0);
	m_Direction = vec2(0, 0);
	m_Owner = -1;
}
	
void CWeapon::SetOwner(int CID)
{
	m_Owner = CID;
}

void CWeapon::OnOwnerDeath(bool IsActive)
{
	m_Owner = -1;
		
	if (IsActive)
	{
		Drop();
	}
	else
	{
		GameServer()->m_World.RemoveEntity(this);
	}
}


void CWeapon::Clear()
{
	GameServer()->m_World.RemoveEntity(this);
}	

void CWeapon::SetPos(vec2 Pos, vec2 Direction, float Radius)
{
	m_Pos = Pos;
	m_Direction = Direction;
	m_ProximityRadius = Radius;
}


bool CWeapon::Fire(float *pKnockback)
{
	if (m_Type < 0 || m_Type >= NUM_CUSTOMWEAPONS)
		return false;
	
	// check if reloading
	if(m_ReloadTimer > 0)
		return false;
	
	// check for ammo

	// weapon knockback to player
	
	// play sound
	if (aCustomWeapon[m_Type].m_Sound >= 0)
		GameServer()->CreateSound(m_Pos, aCustomWeapon[m_Type].m_Sound);
	
	
	// create the projectile & effects
	CreateProjectile();
	
	// set reload
	m_ReloadTimer = aCustomWeapon[m_Type].m_BulletReloadTime * Server()->TickSpeed() / 1000;
	
	// set knockback
	if (pKnockback)
		*pKnockback = aCustomWeapon[m_Type].m_SelfKnockback;
	
	return true;
}


void CWeapon::CreateProjectile()
{

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
	return true;
}

bool CWeapon::Drop()
{
	return true;
}

bool CWeapon::Upgrade()
{
	if (m_PowerLevel < 2)
	{
		m_PowerLevel++;
		return true;
	}
	
	return false;
}

bool CWeapon::AddClip()
{
	if (m_Type < 0 || m_Type >= NUM_CUSTOMWEAPONS)
		return false;
	
	if (aCustomWeapon[m_Type].m_PowerupSize <= 0 || m_MaxAmmo <= 0)
		return false;

	if (m_Ammo < m_MaxAmmo)
	{
		m_Ammo = min(m_Ammo+aCustomWeapon[m_Type].m_PowerupSize, m_MaxAmmo);
		return true;
	}
	
	return false;
}

void CWeapon::Tick()
{
	if(m_ReloadTimer > 0)
		m_ReloadTimer--;
}


void CWeapon::TickPaused()
{
	
}

void CWeapon::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;
}
