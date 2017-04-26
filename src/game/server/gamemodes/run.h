#ifndef GAME_SERVER_GAMEMODES_RUN_H
#define GAME_SERVER_GAMEMODES_RUN_H
#include <game/server/gamecontroller.h>

#define MAX_ROBOTS 512


class CGameControllerCoop : public IGameController
{
private:
	
	enum Enemies
	{
		ENEMY_ALIEN1,
		ENEMY_ROBOT1,
		ENEMY_ROBOT2,
		ENEMY_ALIEN2,
		NUM_ENEMIES
	};

	int m_Enemies[NUM_ENEMIES];
	vec2 m_EnemySpawnPos[MAX_ROBOTS*(NUM_ENEMIES+1)];
	
	int EnemiesLeft()
	{
		int Total = 0;
		
		for (int i = 0; i < NUM_ENEMIES; i++)
			Total += m_Enemies[i];
		
		return Total;
	}
	
	int m_Deaths;
	bool m_RoundWin;
	int m_RoundWinTick;
	
	int m_RoundOverTick;
	
public:
	CGameControllerCoop(class CGameContext *pGameServer);
	
	virtual bool OnEntity(int Index, vec2 Pos);
	void OnCharacterSpawn(class CCharacter *pChr, bool RequestAI = false);
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	void NextLevel(int CID = -1);
	bool GetSpawnPos(int Team, vec2 *pOutPos);
	virtual void Tick();
	
	enum GameState
	{
		STATE_STARTING,
		STATE_GAME,
	};
};
#endif
