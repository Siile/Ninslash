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


	CWeapon(const char *Name,
			int Sound,
			int Cost,
			int Damage,
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
		m_Sound = Sound;
		m_Cost = Cost;
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

const int TurretAttackRange[NUM_CUSTOMWEAPONS] =
{
	0, // W_TOOL,
	0, // W_HAMMER,
	180, // W_CHAINSAW,
	750, // W_PISTOL,
	440, // W_SHOTGUN,
	650, // W_GRENADELAUNCHER,
	780, // W_RIFLE,
	780, // W_ELECTRIC,
	740, // W_LASER,
	600, // W_FLAMER,
};




const CWeapon aCustomWeapon[NUM_CUSTOMWEAPONS] =
{	
	CWeapon(
		"Tool",
		SOUND_TOOL_FIRE,
		0, // cost
		5, // damage
		true, // autofire
		0, // bullet spread
		1, // shot spread
		0, // powerup size
		0, // max ammo
		0, // bullet life
		220, // bullet reload time
		0.0f, // knockback
		0.0f // self knockback
		),
	CWeapon(
		"Hammer",
		SOUND_HAMMER_FIRE,
		0, // cost
		20, // damage
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
		SOUND_CHAINSAW_FIRE,
		0, // cost
		12, // damage
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
		SOUND_GUN_FIRE,
		0, // cost
		11, // damage
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
		SOUND_SHOTGUN_FIRE,
		75, // cost
		7, // damage
		true, // autofire
		0.06f, // bullet spread
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
		SOUND_GRENADE_FIRE,
		100, // cost
		12, // damage
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
		SOUND_RIFLE_FIRE,
		100, // cost
		13, // damage
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
		SOUND_ELECTRO_FIRE,
		100, // cost
		10, // damage
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
		SOUND_LASER_FIRE,
		90, // cost
		35, // damage
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
		SOUND_GRENADE_FIRE,
		90, // cost
		6, // damage
		true, // autofire
		0.1f, // bullet spread
		3, // shot spread
		4, // powerup size
		10, // max ammo
		0.7f, // bullet life
		600, // bullet reload time
		0.0f, // bullet knockback
		2.0f // self knockback
		),
};



#endif
