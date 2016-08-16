#ifndef GAME_SERVER_AI_DM_AI_H
#define GAME_SERVER_AI_DM_AI_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIdm : public CAI
{
public:
	CAIdm(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);

private:
	int m_SkipMoveUpdate;
};

#endif
