#include "playerdata.h"

CPlayerData::CPlayerData()
{
	Reset();
}

void CPlayerData::Die()
{
}

void CPlayerData::Reset()
{
	for (int i = 0; i < 99; i++)
	{
		m_aAmmo[i] = -1;
		m_aPowerLevel[i] = 0;
	}
	
	m_Armor = 0;
	m_Weapon = 0;
	m_Kits = 0;
	m_Score = 0;
}