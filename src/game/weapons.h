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

/*
static const char *aPlayerItemName[NUM_PLAYERITEMS] =
{
	"Heal",
	"Rage",
	"Invisibility",
	"Landmine",
	"Electromine",
	"Shield"
};
*/


struct CWeaponStat
{
	char m_Name[64];
	int m_Sprite;
	int m_Sound;
	int m_Cost;
	int m_Damage;
	bool m_FullAuto;
	float m_BulletSpread;
	int m_ShotSpread;
	int m_PowerupSize;
	int m_MaxAmmo;
	float m_BulletLife;
	int m_BulletReloadTime;
	float m_Knockback;
	float m_SelfKnockback;
	int m_AiAttackRange;


	CWeaponStat(const char *Name,
			int Sound,
			int Damage,
			bool FullAuto,
			float BulletSpread,
			int ShotSpread,
			int PowerupSize,
			int MaxAmmo,
			float BulletLife,
			int BulletReloadTime,
			float Knockback,
			float SelfKnockback,
			float AiAttackRange)
	{
	    //str_copy(m_Name, Name, sizeof(m_Name));
		m_Sound = Sound;
		m_Damage = Damage;
		m_FullAuto = FullAuto;
		m_BulletSpread = BulletSpread;
		m_ShotSpread = ShotSpread;
		m_PowerupSize = PowerupSize;
		m_MaxAmmo = MaxAmmo;
		m_BulletLife = BulletLife;
		m_BulletReloadTime = BulletReloadTime;
		m_Knockback = Knockback;
		m_SelfKnockback = SelfKnockback;
		m_AiAttackRange = AiAttackRange;
	}
};

enum CustomWeapons
{
	W_NONE = 0,
	W_TOOL = 0,
	W_SWORD,
	W_SHOTGUN,
	W_RIFLE,
	W_LASER,
	W_ELECTRIC,
	W_GRENADELAUNCHER,
	W_FLAMER,
	W_CHAINSAW,
	W_SCYTHE,
	W_DROID_WALKER,
	W_DROID_STAR,
	W_ROBOT1,
	NUM_CUSTOMWEAPONS
};



/*
const int BotAttackRange[NUM_CUSTOMWEAPONS] =
{
	120, // W_TOOL,
	270, // W_SWORD,
	440, // W_SHOTGUN,
	780, // W_RIFLE,
	740, // W_LASER,
	780, // W_ELECTRIC,
	650, // W_GRENADELAUNCHER,
	600, // W_FLAMER,
	180, // W_CHAINSAW,
};

const int TurretAttackRange[NUM_CUSTOMWEAPONS] =
{
	0, // W_TOOL,
	0, // W_SWORD,
	180, // W_CHAINSAW,
	//750, // W_PISTOL,
	440, // W_SHOTGUN,
	650, // W_GRENADELAUNCHER,
	780, // W_RIFLE,
	780, // W_ELECTRIC,
	740, // W_LASER,
	600, // W_FLAMER,
};
*/




const CWeaponStat aCustomWeapon[NUM_CUSTOMWEAPONS] =
{	
	CWeaponStat(
		"Tool",
		SOUND_TOOL_FIRE,
		5, // damage
		true, // autofire
		0, // bullet spread
		1, // shot spread
		0, // powerup size
		0, // max ammo
		0, // bullet life
		220, // bullet reload time
		0.0f, // knockback
		0.0f, // self knockback
		0 // ai attack range
		),
	CWeaponStat(
		"Hammer",
		SOUND_HAMMER_FIRE,
		25, // damage
		true, // autofire
		0, // bullet spread
		1, // shot spread
		0, // powerup size
		0, // max ammo
		0.30f, // bullet life
		350, // bullet reload time
		1.0f, // knockback
		0.0f, // self knockback
		270 // ai attack range
		),
	CWeaponStat(
		"Shotgun",
		SOUND_SHOTGUN_FIRE,
		7, // damage
		true, // autofire
		0.06f, // bullet spread
		6, // shot spread
		5, // powerup size
		14, // max ammo
		0.17f, // bullet life
		400, // bullet reload time
		3.0f, // bullet knockback
		4.0f, // self knockback
		440 // ai attack range
		),
		/*
	CWeaponStat(
		"Assault rifle",
		SOUND_RIFLE_FIRE,
		13, // damage
		true, // autofire
		0.08f, // bullet spread
		1, // shot spread
		10, // powerup size
		30, // max ammo
		0.25f, // bullet life
		120, // bullet reload time
		6.5f, // bullet knockback
		1.5f, // self knockback
		780 // ai attack range
		),
		*/
	CWeaponStat(
		"Robot rifle",
		SOUND_WALKER_FIRE,
		6, // damage
		true, // autofire
		0.06f, // bullet spread
		1, // shot spread
		10, // powerup size
		0, // max ammo
		0.35f, // bullet life
		120, // bullet reload time
		1.5f, // bullet knockback
		0.5f, // self knockback
		780 // ai attack range
		),
	CWeaponStat(
		"Laser rifle",
		SOUND_LASER_FIRE,
		40, // damage
		true, // autofire
		0, // bullet spread
		1, // shot spread
		5, // powerup size
		14, // max ammo
		0, // bullet life
		550, // bullet reload time
		0.0f, // bullet knockback
		0.0f, // self knockback
		740 // ai attack range
		),
	CWeaponStat(
		"Electro launcher",
		SOUND_ELECTRO_FIRE,
		10, // damage
		true, // autofire
		0, // bullet spread
		1, // shot spread
		7, // powerup size
		20, // max ammo
		0.3f, // bullet life
		260, // bullet reload time
		12.5f, // bullet knockback
		2.5f, // self knockback
		780 // ai attack range
		),
	CWeaponStat(
		"Grenade launcher",
		SOUND_GRENADE_FIRE,
		12, // damage
		true, // autofire
		0, // bullet spread
		1, // shot spread
		4, // powerup size
		10, // max ammo
		1.4f, // bullet life
		600, // bullet reload time
		0.0f, // bullet knockback
		3.0f, // self knockback
		650 // ai attack range
		),
	CWeaponStat(
		"Flame thrower",
		SOUND_JETPACK1,
		3, // damage
		true, // autofire
		0, // bullet spread
		1, // shot spread
		8, // powerup size
		24, // max ammo
		0.0f, // bullet life
		200, // bullet reload time
		0.0f, // bullet knockback
		0.0f, // self knockback
		400 // ai attack range
		),
	CWeaponStat(
		"Chainsaw",
		SOUND_CHAINSAW_FIRE,
		12, // damage
		true, // autofire
		0, // bullet spread
		1, // shot spread
		4, // powerup size
		10, // max ammo
		0, // bullet life
		500, // bullet reload time
		0.2f, // knockback
		0.0f, // self knockback
		180 // ai attack range
		),
	CWeaponStat(
		"Scythe",
		-1,
		3, // damage
		true, // autofire
		0, // bullet spread
		1, // shot spread
		0, // powerup size
		0, // max ammo
		0, // bullet life
		50, // bullet reload time
		0.2f, // knockback
		0.0f, // self knockback
		750 // ai attack range
		),
	CWeaponStat(
		"Walker weapon",
		SOUND_WALKER_FIRE,
		3, // damage
		true, // autofire
		0.06f, // bullet spread
		1, // shot spread
		10, // powerup size
		0, // max ammo
		0.35f, // bullet life
		120, // bullet reload time
		3.5f, // bullet knockback
		1.5f, // self knockback
		780 // ai attack range
		),
	CWeaponStat(
		"Star Droid weapon",
		SOUND_STAR_FIRE,
		0, // damage
		true, // autofire
		0.3f, // bullet spread
		1, // shot spread
		10, // powerup size
		0, // max ammo
		0.8f, // bullet life
		400, // bullet reload time
		3.5f, // bullet knockback
		1.5f, // self knockback
		500 // ai attack range
		),
	CWeaponStat(
		"Robot1",
		SOUND_WALKER_FIRE,
		3, // damage
		true, // autofire
		0.06f, // bullet spread
		1, // shot spread
		10, // powerup size
		0, // max ammo
		0.35f, // bullet life
		120, // bullet reload time
		3.5f, // bullet knockback
		1.5f, // self knockback
		700 // ai attack range
		),
};



enum WeaponBits
{
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
	SW_BAZOOKA,
	SW_BOUNCER,
	SW_CHAINSAW,
	SW_FLAMER,
	SW_BUBBLER,
	SW_SHURIKEN,
	NUM_SW,
	
	
	// weapon rendering types
	WRT_NONE=0,
	WRT_WEAPON1, // rifles, shotguns etc.
	WRT_WEAPON2, // gun
	WRT_ITEM1,
	WRT_MELEE,
	
	// weapon firing types
	WFT_NONE=0,
	WFT_MELEE, // swords
	WFT_PROJECTILE, // rifles, shotguns etc.
	WFT_CHARGE, // gun 2 & charge weapons
	WFT_HOLD, // chainsaw & flamer
	WFT_THROW // grenades
};



#define WEAPON_GAME_SIZE 15

// todo: move somewhere else and remove this file
inline int GetPart(int Weapon, int Group){ return (Weapon & (15<<(4+Group*4)))>>(4+Group*4); }
inline int GetModularWeapon(int Part1, int Part2){ return (!Part1 && !Part2) ? 0 : (Part2<<8 | Part1<<4 | BIT_WEAPON); }
inline int GetStaticWeapon(int Weapon){ return (Weapon<<4 | BIT_STATICWEAPON | BIT_WEAPON); }
inline int GetStaticType(int Weapon){ return (Weapon>>4); }

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

float WeaponThrowForce(int Weapon);

int WeaponProjectilePosType(int Weapon);

int WeaponBurstCount(int Weapon);
float WeaponBurstReload(int Weapon);

ivec2 GetWeaponVisualSize(int Weapon);
ivec2 GetWeaponVisualSize2(int Weapon);

bool IsLaserWeapon(int Weapon);
int GetLaserRange(int Weapon);
int GetLaserCharge(int Weapon);

int AIAttackRange(int Weapon);

int GetRandomWeaponType();
int GetMuzzleType(int Weapon);
int GetMuzzleAmount(int Weapon);

int GetWeaponFireSound(int Weapon);
int GetWeaponFireSound2(int Weapon);

#endif
