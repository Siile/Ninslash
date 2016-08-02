#ifndef GAME_SERVER_GAMEMODES_BASE_H
#define GAME_SERVER_GAMEMODES_BASE_H
#include <game/server/gamecontroller.h>

class CGameControllerBase : public IGameController
{
public:
	CGameControllerBase(class CGameContext *pGameServer);

	void OnCharacterSpawn(class CCharacter *pChr, bool RequestAI = false);
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	virtual void Snap(int SnappingClient);
	virtual void Tick();
};
#endif
