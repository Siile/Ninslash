#ifndef GAME_SERVER_AI_TDM_AI_H
#define GAME_SERVER_AI_TDM_AI_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAItdm : public CAI
{
public:
	CAItdm(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);

private:
	int m_SkipMoveUpdate;
};

#endif