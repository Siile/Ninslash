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
	SW_INVIS,
	SW_BUBBLER,
	SW_SHURIKEN,
	NUM_SW,
	
	
	// weapon rendering types
	WRT_NONE=0,
	WRT_WEAPON1, // rifles, shotguns etc.
	WRT_WEAPON2, // gun
	WRT_ITEM1,
	WRT_MELEE,
	WRT_MELEESMALL,
	
	// weapon firing types
	WFT_NONE=0,
	WFT_MELEE, // swords
	WFT_PROJECTILE, // rifles, shotguns etc.
	WFT_CHARGE, // gun 2 & charge weapons
	WFT_HOLD, // chainsaw & flamer
	WFT_THROW, // grenades
	WFT_ACTIVATE // some items
};

 

#define WEAPON_GAME_SIZE 15


inline int GetPart(int Weapon, int Group){ return (Weapon & (15<<(4+Group*4)))>>(4+Group*4); }
inline int GetModularWeapon(int Part1, int Part2){ return (!Part1 && !Part2) ? 0 : (Part2<<8 | Part1<<4 | BIT_WEAPON); }
inline int GetStaticWeapon(int Weapon){ return (Weapon<<4 | BIT_STATICWEAPON | BIT_WEAPON); }
inline int GetStaticType(int Weapon){ return 255 & (Weapon>>4); }

inline int GetChargedWeapon(int Weapon, int Charge){ return (Weapon & (15<<4 | 15<<8 | 15)) | Charge<<12; }
inline int GetWeaponCharge(int Weapon){ return (Weapon & (15<<12))>>12; }

inline bool IsWeapon(int Weapon) { return (Weapon & BIT_WEAPON) ? true : false; }
inline bool IsTurret(int Weapon) { return (IsWeapon(Weapon) && (Weapon & BIT_TURRET)) ? true : false; }
inline bool IsBuilding(int Weapon) { return (!IsWeapon(Weapon) && (Weapon & BIT_BUILDING)) ? true : false; }
inline bool IsDroid(int Weapon) { return (!IsWeapon(Weapon) && (Weapon & BIT_DROID)) ? true : false; }
inline bool IsStaticWeapon(int Weapon) { return (IsWeapon(Weapon) && (Weapon & BIT_STATICWEAPON)) ? true : false; }
inline bool IsModularWeapon(int Weapon) { return (IsWeapon(Weapon) && !(Weapon & BIT_STATICWEAPON)) ? true : false; }

inline bool IsOnDeath(int Weapon) { return Weapon & BIT_ONDEATH ? true : false; }

inline int GetBuildingType(int Weapon) { return IsBuilding(Weapon) ? (Weapon & (1023<<6))>>6 : false; }
inline int GetBuildingWeapon(int Building) { return BIT_BUILDING | Building<<6; }
inline int GetDroidType(int Weapon) { return IsDroid(Weapon) ? (Weapon & (1023<<6))>>6 : false; }
inline int GetDroidWeapon(int Droid, bool OnDeath = false) { return BIT_DROID | (OnDeath ? BIT_ONDEATH : 0) | Droid<<6; }

bool ValidForTurret(int Weapon);

int GetShotSpread(int Weapon);
float GetProjectileSpread(int Weapon);
float GetExplosionSize(int Weapon);
float GetExplosionDamage(int Weapon);
float GetProjectileSize(int Weapon);
float GetProjectileSpeed(int Weapon);
float GetProjectileCurvature(int Weapon);
float GetProjectileDamage(int Weapon);
float GetProjectileKnockback(int Weapon);
float GetProjectileLife(int Weapon);
float GetProjectileSprite(int Weapon);
float GetMeleeHitRadius(int Weapon);
vec2 GetWeaponColorswap(int Weapon);
float GetWeaponRenderRecoil(int Weapon);

int GetProjectileTraceType(int Weapon);
float GetWeaponTraceThreshold(int Weapon);

int GetWeaponRenderType(int Weapon);
int GetWeaponFiringType(int Weapon);
int GetExplosionSprite(int Weapon);
int GetExplosionSound(int Weapon);

bool WeaponAimline(int Weapon);

bool AIWeaponCharge(int Weapon);

vec2 GetWeaponRenderOffset(int Weapon);
vec2 GetMuzzleRenderOffset(int Weapon);
vec2 GetProjectileOffset(int Weapon);

float GetWeaponFireRate(int Weapon);
float GetWeaponKnockback(int Weapon);
bool GetWeaponFullAuto(int Weapon);
bool IsProjectileBouncy(int Weapon);
bool IsExplosiveProjectile(int Weapon);
int GetWeaponMaxAmmo(int Weapon);
bool WeaponUseAmmo(int Weapon);
bool IsFlammableProjectile(int Weapon);
float WeaponFlameAmount(int Weapon);
float WeaponElectroAmount(int Weapon);

float ScreenshakeDistance(int Weapon);
float ScreenshakeAmount(int Weapon);

float WeaponThrowForce(int Weapon);

int WeaponProjectilePosType(int Weapon);
bool WeaponAutoPick(int Weapon);

int WeaponBurstCount(int Weapon);
float WeaponBurstReload(int Weapon);

ivec2 GetWeaponVisualSize(int Weapon);
ivec2 GetWeaponVisualSize2(int Weapon);

int WeaponMaxLevel(int Weapon);

bool IsLaserWeapon(int Weapon);
int GetLaserRange(int Weapon);
int GetLaserCharge(int Weapon);

int AIAttackRange(int Weapon);

float GetWeaponLevelCharge(int Weapon);

int GetRandomWeaponType(bool IsSurvival);
int GetMuzzleType(int Weapon);
int GetMuzzleAmount(int Weapon);

int GetWeaponFireSound(int Weapon);
int GetWeaponFireSound2(int Weapon);

#endif
