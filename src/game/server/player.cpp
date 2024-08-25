


#include <new>
#include <engine/shared/config.h>
#include "player.h"

#include <game/weapons.h>
#include <game/buildables.h>
#include "gamemodes/texasrun.h"


MACRO_ALLOC_POOL_ID_IMPL(CPlayer, MAX_CLIENTS)

IServer *CPlayer::Server() const { return m_pGameServer->Server(); }

CPlayer::CPlayer(CGameContext *pGameServer, int ClientID, int Team)
{
	m_DeathTick = 0;
	m_pGameServer = pGameServer;
	m_RespawnTick = Server()->Tick();
	m_DieTick = Server()->Tick();
	m_ScoreStartTick = Server()->Tick();
	m_pCharacter = 0;
	m_ClientID = ClientID;
	m_Team = GameServer()->m_pController->ClampTeam(Team);
	m_SpectatorID = SPEC_FREEVIEW;
	m_LastActionTick = Server()->Tick();
	m_TeamChangeTick = Server()->Tick();
	
	m_Spectate = false;
	m_ActionSpectator = true;
	m_GotSkin = false;
	
	m_Score = 0;
	m_Gold = 0;
	
	m_ActionTimer = 0;
	
	m_InterestPoints = 0;
	m_BroadcastingCaptureStatus = false;
	
	m_EnableAutoSpectating = true;
	
	m_IsBot = false;
	m_pAI = NULL;
	m_ToBeKicked = false;
	m_LatestX = 0;
	
	IncreaseGold(g_Config.m_SvStartGold);
	
	//m_WantedTeam = m_Team;
	//m_Team = TEAM_SPECTATORS;
	
	m_ForceToSpectators = false;
	
	// warm welcome awaits
	m_Welcomed = false;
}

CPlayer::~CPlayer()
{
	if (m_pAI)
		delete m_pAI;

	delete m_pCharacter;
	m_pCharacter = 0;
}


bool CPlayer::IncreaseGold(int Amount)
{
	if (m_Gold < 999)
	{
		m_Gold = min(999, m_Gold+Amount);
		SendInventory();
		return true;
	}

	return false;
}

void CPlayer::SaveData()
{
	if (GetCharacter())
		GetCharacter()->SaveData();
}

void CPlayer::NewRound()
{
	m_Score = 0;
	
	m_InterestPoints = 0;
}

void CPlayer::SelectItem(int Item)
{
	if (GetCharacter() && Item >= 0 && Item < NUM_PLAYERITEMS)
		GetCharacter()->SelectItem(Item);
}

void CPlayer::SendInventory()
{
	if (GetCharacter())
		GetCharacter()->SendInventory();
}

void CPlayer::SwapItem(int Item1, int Item2)
{
	if (GetCharacter())
		GetCharacter()->SwapItem(Item1, Item2);
}

void CPlayer::DropItem(int Slot, vec2 Pos)
{
	if (GetCharacter())
		GetCharacter()->DropItem(Slot, Pos);
}

void CPlayer::InventoryRoll(int Slot)
{
	if (GetCharacter())
		GetCharacter()->InventoryRoll(Slot);
}

void CPlayer::CombineItem(int Item1, int Item2)
{
	if (GetCharacter())
		GetCharacter()->CombineItem(Item1, Item2);
}

void CPlayer::TakePart(int Item1, int Slot, int Item2)
{
	if (GetCharacter())
		GetCharacter()->TakePart(Item1, Slot, Item2);
}

void CPlayer::UseKit(int Kit, vec2 Pos)
{
	if (GetCharacter() && Kit >= 0 && Kit < NUM_BUILDABLES)
		GetCharacter()->UseKit(Kit, Pos);
}


void CPlayer::DropWeapon()
{
	if (GetCharacter())
		GetCharacter()->DropWeapon();
}

int CPlayer::GetTeam()
{
	if (GameServer()->m_pController->IsCoop() && m_IsBot)
		return TEAM_BLUE;

	return m_Team;
}

	
void CPlayer::Tick()
{
#ifdef CONF_DEBUG
	if(!g_Config.m_DbgDummies || m_ClientID < MAX_CLIENTS-g_Config.m_DbgDummies)
#endif
	if(!Server()->ClientIngame(m_ClientID))
		return;

	Server()->SetClientScore(m_ClientID, m_Score);
	
	//if (m_Team != TEAM_SPECTATORS)
	//	m_WantedTeam = m_Team;
	
	// do latency stuff
	{
		IServer::CClientInfo Info;
		if(Server()->GetClientInfo(m_ClientID, &Info))
		{
			m_Latency.m_Accum += Info.m_Latency;
			m_Latency.m_AccumMax = max(m_Latency.m_AccumMax, Info.m_Latency);
			m_Latency.m_AccumMin = min(m_Latency.m_AccumMin, Info.m_Latency);
		}
		// each second
		if(Server()->Tick()%Server()->TickSpeed() == 0)
		{
			m_Latency.m_Avg = m_Latency.m_Accum/Server()->TickSpeed();
			m_Latency.m_Max = m_Latency.m_AccumMax;
			m_Latency.m_Min = m_Latency.m_AccumMin;
			m_Latency.m_Accum = 0;
			m_Latency.m_AccumMin = 1000;
			m_Latency.m_AccumMax = 0;
		}
	}

	//if (m_ForceToSpectators)
	//	ForceToSpectators();
	
	/*
	if(!m_pCharacter && m_DieTick+Server()->TickSpeed()*1 <= Server()->Tick())
	{
		m_Spectate = true;
	}
	else
		m_Spectate = false;
	*/
	
	
	if (m_IsBot)
		m_InterestPoints /= 1.025f;
	else
		m_InterestPoints /= 1.02f;
	
	/*
	if (m_InterestPoints > 0)
		m_InterestPoints--;
	else
	{
		//if (GetCharacter())
		//	m_InterestPoints = int(frandom()*10);
	}
	*/
	

	
	if(!GameServer()->m_World.m_Paused)
	{
		if(!m_pCharacter && m_Team == TEAM_SPECTATORS && m_SpectatorID == SPEC_FREEVIEW)
			m_ViewPos -= vec2(clamp(m_ViewPos.x-m_LatestActivity.m_TargetX, -500.0f, 500.0f), clamp(m_ViewPos.y-m_LatestActivity.m_TargetY, -400.0f, 400.0f));
		
		// dead in survival mode
		if (m_Team != TEAM_SPECTATORS && g_Config.m_SvSurvivalMode && !m_pCharacter && m_DieTick < Server()->Tick() - Server()->TickSpeed()*1)
			m_ViewPos -= vec2(clamp(m_ViewPos.x-m_LatestActivity.m_TargetX, -500.0f, 500.0f), clamp(m_ViewPos.y-m_LatestActivity.m_TargetY, -400.0f, 400.0f));

			
	
		//if(!m_pCharacter && m_DieTick+Server()->TickSpeed()*3 <= Server()->Tick())
		if(!m_pCharacter)
			m_Spawning = true;

		if(m_pCharacter)
		{
			if(m_pCharacter->IsAlive())
			{
				m_LatestX = m_pCharacter->m_Pos.x;
				m_ViewPos = m_pCharacter->m_Pos;
				m_ActionSpectator = true;
			}
			else
			{
				delete m_pCharacter;
				m_pCharacter = 0;
			}
		}
		//else if(m_Spawning && m_RespawnTick <= Server()->Tick())

		else if(m_Spawning)
		{
			if (GameServer()->m_pController->IsInfection() && GetTeam() != TEAM_SPECTATORS)
			{
				if (GameServer()->m_pController->GetRoundState() > TEXAS_STARTED)
					SetTeam(TEAM_BLUE);
				else if (GameServer()->m_pController->GetRoundState() < TEXAS_STARTED)
					SetTeam(TEAM_RED);
			}
			
			if (m_RespawnTick <= Server()->Tick())
				TryRespawn();
		}

		if(g_Config.m_SvBroadcastLock && m_BroadcastLockTick && m_aBroadcast[0] != '\0')
		{
			if(Server()->Tick() > m_BroadcastLockTick + Server()->TickSpeed() * g_Config.m_SvBroadcastLock)
				GameServer()->SendBroadcast(m_aBroadcast, GetCID(), true);
		}
	}
	else
	{
		++m_RespawnTick;
		++m_DieTick;
		++m_ScoreStartTick;
		++m_LastActionTick;
		++m_TeamChangeTick;
 	}
}

void CPlayer::PostTick()
{
	// update latency value
	if(m_PlayerFlags&PLAYERFLAG_SCOREBOARD)
	{
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
				m_aActLatency[i] = GameServer()->m_apPlayers[i]->m_Latency.m_Min;
		}
	}

	// update view pos for spectators
	if(Spectating() && m_SpectatorID != SPEC_FREEVIEW && GameServer()->m_apPlayers[m_SpectatorID])
	//if(m_Spectate && m_SpectatorID != SPEC_FREEVIEW && GameServer()->m_apPlayers[m_SpectatorID])
		m_ViewPos = GameServer()->m_apPlayers[m_SpectatorID]->m_ViewPos;
}


void CPlayer::Snap(int SnappingClient)
{
#ifdef CONF_DEBUG
	if(!g_Config.m_DbgDummies || m_ClientID < MAX_CLIENTS-g_Config.m_DbgDummies)
#endif
	if(!Server()->ClientIngame(m_ClientID))
		return;

	CNetObj_ClientInfo *pClientInfo = static_cast<CNetObj_ClientInfo *>(Server()->SnapNewItem(NETOBJTYPE_CLIENTINFO, m_ClientID, sizeof(CNetObj_ClientInfo)));
	if(!pClientInfo)
		return;

	StrToInts(&pClientInfo->m_Name0, 4, Server()->ClientName(m_ClientID));
	StrToInts(&pClientInfo->m_Clan0, 3, Server()->ClientClan(m_ClientID));
	pClientInfo->m_Country = Server()->ClientCountry(m_ClientID);
	StrToInts(&pClientInfo->m_Topper0, 6, m_TeeInfos.m_TopperName);
	StrToInts(&pClientInfo->m_Eye0, 6, m_TeeInfos.m_EyeName);
	StrToInts(&pClientInfo->m_Head0, 6, m_TeeInfos.m_HeadName);
	StrToInts(&pClientInfo->m_Body0, 6, m_TeeInfos.m_BodyName);
	StrToInts(&pClientInfo->m_Hand0, 6, m_TeeInfos.m_HandName);
	StrToInts(&pClientInfo->m_Foot0, 6, m_TeeInfos.m_FootName);
	pClientInfo->m_ColorBody = m_TeeInfos.m_ColorBody;
	pClientInfo->m_ColorFeet = m_TeeInfos.m_ColorFeet;
	pClientInfo->m_ColorTopper = m_TeeInfos.m_ColorTopper;
	pClientInfo->m_ColorSkin = m_TeeInfos.m_ColorSkin;
	pClientInfo->m_IsBot = m_TeeInfos.m_IsBot;
	pClientInfo->m_BloodColor = m_TeeInfos.m_BloodColor;

	CNetObj_PlayerInfo *pPlayerInfo = static_cast<CNetObj_PlayerInfo *>(Server()->SnapNewItem(NETOBJTYPE_PLAYERINFO, m_ClientID, sizeof(CNetObj_PlayerInfo)));
	if(!pPlayerInfo)
		return;

	pPlayerInfo->m_Latency = SnappingClient == -1 ? m_Latency.m_Min : GameServer()->m_apPlayers[SnappingClient]->m_aActLatency[m_ClientID];
	pPlayerInfo->m_Local = 0;
	pPlayerInfo->m_ClientID = m_ClientID;
	pPlayerInfo->m_Score = m_Score;
	pPlayerInfo->m_Team = m_Team;
	pPlayerInfo->m_Spectating = 0;

	pPlayerInfo->m_Kits = 0;
	
	if (GetCharacter())
	{
		pPlayerInfo->m_Kits = GetCharacter()->m_Kits;
		pPlayerInfo->m_WeaponSlot = GetCharacter()->GetWeaponSlot();
		
		pPlayerInfo->m_Weapon1 = GetCharacter()->GetWeaponType(0);
		pPlayerInfo->m_Weapon2 = GetCharacter()->GetWeaponType(1);
		pPlayerInfo->m_Weapon3 = GetCharacter()->GetWeaponType(2);
		pPlayerInfo->m_Weapon4 = GetCharacter()->GetWeaponType(3);
	}

	if(m_ClientID == SnappingClient)
		pPlayerInfo->m_Local = 1;

	if (g_Config.m_SvSurvivalMode && !GetCharacter() && m_DieTick < Server()->Tick() - Server()->TickSpeed()*1)
		pPlayerInfo->m_Spectating = 1;
	
	if(m_ClientID == SnappingClient && Spectating())
	{
		//pPlayerInfo->m_Team = TEAM_SPECTATORS;
		pPlayerInfo->m_Spectating = 1;
		
		CNetObj_SpectatorInfo *pSpectatorInfo = static_cast<CNetObj_SpectatorInfo *>(Server()->SnapNewItem(NETOBJTYPE_SPECTATORINFO, m_ClientID, sizeof(CNetObj_SpectatorInfo)));
		if(!pSpectatorInfo)
			return;

		pSpectatorInfo->m_SpectatorID = m_SpectatorID;
		pSpectatorInfo->m_X = m_ViewPos.x;
		pSpectatorInfo->m_Y = m_ViewPos.y;
		
		if (m_SpectatorID >= 0)
		{
			CPlayerSpecData d = GameServer()->GetPlayerSpecData(m_SpectatorID);
			
			pPlayerInfo->m_Kits = d.m_Kits;
			pPlayerInfo->m_WeaponSlot = d.m_WeaponSlot;
			
			pPlayerInfo->m_Weapon1 = d.m_aWeapon[0];
			pPlayerInfo->m_Weapon2 = d.m_aWeapon[1];
			pPlayerInfo->m_Weapon3 = d.m_aWeapon[2];
			pPlayerInfo->m_Weapon4 = d.m_aWeapon[3];
		}
	}
}

bool CPlayer::Spectating()
{
	if (m_Team == TEAM_SPECTATORS ||
	(g_Config.m_SvSurvivalMode && !GetCharacter() && m_DieTick < Server()->Tick() - Server()->TickSpeed()*1))
		return true;
	
	return false;
}



void CPlayer::OnDisconnect(const char *pReason)
{
	KillCharacter();

	if(Server()->ClientIngame(m_ClientID))
	{
		char aBuf[512];
		
		if (!m_IsBot)
		{
			if(pReason && *pReason)
				GameServer()->SendChatTarget(-1, "'%s' has left the game (%s)", Server()->ClientName(m_ClientID), pReason);
			else
				GameServer()->SendChatTarget(-1, "'%s' has left the game", Server()->ClientName(m_ClientID));
		}

		str_format(aBuf, sizeof(aBuf), "leave player='%d:%s'", m_ClientID, Server()->ClientName(m_ClientID));
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", aBuf);
	}
	
	if (m_pAI)
	{
		delete m_pAI;
		m_pAI = NULL;
	}
}

void CPlayer::OnPredictedInput(CNetObj_PlayerInput *NewInput)
{
	// skip the input if chat is active
	if((m_PlayerFlags&PLAYERFLAG_CHATTING) && (NewInput->m_PlayerFlags&PLAYERFLAG_CHATTING))
		return;

	if(m_pCharacter)
		m_pCharacter->OnPredictedInput(NewInput);
}

void CPlayer::OnDirectInput(CNetObj_PlayerInput *NewInput)
{
	if(NewInput->m_PlayerFlags&PLAYERFLAG_CHATTING && !m_pAI)
	{
		// skip the input if chat is active
		if(m_PlayerFlags&PLAYERFLAG_CHATTING)
			return;

		// reset input
		if(m_pCharacter)
			m_pCharacter->ResetInput();

		m_PlayerFlags = NewInput->m_PlayerFlags;
 		return;
	}
	
	m_PlayerFlags = NewInput->m_PlayerFlags;

	if (m_pAI)
		m_PlayerFlags = PLAYERFLAG_PLAYING;
	
	if(m_pCharacter)
		m_pCharacter->OnDirectInput(NewInput);

	if(!m_pCharacter && m_Team != TEAM_SPECTATORS && (NewInput->m_Fire&1))
		m_Spawning = true;

	// check for activity
	if(NewInput->m_Direction || m_LatestActivity.m_TargetX != NewInput->m_TargetX ||
		m_LatestActivity.m_TargetY != NewInput->m_TargetY || NewInput->m_Jump ||
		NewInput->m_Fire&1 || NewInput->m_Hook || NewInput->m_Down)
	{
		m_LatestActivity.m_TargetX = NewInput->m_TargetX;
		m_LatestActivity.m_TargetY = NewInput->m_TargetY;
		m_LastActionTick = Server()->Tick();
	}
}

CCharacter *CPlayer::GetCharacter()
{
	if(m_pCharacter && m_pCharacter->IsAlive())
		return m_pCharacter;
	return 0;
}

void CPlayer::KillCharacter(int Weapon)
{
	if(m_pCharacter)
	{
		m_pCharacter->Die(m_ClientID, Weapon);
		delete m_pCharacter;
		m_pCharacter = 0;
	}
}

void CPlayer::Respawn()
{
	if(m_Team != TEAM_SPECTATORS)
		m_Spawning = true;
}



void CPlayer::ForceToSpectators()
{
	/*
	m_ForceToSpectators = false;
	SetTeam(TEAM_SPECTATORS, false);
	m_TeamChangeTick = Server()->Tick();
	*/
}
	
	
void CPlayer::JoinTeam()
{
	/*
	if (m_WantedTeam != m_Team)
	{
		SetTeam(m_WantedTeam, false);
		m_TeamChangeTick = Server()->Tick();
	}
	*/
}
	

	
	
void CPlayer::SetTeam(int Team, bool DoChatMsg)
{
	// clamp the team
	Team = GameServer()->m_pController->ClampTeam(Team);
	if(m_Team == Team)
		return;

	char aBuf[512];

	KillCharacter();

	m_Team = Team;
	//if (Team != TEAM_SPECTATORS)
	//	m_WantedTeam = Team;
	m_LastActionTick = Server()->Tick();
	m_SpectatorID = SPEC_FREEVIEW;
	// we got to wait 0.5 secs before respawning
	if (!(GameServer()->m_pController->IsInfection() && Team == TEAM_BLUE))
		m_RespawnTick = Server()->Tick();
	
	str_format(aBuf, sizeof(aBuf), "team_join player='%d:%s' m_Team=%d", m_ClientID, Server()->ClientName(m_ClientID), m_Team);
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

	GameServer()->m_pController->OnPlayerInfoChange(GameServer()->m_apPlayers[m_ClientID]);
	
	if (m_pAI)
		SetRandomSkin();

	if(Team == TEAM_SPECTATORS)
	{
		// update spectator modes
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->m_SpectatorID == m_ClientID)
				GameServer()->m_apPlayers[i]->m_SpectatorID = SPEC_FREEVIEW;
		}
	}
}

void CPlayer::TryRespawn()
{
	vec2 SpawnPos = vec2(m_LatestX/32, 0);

	// todo remove
	if(!GameServer()->m_pController->CanCharacterSpawn(GetCID()))
		return;

	if(!GameServer()->m_pController->CanSpawn(m_Team, &SpawnPos, m_IsBot))
		return;
	
	m_Spawning = false;
	m_pCharacter = new(m_ClientID) CCharacter(&GameServer()->m_World);
	m_pCharacter->Spawn(this, SpawnPos);
	GameServer()->CreatePlayerSpawn(SpawnPos);
}

bool CPlayer::ForceRespawn(vec2 Pos)
{
	if (m_pCharacter)
		return false;

	m_Spawning = false;
	m_pCharacter = new(m_ClientID) CCharacter(&GameServer()->m_World);
	m_pCharacter->Spawn(this, Pos);
	GameServer()->CreatePlayerSpawn(Pos);
	
	return true;
}

int CPlayer::GetColorID()
{
	return m_TeeInfos.m_ColorSkin;
}

void CPlayer::SetRandomSkin()
{
	return;
	if (m_GotSkin)
		return;
	
	m_GotSkin = true;
	
	if (GetCharacter())
		GetCharacter()->m_Type = CCharacter::PLAYER;
}


void CPlayer::SetAISkin()
{
	GameServer()->Server()->SetClientName(GetCID(), m_AISkin.m_aName);
	str_copy(m_TeeInfos.m_HeadName, m_AISkin.m_aHead, 24);
	str_copy(m_TeeInfos.m_BodyName, m_AISkin.m_aBody, 24);
	str_copy(m_TeeInfos.m_HandName, m_AISkin.m_aHand, 24);
	str_copy(m_TeeInfos.m_FootName, m_AISkin.m_aFoot, 24);
	str_copy(m_TeeInfos.m_TopperName, m_AISkin.m_aTopper, 24);
	str_copy(m_TeeInfos.m_EyeName, m_AISkin.m_aEye, 24);
	
	m_TeeInfos.m_ColorSkin = m_AISkin.m_ColorSkin;
	m_TeeInfos.m_ColorBody = m_AISkin.m_ColorBody;
	m_TeeInfos.m_ColorTopper = m_AISkin.m_ColorTopper;
	m_TeeInfos.m_ColorFeet = m_AISkin.m_ColorFoot;
	m_TeeInfos.m_BloodColor = m_AISkin.m_ColorBlood;
}

void CPlayer::SetCustomSkin(int Type)
{
	return;
}


void CPlayer::AITick()
{
	if (m_pAI)
		m_pAI->Tick();
}
	

bool CPlayer::AIInputChanged()
{
	if (m_pAI)
		return m_pAI->m_InputChanged;

	return false;
}

