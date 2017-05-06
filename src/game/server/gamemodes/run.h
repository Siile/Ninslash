#ifndef GAME_SERVER_GAMEMODES_RUN_H
#define GAME_SERVER_GAMEMODES_RUN_H
#include <game/server/gamecontroller.h>

#define MAX_ENEMIES 512


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

	vec2 m_aEnemySpawnPos[MAX_ENEMIES];
	
	int m_Deaths;
	bool m_RoundWin;
	int m_RoundWinTick;
	int m_RoundOverTick;
	
	// hordes of enemies
	int m_EnemyCount;
	int m_EnemiesLeft;
	
	int m_NumEnemySpawnPos;
	int m_SpawnPosRotation;
	
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
