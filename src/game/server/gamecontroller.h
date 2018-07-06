#ifndef GAME_SERVER_GAMECONTROLLER_H
#define GAME_SERVER_GAMECONTROLLER_H

#include <base/vmath.h>

#define MAX_PICKUPS 1024
#define MAX_DROPPABLES 60


enum BombStatuses
{
	BOMB_IDLE,
	BOMB_CARRIED,
	BOMB_ARMED,
	BOMB_DISARMED
};


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
	int m_ClearBroadcastTick;

	void AutoBalance();
	
	void RespawnPickups();
	void FlashPickups();
	void ClearPickups();

	void DeathMessage();

	void ResetSurvivalRound();
	void KillEveryone();
	
	// for hooking to different gamemodes, e.g. reseting flags
	virtual void NewSurvivalRound();
	
	
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

	void FirstMap();
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

	enum SurvivalStatus
	{
		SURVIVAL_CANJOIN,
		SURVIVAL_NOCANDO,
	};
	
	int m_SurvivalStatus;
	int m_SurvivalDeathTick;
	int m_SurvivalStartTick;
	int m_SurvivalResetTick;
	
	bool m_SurvivalDeathReset;
	
public:

	class CBall *m_pBall;
	
	// CSTT & CSBB
	int GetRoundState(){ return m_GameState; }
	int GetRound(){ return m_Round; }

	virtual int GetDefendingTeam();
	virtual vec2 GetAttackPos();
	
	virtual bool InBombArea(vec2 Pos);
	
	virtual void TriggerBomb();
	virtual void DisarmBomb();
	virtual void ReactorDestroyed();
	
	virtual void AddMapArea(int Team, vec2 Pos);
	virtual bool InMapArea(int Team, vec2 Pos);
	
	virtual void OnSurvivalTimeOut();
	
	void ResetBallRound();
	
	int m_BombStatus;
	vec2 m_BombPos;
	
	void SetPickup(vec2 Pos, int PickupType, int PickupSubtype, int Amount = 1);
	void DropPickup(vec2 Pos, int PickupType, vec2 Force, int PickupSubtype, float Ammo = -1.0f, int PowerLevel = 0);
	void DropWeapon(vec2 Pos, vec2 Force, class CWeapon *pWeapon);
	
	void ReleaseWeapon(class CWeapon *pWeapon);
	bool TriggerWeapon(class CWeapon *pWeapon);
	
	int GetTimeLeft();
	
	int GetRandomWeapon();
	int GetRandomModularWeapon();
	
	//
	const char *m_pGameType;

	int GetAliveCID(int Team = -1);
	int CountPlayersAlive(int Team = -1, bool IgnoreBots = false);
	int CountPlayers(int Team = -1);
	int CountHumans();
	int CountBots();
	int CountBotsAlive();
	
	void TriggerSwitch(vec2 Pos);
	void TriggerEscape();
	
	virtual void NextLevel(int CID = -1);
	
	bool IsCoop() const;
	bool IsTeamplay() const;
	bool IsInfection() const;
	bool IsGameOver() const { return m_GameOverTick != -1; }

	// called when client connects
	void OnPlayerJoin();
	
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

	virtual void DisplayExit(vec2 Pos);
	
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
