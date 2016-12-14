/* Simple yet funny gun game mod for Ninslash - Henritees 2016 */

#ifndef GAME_SERVER_GAMEMODES_GUNGAME_H
#define GAME_SERVER_GAMEMODES_GUNGAME_H

#include <game/server/gamecontroller.h>


class CGameControllerGunGame : public IGameController
{
public:
	CGameControllerGunGame(class CGameContext *pGameServer);

	void DropPickup(vec2 Pos, int PickupType, vec2 Force, int PickupSubtype, float Ammo = -1.0f);


	virtual void Tick();
	virtual void Snap(int SnappingClient);
	virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	virtual void OnCharacterSpawn(class CCharacter *pChr, bool RequestAI);

	virtual void DoWincheck();

private:
	void SendBroadcastInfo(class CPlayer *pWhom);
	void AdvanceWeapon(class CPlayer *pWhom);
	void RemoveWeapon(class CPlayer *pWhom);
	void UpdateWeapon(class CCharacter *pWhom);

	const int GetWeaponID(int index) const;
	const int LastWeapon() const;
};

#endif
