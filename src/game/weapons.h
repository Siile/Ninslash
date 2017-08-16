#ifndef GAME_WEAPONS_H
#define GAME_WEAPONS_H

#include <cstring>
#include <game/generated/protocol.h>


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
	    str_copy(m_Name, Name, sizeof(m_Name));
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
	WEAPON_NONE = -1,
	W_NONE = -1,
	W_TOOL = 0,
	W_HAMMER,
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
	270, // W_HAMMER,
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
	0, // W_HAMMER,
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
		20, // damage
		true, // autofire
		0, // bullet spread
		1, // shot spread
		0, // powerup size
		0, // max ammo
		0, // bullet life
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



#endif
