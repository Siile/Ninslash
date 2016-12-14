/* Simple yet funny gun game mod for Ninslash - Henritees 2016 */

#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/server/ai/dm_ai.h>
#include "gungame.h"

static const int GG_WEAPON_ORDER[] = {
		W_ELECTRIC,
		W_SHOTGUN,
		W_RIFLE,
		W_CHAINSAW,
		W_LASER,
		W_FLAMER,
		W_GRENADELAUNCHER,
		W_HAMMER, // knife
};
static const int GG_NUM_USED_WEAPONS = sizeof(GG_WEAPON_ORDER)/sizeof(GG_WEAPON_ORDER[0]);


CGameControllerGunGame::CGameControllerGunGame(CGameContext *pGameServer) : IGameController(pGameServer)
{
	m_pGameType = "GunGame";
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
	IGameController::Tick();
	AutoBalance();
	GameServer()->UpdateAI();

	// not so fancy... but makes sure the players have only the weapons they may
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(GameServer()->m_apPlayers[i])
			if(GameServer()->m_apPlayers[i]->GetCharacter())
				if(GameServer()->m_apPlayers[i]->GetCharacter()->m_ActiveCustomWeapon != GG_WEAPON_ORDER[GameServer()->m_apPlayers[i]->m_Score])
					UpdateWeapon(GameServer()->m_apPlayers[i]->GetCharacter());
}

void CGameControllerGunGame::OnCharacterSpawn(class CCharacter *pChr, bool RequestAI)
{
	IGameController::OnCharacterSpawn(pChr, RequestAI);

	// init AI
	if(RequestAI)
		pChr->GetPlayer()->m_pAI = new CAIdm(GameServer(), pChr->GetPlayer());

	UpdateWeapon(pChr);
}

int CGameControllerGunGame::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	/*
	 * -- no weapon drops since this gamemode wouldn't make sense otherwise --
	 */


	// make the killer more interesting for autospec
	if(pKiller && (pKiller->GetTeam() != pVictim->GetPlayer()->GetTeam() || !IsTeamplay()))
		pKiller->m_InterestPoints += 60;

	// disadvance the victims weapon stage by one
	RemoveWeapon(pVictim->GetPlayer());

	// do scoring
	if(!pKiller || Weapon == WEAPON_GAME)
		return 0;

	if(pKiller == pVictim->GetPlayer())
	{
		// suicide; doesn't need to be treated special
	}
	else
	{
		if(IsTeamplay() && pVictim->GetPlayer()->GetTeam() == pKiller->GetTeam())
		{
			// teamkill; remove one weapon and warn the player
			RemoveWeapon(pKiller);
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "Be careful not to kill your teammates! (You just killed %s)", Server()->ClientName(pVictim->GetPlayer()->GetCID()));
			GameServer()->SendChatTarget(pKiller->GetCID(), aBuf);
		}
		else
		{
			AdvanceWeapon(pKiller); // normal kill, advance the weapon stage...
			if(pKiller->GetCharacter()) // (a dead killer? - yes, this can indeed happen!)
				pKiller->GetCharacter()->RefillHealth(); // ...and refill the health points
		}
	}

	if(Weapon == WEAPON_SELF)
		pVictim->GetPlayer()->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()*3;

	return 0;
}

void CGameControllerGunGame::AdvanceWeapon(class CPlayer *pWhom)
{
	char aBuf[128];
	int Next = GetWeaponID(pWhom->m_Score+++1);
	if(Next == LastWeapon())
	{
		str_format(aBuf, sizeof(aBuf), "%s reached the final stage and is about to win!", Server()->ClientName(pWhom->GetCID()));
		GameServer()->SendChatTarget(-1, aBuf);
		GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_EN);
	}
	else if(Next < 0) // won
	{
		str_format(aBuf, sizeof(aBuf), "%s did the knife kill and won the game!", Server()->ClientName(pWhom->GetCID()));
		GameServer()->SendChatTarget(-1, aBuf);
		GameServer()->CreateSoundGlobal(SOUND_CTF_CAPTURE);
		EndRound();
		return;
	}
	else
		GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_PL, pWhom->GetCID());

	UpdateWeapon(pWhom->GetCharacter());
}

void CGameControllerGunGame::RemoveWeapon(class CPlayer *pWhom)
{
	pWhom->m_Score = max(0, pWhom->m_Score-1);
	UpdateWeapon(pWhom->GetCharacter());
	GameServer()->CreateSoundGlobal(SOUND_CTF_DROP, pWhom->GetCID());
}

void CGameControllerGunGame::UpdateWeapon(class CCharacter *pWhom)
{
	if(!pWhom)
		return;

	const int CurrentWeapon = GetWeaponID(pWhom->GetPlayer()->m_Score);

	// clear out all weapons...
	for(int i = 0; i < NUM_CUSTOMWEAPONS; i++)
	{
		mem_zero(&pWhom->m_aWeapon[i], sizeof(CCharacter::CustomWeaponStat));
		if(i != CurrentWeapon)
			pWhom->m_aWeapon[i].m_Disabled = true;
	}

	// ...and give him the correct one
	pWhom->GiveCustomWeapon(CurrentWeapon);
	pWhom->SetCustomWeapon(CurrentWeapon);
	SendBroadcastInfo(pWhom->GetPlayer());
}

const int CGameControllerGunGame::GetWeaponID(int index) const
{
	if(index >= GG_NUM_USED_WEAPONS)
		return -1;
	return GG_WEAPON_ORDER[index];
}

const int CGameControllerGunGame::LastWeapon() const
{
	return GG_WEAPON_ORDER[GG_NUM_USED_WEAPONS-1];
}

void CGameControllerGunGame::SendBroadcastInfo(CPlayer *pWhom)
{
	if(pWhom->m_IsBot)
		return;

	// send the player his info through broadcast
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "Weapon %i/%i", pWhom->m_Score+1, GG_NUM_USED_WEAPONS);
	if(pWhom->m_Score == GG_NUM_USED_WEAPONS-1)
		str_append(aBuf, " - Final Stage!", sizeof(aBuf));
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

	pGameInfoObj->m_ScoreLimit = GG_NUM_USED_WEAPONS;

	if (m_RoundTimeLimit > 0)
		pGameInfoObj->m_TimeLimit = m_RoundTimeLimit/60+1;
	else
		pGameInfoObj->m_TimeLimit = 0;

	m_TimeLimit = pGameInfoObj->m_TimeLimit;

	pGameInfoObj->m_RoundNum = (str_length(g_Config.m_SvMaprotation) && g_Config.m_SvRoundsPerMap) ? g_Config.m_SvRoundsPerMap : 0;
	pGameInfoObj->m_RoundCurrent = m_RoundCount+1;
}
