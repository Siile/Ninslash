#ifndef GAME_SERVER_AI_TEXAS_AI_H
#define GAME_SERVER_AI_TEXAS_AI_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAItexas : public CAI
{
public:
	CAItexas(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);

private:
	int m_SkipMoveUpdate;
};

#endif