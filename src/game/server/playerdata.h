#ifndef GAME_SERVER_PLAYERDATA_H
#define GAME_SERVER_PLAYERDATA_H

// stored player data for switching between levels
class CPlayerData
{
private:

public:
	CPlayerData();
	void Die();
	void Reset();
	
	int m_aAmmo[99];
	int m_aPowerLevel[99];
	int m_Weapon;
	int m_Kits;
	int m_Score;
};

#endif
