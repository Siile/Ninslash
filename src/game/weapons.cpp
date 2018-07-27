#include <engine/shared/config.h>
#include <engine/shared/protocol.h>
#include <game/generated/client_data.h>

#include "weapons.h"


const bool ValidForTurret(int Weapon)
{
	if (IsModularWeapon(Weapon) && GetWeaponFiringType(Weapon) == WFT_PROJECTILE)
		return true;
	
	if (IsStaticWeapon(Weapon) && (GetStaticType(Weapon) == SW_BAZOOKA || GetStaticType(Weapon) == SW_BOUNCER))
		return true;
		
	return false;
}

const int GetWeaponCost(int Weapon)
{
	const float Charge = GetWeaponLevelCharge(Weapon);
	
	float cost1 = 0;
	float cost2 = 0;
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_TOOL: cost1 = 5; cost2 = 0; break;
			case SW_BUBBLER: cost1 = 10; cost2 = 20; break;
			case SW_BAZOOKA: cost1 = 20; cost2 = 20; break;
			case SW_BOUNCER: cost1 = 10; cost2 = 20; break;
			case SW_UPGRADE: cost1 = 10; cost2 = 0; break;
			case SW_RESPAWNER: cost1 = 10; cost2 = 0; break;
			case SW_INVIS: cost1 = 5; cost2 = 20; break;
			case SW_SHIELD: cost1 = 5; cost2 = 20; break;
			case SW_CHAINSAW: cost1 = 10; cost2 = 20; break;
			case SW_FLAMER: cost1 = 10; cost2 = 20; break;
			case SW_GUN1: cost1 = 2; cost2 = 0; break;
			case SW_GUN2: cost1 = 2; cost2 = 0; break;
			case SW_GRENADE1: cost1 = 5; cost2 = 0; break;
			case SW_GRENADE2: cost1 = 5; cost2 = 0; break;
			case SW_GRENADE3: cost1 = 5; cost2 = 0; break;
			default: cost1 = 10; cost2 = 20; break;
		};
	}
	else
	{
		cost1 = 10;
		cost2 = 20;
	}
	
	return cost1 + cost2*Charge*(Charge*0.25f+0.75f);
}

const float GetProjectileSprite(int Weapon)
{
	if (IsDroid(Weapon))
	{
		switch (GetDroidType(Weapon))
		{
			case DROIDTYPE_WALKER: return 7;
			case DROIDTYPE_STAR: return 4;
			default: return 0;
		};
	}
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_BUBBLER: return 1;
			case SW_BAZOOKA: return 12;
			case SW_BOUNCER: return 13;
			default: return 0;
		};
	}
	
	
	const int Part1 = GetPart(Weapon, 0);
	const int Part2 = GetPart(Weapon, 1);
	
	if (Part1 == 1 && Part2 == 1) return 1;
	if (Part1 == 1 && Part2 == 2) return 0;
	if (Part1 == 1 && Part2 == 3) return 3;
	if (Part1 == 1 && Part2 == 4) return 3;
	if (Part1 == 1 && Part2 == 5) return 1;
	
	if (Part1 == 2) return 2;
	
	if (Part1 == 3 && Part2 == 1) return 4;
	if (Part1 == 3 && Part2 == 2) return 5;
	if (Part1 == 3 && Part2 == 3) return 6;
	if (Part1 == 3 && Part2 == 4) return 7;
	if (Part1 == 3 && Part2 == 5) return 5;
	
	if (Part1 == 4 && Part2 == 1) return 10;
	if (Part1 == 4 && Part2 == 2) return 9;
	if (Part1 == 4 && Part2 == 3) return 8;
	if (Part1 == 4 && Part2 == 4) return 8;
	if (Part1 == 4 && Part2 == 5) return 8;
	
	return 0;
}

const int GetProjectileTraceType(int Weapon)
{

	if (IsDroid(Weapon))
	{
		switch (GetDroidType(Weapon))
		{
			case DROIDTYPE_WALKER: return -3;
			case DROIDTYPE_STAR: return -3;
			default: return 0;
		};
	}
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_GUN1: return 4;
			case SW_BUBBLER: return -1;
			case SW_SHURIKEN: return -4;
			case SW_BALL: return 8;
			case SW_GRENADE3: return 7;
			case SW_GRENADE2: return 5;
			case SW_GRENADE1: return 4;
			case SW_BAZOOKA: return -1;
			case SW_BOUNCER: return 6;
			default: return 0;
		};
	}
	
	
	const int Part1 = GetPart(Weapon, 0);
	
	if (Part1 == 3) return -3;
	if (Part1 == 2) return -1;
	if (Part1 == 1) return 1;
	if (Part1 == 4) return 2;
	
	return 1;
}

const float GetWeaponTraceThreshold(int Weapon)
{
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_SHURIKEN: return 20.0f;
			case SW_GRENADE3: case SW_GRENADE2: case SW_GRENADE1: return 0.0f;
			default: return 0.0f;
		};
	}
	
	return 0.0f;
}


const bool AIWeaponCharge(int Weapon)
{
	if (GetWeaponFiringType(Weapon) == WFT_THROW || GetWeaponFiringType(Weapon) == WFT_CHARGE)
		return true;
	
	return false;
}

const float GetWeaponLevelCharge(int Weapon)
{
	return GetWeaponCharge(Weapon) / float(max(1, WeaponMaxLevel(Weapon)));
}


const vec2 GetWeaponColorswap(int Weapon)
{
	const float Charge = GetWeaponLevelCharge(Weapon);
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_BOUNCER: return vec2(0.0f+Charge*0.9f, 0.0f+Charge*0.4f);
			case SW_BAZOOKA: return vec2(0.0f+Charge*1.0f, 0.0f+Charge*0.4f);
			case SW_CHAINSAW: return vec2(0.0f+Charge*0.25f, 0.0f+Charge*0.9f);
			case SW_FLAMER: return vec2(0.0f+Charge*0.6f, 0.0f+Charge*0.8f);
			default: return vec2(0.0f, 0.0f);
		};
	}
	
	const int Part1 = GetPart(Weapon, 0);
	
	if (Part1 == 1) return vec2(0.0f+Charge*0.25f, 0.0f+Charge*0.6f);
	if (Part1 == 2) return vec2(0.0f+Charge*0.25f, 0.0f+Charge*0.8f);
	if (Part1 == 3) return vec2(0.9f-Charge*0.2f, 1.0f-Charge*0.6f);
	if (Part1 == 4) return vec2(0.8f-Charge*0.6f, 0.1f-Charge*0.1f);
	if (Part1 == 5) return vec2(0.0f+Charge*0.35f, 0.0f+Charge*0.8f);
	if (Part1 == 6) return vec2(0.0f+Charge*0.35f, 0.0f+Charge*0.8f);
	
	return vec2(0, 0);
}

const float GetProjectileSize(int Weapon)
{
	if (IsDroid(Weapon))
	{
		switch (GetDroidType(Weapon))
		{
			case DROIDTYPE_WALKER: return 1.0f;
			case DROIDTYPE_STAR: return 2.0f;
			default: return 0.0f;
		};
	}
	
	const float Charge = GetWeaponLevelCharge(Weapon);
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_GRENADE1: case SW_GRENADE2: case SW_GRENADE3: return 2.5f;
			case SW_BALL: return 2.0f;
			case SW_SHURIKEN: return 2.5f;
			case SW_BUBBLER: return 0.9f;
			case SW_BAZOOKA: return 1.3f;
			case SW_BOUNCER: return 1.0f - (GetShotSpread(Weapon)-1)*0.15f;
			case SW_GUN1: return 0.7f;
			default: return 1.0f;
		};
	}
	
	const int Part1 = GetPart(Weapon, 0);
	const int Part2 = GetPart(Weapon, 1);
	
	float Size;
	
	switch (Part1)
	{
		case 1: Size = 0.7f; break;
		case 2: Size = 1.2f; break;
		case 3: Size = 1.2f; break;
		case 4: Size = 1.3f; break;
		default: Size = 1.0f; break;
	};
	
	switch (Part2)
	{
		case 1: Size += Charge*0.25f; Size *= 1.2f; break;
		case 2: Size += Charge*0.25f; Size *= 0.65f; break;
		case 3: Size += Charge*0.25f; Size *= 1.3f; break;
		case 4: Size += Charge*0.25f; Size *= 0.8f; break;
		case 5: Size *= 0.8f + GetWeaponCharge(Weapon)*0.25f; break;
		default: break;
	};
	
	return Size;
}

const int GetExplosionSprite(int Weapon)
{
	if (IsBuilding(Weapon))
	{
		switch (GetBuildingType(Weapon))
		{
			case BUILDING_TURRET: return SPRITE_EXPLOSION1_1;
			case BUILDING_TESLACOIL: return SPRITE_EXPLOSION1_1;
			case BUILDING_FLAMETRAP: return SPRITE_EXPLOSION1_1;
			case BUILDING_BARREL: return SPRITE_EXPLOSION1_1;
			case BUILDING_POWERBARREL: return SPRITE_EXPLOSION1_1;
			case BUILDING_REACTOR: return SPRITE_EXPLOSION1_1;
			case BUILDING_GENERATOR: return SPRITE_EXPLOSION1_1;
			default: return SPRITE_EXPLOSION1_1;
		};
	}
	else if (IsDroid(Weapon))
	{
		if (IsOnDeath(Weapon))
		{
			switch (GetDroidType(Weapon))
			{
				case DROIDTYPE_WALKER: return SPRITE_EXPLOSION1_1;
				case DROIDTYPE_STAR: return SPRITE_EXPLOSION1_1;
				case DROIDTYPE_CRAWLER: return SPRITE_EXPLOSION1_1;
				default: return 0;
			};
		}
		
		return 0;
	}
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_GRENADE1: return SPRITE_EXPLOSION1_1;
			case SW_GRENADE2: return SPRITE_EXPLOSION1_1;
			case SW_BAZOOKA: return SPRITE_EXPLOSION1_1;
			case SW_BOMB: return SPRITE_EXPLOSION1_1;
			default: return 0;
		};
	}
	
	if (IsModularWeapon(Weapon))
	{
		return SPRITE_EXPLOSION1_1;
	}
	return 0;
}

const int GetExplosionSound(int Weapon)
{
	if (IsBuilding(Weapon))
	{
		switch (GetBuildingType(Weapon))
		{
			case BUILDING_TURRET: return SOUND_GRENADE_EXPLODE;
			case BUILDING_TESLACOIL: return SOUND_GRENADE_EXPLODE;
			case BUILDING_FLAMETRAP: return SOUND_GRENADE_EXPLODE;
			case BUILDING_BARREL: return SOUND_GRENADE_EXPLODE;
			case BUILDING_POWERBARREL: return SOUND_GRENADE_EXPLODE;
			case BUILDING_REACTOR: return SOUND_GRENADE_EXPLODE;
			case BUILDING_GENERATOR: return SOUND_GRENADE_EXPLODE;
			default: return 0;
		};
	}
	else if (IsDroid(Weapon))
	{
		if (IsOnDeath(Weapon))
		{
			switch (GetDroidType(Weapon))
			{
				case DROIDTYPE_WALKER: return SOUND_GRENADE_EXPLODE;
				case DROIDTYPE_STAR: return SOUND_GRENADE_EXPLODE;
				case DROIDTYPE_CRAWLER: return SOUND_GRENADE_EXPLODE;
				default: return 0;
			};
		}
		
		return 0;
	}
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_BOMB: return SOUND_GRENADE_EXPLODE;
			case SW_GRENADE1: return SOUND_GRENADE_EXPLODE;
			case SW_GRENADE2: return SOUND_GRENADE2_EXPLODE;
			case SW_GRENADE3: return SOUND_BOUNCER_EXPLODE;
			case SW_BAZOOKA: return SOUND_GRENADE_EXPLODE;
			case SW_BOUNCER: return SOUND_BOUNCER_EXPLODE;
			default: return 0;
		};
	}
	
	const int Part1 = GetPart(Weapon, 0);
	
	if (Part1 == 2)
		return SOUND_GRENADE_EXPLODE;
	
	return 0;
}


const int GetWeaponFireSound(int Weapon)
{
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_UPGRADE: return -1;
			case SW_RESPAWNER: return -1;
			case SW_INVIS: return -1;
			case SW_SHIELD: return -1;
			case SW_CHAINSAW: return SOUND_CHAINSAW_FIRE;
			case SW_FLAMER: return SOUND_FLAMER1;
			case SW_BAZOOKA: return SOUND_BAZOOKA_FIRE;
			case SW_BOUNCER: return SOUND_BOUNCER_FIRE;
			case SW_GUN1: return SOUND_GUN_FIRE;
			case SW_GUN2: return SOUND_LASER_FIRE;
			case SW_BOMB: return -1;
			case SW_GRENADE1: return -1;
			case SW_GRENADE2: return -1;
			default: return -1;
		};
	}
	
	const int Part1 = GetPart(Weapon, 0);
	
	if (Part1 == 1) return SOUND_BASE1_FIRE;
	if (Part1 == 2) return SOUND_BASE2_FIRE;
	if (Part1 == 3) return SOUND_BASE3_FIRE;
	if (Part1 == 4) return SOUND_BASE4_FIRE;
	if (Part1 == 5) return SOUND_HAMMER_FIRE;
	if (Part1 == 6) return -1;
	if (Part1 == 7) return -1;
	
	return -1;
}

const int GetWeaponFireSound2(int Weapon)
{
	if (!IsModularWeapon(Weapon))
		return -1;
	
	int Part = GetPart(Weapon, 1);
	
	if (Part == 1) return SOUND_BARREL1_FIRE;
	if (Part == 2) return SOUND_BARREL2_FIRE;
	if (Part == 3) return SOUND_BARREL3_FIRE;
	if (Part == 4) return SOUND_BARREL4_FIRE;
	
	return -1;
}


const float GetExplosionSize(int Weapon)
{
	if (IsBuilding(Weapon))
	{
		switch (GetBuildingType(Weapon))
		{
			case BUILDING_TURRET: return 80.0f;
			case BUILDING_TESLACOIL: return 240.0f;
			case BUILDING_FLAMETRAP: return 150.0f;
			case BUILDING_BARREL: return 200.0f;
			case BUILDING_POWERBARREL: return 300.0f;
			case BUILDING_REACTOR: return 240.0f;
			case BUILDING_GENERATOR: return 150.0f;
			default: return 120.0f;
		};
	}
	else if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_BOMB: return 400.0f;
			case SW_GUN1: return 80.0f;
			case SW_GRENADE1: return 300.0f;
			case SW_GRENADE2: return 320.0f;
			case SW_GRENADE3: return 140.0f;
			case SW_BUBBLER: return 80.0f;
			case SW_BAZOOKA: return 240.0f;
			case SW_BOUNCER: return 140.0f - max(30.0f, (GetShotSpread(Weapon)-1)*15.0f);
			default: return 0.0f;
		};
	}
	else if (IsDroid(Weapon))
	{
		if (IsOnDeath(Weapon))
		{
			switch (GetDroidType(Weapon))
			{
				case DROIDTYPE_WALKER: return 160.0f;
				case DROIDTYPE_STAR: return 220.0f;
				case DROIDTYPE_CRAWLER: return 160.0f;
				default: return 0.0f;
			};
		}
		
		return 0;
	}
	
	float Size = 120 * GetProjectileSize(Weapon);
	
	return Size;
}

const float GetExplosionDamage(int Weapon)
{
	if (IsBuilding(Weapon))
	{
		switch (GetBuildingType(Weapon)) 
		{
			case BUILDING_TURRET: return 20.0f;
			case BUILDING_TESLACOIL: return 60.0f;
			case BUILDING_FLAMETRAP: return 40.0f;
			case BUILDING_BARREL: return 30.0f; break;
			case BUILDING_POWERBARREL: return 60.0f; break;
			case BUILDING_REACTOR: return 120.0f; break;
			case BUILDING_GENERATOR: return 40.0f; break;
			default: return 0; break;
		};
	}
	else if (IsDroid(Weapon))
	{
		if (IsOnDeath(Weapon))
		{
			switch (GetDroidType(Weapon))
			{
				case DROIDTYPE_WALKER: return 30.0f;
				case DROIDTYPE_STAR: return 40.0f;
				case DROIDTYPE_CRAWLER: return 30.0f;
				default: return 0.0f;
			};
		}
	}
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_BOMB: return 240; break;
			case SW_GRENADE1: return 120; break;
			case SW_GRENADE2: return 30; break;
			case SW_BUBBLER: return 14; break;
			case SW_BAZOOKA: return 80; break;
			case SW_BOUNCER: return 24 - max(6.0f, (GetShotSpread(Weapon)-1)*4.0f); break;
			default: return 0;
		};
	}
	
	const int Part1 = GetPart(Weapon, 0);
	
	if (Part1 != 2)
		return 0;
	
	float Size = 26 * (0.4f + GetProjectileSize(Weapon)*0.6f);
	
	return Size;
}


const int GetWeaponRenderType(int Weapon)
{
	if (Weapon == WEAPON_NONE)
		return WRT_NONE;
	
	if (IsModularWeapon(Weapon))
	{
		const int Part1 = GetPart(Weapon, 0);
		const int Part2 = GetPart(Weapon, 1);
		
		if (Part1 == 6)
			return WRT_SPIN;
		
		if (Part1 > 4 || Part2 > 5)
			return WRT_MELEE;
		
		if (!Part1 || !Part2)
			return WRT_NONE;
		
		return WRT_WEAPON1;
	}
	
	switch (GetStaticType(Weapon))
	{
		case SW_CHAINSAW: case SW_FLAMER: case SW_BAZOOKA: case SW_BOUNCER: case SW_BUBBLER: return WRT_WEAPON1;
		case SW_GUN1: case SW_GUN2: return WRT_WEAPON2;
		case SW_TOOL: return WRT_MELEESMALL;
		default: return WRT_ITEM1;
	};
}

const ivec2 GetWeaponVisualSize(int Weapon)
{
	if (IsModularWeapon(Weapon))
	{
		if (GetWeaponRenderType(Weapon) == WRT_MELEE)
			return ivec2(3, 2);
		
		if (GetWeaponRenderType(Weapon) == WRT_SPIN)
			return ivec2(3, 2);
		
		// WRT_WEAPON1
		return ivec2(4, 3);
	}
	
	switch (GetStaticType(Weapon))
	{
		case SW_CHAINSAW: case SW_BUBBLER: return ivec2(7, 3);
		case SW_FLAMER: case SW_BAZOOKA: case SW_BOUNCER: return ivec2(6, 3);
		case SW_GUN1: case SW_GUN2: return ivec2(4, 2);
		case SW_GRENADE1: case SW_GRENADE2: case SW_GRENADE3: return ivec2(2, 3);
		case SW_BALL: return ivec2(4, 4);
		case SW_SHURIKEN: return ivec2(4, 4);
		case SW_TOOL: return ivec2(4, 2);
		
		case SW_MASK1: case SW_MASK2: case SW_MASK3: case SW_MASK4: case SW_MASK5: return ivec2(4, 4);
		case SW_UPGRADE: return ivec2(2, 2);
		case SW_RESPAWNER: return ivec2(2, 4);
		case SW_BOMB: return ivec2(3, 4);
		case SW_INVIS: return ivec2(2, 3);
		case SW_SHIELD: return ivec2(2, 3);
	};
	
	return ivec2(0, 0);
}

const ivec2 GetWeaponVisualSize2(int Weapon)
{
	if (IsModularWeapon(Weapon))
	{
		if (GetPart(Weapon, 1) == 9)
			return ivec2(8, 4);
		
		if (GetWeaponRenderType(Weapon) == WRT_MELEE)
			return ivec2(8, 2);
		
		if (GetWeaponRenderType(Weapon) == WRT_SPIN)
			return ivec2(8, 2);
		
		// WRT_WEAPON1
		return ivec2(4, 3);
	}
	
	return ivec2(0, 0);
}


const int GetWeaponFiringType(int Weapon)
{
	if (Weapon == WEAPON_NONE)
		return WFT_NONE;
	
	if (IsModularWeapon(Weapon))
	{
		if (GetPart(Weapon, 1) == 5)
			return WFT_CHARGE;
		
		switch (GetPart(Weapon, 0))
		{
		case 5: return WFT_MELEE;
		case 6: return WFT_HOLD;
		default: return WFT_PROJECTILE;
		};
	}
	
	switch (GetStaticType(Weapon))
	{
		case SW_SHIELD: case SW_INVIS: case SW_UPGRADE: case SW_RESPAWNER: return WFT_ACTIVATE;
		case SW_CHAINSAW: case SW_FLAMER: return WFT_HOLD;
		case SW_BUBBLER: return WFT_PROJECTILE;
		case SW_BAZOOKA: return WFT_PROJECTILE;
		case SW_BOUNCER: return WFT_PROJECTILE;
		case SW_TOOL: return WFT_MELEE;
		case SW_GUN1: return WFT_PROJECTILE;
		case SW_GUN2: return WFT_CHARGE;
		case SW_GRENADE1: case SW_GRENADE2: case SW_GRENADE3: return WFT_THROW;
		case SW_BOMB: return WFT_ACTIVATE;
		case SW_SHURIKEN: return WFT_THROW;
		case SW_BALL: return WFT_THROW;
		default: return WFT_NONE;
	};
	
	return WFT_NONE;
}


const float GetWeaponRenderRecoil(int Weapon)
{
	if (IsModularWeapon(Weapon))
	{
		switch (GetPart(Weapon, 1))
		{
			case 1: return 13.0f;
			case 2: return 15.0f;
			case 3: return 14.0f;
			case 4: return 12.0f;
			default: return 0.0f;
		};
	}
	
	if (!IsStaticWeapon(Weapon))
		return 12.0f;
	
	switch (GetStaticType(Weapon))
	{
		case SW_BUBBLER: return 15.0f;
		case SW_BAZOOKA: return 18.0f;
		case SW_BOUNCER: return 13.0f;
		case SW_CHAINSAW: return 2.0f;
		case SW_GUN1: case SW_GUN2: return 13.0f;
		default: return 0.0f;
	};
	
	return 0.0f;
}


const vec2 GetWeaponRenderOffset(int Weapon)
{
	if (IsModularWeapon(Weapon))
	{
		if (GetWeaponRenderType(Weapon) == WRT_MELEE)
			return vec2(-12, -2);
		
		//if (GetWeaponRenderType(Weapon) == WRT_SPIN)
		//	return vec2(-12, -2);
		
		return vec2(24, 0);
	}
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_TOOL: return vec2(-20, -4);
			case SW_UPGRADE: return vec2(9, 0);
			case SW_RESPAWNER: return vec2(2, -5);
			case SW_SHIELD: case SW_INVIS: return vec2(4, -3);
			case SW_BOMB: return vec2(8, 0);
			case SW_MASK1: case SW_MASK2: case SW_MASK3: case SW_MASK4: case SW_MASK5: return vec2(16, 0);
			case SW_BAZOOKA: return vec2(30, 0);
			case SW_BOUNCER: return vec2(30, 0);
			case SW_BUBBLER: return vec2(30, 0);
			case SW_CHAINSAW: return vec2(30, 0);
			case SW_FLAMER: return vec2(30, 0);
			case SW_GUN1: case SW_GUN2: return vec2(16, -8);
			case SW_GRENADE1: case SW_GRENADE2: case SW_GRENADE3: return vec2(4, 0);
			case SW_BALL: return vec2(16, 0);
			case SW_SHURIKEN: return vec2(10, 0);
			default: return vec2(0, 0);
		};
	}
	
	return vec2(0, 0);
}

const vec2 GetMuzzleRenderOffset(int Weapon)
{
	if (GetStaticType(Weapon) == SW_GUN1)
		return vec2(20, -5);
	
	if (GetStaticType(Weapon) == SW_BUBBLER)
		return vec2(64, -4);
	if (GetStaticType(Weapon) == SW_BAZOOKA)
		return vec2(60, 0);
	if (GetStaticType(Weapon) == SW_BOUNCER)
		return vec2(62, 0);
	
	if (IsModularWeapon(Weapon))
	{
		switch (GetPart(Weapon, 1))
		{
			case 1: return vec2(66, 0);
			case 2: return vec2(64, 0);
			case 3: return vec2(74, 0);
			case 4: return vec2(60, -3);
			case 5: return vec2(68, 0);
			default: return vec2(50, 0);
		};
	}
	
	return vec2(0, 0);
}


const int WeaponProjectilePosType(int Weapon)
{
	if (IsDroid(Weapon))
	{
		switch (GetDroidType(Weapon))
		{
			case DROIDTYPE_WALKER: return 0;
			case DROIDTYPE_STAR: return 1;
			default: return 0;
		};
	}
	
	if (GetStaticType(Weapon) == SW_BUBBLER)
		return 1;
	
	if (GetStaticType(Weapon) == SW_BAZOOKA)
		return 2;
	
	return 0;
}

const vec2 GetProjectileOffset(int Weapon)
{
	if (IsModularWeapon(Weapon))
	{
		if (GetPart(Weapon, 0) == 6)
			return vec2(0, -14);
		
		switch (GetPart(Weapon, 1))
		{
			case 1: return vec2(60, -11);
			case 2: return vec2(58, -11);
			case 3: return vec2(70, -11);
			case 4: return vec2(56, -11);
			case 5: return vec2(62, -11);
			case 6: return vec2(70, -12);
			case 7: return vec2(70, -12);
			case 8: return vec2(80, -12);
			default: return vec2(50, -11);
		};
	}
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_TOOL: return vec2(26, -4);
			case SW_BAZOOKA: return vec2(65, -8);
			case SW_BOUNCER: return vec2(65, -11);
			case SW_BUBBLER: return vec2(66, -12);
			case SW_CHAINSAW: return vec2(53, -11);
			case SW_FLAMER: return vec2(86, -11);
			case SW_GUN1: case SW_GUN2: return vec2(36, -16);
			default: return vec2(0, 0);
		};
	}
	return vec2(0, 0);
}


const float ScreenshakeAmount(int Weapon)
{
	float d = GetExplosionDamage(Weapon) * 0.2f;
	
	if (d > 0 && WeaponElectroAmount(Weapon) <= 0.0f && WeaponFlameAmount(Weapon) <= 0.0f)
		return d * 0.02f;
	
	return 0.0f;
}


const float GetMeleeHitRadius(int Weapon)
{
	const float Charge = GetWeaponLevelCharge(Weapon);
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_TOOL: return 30.0f;
			case SW_SHURIKEN: return 20.0f;
			case SW_CHAINSAW: return 14.0f+Charge*5.0f;
			case SW_FLAMER: return 24.0f;
			default: return 0.0f;
		};
	}
	
	if (IsModularWeapon(Weapon))
	{
		if (GetPart(Weapon, 0) == 6)
			return 80;
		
		switch (GetPart(Weapon, 1))
		{
			case 6: return 52.0f+Charge*17.0f;
			case 7: return 52.0f+Charge*17.0f;
			case 8: return 46.0f+Charge*17.0f;
			case 9: return 52.0f+Charge*22.0f;
			default: return 0.0f;
		};
	}
	
	return 0.0f;
}


const bool WeaponAimline(int Weapon)
{
	if (IsModularWeapon(Weapon) && (GetPart(Weapon, 0) == 4 || GetPart(Weapon, 1) == 3))
		return true;

	return false;
}

const bool IsLaserWeapon(int Weapon)
{
	if (IsModularWeapon(Weapon) && GetPart(Weapon, 0) == 3 && (GetPart(Weapon, 1) == 2 || GetPart(Weapon, 1) == 3))
		return true;

	return false;
}


const int WeaponMaxLevel(int Weapon)
{
	if (IsModularWeapon(Weapon))
		return 4;

	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_UPGRADE: return 2;
			case SW_BAZOOKA: return 2;
			case SW_FLAMER: return 2;
			case SW_BOUNCER: return 2;
			case SW_CHAINSAW: return 2;
			default: return 0;
		};
	}
	
	return 0;
}


const int GetLaserCharge(int Weapon)
{
	if (IsModularWeapon(Weapon))
	{
		if (GetPart(Weapon, 0) == 3)
		{
			const float Charge = GetWeaponLevelCharge(Weapon);
			
			if (GetPart(Weapon, 1) == 2) return -1;
			if (GetPart(Weapon, 1) == 3) return 60 + Charge*60;
		}
	}

	return 0;
}

const int GetLaserRange(int Weapon)
{
	if (IsModularWeapon(Weapon))
	{
		if (GetPart(Weapon, 0) == 3)
		{
			if (GetPart(Weapon, 1) == 2) return 450;
			if (GetPart(Weapon, 1) == 3) return 700;
		}
	}

	return 0;
}

const int GetMuzzleType(int Weapon)
{
	if (GetStaticType(Weapon) == SW_GUN2)
		return 1;
	
	if (GetStaticType(Weapon) == SW_BUBBLER)
		return 0;
	if (GetStaticType(Weapon) == SW_BAZOOKA)
		return 0;
	if (GetStaticType(Weapon) == SW_BOUNCER)
		return 2;
	
	if (IsModularWeapon(Weapon))
	{
		if (GetPart(Weapon, 0) == 3)
			return 1;
		if (GetPart(Weapon, 0) == 4)
			return 2;
	}

	return 0;
}

const int GetMuzzleAmount(int Weapon)
{
	if (IsModularWeapon(Weapon))
	{
		switch (GetPart(Weapon, 1))
		{
			case 2: return 15;
			default: return 10;
			
		};
	}

	return 10;
}

const float GetProjectileSpeed(int Weapon)
{
	if (IsDroid(Weapon))
	{
		switch (GetDroidType(Weapon))
		{
			case DROIDTYPE_WALKER: return 1400;
			case DROIDTYPE_STAR: return 24;
			default: return 0.0f;
		};
	}
	
	if (GetStaticType(Weapon) == SW_GUN1)
		return 1200.0f;
	
	if (GetStaticType(Weapon) == SW_BUBBLER)
		return 25.0f;
	if (GetStaticType(Weapon) == SW_BAZOOKA)
		return 400.0f;
	if (GetStaticType(Weapon) == SW_BOUNCER)
		return 1500.0f;
	
	const float Charge = GetWeaponLevelCharge(Weapon);
	
	const int Part1 = GetPart(Weapon, 0);
	const int Part2 = GetPart(Weapon, 1);
	
	float Speed;
	
	switch (Part1)
	{
		case 1: Speed = 1400; break;
		case 2: Speed = 900; break;
		case 3: Speed = 1200; break;
		case 4: Speed = 1700; break;
		default: Speed = 1200; break;
	};
	
	if (Part1 == 3 && Part2 == 1)
		Speed += Charge * 400.0f;
	
	switch (Part2)
	{
		case 1: break;
		case 2: Speed *= 1.3f; break;
		case 3: Speed *= 2.5f; break;
		case 4: Speed *= 1.4f; break;
		default: break;
	};

	return Speed;
}

const float GetProjectileCurvature(int Weapon)
{
	if (IsDroid(Weapon))
	{
		switch (GetDroidType(Weapon))
		{
			default: return 0.0f;
		};
	}
	
	if (GetStaticType(Weapon) == SW_GUN1)
		return 2.2f;
	
	if (GetStaticType(Weapon) == SW_BUBBLER)
		return 2400.0f;
	
	if (GetStaticType(Weapon) == SW_BAZOOKA)
		return 0.0f;
	
	if (GetStaticType(Weapon) == SW_BOUNCER)
		return 0.0f;
	
	const int Part1 = GetPart(Weapon, 0);
	const int Part2 = GetPart(Weapon, 1);
	
	float Curvature;
	
	switch (Part1)
	{
		case 1: Curvature = 2.0f; break;
		case 2: Curvature = 6.0f; break;
		case 3: Curvature = 1.5f; break;
		case 4: Curvature = 1.8f; break;
		default: Curvature = 3.0f; break;
	};
	
	switch (Part2)
	{
		case 1: break;
		case 2: Curvature *= 1.0f; break;
		case 3: Curvature *= 0.4f; break;
		case 4: Curvature *= 0.6f; break;
		case 5: Curvature *= 1.0f - GetWeaponCharge(Weapon) * 0.1f; break;
		default: break;
	};
	
	
	
	return Curvature;
}

const int GetShotSpread(int Weapon)
{
	const float Charge = GetWeaponLevelCharge(Weapon);
	
	if (IsStaticWeapon(Weapon))
	{
		if (GetStaticType(Weapon) == SW_BOUNCER)
			return 1 + Charge*2.0f;
		
		return 1;
	}
	
	const int Part1 = GetPart(Weapon, 0);
	const int Part2 = GetPart(Weapon, 1);
	
	int Spread = 1;
	
	if (Part1 == 1 && Part2 == 2) Spread = 5+Charge*2.0f;
	if (Part1 == 2 && Part2 == 2) Spread = 4+Charge*1.0f;
	if (Part1 == 3 && Part2 == 2) Spread = 5+Charge*2.0f;
	if (Part1 == 4 && Part2 == 2) Spread = 4+Charge*2.0f;
	
	return Spread;
}

const float GetProjectileSpread(int Weapon)
{
	if (GetStaticType(Weapon) == SW_GUN1)
		return 0.06f;
	
	if (GetStaticType(Weapon) == SW_BUBBLER)
		return 0.04f;
	
	const int Part2 = GetPart(Weapon, 1);
	
	float Spread = 0.05f;
	
	if (Part2 == 3) Spread = 0.0f;
	
	return Spread;
}

const bool IsFlammableProjectile(int Weapon)
{
	if (Weapon == WEAPON_ACID)
		return 0.0f;
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_FLAMER: return true;
			case SW_BUBBLER: return true;
			default: return false;
		};
	}
	
	if (IsBuilding(Weapon))
	{
		switch (GetBuildingType(Weapon))
		{
			case BUILDING_FLAMETRAP: return true;
			default: return false;
		};
	}
	
	return false;
}


const float WeaponFlameAmount(int Weapon)
{
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_FLAMER: return 1.0f;
			case SW_BUBBLER: return 1.0f;
			default: return 0.0f;
		};
	}
	
	if (IsModularWeapon(Weapon))
	{
		const float Charge = GetWeaponLevelCharge(Weapon);
		
		const int Part1 = GetPart(Weapon, 0);
		const int Part2 = GetPart(Weapon, 1);
	
		if (Part1 == 5 && Part2 == 7 && Charge > 0.5f)
			return 0.0f + Charge * 0.5f;
		
		if (Part1 == 6 && Part2 == 7 && Charge > 0.5f)
			return 0.0f + Charge * 0.5f;
		
		return 0.0f;	
	}
	
	if (IsBuilding(Weapon))
	{
		switch (GetBuildingType(Weapon))
		{
			case BUILDING_FLAMETRAP: return 1.0f;
			default: return 0.0f;
		};
	}
	
	return 0.0f;
}

const int AIAttackRange(int Weapon)
{
	// seeing distance for free hands / no weapon
	if (Weapon == 0)
		return 1000;
	
	if (IsModularWeapon(Weapon))
	{
		const int Part1 = GetPart(Weapon, 0);
		const int Part2 = GetPart(Weapon, 1);
	
		if (Part2 == 2)
			return 600;
		
		if (Part1 == 1)
			return 900;
		
		if (Part1 == 2)
			return 700;
		
		if (Part1 == 3)
			return 900;
		
		if (Part1 == 4)
			return 900;
		
		if (Part1 == 6)
			return 200;
		
		if (Part2 > 5)
			return 250;
		
	}
	
	if (IsBuilding(Weapon))
	{
		switch (GetBuildingType(Weapon))
		{
			case BUILDING_TESLACOIL: return 800;
			default: return 0;
		};
	}
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_RESPAWNER: case SW_SHIELD: case SW_INVIS: return 800;
			case SW_GRENADE1: case SW_GRENADE2: case SW_GRENADE3: return 700;
			case SW_GUN1: return 700;
			case SW_GUN2: return 500;
			case SW_TOOL: return 50;
			case SW_BOMB: return 0;
			case SW_SHURIKEN: return 700;
			case SW_BUBBLER: return 700;
			case SW_BAZOOKA: return 700;
			case SW_BOUNCER: return 700;
			case SW_FLAMER: return 600;
			case SW_CHAINSAW: return 150;
			default: return 0;
		};
	}
	
	return 0;
}


const float WeaponElectroAmount(int Weapon)
{
	if (Weapon == WEAPON_ACID)
		return 0.0f;
	
	if (IsDroid(Weapon))
	{
		switch (GetDroidType(Weapon))
		{
			case DROIDTYPE_WALKER: return 0.5f;
			case DROIDTYPE_STAR: return 1.0f;
			default: return 0.0f;
		};
	}
	
	
	if (IsModularWeapon(Weapon))
	{
		const float Charge = GetWeaponLevelCharge(Weapon);
		
		const int Part1 = GetPart(Weapon, 0);
		const int Part2 = GetPart(Weapon, 1);
	
		if (Part1 == 3)
		{
			if (Part2 == 1) return 1.0f+Charge*0.7f;
			if (Part2 == 2) return 0.5f+Charge*0.3f;
			if (Part2 == 3) return 1.0f+Charge*0.7f;
			if (Part2 == 4) return 0.3f+Charge*0.3f;
		}
		
		if (Part1 == 5 && Part2 == 6 && Charge > 0.5f)
			return 0.0f + Charge * 0.5f;
		
		if (Part1 == 6 && Part2 == 6 && Charge > 0.5f)
			return 0.0f + Charge * 0.5f;
		
		return 0.0f;	
	}
	
	if (IsBuilding(Weapon))
	{
		switch (GetBuildingType(Weapon))
		{
			case BUILDING_LIGHTNINGWALL: return 1.0f;
			case BUILDING_TESLACOIL: return 1.0f;
			default: return 0.0f;
		};
	}
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_GRENADE2: return 0.5f;
			case SW_GUN2: return 0.5f;
		};
	}
	
	return 0.0f;
}

const int WeaponBurstCount(int Weapon)
{
	const float Charge = GetWeaponLevelCharge(Weapon);
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_BUBBLER: return 5;
			case SW_BAZOOKA: return 1+Charge*2.0f;
		};
		
		return 0;
	}
	
	
	if (IsModularWeapon(Weapon))
	{
		const int Part1 = GetPart(Weapon, 0);
		const int Part2 = GetPart(Weapon, 1);
	
		if (Part1 == 1 && Part2 == 2) return 2+Charge*2.0f;
		if (Part1 == 3 && Part2 == 4) return 3+Charge*4.0f;
		
		if (Part2 == 2) return 1+Charge*2.0f;
		if (Part1 == 2 && Part2 == 1) return 1+Charge*2.0f;
	}
	
	return 0;
}

const float WeaponBurstReload(int Weapon)
{
	const float Charge = GetWeaponLevelCharge(Weapon);
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_BUBBLER: return 0.2f;
			case SW_BAZOOKA: return max(0.14f, 1.0f - Charge * 0.5f);
			default: return 0.0f;
		};
	}
	
	if (IsModularWeapon(Weapon))
	{
		const int Part1 = GetPart(Weapon, 0);
		const int Part2 = GetPart(Weapon, 1);
	
		if (Part1 == 1 && Part2 == 2) return 0.4f;
		if (Part1 == 3 && Part2 == 4) return 0.25f;
		
		if (Part1 == 2 && Part2 == 1) return 0.9f-Charge*0.25f;
		
		return 0.9f;
	}
	
	return 1.0f;
}


const float GetProjectileDamage(int Weapon)
{
	const float Charge = GetWeaponLevelCharge(Weapon);
	
	if (IsDroid(Weapon))
	{
		switch (GetDroidType(Weapon))
		{
			case DROIDTYPE_WALKER: return 6.0f;
			case DROIDTYPE_STAR: return 14.0f;
			default: return 0.0f;
		};
	}
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_TOOL: return 10.0f;
			case SW_SHURIKEN: return 100.0f;
			case SW_FLAMER: return 2.0f+Charge*3.0f;
			case SW_CHAINSAW: return 6.0f+Charge*4.0f;
			case SW_BUBBLER: return 6.0f;
			case SW_BAZOOKA: return 10.0f;
			case SW_BOUNCER: return 0.0f+Charge*3.0f;
			case SW_GUN1: return 15.0f;
			case SW_GUN2: return 35.0f;
			default: return 0.0f;
		};
	}

	if (IsModularWeapon(Weapon))
	{
		const int Part1 = GetPart(Weapon, 0);
		const int Part2 = GetPart(Weapon, 1);
		
		if (Part1 == 1)
		{
			if (Part2 == 1) return 20+Charge*12.0f;
			if (Part2 == 2) return 7+Charge*4.0f;
			if (Part2 == 3) return 30+Charge*15.0f;
			if (Part2 == 4) return 16+Charge*5.0f;
		}
		else if (Part1 == 2)
		{

			if (Part2 == 1) return 10+Charge*5.0f;
			if (Part2 == 2) return 4+Charge*4.0f;
			if (Part2 == 3) return 15+Charge*5.0f;
			if (Part2 == 4) return 10+Charge*5.0f;
		}
		else if (Part1 == 3)
		{

			if (Part2 == 1) return 25+Charge*10.0f;
			if (Part2 == 2) return 8+Charge*4.0f;
			if (Part2 == 3) return 30+Charge*20.0f;
			if (Part2 == 4) return 15+Charge*5.0f;
		}
		else if (Part1 == 4)
		{

			if (Part2 == 1) return 32+Charge*12.0f;
			if (Part2 == 2) return 9+Charge*7.0f;
			if (Part2 == 3) return 40+Charge*20.0f;
			if (Part2 == 4) return 25+Charge*10.0f;
		}
		else if (Part1 == 5)
		{

			if (Part2 == 6) return 35+Charge*15.0f;
			if (Part2 == 7) return 30+Charge*10.0f;
			if (Part2 == 8) return 40+Charge*20.0f;
			if (Part2 == 9) return 20+Charge*20.0f;
		}
		else if (Part1 == 6)
		{
			if (Part2 == 6) return (35+Charge*15.0f)/4;
			if (Part2 == 7) return (30+Charge*10.0f)/4;
			if (Part2 == 8) return (40+Charge*20.0f)/4;
			if (Part2 == 9) return (20+Charge*20.0f)/4;
		}
	}
	
	return 0;
}

const int GetRandomWeaponType(bool IsSurvival) 
{
	if (str_comp(g_Config.m_SvGametype, "ball") == 0)
	{
		if (rand()%13 < 8)
			return GetModularWeapon(4, 1+rand()%4);
		if (rand()%13 < 5)
			return GetModularWeapon(2, 1+rand()%4);
		else
			return GetStaticWeapon(SW_BAZOOKA);
	}
	
	
	if (rand()%13 < 5)
		return GetModularWeapon(1+rand()%4, 1+rand()%4);
	
	// swords / melee
	if (rand()%12 < 3)
		return GetModularWeapon(5, 6+rand()%4);
	
	int w = 0;
	
	while (!w || (!IsSurvival && IsStaticWeapon(w) && GetStaticType(w) == SW_RESPAWNER))
		w = GetStaticWeapon(rand()%(NUM_SW-4));
	
	return w;
}

const bool WeaponAutoPick(int Weapon)
{
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_GUN1: case SW_GUN2: case SW_TOOL: return false;
			case SW_MASK1: case SW_MASK2: case SW_MASK3: case SW_MASK4: case SW_MASK5: return false;
			default: return true;
		};
	}
	
	return true;
}

const float GetProjectileKnockback(int Weapon)
{
	if (IsDroid(Weapon))
	{
		switch (GetDroidType(Weapon))
		{
			case DROIDTYPE_WALKER: return 1.0f;
			case DROIDTYPE_STAR: return 2.0f;
			default: return 0.0f;
		};
	}
	
	if (IsModularWeapon(Weapon))
	{
		const float Charge = GetWeaponLevelCharge(Weapon);
		
		const int Part1 = GetPart(Weapon, 0);
		const int Part2 = GetPart(Weapon, 1);
		
		if (Part1 == 1)
		{
			if (Part2 == 1) return 3.0f+Charge*4.0f;
			if (Part2 == 2) return 2.0f;
			if (Part2 == 3) return 4.0f+Charge*4.0f;
			if (Part2 == 4) return 2.0f;
		}
		
		if (Part1 == 2)
		{
			if (Part2 == 1) return 5.0f;
			if (Part2 == 2) return 3.0f;
			if (Part2 == 3) return 5.0f;
			if (Part2 == 4) return 3.0f;
		}
		
		if (Part1 == 3)
		{
			return 0.0f;
		}
		
		if (Part1 == 4)
		{
			if (Part2 == 1) return 6.0f+Charge*5.0f;
			if (Part2 == 2) return 3.0f+Charge*3.0f;
			if (Part2 == 3) return 8.0f+Charge*5.0f;
			if (Part2 == 4) return 5.0f+Charge*4.0f;
		}
		
		if (Part1 == 5)
		{
			if (Part2 == 6) return 5.0f+Charge*5.0f;
			if (Part2 == 7) return 3.0f+Charge*3.0f;
			if (Part2 == 8) return 7.0f+Charge*7.0f;
			if (Part2 == 9) return 15.0f+Charge*10.0f;
		}
		
		if (Part1 == 6)
		{
			if (Part2 == 6) return (5.0f+Charge*5.0f)/4;
			if (Part2 == 7) return (3.0f+Charge*3.0f)/4;
			if (Part2 == 8) return (7.0f+Charge*7.0f)/4;
			if (Part2 == 9) return (15.0f+Charge*10.0f)/4;
		}
	}
	
	return 0.0f;
}

const float GetProjectileLife(int Weapon)
{
	if (IsDroid(Weapon))
	{
		switch (GetDroidType(Weapon))
		{
			case DROIDTYPE_WALKER: return 0.6f;
			case DROIDTYPE_STAR: return 1.2f;
			default: return 0.0f;
		};
	}
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_GUN1: return 0.6f;
			case SW_BUBBLER: return 0.8f;
			case SW_BAZOOKA: return 0.8f;
			case SW_BOUNCER: return 0.8f;
			default: return 0.0f;
		};
	}
	
	float v = 1100.0f / GetProjectileSpeed(Weapon) / ((GetShotSpread(Weapon)+2.0f)/3.0f);
	
	const int Part2 = GetPart(Weapon, 1);
	
	if (Part2 == 2)
	{
		const float Charge = GetWeaponLevelCharge(Weapon);
		v *= (1.0f + Charge*0.25f);
	}
	
	return v;
}

const float WeaponThrowForce(int Weapon)
{
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_BALL: return 1.4f;
			case SW_GRENADE1: case SW_GRENADE2: case SW_GRENADE3: return 1.0f;
			case SW_SHURIKEN: return 1.4f;
			case SW_BOMB: return 0.4f;
			default: return 0.0f;
		};
	}
	
	return 0.0f;
}

const float GetWeaponFireRate(int Weapon)
{
	// 0.0f <= Charge <= 1.0f
	const float Charge = GetWeaponLevelCharge(Weapon);
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_BOMB: return 350;
			case SW_TOOL: return 320;
			case SW_CHAINSAW: return 500;
			case SW_FLAMER: return 200;
			case SW_BAZOOKA: return 640;
			case SW_BOUNCER: return 240;
			case SW_BUBBLER: return 600;
			case SW_GUN1: return 240;
			case SW_GUN2: return 200;
			default: return 0;
		};
	}
	
	const int Part1 = GetPart(Weapon, 0);
	const int Part2 = GetPart(Weapon, 1);
	
	if (Part1 == 1 && Part2 == 2) return 500;
	if (Part1 == 3 && Part2 == 4) return 500 - Charge*30.0f;
	
	float v = 10;
	
	switch (Part1)
	{
		case 1: v = 280; break;
		case 2: v = 490; break;
		case 3: v = 370; break;
		case 4: v = 500; v -= Charge*60.0f; break;
		case 5: v = 400; break;
		case 6: return 50; break;
		default: v = 300; break;
	};
	
	switch (Part2)
	{
		case 1: v -= Charge*80.0f; break;
		case 2: v -= Charge*90.0f; v *= 1.1f; break;
		case 3: v -= Charge*90.0f; v *= 1.2f; break;
		case 4: v -= Charge*80.0f; v *= 0.60f; break;
		case 5: v -= Charge*50.0f; v *= 0.65f; break;
		case 6: v -= Charge*35.0f; v *= 1.0f; break;
		case 7: v -= Charge*50.0f; v *= 0.8f; break;
		case 8: v -= Charge*35.0f; v *= 1.2f; break;
		case 9: v -= Charge*40.0f; v *= 1.1f; break;
		default: break;
	};
	
	return v;
}


const float GetWeaponKnockback(int Weapon)
{
	if (IsModularWeapon(Weapon))
	{
		const float Charge = GetWeaponLevelCharge(Weapon);
		
		const int Part1 = GetPart(Weapon, 0);
		const int Part2 = GetPart(Weapon, 1);
		
		if (Part1 == 1)
		{
			if (Part2 == 1) return 1.5f+Charge*1.2f;
			if (Part2 == 2) return 2.0f+Charge*1.25f;
			if (Part2 == 3) return 2.0f+Charge*1.2f;
			if (Part2 == 4) return 1.0f+Charge*1.0f;
		}
		
		if (Part1 == 2)
		{
			if (Part2 == 1) return 2.0f+Charge*1.25f;
			if (Part2 == 2) return 3.0f+Charge*1.25f;
			if (Part2 == 3) return 2.0f+Charge*1.25f;
			if (Part2 == 4) return 1.5f+Charge*1.2f;
		}
		
		if (Part1 == 3)
		{
			return 0.0f;
		}
		
		if (Part1 == 4)
		{
			if (Part2 == 1) return 1.5f+Charge*1.2f;
			if (Part2 == 2) return 2.0f+Charge*1.25f;
			if (Part2 == 3) return 2.0f+Charge*1.2f;
			if (Part2 == 4) return 1.0f+Charge*1.0f;
		}
	}
	
	return 0.0f;
}


const bool GetWeaponFullAuto(int Weapon)
{
	const float Charge = GetWeaponLevelCharge(Weapon);
	
	// modular weapons
	if (IsModularWeapon(Weapon))
	{
		if (Charge > 0.9f)
			return true;
		
		const int Part2 = GetPart(Weapon, 1);
		
		if (Part2 == 2 || Part2 == 3)
			return false;
		
		return true;
	}
	
	// static weapons
	switch (GetStaticType(Weapon))
	{
		case SW_GUN1: return false;
		default: return true;
	};
	
	return true;
}

const bool IsProjectileBouncy(int Weapon)
{
	if (GetStaticType(Weapon) == SW_BOUNCER)
		return true;
	
	return false;
}


const bool IsExplosiveProjectile(int Weapon)
{
	return true;
}


const int GetWeaponMaxAmmo(int Weapon)
{
	// 0.0f <= Charge <= 1.0f
	const float Charge = GetWeaponLevelCharge(Weapon);
	
	if (IsModularWeapon(Weapon))
	{
		int v = 0;
		
		const int Part1 = GetPart(Weapon, 0);
		const int Part2 = GetPart(Weapon, 1);
		
		if (Part1 == 1)
		{
			if (Part2 == 1) return 25 + Charge*10.0f;
			if (Part2 == 2) return 20 + Charge*10.0f;
			if (Part2 == 3) return 15 + Charge*10.0f;
			if (Part2 == 4) return 35 + Charge*15.0f;
		}
		
		if (Part1 == 2)
		{
			if (Part2 == 1) return 15 + Charge*10.0f;
			if (Part2 == 2) return 10 + Charge*10.0f;
			if (Part2 == 3) return 10 + Charge*10.0f;
			if (Part2 == 4) return 20 + Charge*15.0f;
		}
		
		if (Part1 == 3)
		{
			if (Part2 == 1) return 20 + Charge*10.0f;
			if (Part2 == 2) return 20 + Charge*10.0f;
			if (Part2 == 3) return 15 + Charge*10.0f;
			if (Part2 == 4) return 30 + Charge*15.0f;
		}
		
		if (Part1 == 4)
		{
			if (Part2 == 1) return 15 + Charge*10.0f;
			if (Part2 == 2) return 15 + Charge*10.0f;
			if (Part2 == 3) return 15 + Charge*10.0f;
			if (Part2 == 4) return 20 + Charge*15.0f;
		}
		
		return v;
	}
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_GUN1: return 0;
			case SW_BUBBLER: return 30;
			case SW_BAZOOKA: return 6+Charge*9.0f;
			case SW_BOUNCER: return 20+Charge*20.0f;
			case SW_CHAINSAW: return 15;
			case SW_FLAMER: return 25+Charge*15.0f;
			default: return 0;
		};
	}
	
	return 0;
}

const bool WeaponUseAmmo(int Weapon)
{
	if (IsModularWeapon(Weapon) && GetPart(Weapon, 0) < 5)
		return true;
	
	if (IsStaticWeapon(Weapon))
	{
		const float Charge = GetWeaponLevelCharge(Weapon);
		
		switch (GetStaticType(Weapon))
		{
			case SW_BAZOOKA: return true;
			case SW_BOUNCER: return true;
			case SW_BUBBLER: return true;
			case SW_CHAINSAW: return Charge < 0.9f ? true : false;
			case SW_FLAMER: return true;
			default: return false;
		};
	}
	
	return false;
}




