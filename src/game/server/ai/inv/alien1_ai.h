#ifndef GAME_SERVER_AI_ALIEN1_AI_H
#define GAME_SERVER_AI_ALIEN1_AI_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIalien1 : public CAI
{
public:
	CAIalien1(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);
	void ReceiveDamage(int CID, int Dmg);

private:
	int m_SkipMoveUpdate;
	vec2 m_StartPos;
	
	int m_ShockTimer;
	int m_Skin;
};

#endif
