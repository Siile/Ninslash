#ifndef GAME_SERVER_GAMEMODES_ROAM_H
#define GAME_SERVER_GAMEMODES_ROAM_H
#include <game/server/gamecontroller.h>

class CGameControllerRoam : public IGameController
{
public:
	CGameControllerRoam(class CGameContext *pGameServer);
	
	void OnCharacterSpawn(class CCharacter *pChr, bool RequestAI = false);
	bool CanSpawn(int Team, vec2 *pPos, bool IsBot = false);
	virtual void Tick();
	void AddEnemy(vec2 Pos);
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	
private:
	vec2 m_aBotSpawn[99];
	int m_BotSpawnNum;
};
#endif
