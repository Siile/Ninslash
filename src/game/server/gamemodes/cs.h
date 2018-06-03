

#ifndef GAME_SERVER_GAMEMODES_CS_H
#define GAME_SERVER_GAMEMODES_CS_H
#include <game/server/gamecontroller.h>

class CGameControllerCS : public IGameController
{
private:
	void FindReactors();
	void StartRound();
	virtual void NewSurvivalRound();
	
	bool m_GameState;
	
	int m_AreaCount;
	vec4 m_aArea[9];
	
	void AddToArea(vec2 Pos);
	
	int m_aPlayerWeapon[MAX_CLIENTS*NUM_SLOTS];
	
public:
	CGameControllerCS(class CGameContext *pGameServer);

	void OnCharacterSpawn(class CCharacter *pChr, bool RequestAI = false);
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	virtual void Snap(int SnappingClient);
	virtual void Tick();
};
#endif
