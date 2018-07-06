#ifndef GAME_SERVER_AI_BALL_AI_H
#define GAME_SERVER_AI_BALL_AI_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIball : public CAI
{
public:
	CAIball(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);

private:
	int m_SkipMoveUpdate;
	bool SeekFriend();
	
	vec2 m_EnemyGoalPos;
	vec2 m_OwnGoalPos;
};

#endif
