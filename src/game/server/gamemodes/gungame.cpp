/* Simple yet funny gun game mod for Ninslash - Henritees 2016 */

#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/server/ai/dm_ai.h>
#include <game/server/player.h>
#include "gungame.h"

static const int GG_WEAPON_ORDER[] = {
		W_ELECTRIC,
		W_SHOTGUN,
		W_LASER,
		W_GRENADELAUNCHER,
		W_FLAMER,
		W_CHAINSAW,
		W_RIFLE,
		W_HAMMER, // knife
};
static const int GG_NUM_USED_WEAPONS = sizeof(GG_WEAPON_ORDER)/sizeof(GG_WEAPON_ORDER[0]);


CGameControllerGunGame::CGameControllerGunGame(CGameContext *pGameServer) : IGameController(pGameServer)
{
	m_pGameType = "GUN";
	m_GameFlags = 0;
}

void CGameControllerGunGame::DropPickup(vec2 Pos, int PickupType, vec2 Force, int PickupSubtype, float Ammo)
{
	// do nothing - there should not be any drops
	return;
}

void CGameControllerGunGame::DoWincheck()
{
	// do nothing (win check is being done with every kill)
	return;
}

void CGameControllerGunGame::Tick()
{
	static int LastSvScorelimit = g_Config.m_SvScorelimit;
	if(g_Config.m_SvScorelimit != LastSvScorelimit)
	{
		LastSvScorelimit = g_Config.m_SvScorelimit;
		dbg_msg("gungame", "ending round due to scorelimit change");
		EndRound();
	}

	IGameController::Tick();
	AutoBalance();
	GameServer()->UpdateAI();

	// not so fancy... but makes sure the players have only the weapons they may
/*	for(int i = 0; i < MAX_CLIENTS; i++)
		if(GameServer()->m_apPlayers[i])
			if(GameServer()->m_apPlayers[i]->GetCharacter())
				if(GameServer()->m_apPlayers[i]->GetCharacter()->m_ActiveCustomWeapon != GetWeaponID(GameServer()->m_apPlayers[i]->m_Score))
					UpdateWeapon(GameServer()->m_apPlayers[i]->GetCharacter());*/
}

void CGameControllerGunGame::OnCharacterSpawn(class CCharacter *pChr, bool RequestAI)
{
	IGameController::OnCharacterSpawn(pChr, RequestAI);

	// init AI
	if(RequestAI)
		pChr->GetPlayer()->m_pAI = new CAIdm(GameServer(), pChr->GetPlayer());
	else
		SendBroadcastInfo(pChr->GetPlayer());

}

int CGameControllerGunGame::OnCharacterDeath(CCharacter *pVictim, CPlayer *pKiller, int Weapon)
{
	/*
	 * -- no weapon drops since this gamemode wouldn't make sense otherwise --
	 */

	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "gg", "OnCharacterDeath");

	// disadvance the victims weapon stage by one
	DecreaseScore(pVictim->GetPlayer());
	pVictim->GetPlayer()->m_Statistics.m_Deaths++;

	// do scoring
	if(!pKiller || Weapon == WEAPON_GAME || (GetWeaponID(pKiller->m_Score) != Weapon && Weapon != WEAPON_SELF))
		return 0;

	if(pKiller == pVictim->GetPlayer())
	{
		// suicide - get punished by losing 2 weapon stages!
		DecreaseScore(pVictim->GetPlayer());
	}
	else
	{
		if(IsTeamplay() && pVictim->GetPlayer()->GetTeam() == pKiller->GetTeam())
		{
			// teamkill; remove one weapon and warn the player
			DecreaseScore(pKiller);
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "Be careful not to kill your teammates! (You just killed %s)", Server()->ClientName(pVictim->GetPlayer()->GetCID()));
			GameServer()->SendChatTarget(pKiller->GetCID(), aBuf);
		}
		else
		{
			// normal kill
			if(IncreaseScore(pKiller)); // advance the weapon stage...
				if(pKiller->GetCharacter()) // (a dead killer? - yes, this can indeed happen!)
					pKiller->GetCharacter()->RefillHealth(); // ...and refill the health points
			pKiller->m_Statistics.m_Kills++;
			pKiller->m_InterestPoints += 60;
		}
	}

	//if(Weapon == WEAPON_SELF)
	//	pVictim->GetPlayer()->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()*3;

	return 0;
}

bool CGameControllerGunGame::IncreaseScore(CPlayer *pWhom)
{
	pWhom->m_Score++;

	char aBuf[128];
	if(pWhom->m_Score == WinningScore()-1)
	{
		str_format(aBuf, sizeof(aBuf), "%s reached the final stage and is about to win!", Server()->ClientName(pWhom->GetCID()));
		GameServer()->SendChatTarget(-1, aBuf);
		GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_EN);
	}
	else if(pWhom->m_Score >= WinningScore()) // won
	{
		str_format(aBuf, sizeof(aBuf), "%s did the knife kill and won the game!", Server()->ClientName(pWhom->GetCID()));
		GameServer()->SendChatTarget(-1, aBuf);
		GameServer()->SendChatTarget(-1, "-------------------------------------");
		GameServer()->CreateSoundGlobal(SOUND_CTF_CAPTURE);
		EndRound();
		return false;
	}
	else if(g_Config.m_SvScorelimit > 1 && pWhom->m_Score % g_Config.m_SvScorelimit == 0)
		GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_PL, pWhom->GetCID());

	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "gg", "IncreaseScore2");
	
	UpdateWeapon(pWhom->GetCharacter());
	if(g_Config.m_SvScorelimit > 1)
		return pWhom->m_Score % g_Config.m_SvScorelimit == 0;
	return true;
}

bool CGameControllerGunGame::DecreaseScore(CPlayer *pWhom)
{
	pWhom->m_Score = max(0, pWhom->m_Score-1);
	bool HasLostWeapon;
	if(g_Config.m_SvScorelimit > 1)
		HasLostWeapon = pWhom->m_Score % g_Config.m_SvScorelimit == g_Config.m_SvScorelimit - 1;
	else
		HasLostWeapon = true;

	if(HasLostWeapon)
		GameServer()->CreateSoundGlobal(SOUND_CTF_DROP, pWhom->GetCID());
	return HasLostWeapon;
}

void CGameControllerGunGame::UpdateWeapon(CCharacter *pWhom)
{
	if(!pWhom)
		return;

	const int WantedWeapon = GetWeaponID(pWhom->GetPlayer()->m_Score);

	// clear out all weapons...
	for(int i = 0; i < NUM_CUSTOMWEAPONS; i++)
	{
		mem_zero(&pWhom->m_aWeapon[i], sizeof(CCharacter::CustomWeaponStat));
		if(i != WantedWeapon)
			pWhom->m_aWeapon[i].m_Disabled = true;
	}

	// ...and give him the correct one
	pWhom->GiveCustomWeapon(WantedWeapon);
	pWhom->SetCustomWeapon(WantedWeapon);
	SendBroadcastInfo(pWhom->GetPlayer());
}

const int CGameControllerGunGame::GetStage(CPlayer *pWhom) const
{
	return GetStage(pWhom->m_Score);
}

const int CGameControllerGunGame::GetStage(int Score) const
{
	if(g_Config.m_SvScorelimit > 1)
		return (int)((float)Score/(float)g_Config.m_SvScorelimit);
	return Score;
}

const int CGameControllerGunGame::GetWeaponID(int Score) const
{
	int index = GetStage(Score);
	if(index >= GG_NUM_USED_WEAPONS)
		return -1;
	return GG_WEAPON_ORDER[index];
}

const int CGameControllerGunGame::LastWeapon() const
{
	return GG_WEAPON_ORDER[GG_NUM_USED_WEAPONS-1];
}

const int CGameControllerGunGame::WinningScore() const
{
	if(g_Config.m_SvScorelimit > 1)
		return GG_NUM_USED_WEAPONS*g_Config.m_SvScorelimit-(g_Config.m_SvScorelimit-1);
	return GG_NUM_USED_WEAPONS;
}

void CGameControllerGunGame::SendBroadcastInfo(CPlayer *pWhom)
{
	if(pWhom->m_IsBot)
		return;

	// send the player his info through broadcast
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "Weapon %i/%i", GetStage(pWhom)+1, GG_NUM_USED_WEAPONS);
	if(pWhom->m_Score == WinningScore()-1)
		str_append(aBuf, " - Final Stage!", sizeof(aBuf));
	else if(g_Config.m_SvScorelimit > 1)
	{
		char aScore[32];
		str_format(aScore, sizeof(aScore), "\n%i more kill%s to advance",
				   g_Config.m_SvScorelimit-(pWhom->m_Score%g_Config.m_SvScorelimit),
				   g_Config.m_SvScorelimit-(pWhom->m_Score%g_Config.m_SvScorelimit) == 1 ? "" : "s");
		str_append(aBuf, aScore, sizeof(aBuf));
	}
	GameServer()->SendBroadcast(aBuf, pWhom->GetCID(), true);
}

void CGameControllerGunGame::Snap(int SnappingClient)
{
	CNetObj_GameInfo *pGameInfoObj = (CNetObj_GameInfo *)Server()->SnapNewItem(NETOBJTYPE_GAMEINFO, 0, sizeof(CNetObj_GameInfo));
	if(!pGameInfoObj)
		return;

	if (m_ResetTime)
	{
		m_ResetTime = false;
		if (m_RoundTimeLimit > 0)
			m_RoundStartTick = Server()->Tick() - Server()->TickSpeed()*(60-m_RoundTimeLimit%60);
		else
			m_RoundStartTick = Server()->Tick();
	}

	pGameInfoObj->m_GameFlags = m_GameFlags;
	pGameInfoObj->m_GameStateFlags = 0;
	if(m_GameOverTick != -1)
		pGameInfoObj->m_GameStateFlags |= GAMESTATEFLAG_GAMEOVER;
	if(m_SuddenDeath)
		pGameInfoObj->m_GameStateFlags |= GAMESTATEFLAG_SUDDENDEATH;
	if(GameServer()->m_World.m_Paused)
		pGameInfoObj->m_GameStateFlags |= GAMESTATEFLAG_PAUSED;
	pGameInfoObj->m_RoundStartTick = m_RoundStartTick;
	pGameInfoObj->m_WarmupTimer = m_Warmup;

	pGameInfoObj->m_ScoreLimit = WinningScore();

	if (m_RoundTimeLimit > 0)
		pGameInfoObj->m_TimeLimit = m_RoundTimeLimit/60+1;
	else
		pGameInfoObj->m_TimeLimit = 0;

	m_TimeLimit = pGameInfoObj->m_TimeLimit;

	pGameInfoObj->m_RoundNum = (str_length(g_Config.m_SvMaprotation) && g_Config.m_SvRoundsPerMap) ? g_Config.m_SvRoundsPerMap : 0;
	pGameInfoObj->m_RoundCurrent = m_RoundCount+1;
}

void CGameControllerGunGame::EndRound()
{
	IGameController::EndRound();

	// collect some stats if we got a winner
	bool GotWinner = false;
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(GameServer()->m_apPlayers[i])
			if((GotWinner = (GameServer()->m_apPlayers[i]->m_Score >= WinningScore())))
				break;
	if(!GotWinner)
		return;
	else
		dbg_msg("game", "round ended without a winner");

	char aBuf[256];

	// most best K/D
	int pv = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(GameServer()->m_apPlayers[i])
			if((float)GameServer()->m_apPlayers[i]->m_Statistics.m_Kills/(float)GameServer()->m_apPlayers[i]->m_Statistics.m_Deaths >
					(float)GameServer()->m_apPlayers[i]->m_Statistics.m_Kills/(float)GameServer()->m_apPlayers[i]->m_Statistics.m_Deaths)
				pv = i;
	str_format(aBuf, sizeof(aBuf), "Highest K/D ratio: %s (%.2f)", Server()->ClientName(pv), (float)GameServer()->m_apPlayers[pv]->m_Statistics.m_Kills/(float)GameServer()->m_apPlayers[pv]->m_Statistics.m_Deaths);
	GameServer()->SendChatTarget(-1, aBuf);

	// most kills
	pv = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(GameServer()->m_apPlayers[i])
			if(GameServer()->m_apPlayers[i]->m_Statistics.m_Kills > GameServer()->m_apPlayers[pv]->m_Statistics.m_Kills)
				pv = i;
	str_format(aBuf, sizeof(aBuf), "Most Kills: %s (%i)", Server()->ClientName(pv), GameServer()->m_apPlayers[pv]->m_Statistics.m_Kills);
	GameServer()->SendChatTarget(-1, aBuf);

	// most deaths
	pv = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(GameServer()->m_apPlayers[i])
			if(GameServer()->m_apPlayers[i]->m_Statistics.m_Deaths > GameServer()->m_apPlayers[pv]->m_Statistics.m_Deaths)
				pv = i;
	str_format(aBuf, sizeof(aBuf), "Most Deaths: %s (%i)", Server()->ClientName(pv), GameServer()->m_apPlayers[pv]->m_Statistics.m_Deaths);
	GameServer()->SendChatTarget(-1, aBuf);

	GameServer()->SendChatTarget(-1, "-------------------------------------");
}

bool CGameControllerGunGame::CanSeePickup(int CID, int Type, int Subtype)
{
	return Type == POWERUP_WEAPON && GetWeaponID(GameServer()->m_apPlayers[CID]->m_Score) == Subtype;
}

bool CGameControllerGunGame::CanDropWeapon(class CCharacter *pCharacter)
{
	// in gungame you cannot drop your weapon
	return false;
}

int CGameControllerGunGame::GetLockedWeapon(class CCharacter *pCharacter)
{
	int super = IGameController::GetLockedWeapon(pCharacter);
	if(super == -1)
	{
		int own = GetWeaponID(pCharacter->GetPlayer()->m_Score);
		if(own == -1)
			return LastWeapon();
		return own;
	}
	return super;
}