#ifndef GAME_WEAPONS_H
#define GAME_WEAPONS_H

#include <cstring>
#include <game/generated/protocol.h>
#include <base/vmath.h>

enum PlayerItems
{
	PLAYERITEM_FILL,
	PLAYERITEM_RAGE,
	PLAYERITEM_INVISIBILITY,
	PLAYERITEM_LANDMINE,
	PLAYERITEM_ELECTROMINE,
	PLAYERITEM_SHIELD,
	PLAYERITEM_FUEL,
	PLAYERITEM_UPGRADE,
	NUM_PLAYERITEMS,
};

enum WeaponBits
{
	// 4 first bits of weapon int are flags / statuses, 5 in case of droids
	BIT_WEAPON = 1<<0,
	BIT_TURRET = 1<<1,
	BIT_BUILDING = 1<<2,
	BIT_STATICWEAPON = 1<<3,
	BIT_DROID = 1<<3,
	BIT_ONDEATH = 1<<4,
	
	// game / world
	WEAPON_NONE = 0,
	_WEAPON_GAME = 1<<1,
	_WEAPON_WORLD = 1<<2,
	WEAPON_ACID = 1<<3,
	
	// static / non-modular weapons - needs to have the sprites in same order
	SW_TOOL = 0,
	SW_GUN1,
	SW_GUN2,
	SW_GRENADE1,
	SW_GRENADE2,
	SW_GRENADE3,
	SW_BAZOOKA,
	SW_BOUNCER,
	SW_CHAINSAW,
	SW_FLAMER,
	SW_UPGRADE,
	SW_SHIELD,
	SW_RESPAWNER,
	SW_MASK1,
	SW_MASK2,
	SW_MASK3,
	SW_MASK4,
	SW_MASK5,
	SW_INVIS,
	SW_ELECTROWALL,
	SW_AREASHIELD,
	SW_CLUSTER,
	SW_SHURIKEN,
	SW_BOMB,
	SW_BALL,
	NUM_SW,
	
	
	// weapon rendering types
	WRT_NONE=0,
	WRT_WEAPON1, // rifles, shotguns etc.
	WRT_WEAPON2, // gun
	WRT_ITEM1,
	WRT_MELEE,
	WRT_MELEESMALL,
	WRT_SPIN,
	
	// weapon firing types
	WFT_NONE=0,
	WFT_MELEE, // swords
	WFT_PROJECTILE, // rifles, shotguns etc.
	WFT_CHARGE, // gun 2 & charge weapons
	WFT_HOLD, // chainsaw & flamer
	WFT_THROW, // grenades
	WFT_ACTIVATE // some items
};


static const char *s_TipText[NUM_SW] = {
	"Repair tool",
	"",
	"",
	"Grenade",
	"Electric grenade",
	"Supply grenade",
	"",
	"",
	"",
	"",
	"Weapon upgrade",
	"Energy shield",
	"Respawn device",
	"Mask of regeneration",
	"Mask of speed",
	"Mask of protection",
	"Mask of plenty",
	"Mask of melee",
	"Invisibility device",
	"",
	"",
	"Bomb (for destroying reactors)",
	""
};
 

#define WEAPON_GAME_SIZE 15

inline const bool IsWeapon(int Weapon) { return (Weapon & BIT_WEAPON) ? true : false; }
inline const bool IsTurret(int Weapon) { return (IsWeapon(Weapon) && (Weapon & BIT_TURRET)) ? true : false; }
inline const bool IsBuilding(int Weapon) { return (!IsWeapon(Weapon) && (Weapon & BIT_BUILDING)) ? true : false; }
inline const bool IsDroid(int Weapon) { return (!IsWeapon(Weapon) && (Weapon & BIT_DROID)) ? true : false; }
inline const bool IsStaticWeapon(int Weapon) { return (IsWeapon(Weapon) && (Weapon & BIT_STATICWEAPON)) ? true : false; }
inline bool const IsModularWeapon(int Weapon) { return (IsWeapon(Weapon) && !(Weapon & BIT_STATICWEAPON)) ? true : false; }

inline const int GetPart(int Weapon, int Group){ return (Weapon & (15<<(4+Group*4)))>>(4+Group*4); }
inline const int GetModularWeapon(int Part1, int Part2){ return (!Part1 && !Part2) ? 0 : (Part2<<8 | Part1<<4 | BIT_WEAPON); }
inline const int GetStaticWeapon(int Weapon){ return (Weapon<<4 | BIT_STATICWEAPON | BIT_WEAPON); }
inline const int GetStaticType(int Weapon){ return IsStaticWeapon(Weapon) ? 255 & (Weapon>>4) : 0; }

inline const int GetChargedWeapon(int Weapon, int Charge){ return (Weapon & (15<<4 | 15<<8 | 15)) | Charge<<12; }
inline const int GetWeaponCharge(int Weapon){ return (Weapon & (15<<12))>>12; }

inline const bool IsOnDeath(int Weapon) { return Weapon & BIT_ONDEATH ? true : false; }

inline const int GetBuildingType(int Weapon) { return IsBuilding(Weapon) ? (Weapon & (1023<<6))>>6 : 0; }
inline const int GetBuildingWeapon(int Building) { return BIT_BUILDING | Building<<6; }
inline const int GetDroidType(int Weapon) { return IsDroid(Weapon) ? (Weapon & (1023<<6))>>6 : 0; }
inline const int GetDroidWeapon(int Droid, bool OnDeath = false) { return BIT_DROID | (OnDeath ? BIT_ONDEATH : 0) | Droid<<6; }

const bool ValidForTurret(int Weapon);

const int GetShotSpread(int Weapon);
const float GetProjectileSpread(int Weapon);
const float GetExplosionSize(int Weapon);
const float GetExplosionDamage(int Weapon);
const float GetProjectileSize(int Weapon);
const float GetProjectileSpeed(int Weapon);
const float GetProjectileCurvature(int Weapon);
const float GetProjectileDamage(int Weapon);
const float GetProjectileKnockback(int Weapon);
const float GetProjectileLife(int Weapon);
const float GetProjectileSprite(int Weapon);
const float GetMeleeHitRadius(int Weapon);
const vec2 GetWeaponColorswap(int Weapon);
const float GetWeaponRenderRecoil(int Weapon);

const int GetProjectileTraceType(int Weapon);
const float GetWeaponTraceThreshold(int Weapon);

const int GetWeaponRenderType(int Weapon);
const int GetWeaponFiringType(int Weapon);
const int GetExplosionSprite(int Weapon);
const int GetExplosionSound(int Weapon);

const bool WeaponAimline(int Weapon);

const bool AIWeaponCharge(int Weapon);

const int GetWeaponCost(int Weapon);

const vec2 GetWeaponRenderOffset(int Weapon);
const vec2 GetMuzzleRenderOffset(int Weapon);
const vec2 GetProjectileOffset(int Weapon);
const vec2 GetHandOffset(int Weapon);

const float GetWeaponFireRate(int Weapon);
const float GetWeaponKnockback(int Weapon);
const bool GetWeaponFullAuto(int Weapon);
const int IsProjectileBouncy(int Weapon);
const bool IsExplosiveProjectile(int Weapon);
const int GetWeaponMaxAmmo(int Weapon);
const bool WeaponUseAmmo(int Weapon);
const bool IsFlammableProjectile(int Weapon);
const float WeaponFlameAmount(int Weapon);
const float WeaponElectroAmount(int Weapon);

const float ScreenshakeDistance(int Weapon);
const float ScreenshakeAmount(int Weapon);

const float WeaponThrowForce(int Weapon);

const int WeaponProjectilePosType(int Weapon);
const bool WeaponAutoPick(int Weapon);

const int WeaponBurstCount(int Weapon);
const float WeaponBurstReload(int Weapon);

const ivec2 GetWeaponVisualSize(int Weapon);
const ivec2 GetWeaponVisualSize2(int Weapon);

const int WeaponMaxLevel(int Weapon);

const bool IsLaserWeapon(int Weapon);
const int GetLaserRange(int Weapon);
const int GetLaserCharge(int Weapon);

const int AIAttackRange(int Weapon);

const float GetWeaponLevelCharge(int Weapon);

const int GetRandomWeaponType(bool IsSurvival);
const int GetMuzzleType(int Weapon);
const int GetMuzzleAmount(int Weapon);

const int GetWeaponFireSound(int Weapon);
const int GetWeaponFireSound2(int Weapon);

#endif
