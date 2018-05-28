

#ifndef GAME_SERVER_GAMECONTEXT_H
#define GAME_SERVER_GAMECONTEXT_H

#include <engine/server.h>
#include <engine/console.h>
#include <engine/shared/memheap.h>
#include <engine/storage.h> // MapGen

#include <game/layers.h>
#include <game/voting.h>

#include "eventhandler.h"
#include "gamecontroller.h"
#include "gameworld.h"
#include "player.h"
#include "mapgen.h"
#include "lastseen.h"

/*
	Tick
		Game Context (CGameContext::tick)
			Game World (GAMEWORLD::tick)
				Reset world if requested (GAMEWORLD::reset)
				All entities in the world (ENTITY::tick)
				All entities in the world (ENTITY::tick_defered)
				Remove entities marked for deletion (GAMEWORLD::remove_entities)
			Game Controller (GAMECONTROLLER::tick)
			All players (CPlayer::tick)


	Snap
		Game Context (CGameContext::snap)
			Game World (GAMEWORLD::snap)
				All entities in the world (ENTITY::snap)
			Game Controller (GAMECONTROLLER::snap)
			Events handler (EVENT_HANDLER::snap)
			All players (CPlayer::snap)

*/

struct CPlayerSpecData
{
	CPlayerSpecData()
	{
		m_Kits = 0;
		m_WeaponSlot = 0;
		
		for (int i = 0; i < 4; i++)
			m_aWeapon[i] = 0;
	}
	
	int m_WeaponSlot;
	int m_aWeapon[4];
	int m_Kits;
};

class CGameContext : public IGameServer
{
	IServer *m_pServer;
	class IConsole *m_pConsole;
	CLayers m_Layers;
	CCollision m_Collision;
	CNetObjHandler m_NetObjHandler;
	CTuningParams m_Tuning;
	// MapGen
	CMapGen m_MapGen;
	IStorage *m_pStorage;

	static void ConTuneParam(IConsole::IResult *pResult, void *pUserData);
	static void ConTuneReset(IConsole::IResult *pResult, void *pUserData);
	static void ConTuneDump(IConsole::IResult *pResult, void *pUserData);
	static void ConPause(IConsole::IResult *pResult, void *pUserData);
	static void ConChangeMap(IConsole::IResult *pResult, void *pUserData);
	static void ConRestart(IConsole::IResult *pResult, void *pUserData);
	static void ConBroadcast(IConsole::IResult *pResult, void *pUserData);
	static void ConSay(IConsole::IResult *pResult, void *pUserData);
	static void ConSetTeam(IConsole::IResult *pResult, void *pUserData);
	static void ConSetTeamAll(IConsole::IResult *pResult, void *pUserData);
	static void ConSwapTeams(IConsole::IResult *pResult, void *pUserData);
	static void ConShuffleTeams(IConsole::IResult *pResult, void *pUserData);
	static void ConLockTeams(IConsole::IResult *pResult, void *pUserData);
	static void ConAddVote(IConsole::IResult *pResult, void *pUserData);
	static void ConRemoveVote(IConsole::IResult *pResult, void *pUserData);
	static void ConForceVote(IConsole::IResult *pResult, void *pUserData);
	static void ConClearVotes(IConsole::IResult *pResult, void *pUserData);
	static void ConVote(IConsole::IResult *pResult, void *pUserData);
	static void ConchainSpecialMotdupdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);

	CGameContext(int Resetting);
	void Construct(int Resetting);
	
	bool m_Resetting;
public:
	IServer *Server() const { return m_pServer; }
	class IConsole *Console() { return m_pConsole; }
	CCollision *Collision() { return &m_Collision; }
	CTuningParams *Tuning() { return &m_Tuning; }
	// MapGen
	CLayers *Layers() { return &m_Layers; }
	IStorage *Storage() const { return m_pStorage; }
	CMapGen *MapGen() { return &m_MapGen; }

	CGameContext();
	~CGameContext();

	void Clear();

	void ReloadMap();
	
	CEventHandler m_Events;
	CPlayer *m_apPlayers[MAX_CLIENTS];

	IGameController *m_pController;
	CGameWorld m_World;
	
	CPlayerSpecData GetPlayerSpecData(int ClientID);
	
	int m_aMostInterestingPlayer[2];
	
	void UpdateSpectators();
	
	// helper functions
	class CCharacter *GetPlayerChar(int ClientID);

	int m_LockTeams;

	// voting
	void StartVote(const char *pDesc, const char *pCommand, const char *pReason);
	void EndVote();
	void SendVoteSet(int ClientID);
	void SendVoteStatus(int ClientID, int Total, int Yes, int No);
	void AbortVoteKickOnDisconnect(int ClientID);

	int m_VoteCreator;
	int64 m_VoteCloseTime;
	bool m_VoteUpdate;
	int m_VotePos;
	char m_aVoteDescription[VOTE_DESC_LENGTH];
	char m_aVoteCommand[VOTE_CMD_LENGTH];
	char m_aVoteReason[VOTE_REASON_LENGTH];
	int m_NumVoteOptions;
	int m_VoteEnforce;
	enum
	{
		VOTE_ENFORCE_UNKNOWN=0,
		VOTE_ENFORCE_NO,
		VOTE_ENFORCE_YES,
	};
	CHeap *m_pVoteOptionHeap;
	CVoteOptionServer *m_pVoteOptionFirst;
	CVoteOptionServer *m_pVoteOptionLast;

	// helper functions
	void CreateFlameHit(vec2 Pos);
	void CreateBuildingHit(vec2 Pos);
	void CreateDamageInd(vec2 Pos, float AngleMod, int Damage, int ClientID);
	void CreateRepairInd(vec2 Pos);
	void CreateExplosion(vec2 Pos, int Owner, int Weapon);
	void SendEffect(int ClientID, int EffectID);
	void CreateHammerHit(vec2 Pos);
	void CreateEffect(int FX, vec2 Pos);
	int CreateDeathray(vec2 Pos);
	void CreatePlayerSpawn(vec2 Pos);
	void CreateDeath(vec2 Pos, int Who);
	void CreateSound(vec2 Pos, int Sound, int Mask=-1);
	void CreateSoundGlobal(int Sound, int Target=-1);
	
	bool BuildableSpot(vec2 Pos);
	bool AddBlock(int Type, vec2 Pos);
	void DamageBlocks(vec2 Pos, int Damage, int Range);
	void OnBlockChange(vec2 Pos);
	
	class CWeapon *NewWeapon(int Part1, int Part2);
	class CWeapon *NewWeapon(int Weapon);
	
	bool RespawnAlly(vec2 Pos, int Team);
	
	bool AddBuilding(int Kit, vec2 Pos, int Owner);
	
	void Shop(class CPlayer *pPlayer, int Slot);
	
	void CreateProjectile(int DamageOwner, int Weapon, int Charge, vec2 Pos, vec2 Direction, class CBuilding *OwnerBuilding = NULL);
	void CreateMeleeHit(int DamageOwner, int Weapon, float Dmg, vec2 Pos, vec2 Direction);

	void ClearFlameHits();
	
	bool m_aFlameHit[MAX_CLIENTS];
	
	void Repair(vec2 Pos);
	void AmmoFill(vec2 Pos, int Weapon);
	
	enum
	{
		CHAT_ALL=-2,
		CHAT_SPEC=-1,
		CHAT_RED=0,
		CHAT_BLUE=1
	};

	// network
	void SendChatTarget(int To, const char *pText);
	void SendChat(int ClientID, int Team, const char *pText);
	void SendEmoticon(int ClientID, int Emoticon);
	void SendWeaponPickup(int ClientID, int Weapon);
	void SendBroadcast(const char *pText, int ClientID, bool Lock = false);


	//
	void CheckPureTuning();
	void SendTuningParams(int ClientID);

	//
	void SwapTeams();

	//
	void UpdateAI();
	
	// engine events
	virtual void OnInit();
	virtual void OnConsoleInit();
	virtual void OnShutdown();

	virtual void OnTick();
	virtual void OnPreSnap();
	virtual void OnSnap(int ClientID);
	virtual void OnPostSnap();

	virtual void OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientID);

	virtual void AddZombie();
	virtual bool AIInputUpdateNeeded(int ClientID);
	virtual void AIUpdateInput(int ClientID, int *Data);
	

	int CountBots(bool SkipSpecialTees = false);
	int CountBotsAlive(bool SkipSpecialTees = false);
	//int CountHumans();
	int CountHumansAlive();
	
	virtual void OnClientConnected(int ClientID, bool AI = false);
	virtual void OnClientEnter(int ClientID);
	virtual void OnClientDrop(int ClientID, const char *pReason);
	virtual void OnClientDirectInput(int ClientID, void *pInput);
	virtual void OnClientPredictedInput(int ClientID, void *pInput);

	virtual bool IsClientReady(int ClientID);
	virtual bool IsClientPlayer(int ClientID);

	virtual const char *GameType();
	virtual const char *Version();
	virtual const char *NetVersion();
	
	// MapGen
	virtual void SaveMap(const char *path);

	vec2 GetNearHumanSpawnPos(bool AllowVision = false);
	vec2 GetFarHumanSpawnPos(bool AllowVision = false);
	int DistanceToHuman(vec2 Pos);
	
	void AddBot();
	void KickBots();
	void KickBot(int ClientID);
	
	bool IsBot(int ClientID);
	bool IsHuman(int ClientID);
	
	int m_BroadcastLockTick;
	
	bool m_ShowWaypoints;
	bool m_ShowAiState;

	static CLastSeen m_LastSeen;
};

inline int CmaskAll() { return -1; }
inline int CmaskOne(int ClientID) { return 1<<ClientID; }
inline int CmaskAllExceptOne(int ClientID) { return 0x7fffffff^CmaskOne(ClientID); }
inline bool CmaskIsSet(int Mask, int ClientID) { return (Mask&CmaskOne(ClientID)) != 0; }
#endif
