#include <base/system.h>
#include <base/math.h>

#include "playerdata.h"

CPlayerData::CPlayerData(const char *pName, int ColorID)
{
	m_pChild1 = 0;
	m_pChild2 = 0;
	
	str_copy(m_aName, pName, 16);
	m_ColorID = ColorID;
	
	Reset();
}

void CPlayerData::Die()
{
}

void CPlayerData::Add(CPlayerData *pPlayerData)
{
	int Color = pPlayerData->m_ColorID;
	
	if (Color < m_ColorID)
	{
		if (m_pChild1)
			m_pChild1->Add(pPlayerData);
		else
			m_pChild1 = pPlayerData;
	}
	else
	{
		if (m_pChild2)
			m_pChild2->Add(pPlayerData);
		else
			m_pChild2 = pPlayerData;
	}
}

CPlayerData *CPlayerData::Get(const char *pName, int ColorID)
{
	if (ColorID == m_ColorID)
	{
		if(str_comp(pName, m_aName) == 0)
			return this;
	}
	
	if (ColorID < m_ColorID)
	{
		if (m_pChild1)
			return m_pChild1->Get(pName, ColorID);
		else
			return 0;
	}
	else
	{
		if (m_pChild2)
			return m_pChild2->Get(pName, ColorID);
		else
			return 0;
	}
	
	return 0;
}


int CPlayerData::GetHighScore(int Score)
{
	int s = max(m_HighestLevel, Score);

	if (m_pChild1)
		s = max(s, m_pChild1->GetHighScore(s));

	if (m_pChild2)
		s = max(s, m_pChild2->GetHighScore(s));

	return s;
}


void CPlayerData::Reset()
{
	for (int i = 0; i < 99; i++)
	{
		m_aWeaponType[i] = 0;
		m_aWeaponAmmo[i] = 0;
		
		m_aAmmo[i] = -1;
	}
	
	m_Armor = 0;
	m_Weapon = 0;
	m_Kits = 0;
	m_Score = 0;
	m_Gold = 0;
	m_HighestLevel = 0;
	m_HighestLevelSeed = 0;
}