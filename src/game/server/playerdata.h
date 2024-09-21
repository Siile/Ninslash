#ifndef GAME_SERVER_PLAYERDATA_H
#define GAME_SERVER_PLAYERDATA_H

// stored player data for switching between levels
class CPlayerData
{
private:
	CPlayerData *m_pChild1;
	CPlayerData *m_pChild2;

	IStorage *m_pStorage;

public:
	CPlayerData(const char *pName, int ColorID, IStorage *pStorage);
	void Die();
	void Reset();
	
	int m_aWeaponType[99];
	int m_aWeaponAmmo[99];
	
	int m_aAmmo[99];
	int m_Weapon;
	int m_Armor;
	int m_Kits;
	int m_Score;
	int m_Gold;
	
	int m_HighestLevel;
	int m_HighestLevelSeed;
	
	int m_ColorID;
	
	char m_aName[16];
	
	void Add(CPlayerData *pPlayerData);
	CPlayerData *Get(const char *pName, int ColorID);
	
	int GetHighScore(int Score);
	int GetPlayerCount(int Score);

	void LoadDataFromFile();
	void SaveToFile();
};

#endif
