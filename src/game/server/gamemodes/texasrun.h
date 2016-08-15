#ifndef GAME_SERVER_GAMEMODES_TEXASRUN_H
#define GAME_SERVER_GAMEMODES_TEXASRUN_H
#include <game/server/gamecontroller.h>

enum TexasGameState
{
	TEXAS_STARTING,
	TEXAS_STARTED,
	TEXAS_FIRSTDEATH,
	TEXAS_ENDING
};

class CGameControllerTexasRun : public IGameController
{
public:
	CGameControllerTexasRun(class CGameContext *pGameServer);

	void OnCharacterSpawn(class CCharacter *pChr, bool RequestAI = false);
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	virtual void Snap(int SnappingClient);
	virtual void Tick();
	
private:
	int m_EndTick;
	int m_GameStateLockTick;
	
	void SelectStartingDead();
	void MovePlayersToRed();
};
#endif
