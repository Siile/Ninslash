#ifndef GAME_SERVER_AI_DM_AI_H
#define GAME_SERVER_AI_DM_AI_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIbase : public CAI
{
public:
	CAIbase(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);
	void ReceiveDamage(int CID, int Dmg);

private:
	int m_SkipMoveUpdate;
};

#endif
