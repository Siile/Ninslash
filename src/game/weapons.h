#ifndef GAME_WEAPONS_H
#define GAME_WEAPONS_H

#include <cstring>
#include <generated/protocol.h>
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

// Flags
enum BitFlags {
    FLAG_NONE       = 0,
    FLAG_WEAPON     = 1 << 0,
    FLAG_TURRET     = 1 << 1,
    FLAG_BUILDING   = 1 << 2,
    FLAG_STATIC     = 1 << 3,
    FLAG_DROID      = 1 << 3,
    FLAG_ONDEATH    = 1 << 4
};

// Special
enum WeaponIDs {
    WEAPON_NONE = 0,
    WEAPON_ACID = 1 << 3
};

enum StaticWeaponType {
    SW_TOOL,
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
    SW_SYRINGE,
    SW_CLUSTER,
    SW_SHURIKEN,
    SW_CLAW,
    SW_BOMB,
    SW_BALL,
    NUM_STATIC_WEAPONS
};

enum WeaponRenderType {
    WRT_NONE,
    WRT_WEAPON1,
    WRT_WEAPON2,
    WRT_ITEM1,
    WRT_MELEE,
    WRT_MELEESMALL,
    WRT_SPIN
};

enum WeaponFiringType {
    WFT_NONE,
    WFT_MELEE,
    WFT_PROJECTILE,
    WFT_CHARGE,
    WFT_HOLD,
    WFT_THROW,
    WFT_ACTIVATE
};

enum WeaponPartGroup {
    PART_GROUP1 = 0,
    PART_GROUP2 = 1,
    MAX_WEAPON_PARTS = 2
};

enum WeaponPart1 {
    PART1_BASE1 = 1,
    PART1_BASE2,
    PART1_BASE3,
    PART1_BASE4,
    PART1_MELEE = 5,
    PART1_SPIN = 6
};

enum WeaponPart2 {
    PART2_BARREL1 = 1,
    PART2_BARREL2,
    PART2_BARREL3,
    PART2_BARREL4,
    PART2_CHARGE = 5,
    PART2_MELEE1 = 6,
    PART2_MELEE2,
    PART2_MELEE3,
    PART2_MELEE4
};

#define WEAPON_GAME_SIZE 15

inline bool IsWeapon(int Weapon) { return (Weapon & FLAG_WEAPON) != 0; }
inline bool IsTurret(int Weapon) { return IsWeapon(Weapon) && (Weapon & FLAG_TURRET); }
inline bool IsBuilding(int Weapon) { return !IsWeapon(Weapon) && (Weapon & FLAG_BUILDING); }
inline bool IsDroid(int Weapon) { return !IsWeapon(Weapon) && (Weapon & FLAG_DROID); }
inline bool IsStaticWeapon(int Weapon) { return IsWeapon(Weapon) && (Weapon & FLAG_STATIC); }
inline bool IsModularWeapon(int Weapon) { return IsWeapon(Weapon) && !(Weapon & FLAG_STATIC); }

inline int GetPart(int Weapon, int group) { return (Weapon & (15 << (4 + group * 4))) >> (4 + group * 4); }
inline int GetModularWeapon(int part1, int part2) { return (!part1 && !part2) ? 0 : (part2 << 8 | part1 << 4 | FLAG_WEAPON); }
inline int GetStaticWeapon(StaticWeaponType type) { return (static_cast<int>(type) << 4 | FLAG_STATIC | FLAG_WEAPON); }
inline StaticWeaponType GetStaticType(int Weapon) { return IsStaticWeapon(Weapon) ? static_cast<StaticWeaponType>(255 & (Weapon >> 4)) : SW_TOOL; }

inline int GetChargedWeapon(int Weapon, int charge) { return (Weapon & (15 << 4 | 15 << 8 | 15)) | (charge << 12); }
inline int GetWeaponCharge(int Weapon) { return (Weapon & (15 << 12)) >> 12; }
inline bool IsOnDeath(int Weapon) { return Weapon & FLAG_ONDEATH; }

inline const int GetBuildingType(int Weapon) { return IsBuilding(Weapon) ? (Weapon & (1023<<6))>>6 : 0; }
inline const int GetBuildingWeapon(int Building) { return FLAG_BUILDING | Building<<6; }
inline const int GetDroidType(int Weapon) { return IsDroid(Weapon) ? (Weapon & (1023<<6))>>6 : 0; }
inline const int GetDroidWeapon(int Droid, bool OnDeath = false) { return FLAG_DROID | (OnDeath ? FLAG_ONDEATH : 0) | Droid<<6; }

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
