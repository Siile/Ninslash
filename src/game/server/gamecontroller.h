

#ifndef GAME_SERVER_GAMECONTROLLER_H
#define GAME_SERVER_GAMECONTROLLER_H

#include <base/vmath.h>

#define MAX_PICKUPS 1024
#define MAX_DROPPABLES 40

/*
	Class: Game Controller
		Controls the main game logic. Keeping track of team and player score,
		winning conditions and specific game logic.
*/
class IGameController
{
	vec2 m_aaSpawnPoints[3][64];
	int m_aNumSpawnPoints[3];

	class CGameContext *m_pGameServer;
	class IServer *m_pServer;

	
	
	// store pickup pointers
	class CPickup *m_apPickup[MAX_PICKUPS];
	int m_PickupCount;
	bool m_DroppablesCreated;
	
	// for item drops
	int m_PickupDropCount;
	
	void CreateDroppables();
	
protected:
	void AutoBalance();
	
	void RespawnPickups();
	void FlashPickups();
	void ClearPickups();

	// CSTT & CSBB
	int m_Round;
	int m_GameState;
	
	
	//
	CGameContext *GameServer() const { return m_pGameServer; }
	IServer *Server() const { return m_pServer; }

	struct CSpawnEval
	{
		CSpawnEval()
		{
			m_Got = false;
			m_FriendlyTeam = -1;
			m_Pos = vec2(100,100);
			m_Used = false;
		}

		vec2 m_Pos;
		bool m_Got;
		int m_FriendlyTeam;
		float m_Score;
		bool m_Used;
	};

	float EvaluateSpawnPos(CSpawnEval *pEval, vec2 Pos);
	void EvaluateSpawnType(CSpawnEval *pEval, int Type);
	bool EvaluateSpawn(class CPlayer *pP, vec2 *pPos);

	void CycleMap();
	void ResetGame();

	char m_aMapWish[128];

	int m_RoundTimeLimit;
	bool m_ResetTime;

	int m_RoundStartTick;
	int m_GameOverTick;
	int m_SuddenDeath;

	int m_aTeamscore[2];

	int m_Warmup;
	int m_RoundCount;

	int m_GameFlags;
	int m_UnbalancedTick;
	bool m_ForceBalanced;
	
	int m_TimeLimit;

public:
	// CSTT & CSBB
	int GetRoundState(){ return m_GameState; }
	int GetRound(){ return m_Round; }

	virtual int GetDefendingTeam();
	
	void SetPickup(vec2 Pos, int PickupType, int PickupSubtype, int Amount = 1);
	void DropPickup(vec2 Pos, int PickupType, vec2 Force, int PickupSubtype, float Ammo = -1.0f);
	
	int GetTimeLeft();
	
	virtual void NextGameState(int NextGameState = -1, float InHowManySeconds = 0.0f);
	
	int GetRandomWeapon(int WeaponLevel);
	
	//
	const char *m_pGameType;

	int CountPlayers(int Team = -1);
	int CountBots();
	
	void TriggerSwitch(vec2 Pos);
	
	bool IsCoop() const;
	bool IsTeamplay() const;
	bool IsInfection() const;
	bool IsGameOver() const { return m_GameOverTick != -1; }

	IGameController(class CGameContext *pGameServer);
	virtual ~IGameController();

	virtual void DoWincheck();
	
	/*
	virtual class CCharacter *GetHead();
	virtual void SetHead(class CCharacter *Boss);
	*/

	void DoWarmup(int Seconds);

	void StartRound();
	void EndRound();
	void ChangeMap(const char *pToMap);

	bool IsFriendlyFire(int ClientID1, int ClientID2);

	bool IsForceBalanced();

	/*

	*/
	virtual bool CanBeMovedOnBalance(int ClientID);

	virtual void Tick();

	virtual void Snap(int SnappingClient);

	/*
		Function: on_entity
			Called when the map is loaded to process an entity
			in the map.

		Arguments:
			index - Entity index.
			pos - Where the entity is located in the world.

		Returns:
			bool?
	*/
	virtual bool OnEntity(int Index, vec2 Pos);
	virtual bool OnNonPickupEntity(int Index, vec2 Pos);

	/*
		Function: on_CCharacter_spawn
			Called when a CCharacter spawns into the game world.

		Arguments:
			chr - The CCharacter that was spawned.
	*/
	virtual void OnCharacterSpawn(class CCharacter *pChr, bool RequestAI = false);
	
	
	virtual bool GetSpawnPos(int Team, vec2 *pOutPos);
	

	/*
		Function: on_CCharacter_death
			Called when a CCharacter in the world dies.

		Arguments:
			victim - The CCharacter that died.
			killer - The player that killed it.
			weapon - What weapon that killed it. Can be -1 for undefined
				weapon when switching team or player suicides.
	*/
	virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	
	// for ctf ai
	virtual vec2 GetFlagPos(int Team);
	virtual int GetFlagState(int Team);
	
	virtual bool CanCharacterSpawn(int ClientID);
	virtual bool CanSeePickup(int CID, int Type, int Subtype); // for gungame
	virtual bool CanDropWeapon(class CCharacter *pCharacter);

	virtual class CBomb *GetBomb();
	virtual class CFlag *GetClosestBase(vec2 Pos, int Team = -1);
	virtual class CFlag *GetRandomBase(int NotThisTeam = -1);
	virtual class CFlag *GetUndefendedBase(int Team = -1);
	virtual int Defenders(class CFlag *Base);
	
	virtual int CountBases(int Team = -1);
	
	virtual void OnPlayerInfoChange(class CPlayer *pP);

	//
	virtual bool CanSpawn(int Team, vec2 *pPos, bool IsBot = false);

	/*

	*/
	virtual int GetLockedWeapon(class CCharacter *pCharacter);
	
	virtual const char *GetTeamName(int Team);
	virtual int GetAutoTeam(int NotThisID);
	virtual bool CanJoinTeam(int Team, int NotThisID);
	bool CheckTeamBalance();
	bool CanChangeTeam(CPlayer *pPplayer, int JoinTeam);
	int ClampTeam(int Team);

	virtual void PostReset();
};

#endif
