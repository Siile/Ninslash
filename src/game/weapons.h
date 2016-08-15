#ifndef GAME_WEAPONS_H
#define GAME_WEAPONS_H

#include <cstring>
#include <game/generated/protocol.h>


enum PlayerItems
{
	PLAYERITEM_HEAL,
	PLAYERITEM_RAGE,
	PLAYERITEM_INVISIBILITY,
	PLAYERITEM_LANDMINE,
	PLAYERITEM_ELECTROMINE,
	PLAYERITEM_SHIELD,
	PLAYERITEM_FUEL,
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


struct CWeapon
{
	char m_Name[64];
	int m_Sprite;
	int m_ParentWeapon;
	int m_ProjectileType;
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
	int m_Extra1;


	CWeapon(const char *Name,
			//int Sprite,
			int ParentWeapon,
			int ProjectileType,
			int Sound,
			int Cost,
			int Damage,
			int Extra1,
			bool FullAuto,
			float BulletSpread,
			int ShotSpread,
			int PowerupSize,
			int MaxAmmo,
			float BulletLife,
			int BulletReloadTime,
			float Knockback,
			float SelfKnockback)
	{
	    str_copy(m_Name, Name, sizeof(m_Name));
		//m_Sprite = Sprite;
		m_ParentWeapon = ParentWeapon;
		m_ProjectileType = ProjectileType;
		m_Sound = Sound;
		m_Cost = Cost;
		m_Damage = Damage;
		m_Extra1 = Extra1;
		m_FullAuto = FullAuto;
		m_BulletSpread = BulletSpread;
		m_ShotSpread = ShotSpread;
		m_PowerupSize = PowerupSize;
		m_MaxAmmo = MaxAmmo;
		m_BulletLife = BulletLife;
		m_BulletReloadTime = BulletReloadTime;
		m_Knockback = Knockback;
		m_SelfKnockback = SelfKnockback;
	}
};

enum CustomWeapons
{
	W_TOOL,
	W_HAMMER,
	W_CHAINSAW,
	W_PISTOL,
	W_SHOTGUN,
	W_GRENADELAUNCHER,
	W_RIFLE,
	W_ELECTRIC,
	W_LASER,
	W_FLAMER,
	NUM_CUSTOMWEAPONS
};




const int BotAttackRange[NUM_CUSTOMWEAPONS] =
{
	120, // W_TOOL,
	270, // W_HAMMER,
	180, // W_CHAINSAW,
	750, // W_PISTOL,
	440, // W_SHOTGUN,
	650, // W_GRENADELAUNCHER,
	780, // W_RIFLE,
	780, // W_ELECTRIC,
	740, // W_LASER,
	600, // W_FLAMER,
};


enum ProjectileTypes
{
	PROJTYPE_NONE,
	PROJTYPE_HAMMER,
	PROJTYPE_CHAINSAW,
	PROJTYPE_FLYHAMMER,
	PROJTYPE_SWORD,
	PROJTYPE_BULLET,
	PROJTYPE_PELLET,
	PROJTYPE_LASER,
	PROJTYPE_FLAME,
	PROJTYPE_LIGHTNING,
	PROJTYPE_ELECTRO,
	PROJTYPE_ELECTRIC,
	PROJTYPE_GRENADE,
	PROJTYPE_MINE
};

enum WeaponExtraFeature
{
	NO_EXTRA1,
	EXPLOSIVE,
	MEGAROCKETS,
	DOOMROCKETS,
	SLEEPEFFECT,
	ELECTRIC,
	SMOKE,
	NUM_EXTRA_FEATURES
};


const CWeapon aCustomWeapon[NUM_CUSTOMWEAPONS] =
{	
	CWeapon(
		"Tool",
		WEAPON_TOOL,
		PROJTYPE_HAMMER,
		SOUND_TOOL_FIRE,
		0, // cost
		5, // damage
		0, // extra1
		true, // autofire
		0, // bullet spread
		1, // shot spread
		0, // powerup size
		0, // max ammo
		0, // bullet life
		180, // bullet reload time
		0.7f, // knockback
		0.0f // self knockback
		),
	CWeapon(
		"Hammer",
		WEAPON_HAMMER,
		PROJTYPE_HAMMER,
		SOUND_HAMMER_FIRE,
		0, // cost
		20, // damage
		0, // extra1
		true, // autofire
		0, // bullet spread
		1, // shot spread
		0, // powerup size
		0, // max ammo
		0, // bullet life
		250, // bullet reload time
		1.0f, // knockback
		0.0f // self knockback
		),
	CWeapon(
		"Chainsaw",
		WEAPON_CHAINSAW,
		PROJTYPE_CHAINSAW,
		SOUND_CHAINSAW_FIRE,
		0, // cost
		12, // damage
		0, // extra1
		true, // autofire
		0, // bullet spread
		1, // shot spread
		4, // powerup size
		10, // max ammo
		0, // bullet life
		500, // bullet reload time
		0.2f, // knockback
		0.0f // self knockback
		),
	CWeapon(
		"Pistol",
		WEAPON_GUN,
		PROJTYPE_BULLET,
		SOUND_GUN_FIRE,
		0, // cost
		11, // damage
		0, // extra1
		true, // autofire
		0.04f, // bullet spread
		1, // shot spread
		7, // powerup size
		14, // max ammo
		0.2f, // bullet life
		180, // bullet reload time
		5.0f, // bullet knockback
		0.5f // self knockback
		),
	CWeapon(
		"Shotgun",
		WEAPON_SHOTGUN,
		PROJTYPE_PELLET,
		SOUND_SHOTGUN_FIRE,
		75, // cost
		6, // damage
		0, // extra1
		true, // autofire
		0.04f, // bullet spread
		6, // shot spread
		4, // powerup size
		10, // max ammo
		0.17f, // bullet life
		400, // bullet reload time
		3.0f, // bullet knockback
		4.0f // self knockback
		),
	CWeapon(
		"Grenade launcher",
		WEAPON_GRENADE,
		PROJTYPE_GRENADE,
		SOUND_GRENADE_FIRE,
		100, // cost
		12, // damage
		0, // extra1
		true, // autofire
		0, // bullet spread
		1, // shot spread
		4, // powerup size
		10, // max ammo
		1.4f, // bullet life
		600, // bullet reload time
		0.0f, // bullet knockback
		3.0f // self knockback
		),
	CWeapon(
		"Assault rifle",
		WEAPON_RIFLE,
		PROJTYPE_BULLET,
		SOUND_RIFLE_FIRE,
		100, // cost
		13, // damage
		0, // extra1
		true, // autofire
		0.08f, // bullet spread
		1, // shot spread
		10, // powerup size
		30, // max ammo
		0.25f, // bullet life
		120, // bullet reload time
		6.5f, // bullet knockback
		1.5f // self knockback
		),
	CWeapon(
		"Electro launcher",
		WEAPON_ELECTRIC,
		PROJTYPE_ELECTRIC,
		SOUND_ELECTRO_FIRE,
		100, // cost
		10, // damage
		0, // extra1
		true, // autofire
		0, // bullet spread
		1, // shot spread
		7, // powerup size
		20, // max ammo
		0.4f, // bullet life
		220, // bullet reload time
		12.5f, // bullet knockback
		1.5f // self knockback
		),
	CWeapon(
		"Laser rifle",
		WEAPON_LASER,
		PROJTYPE_LASER,
		SOUND_LASER_FIRE,
		90, // cost
		25, // damage
		0, // extra1
		true, // autofire
		0, // bullet spread
		1, // shot spread
		5, // powerup size
		14, // max ammo
		0, // bullet life
		430, // bullet reload time
		0.0f, // bullet knockback
		0.0f // self knockback
		),
	CWeapon(
		"Flame thrower",
		WEAPON_FLAMER,
		PROJTYPE_FLAME,
		SOUND_GRENADE_FIRE,
		90, // cost
		6, // damage
		0, // extra1
		true, // autofire
		0, // bullet spread
		1, // shot spread
		4, // powerup size
		10, // max ammo
		0.7f, // bullet life
		600, // bullet reload time
		0.0f, // bullet knockback
		2.0f // self knockback
		),
};



#endif