#include "playerdata.h"

CPlayerData::CPlayerData()
{
	Die();
}

void CPlayerData::Die()
{
	for (int i = 0; i < 99; i++)
		m_aAmmo[i] = -1;
	
	m_Weapon = 0;
}