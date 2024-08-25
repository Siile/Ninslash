#ifndef GAME_SERVER_GAMEMODES_CS_H
#define GAME_SERVER_GAMEMODES_CS_H
#include <game/server/gamecontroller.h>

class CGameControllerCS : public IGameController
{
private:
	void FindReactors();
	void StartRound();
	virtual void NewSurvivalRound();
	
	int m_GameState;
	int m_RoundWinner;
	
	int m_AreaCount;
	vec4 m_aArea[9];
	
	void AddToArea(vec2 Pos);
	
	int m_aPlayerWeapon[MAX_CLIENTS*NUM_SLOTS];
	int m_aPlayerArmor[MAX_CLIENTS];
	int m_aPlayerKits[MAX_CLIENTS];
	
	bool m_Bomb;
	class CRadar *m_pBombRadar;
	
public:
	CGameControllerCS(class CGameContext *pGameServer);

	void OnCharacterSpawn(class CCharacter *pChr, bool RequestAI = false);
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	virtual void Snap(int SnappingClient);
	virtual void Tick();
	
	virtual bool InBombArea(vec2 Pos);
	virtual vec2 GetAttackPos();
	virtual void TriggerBomb();
	virtual void DisarmBomb();
	virtual void ReactorDestroyed();
	
	virtual void OnSurvivalTimeOut();
};
#endif
