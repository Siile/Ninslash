#ifndef GAME_SERVER_GAMEMODES_BASE_H
#define GAME_SERVER_GAMEMODES_BASE_H
#include <game/server/gamecontroller.h>

#define MAX_ENEMIES 512

class CGameControllerBase : public IGameController
{
private:
	
	enum Enemies
	{
		ENEMY_ALIEN1,
		ENEMY_ALIEN2,
		ENEMY_ROBOT1,
		ENEMY_ROBOT2,
		ENEMY_ROBOT3,
		ENEMY_BUNNY1,
		ENEMY_BUNNY2,
		ENEMY_PYRO1,
		ENEMY_PYRO2,
		NUM_ENEMIES
	};

	vec2 m_aEnemySpawnPos[MAX_ENEMIES];
	
	int m_Wave;
	int m_WaveStartTick;
	int m_Deaths;
	bool m_RoundWin;
	int m_RoundWinTick;
	int m_RoundOverTick;
	
	int m_NoPlayersTick;
	
	bool m_GameOverBroadcast;
	
	// hordes of enemies
	int m_EnemyCount;
	int m_EnemiesLeft;
	
	int m_NumEnemySpawnPos;
	int m_SpawnPosRotation;
	
	void NextWave();
	
	int ReactorsLeft();
	
	int m_Bosses;
	int m_Crawlers;
	
public:
	CGameControllerBase(class CGameContext *pGameServer);
	
	virtual bool OnEntity(int Index, vec2 Pos);
	void OnCharacterSpawn(class CCharacter *pChr, bool RequestAI = false);
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	bool GetSpawnPos(int Team, vec2 *pOutPos);
	virtual void Tick();
	
	enum GameState
	{
		STATE_STARTING,
		STATE_GAME,
	};
};
#endif
