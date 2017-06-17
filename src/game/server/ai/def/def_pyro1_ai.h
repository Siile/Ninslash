#ifndef GAME_SERVER_AI_DEF_DEF_PYRO1_AI_H
#define GAME_SERVER_AI_DEF_DEF_PYRO1_AI_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIdefpyro1 : public CAI
{
public:
	CAIdefpyro1(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);
	void ReceiveDamage(int CID, int Dmg);

private:
	int m_SkipMoveUpdate;
	bool m_Triggered;
	vec2 m_StartPos;
	
	int m_ShockTimer;
};

#endif
