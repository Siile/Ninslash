


#include <new>
#include <engine/shared/config.h>
#include "player.h"

#include <game/weapons.h>
#include <game/server/botnames.h>
#include "gamemodes/texasrun.h"


MACRO_ALLOC_POOL_ID_IMPL(CPlayer, MAX_CLIENTS)

IServer *CPlayer::Server() const { return m_pGameServer->Server(); }

CPlayer::CPlayer(CGameContext *pGameServer, int ClientID, int Team)
{
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
	
	m_Score = 0;
	
	m_DeathTick = 0;
	m_ActionTimer = 0;
	
	m_InterestPoints = 0;
	m_BroadcastingCaptureStatus = false;
	
	m_EnableEmoticonGrenades = true;
	m_EnableWeaponInfo = 2;
	m_EnableAutoSpectating = true;
	
	m_IsBot = false;
	m_pAI = NULL;
	m_ToBeKicked = false;
	
	//m_WantedTeam = m_Team;
	//m_Team = TEAM_SPECTATORS;
	
	/*
	if(str_comp(g_Config.m_SvGametype, "cstt") == 0)
		m_ForceToSpectators = true;
	else
		m_ForceToSpectators = false;
	*/
	
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

void CPlayer::NewRound()
{
	m_Score = 0;
	
	m_InterestPoints = 0;
}

void CPlayer::SelectWeapon(int Weapon, int Group)
{
	if (GetCharacter() && Weapon >= 0 && Weapon < NUM_WEAPONS && Group >= 0 && Group < 3)
		GetCharacter()->SelectWeapon(Weapon, Group);
}

void CPlayer::SelectItem(int Item)
{
	if (GetCharacter() && Item >= 0 && Item < NUM_PLAYERITEMS)
		GetCharacter()->SelectItem(Item);
}



void CPlayer::UseKit(int Kit, vec2 Pos)
{
	if (GetCharacter() && Kit >= 0 && Kit < NUM_KITS)
		GetCharacter()->UseKit(Kit, Pos);
}


void CPlayer::DropWeapon()
{
	if (GetCharacter())
		GetCharacter()->DropWeapon();
}
	
void CPlayer::SwitchGroup()
{
	if (GetCharacter())
		GetCharacter()->SwitchGroup();
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
		
		//if(!m_pCharacter && m_DieTick+Server()->TickSpeed()*3 <= Server()->Tick())
		if(!m_pCharacter)
			m_Spawning = true;

		if(m_pCharacter)
		{
			if(m_pCharacter->IsAlive())
			{
				m_ViewPos = m_pCharacter->m_Pos;
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
	if(m_Team == TEAM_SPECTATORS && m_SpectatorID != SPEC_FREEVIEW && GameServer()->m_apPlayers[m_SpectatorID])
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
	pClientInfo->m_Body = m_TeeInfos.m_Body;
	pClientInfo->m_ColorBody = m_TeeInfos.m_ColorBody;
	pClientInfo->m_ColorFeet = m_TeeInfos.m_ColorFeet;
	pClientInfo->m_ColorTopper = m_TeeInfos.m_ColorTopper;
	pClientInfo->m_ColorSkin = m_TeeInfos.m_ColorSkin;

	CNetObj_PlayerInfo *pPlayerInfo = static_cast<CNetObj_PlayerInfo *>(Server()->SnapNewItem(NETOBJTYPE_PLAYERINFO, m_ClientID, sizeof(CNetObj_PlayerInfo)));
	if(!pPlayerInfo)
		return;

	pPlayerInfo->m_Latency = SnappingClient == -1 ? m_Latency.m_Min : GameServer()->m_apPlayers[SnappingClient]->m_aActLatency[m_ClientID];
	pPlayerInfo->m_Local = 0;
	pPlayerInfo->m_ClientID = m_ClientID;
	pPlayerInfo->m_Score = m_Score;
	pPlayerInfo->m_Team = m_Team;
	
	/*
	if (pPlayerInfo->m_Team != TEAM_SPECTATORS)
	{
		if (SnappingClient != GetCID())
			pPlayerInfo->m_Team = m_Team;
		else
		{
			if (GetCharacter() || GameServer()->m_pController->IsGameOver() ||  GameServer()->m_aMostInterestingPlayer[m_Team] < 0)
				pPlayerInfo->m_Team = m_Team;
			else
				pPlayerInfo->m_Team = TEAM_SPECTATORS;
		}
	}
	*/
	
	
	// snap weapons
	pPlayerInfo->m_Weapons = 0;
	pPlayerInfo->m_Kits = 0;
	
	if (GetCharacter())
	{
		for (int i = 0; i < NUM_WEAPONS; i++)
		{
			if (GetCharacter()->GotWeapon(i))
			{
				pPlayerInfo->m_Weapons |= 1 << i;
			}
		}
		
		pPlayerInfo->m_Kits = GetCharacter()->m_Kits;
		
		pPlayerInfo->m_Item1 = GetCharacter()->m_aItem[0];
		pPlayerInfo->m_Item2 = GetCharacter()->m_aItem[1];
		pPlayerInfo->m_Item3 = GetCharacter()->m_aItem[2];
		pPlayerInfo->m_Item4 = GetCharacter()->m_aItem[3];
		pPlayerInfo->m_Item5 = GetCharacter()->m_aItem[4];
		pPlayerInfo->m_Item6 = GetCharacter()->m_aItem[5];
	}
	
	

	if(m_ClientID == SnappingClient)
		pPlayerInfo->m_Local = 1;

	if(m_ClientID == SnappingClient && m_Team == TEAM_SPECTATORS)
	{
		CNetObj_SpectatorInfo *pSpectatorInfo = static_cast<CNetObj_SpectatorInfo *>(Server()->SnapNewItem(NETOBJTYPE_SPECTATORINFO, m_ClientID, sizeof(CNetObj_SpectatorInfo)));
		if(!pSpectatorInfo)
			return;

		pSpectatorInfo->m_SpectatorID = m_SpectatorID;
		pSpectatorInfo->m_X = m_ViewPos.x;
		pSpectatorInfo->m_Y = m_ViewPos.y;
	}
	
	/*
	if(m_ClientID == SnappingClient && pPlayerInfo->m_Team == TEAM_SPECTATORS)
	{
		CNetObj_SpectatorInfo *pSpectatorInfo = static_cast<CNetObj_SpectatorInfo *>(Server()->SnapNewItem(NETOBJTYPE_SPECTATORINFO, m_ClientID, sizeof(CNetObj_SpectatorInfo)));
		if(!pSpectatorInfo)
		{
			// SPEC_FREEVIEW

			//pSpectatorInfo->m_X = 0;
			//pSpectatorInfo->m_Y = 0;
		}
		else
		{
			pSpectatorInfo->m_SpectatorID = m_SpectatorID;
			pSpectatorInfo->m_X = m_ViewPos.x;
			pSpectatorInfo->m_Y = m_ViewPos.y;
		}
	}
	*/
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
				str_format(aBuf, sizeof(aBuf), "'%s' has left the game (%s)", Server()->ClientName(m_ClientID), pReason);
			else
				str_format(aBuf, sizeof(aBuf), "'%s' has left the game", Server()->ClientName(m_ClientID));
			GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		}

		str_format(aBuf, sizeof(aBuf), "leave player='%d:%s'", m_ClientID, Server()->ClientName(m_ClientID));
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", aBuf);
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
		NewInput->m_Fire&1 || NewInput->m_Hook)
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
	
	/* skip this
	if(DoChatMsg)
	{
		str_format(aBuf, sizeof(aBuf), "'%s' joined the %s", Server()->ClientName(m_ClientID), GameServer()->m_pController->GetTeamName(Team));
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
	}
	*/

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
	vec2 SpawnPos;

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


void CPlayer::SetRandomSkin()
{
	switch (rand()%15)
	{
	case 0: str_copy(m_TeeInfos.m_TopperName, "tigerboy", 64); break;	
	case 1: str_copy(m_TeeInfos.m_TopperName, "emo", 64); break;
	case 2: str_copy(m_TeeInfos.m_TopperName, "emo2", 64); break;
	case 3: str_copy(m_TeeInfos.m_TopperName, "dr", 64); break;
	case 4: str_copy(m_TeeInfos.m_TopperName, "gentlenin", 64); break;
	case 5: str_copy(m_TeeInfos.m_TopperName, "gentlenin2", 64); break;
	case 6: str_copy(m_TeeInfos.m_TopperName, "casual", 64); break;
	case 7: str_copy(m_TeeInfos.m_TopperName, "casual2", 64); break;
	case 8: str_copy(m_TeeInfos.m_TopperName, "casual3", 64); break;
	case 9: str_copy(m_TeeInfos.m_TopperName, "pipo", 64); break;
	case 10: str_copy(m_TeeInfos.m_TopperName, "nitters", 64); break;
	case 11: str_copy(m_TeeInfos.m_TopperName, "raiden", 64); break;
	case 12: str_copy(m_TeeInfos.m_TopperName, "afro", 64); break;
	case 13: str_copy(m_TeeInfos.m_TopperName, "long", 64); break;
	default: str_copy(m_TeeInfos.m_TopperName, "default", 64);
	};	
	
	switch (rand()%5)
	{
	case 0: str_copy(m_TeeInfos.m_EyeName, "halfhollow", 64); break;	
	case 1: str_copy(m_TeeInfos.m_EyeName, "single", 64); break;
	case 2: str_copy(m_TeeInfos.m_EyeName, "birdy", 64); break;
	case 3: str_copy(m_TeeInfos.m_EyeName, "lid", 64); break;
	default: str_copy(m_TeeInfos.m_EyeName, "default", 64);
	};
	
	/*
	str_copy(m_TeeInfos.m_TopperName, "none", 64);
	str_copy(m_TeeInfos.m_EyeName, "x_robot", 64);
	
	m_TeeInfos.m_Body = 3;
	m_TeeInfos.m_ColorSkin = 10747862;
	m_TeeInfos.m_ColorBody = 0;
	m_TeeInfos.m_ColorFeet = 0;
	m_TeeInfos.m_ColorTopper = 0;
	*/
	
	m_TeeInfos.m_Body = rand()%NUM_BODIES;
	m_TeeInfos.m_ColorTopper = rand()*(0xFFFFFF/RAND_MAX);
	m_TeeInfos.m_ColorSkin = rand()*(0xFFFFFF/RAND_MAX);
	m_TeeInfos.m_ColorBody = rand()*(0xFFFFFF/RAND_MAX);
	m_TeeInfos.m_ColorFeet = rand()*(0xFFFFFF/RAND_MAX);
	
	// generate random name
	char aBotName[128];
	str_format(aBotName, sizeof(aBotName), "%s%s", aBotName1[rand()%(sizeof(aBotName1)/sizeof(aBotName1[0]))], aBotName2[rand()%(sizeof(aBotName2)/sizeof(aBotName2[0]))]);
	GameServer()->Server()->SetClientName(GetCID(), aBotName);
}


void CPlayer::SetCustomSkin(int Type)
{
	if (Type == 1)
	{
		str_copy(m_TeeInfos.m_TopperName, "meganin", 64);
		str_copy(m_TeeInfos.m_EyeName, "birdy", 64);
		m_TeeInfos.m_Body = 0;
		m_TeeInfos.m_ColorTopper = 10419968;
		m_TeeInfos.m_ColorSkin = 10747862;
		m_TeeInfos.m_ColorBody = 9174784;
		m_TeeInfos.m_ColorFeet = 10354432;
		
		char aBotName[128];
		str_format(aBotName, sizeof(aBotName), "%s%s-T1", aBotName1[rand()%(sizeof(aBotName1)/sizeof(aBotName1[0]))], aBotName2[rand()%(sizeof(aBotName2)/sizeof(aBotName2[0]))]);
		GameServer()->Server()->SetClientName(GetCID(), aBotName);
	}
	
	if (Type == 2)
	{
		str_copy(m_TeeInfos.m_TopperName, "none", 64);
		str_copy(m_TeeInfos.m_EyeName, "x_robo1", 64);
		m_TeeInfos.m_Body = 3;
		m_TeeInfos.m_ColorTopper = 10747862;
		m_TeeInfos.m_ColorSkin = 10747862;
		m_TeeInfos.m_ColorBody = 10747862;
		m_TeeInfos.m_ColorFeet = 10747862;
		
		GameServer()->Server()->SetClientName(GetCID(), "");
	}
	
	if (Type == 3)
	{
		str_copy(m_TeeInfos.m_TopperName, "none", 64);
		str_copy(m_TeeInfos.m_EyeName, "x_robo2", 64);
		m_TeeInfos.m_Body = 4;
		m_TeeInfos.m_ColorTopper = 10747862;
		m_TeeInfos.m_ColorSkin = 10747862;
		m_TeeInfos.m_ColorBody = 10747862;
		m_TeeInfos.m_ColorFeet = 10747862;
		
		GameServer()->Server()->SetClientName(GetCID(), "");
	}
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

