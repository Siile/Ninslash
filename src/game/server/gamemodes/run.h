#ifndef GAME_SERVER_GAMEMODES_RUN_H
#define GAME_SERVER_GAMEMODES_RUN_H
#include <game/server/gamecontroller.h>

#define MAX_ROBOTS 24

class CGameControllerCoop : public IGameController
{
private:
	int m_Robots1;
	int m_Robots2;
	vec2 m_Robot1SpawnPos[MAX_ROBOTS];
	vec2 m_Robot2SpawnPos[MAX_ROBOTS];
	
public:
	CGameControllerCoop(class CGameContext *pGameServer);
	
	virtual bool OnEntity(int Index, vec2 Pos);
	void OnCharacterSpawn(class CCharacter *pChr, bool RequestAI = false);
	bool GetSpawnPos(int Team, vec2 *pOutPos);
	virtual void Tick();
	
	enum GameState
	{
		STATE_STARTING,
		STATE_GAME,
	};
};
#endif
