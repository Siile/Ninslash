/* Simple yet funny gun game mod for Ninslash - Henritees 2016 */

#include <engine/config.h>
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
	m_GameFlags = GAMEFLAG_FLAGS;
}

void CGameControllerGunGame::DropPickup(vec2 Pos, int PickupType, vec2 Force, int PickupSubtype, float Ammo)
{
	// do nothing
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
	else
		SendBroadcastInfo(pChr->GetPlayer());

	pChr->GetPlayer()->m_Score = 0;
	UpdateWeapon(pChr);
}

int CGameControllerGunGame::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	// -- no weapon drops since this gamemode wouldn't make sense otherwise --

	pVictim->GetPlayer()->m_Score = 0;
	SendBroadcastInfo(pVictim->GetPlayer());

	if(pKiller && (pKiller->GetTeam() != pVictim->GetPlayer()->GetTeam() || !IsTeamplay()))
	{
		//pKiller->m_Score++;
		pKiller->m_InterestPoints += 60;
	}

	// do scoreing
	if(!pKiller || Weapon == WEAPON_GAME)
		return 0;

	if(pKiller == pVictim->GetPlayer())
	{
		if (!(IsInfection() && pVictim->GetPlayer()->GetTeam() == TEAM_BLUE))
			pVictim->GetPlayer()->m_Score = 0; // suicide, reset weapon
	}
	else
	{
		if(IsTeamplay() && pVictim->GetPlayer()->GetTeam() == pKiller->GetTeam())
		{
			RemoveWeapon(pKiller); // teamkill, remove 1 weapon
		}
		else
			AdvanceWeapon(pKiller); // normal kill, advance by 1 weapon
	}

	if(Weapon == WEAPON_SELF)
		pVictim->GetPlayer()->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()*3.0f;

	SendBroadcastInfo(pKiller);
	return 0;
}

void CGameControllerGunGame::AdvanceWeapon(class CPlayer *pWhom)
{
	char aBuf[128];

	int Next = NextWeapon(++(pWhom->m_Score));
	if(Next == LastWeapon())
	{
		str_format(aBuf, sizeof(aBuf), "%s reached the final stage and is about to win!", Server()->ClientName(pWhom->GetCID()));
		GameServer()->SendChatTarget(-1, aBuf);
	}
	else if(Next == -1) // won
	{
		str_format(aBuf, sizeof(aBuf), "%s did the hammer kill and won the game!", Server()->ClientName(pWhom->GetCID()));
		GameServer()->SendChatTarget(-1, aBuf);
		EndRound();
		return;
	}

	UpdateWeapon(pWhom->GetCharacter());
}

void CGameControllerGunGame::RemoveWeapon(class CPlayer *pWhom)
{
	char aBuf[128];

	pWhom->m_Score = max(0, pWhom->m_Score-1);

	// send the player his info
	str_format(aBuf, sizeof(aBuf), "Weapon %i/%i", pWhom->m_Score, GG_NUM_USED_WEAPONS);
	GameServer()->SendBroadcast(aBuf, pWhom->GetCID(), true);

	UpdateWeapon(pWhom->GetCharacter());
}

void CGameControllerGunGame::UpdateWeapon(class CCharacter *pWhom)
{
	if(!pWhom)
		return;

	// clear out all weapons...
	for(int i = 0; i < NUM_CUSTOMWEAPONS; i++)
	{
		mem_zero(&pWhom->m_aWeapon[i], sizeof(CCharacter::CustomWeaponStat));
		if(i != GG_WEAPON_ORDER[pWhom->GetPlayer()->m_Score])
			pWhom->m_aWeapon[i].m_Disabled = true;
	}

	// ...and give him the correct one
	pWhom->GiveCustomWeapon(GG_WEAPON_ORDER[pWhom->GetPlayer()->m_Score]);
	pWhom->SetCustomWeapon(GG_WEAPON_ORDER[pWhom->GetPlayer()->m_Score]);

}

int CGameControllerGunGame::NextWeapon(int current)
{
	if(current+1 >= GG_NUM_USED_WEAPONS)
		return -1;
	return GG_WEAPON_ORDER[current+1];
}

int CGameControllerGunGame::PrevWeapon(int current)
{
	if(current <= 0)
		return GG_WEAPON_ORDER[0];
	return GG_WEAPON_ORDER[current-1];
}

const int CGameControllerGunGame::LastWeapon() const
{
	return GG_WEAPON_ORDER[GG_NUM_USED_WEAPONS-1];
}

void CGameControllerGunGame::DoWincheck()
{
	// do nothing
	return;
}

void CGameControllerGunGame::SendBroadcastInfo(CPlayer *pWhom)
{
	// send the player his info
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "Weapon %i/%i", pWhom->m_Score, GG_NUM_USED_WEAPONS);
	if(pWhom->m_Score == GG_NUM_USED_WEAPONS-1)
		str_append(aBuf, " - Final Stage!", sizeof(aBuf));
	GameServer()->SendBroadcast(aBuf, pWhom->GetCID(), true);
}
