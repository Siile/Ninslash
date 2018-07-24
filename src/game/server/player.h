#ifndef GAME_SERVER_PLAYER_H
#define GAME_SERVER_PLAYER_H

#include "entities/character.h"
#include "gamecontext.h"
#include "ai.h"

#include <game/weapons.h>

enum Skins
{
	SKIN_ALIEN1,
	SKIN_ALIEN2,
	SKIN_ALIEN3,
	SKIN_ALIEN4,
	SKIN_BUNNY1,
	SKIN_BUNNY2,
	SKIN_BUNNY3,
	SKIN_BUNNY4,
	SKIN_FOXY1,
	SKIN_ROBO1,
	SKIN_ROBO2,
	SKIN_ROBO3,
	SKIN_ROBO4,
	SKIN_ROBO5,
	SKIN_PYRO1,
	SKIN_PYRO2,
	SKIN_SKELETON1,
	SKIN_SKELETON2,
	SKIN_SKELETON3,
};


// player object
class CPlayer
{
	MACRO_ALLOC_POOL_ID()

public:
	CPlayer(CGameContext *pGameServer, int ClientID, int Team);
	~CPlayer();
	
	void Init(int CID);

	void SaveData();	
	void NewRound();
	
	bool ForceRespawn(vec2 Pos);
	void TryRespawn();
	void Respawn();
	void SetTeam(int Team, bool DoChatMsg=true);
	
	//int GetTeam() const { return m_Team; };
	int GetTeam();
	
	bool Spectating();
	
	int GetColorID();
	
	/*
	int GetTeam()
	{
		if (m_Team != TEAM_SPECTATORS);
			return m_Team;
		return m_WantedTeam;
	};
	*/
	
	int GetCID() const { return m_ClientID; };

	void Tick();
	void PostTick();
	void Snap(int SnappingClient);

	void OnDirectInput(CNetObj_PlayerInput *NewInput);
	void OnPredictedInput(CNetObj_PlayerInput *NewInput);
	void OnDisconnect(const char *pReason);

	void KillCharacter(int Weapon = WEAPON_GAME);
	CCharacter *GetCharacter();
	
	// bomb planting & defusing
	int m_ActionTimer;
	
	//---------------------------------------------------------
	// this is used for snapping so we know how we can clip the view for the player
	vec2 m_ViewPos;

	// states if the client is chatting, accessing a menu etc.
	int m_PlayerFlags;

	// used for snapping to just update latency if the scoreboard is active
	int m_aActLatency[MAX_CLIENTS];

	// used for spectator mode
	int m_SpectatorID;

	bool m_IsReady;
	
	bool m_BroadcastingCaptureStatus;

	bool m_ActionSpectator;
	
	int m_Gold;
	
	int GetGold() { return m_Gold; }
	void ReduceGold(int Amount) { m_Gold = max(0, m_Gold-Amount); }
	bool IncreaseGold(int Amount);
	
	//
	int m_Vote;
	int m_VotePos;
	//
	int m_LastVoteCall;
	int m_LastVoteTry;
	int m_LastChat;
	int m_LastSetTeam;
	int m_LastSetSpectatorMode;
	int m_LastChangeInfo;
	int m_LastEmote;
	int m_LastKill;


	struct
	{
		char m_TopperName[64];
		char m_EyeName[64];
		int m_Body;
		int m_ColorBody;
		int m_ColorFeet;
		int m_ColorTopper;
		int m_ColorSkin;
		bool m_IsBot;
		int m_BloodColor;
	} m_TeeInfos;
	
	
	float m_InterestPoints;
	
	int m_DeathTick;
	int m_RespawnTick;
	int m_DieTick;
	int m_Score;
	int m_ScoreStartTick;
	bool m_ForceBalanced;
	int m_LastActionTick;
	int m_TeamChangeTick;
	struct
	{
		int m_TargetX;
		int m_TargetY;
	} m_LatestActivity;

	// network latency calculations
	struct
	{
		int m_Accum;
		int m_AccumMin;
		int m_AccumMax;
		int m_Avg;
		int m_Min;
		int m_Max;
	} m_Latency;
	
	CAI *m_pAI;
	bool m_IsBot;
	
	void AITick();
	bool AIInputChanged();
	
	bool m_ToBeKicked;
	char m_aBroadcast[256];
	int m_BroadcastLockTick;
	
	// custom
	void SelectItem(int Item);
	void UseKit(int Kit, vec2 Pos);
	void DropWeapon();
	
	// inventory
	void InventoryRoll();
	void DropItem(int Slot, vec2 Pos);
	void SwapItem(int Item1, int Item2);
	void CombineItem(int Item1, int Item2);
	void TakePart(int Item1, int Slot, int Item2);
	void SendInventory();
	
	void JoinTeam();
	
	bool m_ForceToSpectators;

	//int m_WantedTeam;
	
	// settings
	bool m_EnableAutoSpectating;
	
	// warm welcome
	bool m_Welcomed;
	
	void SetRandomSkin();
	void SetCustomSkin(int Type);

	
private:
	bool m_Spectate;
	bool m_GotSkin;
	
	CCharacter *m_pCharacter;
	CGameContext *m_pGameServer;

	CGameContext *GameServer() const { return m_pGameServer; }
	IServer *Server() const;

	//
	void ForceToSpectators();
	
	//
	bool m_Spawning;
	int m_ClientID;
	int m_Team;
};

#endif
