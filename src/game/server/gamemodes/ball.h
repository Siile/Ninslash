#ifndef GAME_SERVER_GAMEMODES_BALL_H
#define GAME_SERVER_GAMEMODES_BALL_H
#include <game/server/gamecontroller.h>

class CGameControllerBall : public IGameController
{
public:
	CGameControllerBall(class CGameContext *pGameServer);

	void OnCharacterSpawn(class CCharacter *pChr, bool RequestAI = false);
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	virtual void Snap(int SnappingClient);
	virtual void Tick();
	
	virtual void AddMapArea(int Team, vec2 Pos);
	virtual bool InMapArea(int Team, vec2 Pos);
	virtual vec2 GetGoalArea(int Team);
	
	vec4 m_GoalArea[2];
	bool m_GoalAreaSet[2];
	
	int m_RoundStartTick;
	int m_RoundEndTick;
};
#endif
