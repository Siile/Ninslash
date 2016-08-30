#ifndef GAME_SERVER_AI_CTF_AI_H
#define GAME_SERVER_AI_CTF_AI_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIctf : public CAI
{
public:
	CAIctf(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);

private:
};

#endif
