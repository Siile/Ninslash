#ifndef GAME_SERVER_AI_ROBOT2_AI_H
#define GAME_SERVER_AI_ROBOT2_AI_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIrobot2 : public CAI
{
public:
	CAIrobot2(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);

private:
	int m_SkipMoveUpdate;
	bool m_Triggered;
	vec2 m_StartPos;
};

#endif
