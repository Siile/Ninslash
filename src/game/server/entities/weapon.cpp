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
}

void CWeapon::Reset()
{
	m_MaxAmmo = aCustomWeapon[m_Type].m_MaxAmmo;
	m_Ammo = m_MaxAmmo;
}

void CWeapon::OnOwnerDeath(bool IsActive)
{
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
	
}


void CWeapon::TickPaused()
{
	
}

void CWeapon::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;
}
