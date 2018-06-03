#ifndef GAME_SERVER_AI_DEF_AI_H
#define GAME_SERVER_AI_DEF_AI_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIdef : public CAI
{
public:
	CAIdef(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);

	enum State
	{
		AISTATE_IDLE,
		AISTATE_FINDSHOP,
		AISTATE_SHOP,
		AISTATE_DEF,
		AISTATE_ATTACK,
	};
	
private:
	int m_SkipMoveUpdate;
	bool SeekFriend();
	int m_State;
	int m_StateTimer;
	
	
	bool m_Shopped;
	bool FindShop();
	vec2 m_ShopPos;
};

#endif
