#include <cstring>
#include <new>
#include <base/math.h>
#include <engine/shared/config.h>
#include <engine/shared/datafile.h> // MapGen
#include <engine/map.h>
#include <engine/console.h>
#include "gamecontext.h"
#include <game/version.h>
#include <game/collision.h>
#include <game/gamecore.h> 
#include "gamemodes/dm.h"
#include "gamemodes/tdm.h"
#include "gamemodes/ctf.h"
#include "gamemodes/run.h"
#include "gamemodes/base.h"
#include "gamemodes/texasrun.h"

#include <game/server/entities/block.h>
#include <game/server/entities/pickup.h>
#include <game/server/entities/projectile.h>
#include <game/server/entities/laser.h>
#include <game/server/entities/building.h>
#include <game/server/entities/turret.h>
#include <game/server/entities/teslacoil.h>
#include <game/server/entities/droid.h>
#include <game/server/entities/superexplosion.h>

#include <game/server/entities/weapon.h>

#include <game/server/playerdata.h>

#include <game/server/ai_protocol.h>
#include <game/server/ai.h>

#include <game/buildables.h>

const int ExplosionDmg = 40;
const int MineExplosionDmg = 20;


enum
{
	RESET,
	NO_RESET
};

void CGameContext::Construct(int Resetting)
{
	m_Resetting = 0;
	m_pServer = 0;

	for(int i = 0; i < MAX_CLIENTS; i++)
		m_apPlayers[i] = 0;

	m_BroadcastLockTick = 0;
	
	m_pController = 0;
	m_VoteCloseTime = 0;
	m_pVoteOptionFirst = 0;
	m_pVoteOptionLast = 0;
	m_NumVoteOptions = 0;
	m_LockTeams = 0;
	
	ClearFlameHits();
	
	m_aMostInterestingPlayer[0] = -1;
	m_aMostInterestingPlayer[1] = -1;
	
	m_ShowWaypoints = false;
	m_ShowAiState = false;
	m_FreezeCharacters = false;

	if(Resetting==NO_RESET)
		m_pVoteOptionHeap = new CHeap();
}

CGameContext::CGameContext(int Resetting)
{
	Construct(Resetting);
}

CGameContext::CGameContext()
{
	Construct(NO_RESET);
}

CGameContext::~CGameContext()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
		delete m_apPlayers[i];
	if(!m_Resetting)
		delete m_pVoteOptionHeap;
}

void CGameContext::Clear()
{
	CHeap *pVoteOptionHeap = m_pVoteOptionHeap;
	CVoteOptionServer *pVoteOptionFirst = m_pVoteOptionFirst;
	CVoteOptionServer *pVoteOptionLast = m_pVoteOptionLast;
	int NumVoteOptions = m_NumVoteOptions;
	CTuningParams Tuning = m_Tuning;

	m_Resetting = true;
	this->~CGameContext();
	mem_zero(this, sizeof(*this));
	new (this) CGameContext(RESET);

	m_pVoteOptionHeap = pVoteOptionHeap;
	m_pVoteOptionFirst = pVoteOptionFirst;
	m_pVoteOptionLast = pVoteOptionLast;
	m_NumVoteOptions = NumVoteOptions;
	m_Tuning = Tuning;
}


CPlayerSpecData CGameContext::GetPlayerSpecData(int ClientID)
{
	CPlayerSpecData data;
	CCharacter *pCharacter = GetPlayerChar(ClientID);
	
	if (!pCharacter)
		return data;
	
	data.m_Kits = pCharacter->m_Kits;
	data.m_WeaponSlot = pCharacter->GetWeaponSlot();
		
	for (int i = 0; i < 4; i++)
		data.m_aWeapon[i] = pCharacter->GetWeaponType(i);
	
	return data;
}



class CCharacter *CGameContext::GetPlayerChar(int ClientID)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || !m_apPlayers[ClientID])
		return NULL;
	
	return m_apPlayers[ClientID]->GetCharacter();
}

void CGameContext::CreateBuildingHit(vec2 Pos)
{
	CNetEvent_BuildingHit *pEvent = (CNetEvent_BuildingHit *)m_Events.Create(NETEVENTTYPE_BUILDINGHIT, sizeof(CNetEvent_BuildingHit));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
	}
}

void CGameContext::CreateFlameHit(vec2 Pos)
{
	CNetEvent_FlameHit *pEvent = (CNetEvent_FlameHit *)m_Events.Create(NETEVENTTYPE_FLAMEHIT, sizeof(CNetEvent_FlameHit));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
	}
}

void CGameContext::CreateDamageInd(vec2 Pos, float Angle, int Damage, int ClientID)
{
	CNetEvent_DamageInd *pEvent = (CNetEvent_DamageInd *)m_Events.Create(NETEVENTTYPE_DAMAGEIND, sizeof(CNetEvent_DamageInd));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_Angle = (int)(Angle*256.0f + frandom()*200 - frandom()*200);
		//pEvent->m_Angle = (int)(Angle*256.0f);
		pEvent->m_Damage = Damage;
		pEvent->m_ClientID = ClientID;
	}
}

void CGameContext::CreateHammerHit(vec2 Pos)
{
	// create the event
	CNetEvent_HammerHit *pEvent = (CNetEvent_HammerHit *)m_Events.Create(NETEVENTTYPE_HAMMERHIT, sizeof(CNetEvent_HammerHit));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
	}
}


int CGameContext::CreateDeathray(vec2 Pos)
{
	// get height
	vec2 To = Pos + vec2(0, 1200);

	Collision()->IntersectLine(Pos, To, 0x0, &To);
	
	int Height = To.y - Pos.y + 14;
	
	// create the event
	CNetEvent_Lazer *pEvent = (CNetEvent_Lazer *)m_Events.Create(NETEVENTTYPE_LAZER, sizeof(CNetEvent_Lazer));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_Height = Height;
	}
	
	return Height;
}


bool CGameContext::BuildableSpot(vec2 Pos)
{
	if (Collision()->GetCollisionAt(Pos.x, Pos.y)&CCollision::COLFLAG_SOLID)
		return false;
	
	// todo: other entities
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CCharacter *pCharacter = GetPlayerChar(i);
		
		if (pCharacter && abs(Pos.x - pCharacter->m_Pos.x) < 32.0f && abs(Pos.y - pCharacter->m_Pos.y + 10) < 64.0f)
			return false;
	}
	
	return true;
}
	

bool CGameContext::AddBlock(int Type, vec2 Pos)
{
	if (!BuildableSpot(Pos))
		return false;
	
	// remove existing blocks
	vec2 BPos = vec2(int(Pos.x / 32)*32, int(Pos.y / 32)*32);
	CBlock *apEnts[16];
	int Num = m_World.FindBlocks(BPos, ivec2(1, 1), (CEntity**)apEnts, 16);

	for (int i = 0; i < Num; ++i)
	{
		CBlock *pTarget = apEnts[i];
		pTarget->Destroy();
	}
	
	// add new one
	new CBlock(&m_World, Type, Pos);
	return true;
}


void CGameContext::DamageBlocks(vec2 Pos, int Damage, int Range)
{
	vec2 BPos = vec2(int(Pos.x / 32)*32, int(Pos.y / 32)*32);
	
	CBlock *apEnts[1024];
	int Num = m_World.FindBlocks(BPos, ivec2(1, 1)*Range, (CEntity**)apEnts, 1024);

	for (int i = 0; i < Num; ++i)
	{
		CBlock *pTarget = apEnts[i];
		
		if (Range > 8)
		{
			float Radius = Range;
			float InnerRadius = Radius * 0.5f;
			vec2 Diff = pTarget->m_Pos - Pos + vec2(16, 16);
			
			float l = length(Diff);
			l = 1-clamp((l-InnerRadius)/(Radius-InnerRadius), 0.0f, 1.0f);
			float Dmg = Damage * l;
							
			if((int)Dmg && Dmg > 0.0f)
				pTarget->TakeDamage((int)Dmg);
		}
		else
			pTarget->TakeDamage(Damage);
	}
}


void CGameContext::CreateEffect(int FX, vec2 Pos)
{
	// create the event
	CNetEvent_FX *pEvent = (CNetEvent_FX *)m_Events.Create(NETEVENTTYPE_FX, sizeof(CNetEvent_FX));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_FX = FX;
	}
}


CWeapon *CGameContext::NewWeapon(int Part1, int Part2)
{
	return new CWeapon(&m_World, Part1<<4 | Part2<<8 | 1<<0);
}

CWeapon *CGameContext::NewWeapon(int Weapon)
{
	return new CWeapon(&m_World, Weapon);
}


bool CGameContext::AddBuilding(int Kit, vec2 Pos, int Owner)
{
	//float OffsetY = -(int(Pos.y)%32) + 12;
	float CheckRange = 40.0f;
	
	if (!g_Config.m_SvEnableBuilding)
		return false;
	
	
	if (Kit == BUILDABLE_BLOCK1)
		CheckRange = 32.0f;

	// check sanity
	/*
	if (!Collision()->GetCollisionAt(Pos.x-24, Pos.y+24)&CCollision::COLFLAG_SOLID || 
		!Collision()->GetCollisionAt(Pos.x+24, Pos.y+24)&CCollision::COLFLAG_SOLID ||
		Collision()->IsForceTile(Pos.x, Pos.y+24) != 0)
		return false;
		*/
	
	// check for close by buildings
	CBuilding *apEnts[16];
	int Num = m_World.FindEntities(Pos, 32, (CEntity**)apEnts, 16, CGameWorld::ENTTYPE_BUILDING);

	for (int i = 0; i < Num; ++i)
	{
		CBuilding *pTarget = apEnts[i];
		
		if (distance(pTarget->m_Pos, Pos) < CheckRange)
			return false;
	}
	

	if (Kit == BUILDABLE_BLOCK1)
	{
		AddBlock(1, Pos);
		return true;
	}
	
	if (Kit == BUILDABLE_BARREL)
	{
		new CBuilding(&m_World, Pos, BUILDING_BARREL, TEAM_NEUTRAL);
		return true;
	}

	if (Kit == BUILDABLE_TURRET)
	{
			new CBuilding(&m_World, Pos+vec2(0, 8), BUILDING_STAND, TEAM_NEUTRAL);
			return true;
	}
	
	if (Kit == BUILDABLE_LIGHTNINGWALL)
	{
			new CBuilding(&m_World, Pos+vec2(0, -14), BUILDING_LIGHTNINGWALL, TEAM_NEUTRAL);
			return true;
	}
	
	if (Kit == BUILDABLE_TESLACOIL)
	{
		
		int Team = m_apPlayers[Owner]->GetTeam();
		if (!m_pController->IsTeamplay())
			Team = m_apPlayers[Owner]->GetCID();
		
		CTeslacoil *Tesla = new CTeslacoil(&m_World, Pos+vec2(0, +35), Team, Owner);
		Tesla->m_DamageOwner = Owner;
		return true;
	}
	
	if (Kit == BUILDABLE_FLAMETRAP)
	{
			CBuilding *pFlametrap = new CBuilding(&m_World, Pos+vec2(0, -18), BUILDING_FLAMETRAP, TEAM_NEUTRAL);
			
			if (Collision()->IsTileSolid(Pos.x+32, Pos.y))
			{
				pFlametrap->m_Mirror = true;
				pFlametrap->m_Pos.x += 13;
			}
			else
				pFlametrap->m_Pos.x -= 12;
		
			return true;
	}
	
	return false;
}



void CGameContext::ClearFlameHits()
{
	for (int i = 0; i < MAX_CLIENTS; i++)
		m_aFlameHit[i] = false;
}




void CGameContext::CreateMeleeHit(int DamageOwner, int Weapon, float Dmg, vec2 Pos, vec2 Direction)
{
	float ProximityRadius = GetMeleeHitRadius(Weapon);
	float Damage = GetProjectileDamage(Weapon);
	
	//AddBlock(1, Pos);
	
	// for testing the collision
	//CreateBuildingHit(Pos);
	
	// player collision
	{
		CCharacter *apEnts[MAX_CLIENTS];
		int Num = m_World.FindEntities(Pos, ProximityRadius, (CEntity**)apEnts,
														MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

		for (int i = 0; i < Num; ++i)
		{
			CCharacter *pTarget = apEnts[i];

			if (pTarget->GetPlayer()->GetCID() == DamageOwner || pTarget->IgnoreCollision())
				continue;
			
			if (GetStaticType(Weapon) == SW_FLAMER && Collision()->IntersectLine(Pos, pTarget->m_Pos, NULL, NULL))
				continue;
			
			
			if (GetStaticType(Weapon) == SW_FLAMER)
			{
				if (m_aFlameHit[pTarget->GetPlayer()->GetCID()])
					continue;
				
				m_aFlameHit[pTarget->GetPlayer()->GetCID()] = true;
			}
			else
			{
				if (GetStaticType(Weapon) == SW_CHAINSAW)
					CreateEffect(FX_BLOOD2, (Pos+pTarget->m_Pos)/2.0f + vec2(0, -4));
				else
					CreateEffect(FX_BLOOD1, (Pos+pTarget->m_Pos)/2.0f + vec2(0, -4));
			}
			
			float f = WeaponFlameAmount(Weapon);
			if (f > 0.0f)
				pTarget->SetAflame(f, DamageOwner, Weapon);
			
			pTarget->TakeDamage(DamageOwner, Weapon, Damage * Dmg, normalize(vec2(frandom()-0.5f, frandom()-0.5f))*2.0f, Pos);
		}
	}
	
	// buildings
	{
		CBuilding *apEnts[MAX_CLIENTS];
		int Num = m_World.FindEntities(Pos, ProximityRadius, (CEntity**)apEnts,
														MAX_CLIENTS, CGameWorld::ENTTYPE_BUILDING);

		for (int i = 0; i < Num; ++i)
		{
			CBuilding *pTarget = apEnts[i];
			
			// skip own buildings in co-op
			if (m_pController->IsCoop() && (pTarget->m_Type == BUILDING_TURRET || pTarget->m_Type == BUILDING_TESLACOIL || pTarget->m_Type == BUILDING_REACTOR))
				if (DamageOwner >= 0 && DamageOwner < MAX_CLIENTS)
				{
					CPlayer *pPlayer = m_apPlayers[DamageOwner];
					if(pTarget->m_Team >= 0 && pPlayer && !pPlayer->m_IsBot)
						continue;
				}
			
			if (pTarget->m_Collision)
			{
				if (GetStaticType(Weapon) == SW_FLAMER)
					;
				else if (GetStaticType(Weapon) == SW_CHAINSAW)
					CreateEffect(FX_BLOOD2, (Pos+pTarget->m_Pos)/2.0f + vec2(0, -4));
				else
					CreateEffect(FX_BLOOD1, (Pos+pTarget->m_Pos)/2.0f + vec2(0, -4));
				
				pTarget->TakeDamage(Damage * Dmg, DamageOwner, Weapon);
				
				if (GetStaticType(Weapon) == SW_FLAMER)
					CreateFlameHit((Pos+pTarget->m_Pos)/2.0f+vec2(frandom()-frandom(), frandom()-frandom())*8.0f);
				else
					CreateBuildingHit((Pos+pTarget->m_Pos)/2.0f);
			}
		}
	}
	
	// droids & walkers
	{
		CDroid *apEnts[MAX_CLIENTS];
		int Num = m_World.FindEntities(Pos, ProximityRadius, (CEntity**)apEnts,
										MAX_CLIENTS, CGameWorld::ENTTYPE_DROID);

		for (int i = 0; i < Num; ++i)
		{
			CDroid *pTarget = apEnts[i];

			if (pTarget->m_Health <= 0)
				continue;

			pTarget->TakeDamage(vec2(0, 0), Damage * Dmg, DamageOwner, vec2(0, 0), Weapon);
			
			if (GetStaticType(Weapon) == SW_FLAMER)
				;
			else if (GetStaticType(Weapon) == SW_CHAINSAW)
				CreateEffect(FX_BLOOD2, (Pos+pTarget->m_Pos)/2.0f + vec2(0, -4));
			else
				CreateEffect(FX_BLOOD1, (Pos+pTarget->m_Pos)/2.0f + vec2(0, -4));
		}
	}
}


void CGameContext::CreateProjectile(int DamageOwner, int Weapon, int Charge, vec2 Pos, vec2 Direction, CBuilding *OwnerBuilding)
{
	// less damage for bots in co-op
	float Dmg = 1.0f;
	if (m_pController->IsCoop() && (DamageOwner < 0 || IsBot(DamageOwner)))
		Dmg = 0.5f;

	vec2 Vel = vec2(0, 0);
	
	if (GetPlayerChar(DamageOwner))
		Vel = GetPlayerChar(DamageOwner)->GetVel();
	
	// sword hit
	if (IsModularWeapon(Weapon))
	{
		if (GetPart(Weapon, 0) > 4)
		{
			CreateMeleeHit(DamageOwner, Weapon, Dmg, Pos, Direction);
			//CreateMeleeHit(DamageOwner, Weapon, Dmg, Pos+GetProjectileOffset(Weapon)*Direction, Direction);
			return;
		}
	}
	
	if (IsStaticWeapon(Weapon))
	{
		if (GetStaticType(Weapon) == SW_SHURIKEN)
		{
			CreateMeleeHit(DamageOwner, Weapon, Dmg, Pos, Direction);
			return;
		}
		else if (GetStaticType(Weapon) == SW_CHAINSAW)
		{
			CreateMeleeHit(DamageOwner, Weapon, Dmg, Pos, Direction);
			return;
		}
		else if (GetStaticType(Weapon) == SW_FLAMER)
		{
			ClearFlameHits();
			for (int i = 0; i < 4; i++)
			{
				vec2 To = Pos+Direction*i*58;
				
				Collision()->IntersectLine(Pos, To, 0x0, &To);
				CreateMeleeHit(DamageOwner, Weapon, Dmg, To, Direction);
			
				// to visualize hit points
				//CreateFlameHit(To);
			}
			ClearFlameHits();
			return;
		}
	}
	
	// define the projectile type
	int Explosion = 0;
	int HitSound = -1;
	float BulletSpread = GetProjectileSpread(Weapon);
	float Damage = GetProjectileDamage(Weapon);
	float Knockback = GetProjectileKnockback(Weapon);
	float BulletLife = GetProjectileLife(Weapon);
	
	int ShotSpread = GetShotSpread(Weapon);
	
	// laser pistol
	if (IsStaticWeapon(Weapon) && GetStaticType(Weapon) == SW_GUN2)
	{
		new CLaser(&m_World, Pos, Direction, 200.0f+Charge*3.5f, DamageOwner, Weapon, Damage * Dmg * (0.1f + Charge*0.009f), Charge);
		return;
	}
	
	
	if (IsLaserWeapon(Weapon))
	{
		for (int i = 0; i < ShotSpread; i++)
		{
			float Angle = GetAngle(Direction);
			Angle -= (ShotSpread-1)/2.0f * pi/180 * 4;
			Angle += i * pi/180 * 4;
			Angle += (frandom()-frandom())*BulletSpread;
			new CLaser(&m_World, Pos, vec2(cosf(Angle), sinf(Angle)), GetLaserRange(Weapon), DamageOwner, Weapon, Damage * Dmg, GetLaserCharge(Weapon));
		}
		return;
	}
	
	
	CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
	Msg.AddInt(ShotSpread);

	for (int i = 0; i < ShotSpread; i++)
	{
		float Angle = GetAngle(Direction);
		Angle -= (ShotSpread-1)/2.0f * pi/180 * 4;
		Angle += i * pi/180 * 4;
		Angle += (frandom()-frandom())*BulletSpread;

		CProjectile *pProj = new CProjectile(&m_World,
			Weapon,
			DamageOwner,
			Pos,
			vec2(cosf(Angle), sinf(Angle)),
			Vel,
			(int)(Server()->TickSpeed()*BulletLife),
			Damage * Dmg,
			Explosion,
			Knockback,
			HitSound);
			
		pProj->m_OwnerBuilding = OwnerBuilding;

		// pack the Projectile and send it to the client Directly
		CNetObj_Projectile p;
		pProj->FillInfo(&p);

		for(unsigned i = 0; i < sizeof(CNetObj_Projectile)/sizeof(int); i++)
			Msg.AddInt(((int *)&p)[i]);
	}

	if (DamageOwner >= 0 && DamageOwner < MAX_CLIENTS)
		Server()->SendMsg(&Msg, 0, DamageOwner);
}





void CGameContext::AmmoFill(vec2 Pos, int Weapon)
{
	CNetEvent_AmmoFill *pEvent = (CNetEvent_AmmoFill *)m_Events.Create(NETEVENTTYPE_AMMOFILL, sizeof(CNetEvent_AmmoFill));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_Weapon = Weapon;
	}
}


void CGameContext::Repair(vec2 Pos)
{
	float CheckRange = 42.0f;
	
	// check if there's turret base near
	CBuilding *apEnts[16];
	int Num = m_World.FindEntities(Pos, 32, (CEntity**)apEnts, 16, CGameWorld::ENTTYPE_BUILDING);

	for (int i = 0; i < Num; ++i)
	{
		CBuilding *pTarget = apEnts[i];
			
		if (distance(pTarget->m_Pos, Pos) < CheckRange)
		{
			if (pTarget->Repair())
			{
				CNetEvent_Repair *pEvent = (CNetEvent_Repair *)m_Events.Create(NETEVENTTYPE_REPAIR, sizeof(CNetEvent_Repair));
				if(pEvent)
				{
					pEvent->m_X = (int)(pTarget->m_Pos.x + Pos.x)/2;
					pEvent->m_Y = (int)(pTarget->m_Pos.y + Pos.y)/2;
				}
			}
			
			CreateBuildingHit(Pos);
		}
	}
}


void CGameContext::CreateExplosion(vec2 Pos, int Owner, int Weapon)
{
	float Dmg2 = 1.0f;

	if (m_pController->IsCoop() && IsBot(Owner))
		Dmg2 = 0.6f;

	// create the event
	CNetEvent_Explosion *pEvent = (CNetEvent_Explosion *)m_Events.Create(NETEVENTTYPE_EXPLOSION, sizeof(CNetEvent_Explosion));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_Weapon = Weapon;
	}
	
	if (GetExplosionSound(Weapon))
		CreateSound(Pos, GetExplosionSound(Weapon));
	
	// deal damage
	if (!GetExplosionDamage(Weapon))
		return;
	
	CCharacter *apEnts[MAX_CLIENTS];
	float Radius = GetExplosionSize(Weapon)*0.7f;
	float InnerRadius = Radius*0.5f;
	
	DamageBlocks(Pos, GetExplosionDamage(Weapon)*0.5f, Radius*0.8f);
	
	int Num = m_World.FindEntities(Pos, Radius, (CEntity**)apEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
	for(int i = 0; i < Num; i++)
	{
		vec2 Diff = apEnts[i]->m_Pos - Pos - vec2(0, 8);
		vec2 ForceDir(0,1);
		float l = length(Diff);
		if(l)
			ForceDir = normalize(Diff);
		l = 1-clamp((l-InnerRadius)/(Radius-InnerRadius), 0.0f, 1.0f);
		float Dmg = GetExplosionDamage(Weapon) * l;
						
		if((int)Dmg && Dmg > 0.0f)
			apEnts[i]->TakeDamage(Owner, Weapon, (int)Dmg*Dmg2, ForceDir*Dmg*0.3f, vec2(0, 0));
	}
	
	CBuilding *apBuildings[32];
	Num = m_World.FindEntities(Pos, Radius, (CEntity**)apBuildings, 32, CGameWorld::ENTTYPE_BUILDING);
	for(int i = 0; i < Num; i++)
	{
		vec2 Diff = apBuildings[i]->m_Pos - Pos - vec2(0, 8);
		
		float l = length(Diff);
		l = 1-clamp((l-InnerRadius)/(Radius-InnerRadius), 0.0f, 1.0f);
		float Dmg = GetExplosionDamage(Weapon) * l;
						
		if((int)Dmg && Dmg > 0.0f)
			apBuildings[i]->TakeDamage((int)Dmg*Dmg2, Owner, Weapon);
	}
	
	{
		CPickup *apPickups[64];
		Num = m_World.FindEntities(Pos, Radius, (CEntity**)apPickups, 64, CGameWorld::ENTTYPE_PICKUP);
		for(int i = 0; i < Num; i++)
		{
			vec2 Diff = apPickups[i]->m_Pos - Pos - vec2(0, 8);
			vec2 ForceDir(0,1);
			float l = length(Diff);
			if(l)
				ForceDir = normalize(Diff);
			l = 1-clamp((l-InnerRadius)/(Radius-InnerRadius), 0.0f, 1.0f);
			float Dmg = GetExplosionDamage(Weapon) * l;
							
			if((int)Dmg && Dmg > 0.0f)
				apPickups[i]->AddForce(ForceDir*Dmg*0.3f); //
		}
	}
	
	CDroid *apDEnts[MAX_CLIENTS];
	int DNum = m_World.FindEntities(Pos, Radius, (CEntity**)apDEnts,
									MAX_CLIENTS, CGameWorld::ENTTYPE_DROID);
	
	for (int i = 0; i < DNum; ++i)
	{
		CDroid *pTarget = apDEnts[i];

		if (pTarget->m_Health <= 0)
			continue;

		vec2 Diff = pTarget->m_Pos - Pos - vec2(0, 8);
		vec2 ForceDir(0, 1);
		float l = length(Diff);
		if(l)
			ForceDir = normalize(Diff);
		l = 1-clamp((l-InnerRadius)/(Radius-InnerRadius), 0.0f, 1.0f);
		float Dmg = GetExplosionDamage(Weapon) * l;
						
		if((int)Dmg && Dmg > 0.0f)
			pTarget->TakeDamage(ForceDir*Dmg*0.3f, (int)Dmg*Dmg2, Owner, vec2(0, 0), Weapon);
	}
}



void CGameContext::SendEffect(int ClientID, int EffectID)
{
	CNetEvent_Effect *pEvent = (CNetEvent_Effect *)m_Events.Create(NETEVENTTYPE_EFFECT, sizeof(CNetEvent_Effect));
	if(pEvent)
	{
		pEvent->m_ClientID = ClientID;
		pEvent->m_EffectID = EffectID;
	}
}


void CGameContext::CreatePlayerSpawn(vec2 Pos)
{
	// create the event
	CNetEvent_Spawn *ev = (CNetEvent_Spawn *)m_Events.Create(NETEVENTTYPE_SPAWN, sizeof(CNetEvent_Spawn));
	if(ev)
	{
		ev->m_X = (int)Pos.x;
		ev->m_Y = (int)Pos.y;
	}
}

void CGameContext::CreateDeath(vec2 Pos, int ClientID)
{
	// create the event
	CNetEvent_Death *pEvent = (CNetEvent_Death *)m_Events.Create(NETEVENTTYPE_DEATH, sizeof(CNetEvent_Death));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_ClientID = ClientID;
	}
}

void CGameContext::CreateSound(vec2 Pos, int Sound, int Mask)
{
	if (Sound < 0)
		return;

	// create a sound
	CNetEvent_SoundWorld *pEvent = (CNetEvent_SoundWorld *)m_Events.Create(NETEVENTTYPE_SOUNDWORLD, sizeof(CNetEvent_SoundWorld), Mask);
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_SoundID = Sound;
	}
}

void CGameContext::CreateSoundGlobal(int Sound, int Target)
{
	if (Sound < 0)
		return;

	CNetMsg_Sv_SoundGlobal Msg;
	Msg.m_SoundID = Sound;
	if(Target == -2)
		Server()->SendPackMsg(&Msg, MSGFLAG_NOSEND, -1);
	else
	{
		int Flag = MSGFLAG_VITAL;
		if(Target != -1)
			Flag |= MSGFLAG_NORECORD;
		Server()->SendPackMsg(&Msg, Flag, Target);
	}
}





bool CGameContext::IsBot(int ClientID)
{
	if (ClientID < 0 || ClientID >= MAX_CLIENTS)
		return false;
	
	if(m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_IsBot)
		return true;
	
	return false;
}

bool CGameContext::IsHuman(int ClientID)
{
	if (ClientID < 0 || ClientID >= MAX_CLIENTS)
		return false;
	
	if(m_apPlayers[ClientID] && !m_apPlayers[ClientID]->m_pAI)
		return true;
	
	return false;
}



void CGameContext::SendChatTarget(int To, const char *pText)
{
	// skip sending to bots
	if (IsBot(To))
		return;
	
	CNetMsg_Sv_Chat Msg;
	Msg.m_Team = 0;
	Msg.m_ClientID = -1;
	Msg.m_pMessage = pText;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, To);
}


void CGameContext::SendChat(int ChatterClientID, int Team, const char *pText)
{
	char aBuf[256];
	if(ChatterClientID >= 0 && ChatterClientID < MAX_CLIENTS)
		str_format(aBuf, sizeof(aBuf), "%d:%d:%s: %s", ChatterClientID, Team, Server()->ClientName(ChatterClientID), pText);
	else
		str_format(aBuf, sizeof(aBuf), "*** %s", pText);
	Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, Team!=CHAT_ALL?"teamchat":"chat", aBuf);

	if(Team == CHAT_ALL)
	{
		CNetMsg_Sv_Chat Msg;
		Msg.m_Team = 0;
		Msg.m_ClientID = ChatterClientID;
		Msg.m_pMessage = pText;
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
	}
	else
	{
		CNetMsg_Sv_Chat Msg;
		Msg.m_Team = 1;
		Msg.m_ClientID = ChatterClientID;
		Msg.m_pMessage = pText;

		// pack one for the recording only
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NOSEND, -1);

		// send to the clients
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_apPlayers[i] && !IsBot(i) && m_apPlayers[i]->GetTeam() == Team)
				Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, i);
		}
	}
}

void CGameContext::SendEmoticon(int ClientID, int Emoticon)
{
	CNetMsg_Sv_Emoticon Msg;
	Msg.m_ClientID = ClientID;
	Msg.m_Emoticon = Emoticon;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
}

void CGameContext::SendWeaponPickup(int ClientID, int Weapon)
{
	CNetMsg_Sv_WeaponPickup Msg;
	Msg.m_Weapon = Weapon;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}


void CGameContext::SendBroadcast(const char *pText, int ClientID, bool Lock)
{
	CNetMsg_Sv_Broadcast Msg;
	Msg.m_pMessage = pText;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
	if(ClientID < 0)
	{
		if(Lock)
			m_BroadcastLockTick = Server()->Tick() + g_Config.m_SvBroadcastLock * Server()->TickSpeed();
	}
	else
	{
		str_copy(m_apPlayers[ClientID]->m_aBroadcast, Lock ? pText : "", sizeof(m_apPlayers[ClientID]->m_aBroadcast));
		m_apPlayers[ClientID]->m_BroadcastLockTick = Lock ? Server()->Tick() : 0;
	}
}

//
void CGameContext::StartVote(const char *pDesc, const char *pCommand, const char *pReason)
{
	// check if vote time has expired or is invalid
	if (time_get() > m_VoteCloseTime || m_VoteCloseTime < time_get() - time_freq()*25)
		m_VoteCloseTime = 0;
	
	// check if a vote is already running
	if(m_VoteCloseTime)
		return;
	
	// reset votes
	m_VoteEnforce = VOTE_ENFORCE_UNKNOWN;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			m_apPlayers[i]->m_Vote = 0;
			m_apPlayers[i]->m_VotePos = 0;
		}
	}

	// start vote
	m_VoteCloseTime = time_get() + time_freq()*25;
	str_copy(m_aVoteDescription, pDesc, sizeof(m_aVoteDescription));
	str_copy(m_aVoteCommand, pCommand, sizeof(m_aVoteCommand));
	str_copy(m_aVoteReason, pReason, sizeof(m_aVoteReason));
	SendVoteSet(-1);
	m_VoteUpdate = true;
}


void CGameContext::EndVote()
{
	m_VoteCloseTime = 0;
	SendVoteSet(-1);
}

void CGameContext::SendVoteSet(int ClientID)
{
	CNetMsg_Sv_VoteSet Msg;
	if(m_VoteCloseTime)
	{
		Msg.m_Timeout = (m_VoteCloseTime-time_get())/time_freq();
		Msg.m_pDescription = m_aVoteDescription;
		Msg.m_pReason = m_aVoteReason;
	}
	else
	{
		Msg.m_Timeout = 0;
		Msg.m_pDescription = "";
		Msg.m_pReason = "";
	}
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGameContext::SendVoteStatus(int ClientID, int Total, int Yes, int No)
{
	CNetMsg_Sv_VoteStatus Msg = {0};
	Msg.m_Total = Total;
	Msg.m_Yes = Yes;
	Msg.m_No = No;
	Msg.m_Pass = Total - (Yes+No);

	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);

}

void CGameContext::AbortVoteKickOnDisconnect(int ClientID)
{
	if(m_VoteCloseTime && ((!str_comp_num(m_aVoteCommand, "kick ", 5) && str_toint(&m_aVoteCommand[5]) == ClientID) ||
		(!str_comp_num(m_aVoteCommand, "set_team ", 9) && str_toint(&m_aVoteCommand[9]) == ClientID)))
		m_VoteCloseTime = -1;
}


void CGameContext::CheckPureTuning()
{
	// might not be created yet during start up
	if(!m_pController)
		return;

	if(	str_comp(m_pController->m_pGameType, "DM")==0 ||
		str_comp(m_pController->m_pGameType, "TDM")==0 ||
		str_comp(m_pController->m_pGameType, "DEF")==0 ||
		str_comp(m_pController->m_pGameType, "INF")==0 ||
		str_comp(m_pController->m_pGameType, "INV")==0 ||
		str_comp(m_pController->m_pGameType, "GUN")==0 ||
		str_comp(m_pController->m_pGameType, "CTF")==0)
	{
		CTuningParams p;
		if(mem_comp(&p, &m_Tuning, sizeof(p)) != 0)
		{
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "resetting tuning due to pure server");
			m_Tuning = p;
		}
	}
}

void CGameContext::SendTuningParams(int ClientID)
{
	CheckPureTuning();

	CMsgPacker Msg(NETMSGTYPE_SV_TUNEPARAMS);
	int *pParams = (int *)&m_Tuning;
	for(unsigned i = 0; i < sizeof(m_Tuning)/sizeof(int); i++)
		Msg.AddInt(pParams[i]);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}


void CGameContext::UpdateSpectators()
{
	bool Found[2] = {false, false};
		
	// check validity
	for (int i = 0; i < 2; i++)
	{
		if (m_aMostInterestingPlayer[i] >= 0)
		{
			// player left or something
			if (!m_apPlayers[m_aMostInterestingPlayer[i]])
			{
				m_aMostInterestingPlayer[i] = -1;
			}
			else
			{
				// player is a spectator
				if (m_apPlayers[m_aMostInterestingPlayer[i]]->Spectating())
					m_aMostInterestingPlayer[i] = -1;
			}
		}
	}


	// find the most interesting player of both teams
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		
		// player and character exists
		if (m_apPlayers[i] && m_apPlayers[i]->m_EnableAutoSpectating && m_apPlayers[i]->GetCharacter() && m_apPlayers[i]->GetCharacter()->IsAlive() &&
			(!m_apPlayers[i] || !g_Config.m_SvSpectateOnlyHumans))
		{
			int Team = m_apPlayers[i]->GetTeam();
			
			// team is correct
			if(Team == TEAM_RED || Team == TEAM_BLUE)
			{
				// most interesting player exists
				int Points = -1;
				int Player = m_aMostInterestingPlayer[Team];
				
				m_apPlayers[i]->m_InterestPoints += frandom();
				
				if (Player >= 0)
					if (m_apPlayers[Player] && m_apPlayers[Player]->GetCharacter())
						Points = m_apPlayers[Player]->m_InterestPoints;
					
					
				if (m_apPlayers[i]->m_InterestPoints > Points)
				{
					m_aMostInterestingPlayer[Team] = i;
					Found[Team] = true;				
				}
			}
		}
	}


	// update the spectator views
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		//if(m_apPlayers[i] && (m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS || !m_apPlayers[i]->GetCharacter()) && !m_apPlayers[i]->m_IsBot)
		if(m_apPlayers[i] && !m_apPlayers[i]->m_IsBot && m_apPlayers[i]->m_ActionSpectator && m_apPlayers[i]->Spectating())
		{
			if (!m_apPlayers[i]->m_LastSetSpectatorMode)
				m_apPlayers[i]->m_LastSetSpectatorMode = Server()->Tick() - Server()->TickSpeed()*g_Config.m_SvSpectatorUpdateTime;
			else
			{
				if (m_apPlayers[i]->m_LastSetSpectatorMode+Server()->TickSpeed()*g_Config.m_SvSpectatorUpdateTime < Server()->Tick())	
				{
					int WantedPlayer = -1;
					
					/*
					if (!m_pController->IsTeamplay())
					{
						if (m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
						{
							
							
							
						}
					}
					else*/
					{
						int Team = m_apPlayers[i]->GetTeam();
							
						// get the correct player
						if (Team == TEAM_RED || Team == TEAM_BLUE)
						{
							WantedPlayer = m_aMostInterestingPlayer[Team];
								
							// update the view
							if (WantedPlayer >= 0 && m_apPlayers[i]->m_SpectatorID != WantedPlayer && Found[Team])
							{
								m_apPlayers[i]->m_LastSetSpectatorMode = Server()->Tick();
								m_apPlayers[i]->m_SpectatorID = WantedPlayer;
								Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "cstt", "Spectator id changed");
							}
						}
					}
				}
			}
		}
	}
}



void CGameContext::SwapTeams()
{
	if(!m_pController->IsTeamplay() || m_pController->IsInfection())
		return;
	
	SendChat(-1, CGameContext::CHAT_ALL, "Teams were swapped");

	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
			m_apPlayers[i]->SetTeam(m_apPlayers[i]->GetTeam()^1, false);
	}

	(void)m_pController->CheckTeamBalance();
}


void CGameContext::OnTick()
{
	// check tuning
	CheckPureTuning();

	// copy tuning
	m_World.m_Core.m_Tuning = m_Tuning;
	m_World.m_Core.ClearMonsters();
	m_World.m_Core.ClearImpacts();
	m_World.Tick();

	//if(world.paused) // make sure that the game object always updates
	m_pController->Tick();

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			m_apPlayers[i]->Tick();
			m_apPlayers[i]->PostTick();
		}
	}

	// update voting
	if(m_VoteCloseTime)
	{
		// abort the kick-vote on player-leave
		if(m_VoteCloseTime == -1)
		{
			SendChat(-1, CGameContext::CHAT_ALL, "Vote aborted");
			EndVote();
		}
		else
		{
			int Total = 0, Yes = 0, No = 0;
			if(m_VoteUpdate)
			{
				// count votes
				char aaBuf[MAX_CLIENTS][NETADDR_MAXSTRSIZE] = {{0}};
				for(int i = 0; i < MAX_CLIENTS; i++)
					if(m_apPlayers[i] && !IsBot(i))
						Server()->GetClientAddr(i, aaBuf[i], NETADDR_MAXSTRSIZE);
				bool aVoteChecked[MAX_CLIENTS] = {0};
				for(int i = 0; i < MAX_CLIENTS; i++)
				{
					if(!m_apPlayers[i] || IsBot(i) || m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS || aVoteChecked[i])	// don't count in votes by spectators
						continue;

					int ActVote = m_apPlayers[i]->m_Vote;
					int ActVotePos = m_apPlayers[i]->m_VotePos;

					// check for more players with the same ip (only use the vote of the one who voted first)
					for(int j = i+1; j < MAX_CLIENTS; ++j)
					{
						if(!m_apPlayers[j] ||  IsBot(i) || aVoteChecked[j] || str_comp(aaBuf[j], aaBuf[i]))
							continue;

						aVoteChecked[j] = true;
						if(m_apPlayers[j]->m_Vote && (!ActVote || ActVotePos > m_apPlayers[j]->m_VotePos))
						{
							ActVote = m_apPlayers[j]->m_Vote;
							ActVotePos = m_apPlayers[j]->m_VotePos;
						}
					}

					Total++;
					if(ActVote > 0)
						Yes++;
					else if(ActVote < 0)
						No++;
				}

				if(Yes >= Total/2+1)
					m_VoteEnforce = VOTE_ENFORCE_YES;
				else if(No >= (Total+1)/2)
					m_VoteEnforce = VOTE_ENFORCE_NO;
			}

			if(m_VoteEnforce == VOTE_ENFORCE_YES)
			{
				Server()->SetRconCID(IServer::RCON_CID_VOTE);
				Console()->ExecuteLine(m_aVoteCommand);
				Server()->SetRconCID(IServer::RCON_CID_SERV);
				EndVote();
				SendChat(-1, CGameContext::CHAT_ALL, "Vote passed");

				if(m_apPlayers[m_VoteCreator])
					m_apPlayers[m_VoteCreator]->m_LastVoteCall = 0;
			}
			else if(m_VoteEnforce == VOTE_ENFORCE_NO || time_get() > m_VoteCloseTime)
			{
				EndVote();
				SendChat(-1, CGameContext::CHAT_ALL, "Vote failed");
			}
			else if(m_VoteUpdate)
			{
				m_VoteUpdate = false;
				SendVoteStatus(-1, Total, Yes, No);
			}
		}
	}


#ifdef CONF_DEBUG
	if(g_Config.m_DbgDummies)
	{
		for(int i = 0; i < g_Config.m_DbgDummies ; i++)
		{
			CNetObj_PlayerInput Input = {0};
			Input.m_Direction = (i&1)?-1:1;
			m_apPlayers[MAX_CLIENTS-i-1]->OnPredictedInput(&Input);
		}
	}
#endif
}



bool CGameContext::AIInputUpdateNeeded(int ClientID)
{
	if(m_apPlayers[ClientID])
		return m_apPlayers[ClientID]->AIInputChanged();
		
	return false;
}


void CGameContext::UpdateAI()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && IsBot(i))
			m_apPlayers[i]->AITick();
	}
}


/*
enum InputList
{
	INPUT_MOVE = 0,
	INPUT_SHOOT = 4,
	INPUT_JUMP = 3,
	INPUT_HOOK = 5
	INPUT_DOWN = 6
	
	//1 & 2 vectors for weapon direction
};
*/


void CGameContext::AIUpdateInput(int ClientID, int *Data)
{
	if(m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_pAI)
		m_apPlayers[ClientID]->m_pAI->UpdateInput(Data);
}



// Server hooks
void CGameContext::AddZombie()
{
	Server()->AddZombie();
}


void CGameContext::OnClientDirectInput(int ClientID, void *pInput)
{
	if(!m_World.m_Paused)
		m_apPlayers[ClientID]->OnDirectInput((CNetObj_PlayerInput *)pInput);
}

void CGameContext::OnClientPredictedInput(int ClientID, void *pInput)
{
	if(!m_World.m_Paused)
		m_apPlayers[ClientID]->OnPredictedInput((CNetObj_PlayerInput *)pInput);
}

void CGameContext::OnClientEnter(int ClientID)
{
	//world.insert_entity(&players[client_id]);
	m_apPlayers[ClientID]->Respawn();
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "'%s' joined the fun", Server()->ClientName(ClientID));
	SendChat(-1, CGameContext::CHAT_ALL, aBuf);

	str_format(aBuf, sizeof(aBuf), "team_join player='%d:%s' team=%d", ClientID, Server()->ClientName(ClientID), m_apPlayers[ClientID]->GetTeam());
	Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

	m_LastSeen.SendLastSeenMsg(this, ClientID);

	if (m_pController->IsCoop() && g_Config.m_SvMapGen)
	{
		char aBuf[256];
		if (!g_Config.m_SvInvFails)
			str_format(aBuf, sizeof(aBuf), "Level %d", g_Config.m_SvMapGenLevel);
		else if (g_Config.m_SvInvFails == 1)
			str_format(aBuf, sizeof(aBuf), "Level %d - Second try", g_Config.m_SvMapGenLevel);
		else
			str_format(aBuf, sizeof(aBuf), "Level %d - Last chance", g_Config.m_SvMapGenLevel);
		SendBroadcast(aBuf, ClientID);
	}
	
	m_VoteUpdate = true;
}

void CGameContext::OnClientConnected(int ClientID, bool AI)
{
	// Check which team the player should be on
	const int StartTeam = g_Config.m_SvTournamentMode ? TEAM_SPECTATORS : m_pController->GetAutoTeam(ClientID);

	m_apPlayers[ClientID] = new(ClientID) CPlayer(this, ClientID, StartTeam);
	//players[client_id].init(client_id);
	//players[client_id].client_id = client_id;

	m_apPlayers[ClientID]->m_IsBot = AI;
	
	(void)m_pController->CheckTeamBalance();

#ifdef CONF_DEBUG
	if(g_Config.m_DbgDummies)
	{
		if(ClientID >= MAX_CLIENTS-g_Config.m_DbgDummies)
			return;
	}
#endif

	// send active vote
	if(m_VoteCloseTime)
		SendVoteSet(ClientID);

	// send motd
	/* skip motd
	CNetMsg_Sv_Motd Msg;
	Msg.m_pMessage = g_Config.m_SvMotd;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
	*/
}

void CGameContext::OnClientDrop(int ClientID, const char *pReason)
{
	m_LastSeen.OnClientDrop(this, ClientID);
	AbortVoteKickOnDisconnect(ClientID);
	m_apPlayers[ClientID]->OnDisconnect(pReason);
	delete m_apPlayers[ClientID];
	m_apPlayers[ClientID] = 0;

	(void)m_pController->CheckTeamBalance();
	m_VoteUpdate = true;

	// update spectator modes
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_SpectatorID == ClientID)
			m_apPlayers[i]->m_SpectatorID = SPEC_FREEVIEW;
	}
	
	if (Server()->PlayerData(ClientID))
		Server()->PlayerData(ClientID)->Reset();
}

void CGameContext::OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientID)
{
	void *pRawMsg = m_NetObjHandler.SecureUnpackMsg(MsgID, pUnpacker);
	CPlayer *pPlayer = m_apPlayers[ClientID];

	if(!pRawMsg)
	{
		if(g_Config.m_Debug)
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "dropped weird message '%s' (%d), failed on '%s'", m_NetObjHandler.GetMsgName(MsgID), MsgID, m_NetObjHandler.FailedMsgOn());
			Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "server", aBuf);
		}
		return;
	}

	if(Server()->ClientIngame(ClientID))
	{
		if(MsgID == NETMSGTYPE_CL_SAY)
		{
			if(g_Config.m_SvSpamprotection && pPlayer->m_LastChat && pPlayer->m_LastChat+Server()->TickSpeed() > Server()->Tick())
				return;

			CNetMsg_Cl_Say *pMsg = (CNetMsg_Cl_Say *)pRawMsg;
			int Team = pMsg->m_Team ? pPlayer->GetTeam() : CGameContext::CHAT_ALL;
			
			// trim right and set maximum length to 128 utf8-characters
			int Length = 0;
			const char *p = pMsg->m_pMessage;
			const char *pEnd = 0;
			while(*p)
 			{
				const char *pStrOld = p;
				int Code = str_utf8_decode(&p);

				// check if unicode is not empty
				if(Code > 0x20 && Code != 0xA0 && Code != 0x034F && (Code < 0x2000 || Code > 0x200F) && (Code < 0x2028 || Code > 0x202F) &&
					(Code < 0x205F || Code > 0x2064) && (Code < 0x206A || Code > 0x206F) && (Code < 0xFE00 || Code > 0xFE0F) &&
					Code != 0xFEFF && (Code < 0xFFF9 || Code > 0xFFFC))
				{
					pEnd = 0;
				}
				else if(pEnd == 0)
					pEnd = pStrOld;

				if(++Length >= 127)
				{
					*(const_cast<char *>(p)) = 0;
					break;
				}
 			}
			if(pEnd != 0)
				*(const_cast<char *>(pEnd)) = 0;

			// drop empty and autocreated spam messages (more than 16 characters per second)
			if(Length == 0 || (g_Config.m_SvSpamprotection && pPlayer->m_LastChat && pPlayer->m_LastChat+Server()->TickSpeed()*((15+Length)/16) > Server()->Tick()))
				return;

			bool SendToTeam = true;
			bool SkipSending = false;
			
			pPlayer->m_LastChat = Server()->Tick();

			
			if ( strcmp(pMsg->m_pMessage, "/color") == 0 )
			{
				char aBuf[128]; str_format(aBuf, sizeof(aBuf), "Body: %d, feet: %d, skin: %d, topper: %d", m_apPlayers[ClientID]->m_TeeInfos.m_ColorBody, m_apPlayers[ClientID]->m_TeeInfos.m_ColorFeet, m_apPlayers[ClientID]->m_TeeInfos.m_ColorSkin, m_apPlayers[ClientID]->m_TeeInfos.m_ColorTopper);
				SendChatTarget(ClientID, aBuf);
				SkipSending = true;
			}
			

			if ( strcmp(pMsg->m_pMessage, "/showwaypoints") == 0 )
			{
				m_ShowWaypoints = !m_ShowWaypoints;
				SkipSending = true;
			}
			
			if ( strcmp(pMsg->m_pMessage, "/showaistate") == 0 )
			{
				m_ShowAiState = !m_ShowAiState;
				SkipSending = true;
			}
			
		
			if (!SkipSending)
			{
				if (SendToTeam)
					SendChat(ClientID, Team, pMsg->m_pMessage);
				else
					SendChatTarget(ClientID, pMsg->m_pMessage);
			}
		}
		else if(MsgID == NETMSGTYPE_CL_CALLVOTE)
		{
			if(g_Config.m_SvSpamprotection && pPlayer->m_LastVoteTry && pPlayer->m_LastVoteTry+Server()->TickSpeed()*1 > Server()->Tick())
				return;

			int64 Now = Server()->Tick();
			pPlayer->m_LastVoteTry = Now;
			if(pPlayer->GetTeam() == TEAM_SPECTATORS)
			{
				SendChatTarget(ClientID, "Spectators aren't allowed to start a vote.");
				return;
			}

			if(m_VoteCloseTime)
			{
				SendChatTarget(ClientID, "Wait for current vote to end before calling a new one.");
				return;
			}

			int Timeleft = pPlayer->m_LastVoteCall + Server()->TickSpeed()*60 - Now;
			if(pPlayer->m_LastVoteCall && Timeleft > 0)
			{
				char aChatmsg[512] = {0};
				str_format(aChatmsg, sizeof(aChatmsg), "You must wait %d seconds before making another vote", (Timeleft/Server()->TickSpeed())+1);
				SendChatTarget(ClientID, aChatmsg);
				return;
			}

			char aChatmsg[512] = {0};
			char aDesc[VOTE_DESC_LENGTH] = {0};
			char aCmd[VOTE_CMD_LENGTH] = {0};
			CNetMsg_Cl_CallVote *pMsg = (CNetMsg_Cl_CallVote *)pRawMsg;
			const char *pReason = pMsg->m_Reason[0] ? pMsg->m_Reason : "No reason given";

			if(str_comp_nocase(pMsg->m_Type, "option") == 0)
			{
				CVoteOptionServer *pOption = m_pVoteOptionFirst;
				
				while(pOption)
				{
					if(str_comp_nocase(pMsg->m_Value, pOption->m_aDescription) == 0)
					{
						str_format(aChatmsg, sizeof(aChatmsg), "'%s' called vote to change server option '%s' (%s)", Server()->ClientName(ClientID),
									pOption->m_aDescription, pReason);
						str_format(aDesc, sizeof(aDesc), "%s", pOption->m_aDescription);
						str_format(aCmd, sizeof(aCmd), "%s", pOption->m_aCommand);
						break;
					}

					pOption = pOption->m_pNext;
				}

				if(!pOption)
				{
					//str_format(aDesc, sizeof(aDesc), "%s", pOption->m_aDescription);
					//str_format(aCmd, sizeof(aCmd), "%s", pOption->m_aCommand);
						
					str_format(aChatmsg, sizeof(aChatmsg), "'%s' isn't an option on this server", pMsg->m_Value);
					SendChatTarget(ClientID, aChatmsg);
					
					return;
				}
			}
			else if(str_comp_nocase(pMsg->m_Type, "kick") == 0)
			{
				if(!g_Config.m_SvVoteKick)
				{
					SendChatTarget(ClientID, "Server does not allow voting to kick players");
					return;
				}

				if(g_Config.m_SvVoteKickMin)
				{
					int PlayerNum = 0;
					for(int i = 0; i < MAX_CLIENTS; ++i)
						if(m_apPlayers[i] && m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
							++PlayerNum;

					if(PlayerNum < g_Config.m_SvVoteKickMin)
					{
						str_format(aChatmsg, sizeof(aChatmsg), "Kick voting requires %d players on the server", g_Config.m_SvVoteKickMin);
						SendChatTarget(ClientID, aChatmsg);
						return;
					}
				}

				int KickID = str_toint(pMsg->m_Value);
				if(KickID < 0 || !m_apPlayers[KickID])
				{
					SendChatTarget(ClientID, "Invalid client id to kick");
					return;
				}
				if(KickID == ClientID)
				{
					SendChatTarget(ClientID, "You can't kick yourself");
					return;
				}
				if(Server()->IsAuthed(KickID))
				{
					SendChatTarget(ClientID, "You can't kick admins");
					char aBufKick[128];
					str_format(aBufKick, sizeof(aBufKick), "'%s' called for vote to kick you", Server()->ClientName(ClientID));
					SendChatTarget(KickID, aBufKick);
					return;
				}

				str_format(aChatmsg, sizeof(aChatmsg), "'%s' called for vote to kick '%s' (%s)", Server()->ClientName(ClientID), Server()->ClientName(KickID), pReason);
				str_format(aDesc, sizeof(aDesc), "Kick '%s'", Server()->ClientName(KickID));
				if (!g_Config.m_SvVoteKickBantime)
					str_format(aCmd, sizeof(aCmd), "kick %d Kicked by vote", KickID);
				else
				{
					char aAddrStr[NETADDR_MAXSTRSIZE] = {0};
					Server()->GetClientAddr(KickID, aAddrStr, sizeof(aAddrStr));
					str_format(aCmd, sizeof(aCmd), "ban %s %d Banned by vote", aAddrStr, g_Config.m_SvVoteKickBantime);
				}
			}
			else if(str_comp_nocase(pMsg->m_Type, "spectate") == 0)
			{
				if(!g_Config.m_SvVoteSpectate)
				{
					SendChatTarget(ClientID, "Server does not allow voting to move players to spectators");
					return;
				}

				int SpectateID = str_toint(pMsg->m_Value);
				if(SpectateID < 0 || SpectateID >= MAX_CLIENTS || !m_apPlayers[SpectateID] || m_apPlayers[SpectateID]->GetTeam() == TEAM_SPECTATORS)
				{
					SendChatTarget(ClientID, "Invalid client id to move");
					return;
				}
				if(SpectateID == ClientID)
				{
					SendChatTarget(ClientID, "You can't move yourself");
					return;
				}

				str_format(aChatmsg, sizeof(aChatmsg), "'%s' called for vote to move '%s' to spectators (%s)", Server()->ClientName(ClientID), Server()->ClientName(SpectateID), pReason);
				str_format(aDesc, sizeof(aDesc), "move '%s' to spectators", Server()->ClientName(SpectateID));
				str_format(aCmd, sizeof(aCmd), "set_team %d -1 %d", SpectateID, g_Config.m_SvVoteSpectateRejoindelay);
			}
			
			// do nothing
			if(str_comp(aCmd, "null") == 0)
			{
				return;
			}

			if(aCmd[0])
			{
				SendChat(-1, CGameContext::CHAT_ALL, aChatmsg);
				StartVote(aDesc, aCmd, pReason);
				pPlayer->m_Vote = 1;
				pPlayer->m_VotePos = m_VotePos = 1;
				m_VoteCreator = ClientID;
				pPlayer->m_LastVoteCall = Now;
			}
		}
		else if(MsgID == NETMSGTYPE_CL_VOTE)
		{
			if(!m_VoteCloseTime)
				return;

			if(pPlayer->m_Vote == 0)
			{
				CNetMsg_Cl_Vote *pMsg = (CNetMsg_Cl_Vote *)pRawMsg;
				if(!pMsg->m_Vote)
					return;

				pPlayer->m_Vote = pMsg->m_Vote;
				pPlayer->m_VotePos = ++m_VotePos;
				m_VoteUpdate = true;
			}
		}
		else if (MsgID == NETMSGTYPE_CL_SETTEAM && !m_World.m_Paused)
		{
			CNetMsg_Cl_SetTeam *pMsg = (CNetMsg_Cl_SetTeam *)pRawMsg;

			if(pPlayer->GetTeam() == pMsg->m_Team || (g_Config.m_SvSpamprotection && pPlayer->m_LastSetTeam && pPlayer->m_LastSetTeam+Server()->TickSpeed()*1 > Server()->Tick()))
				return;
			
			pPlayer->m_LastSetTeam = Server()->Tick();
			if(pPlayer->GetTeam() == TEAM_SPECTATORS || pMsg->m_Team == TEAM_SPECTATORS)
				m_VoteUpdate = true;
			
			pPlayer->SetTeam(pMsg->m_Team);
			pPlayer->m_TeamChangeTick = Server()->Tick();
			
			/*
			if(pMsg->m_Team != TEAM_SPECTATORS && m_LockTeams)
			{
				pPlayer->m_LastSetTeam = Server()->Tick();
				SendBroadcast("Teams are locked", ClientID);
				return;
			}
			*/

			/*
			if(pPlayer->m_TeamChangeTick > Server()->Tick())
			{
				pPlayer->m_LastSetTeam = Server()->Tick();
				int TimeLeft = (pPlayer->m_TeamChangeTick - Server()->Tick())/Server()->TickSpeed();
				char aBuf[128];
				str_format(aBuf, sizeof(aBuf), "Time to wait before changing team: %02d:%02d", TimeLeft/60, TimeLeft%60);
				SendBroadcast(aBuf, ClientID);
				return;
			}
			

			// Switch team on given client and kill/respawn him
			if(m_pController->CanJoinTeam(pMsg->m_Team, ClientID))
			{
				if(m_pController->CanChangeTeam(pPlayer, pMsg->m_Team))
				{
					pPlayer->m_LastSetTeam = Server()->Tick();
					if(pPlayer->GetTeam() == TEAM_SPECTATORS || pMsg->m_Team == TEAM_SPECTATORS)
						m_VoteUpdate = true;
					pPlayer->SetTeam(pMsg->m_Team);
					//(void)m_pController->CheckTeamBalance();
					pPlayer->m_TeamChangeTick = Server()->Tick();
				}
				//else
				//	SendBroadcast("Teams must be balanced, please join other team", ClientID);
			}
			else
			{
				char aBuf[128];
				str_format(aBuf, sizeof(aBuf), "Only %d active players are allowed", Server()->MaxClients()-g_Config.m_SvSpectatorSlots);
				SendBroadcast(aBuf, ClientID);
			}
			*/
		}
		else if (MsgID == NETMSGTYPE_CL_SETSPECTATORMODE && !m_World.m_Paused)
		{
			CNetMsg_Cl_SetSpectatorMode *pMsg = (CNetMsg_Cl_SetSpectatorMode *)pRawMsg;

			pPlayer->m_ActionSpectator = false;
			
			if((pPlayer->GetTeam() != TEAM_SPECTATORS && !g_Config.m_SvSurvivalMode) || 
				pPlayer->m_SpectatorID == pMsg->m_SpectatorID || ClientID == pMsg->m_SpectatorID ||
				(g_Config.m_SvSpamprotection && pPlayer->m_LastSetSpectatorMode && pPlayer->m_LastSetSpectatorMode+Server()->TickSpeed()*1 > Server()->Tick()))
				return;

			pPlayer->m_LastSetSpectatorMode = Server()->Tick();
			if(pMsg->m_SpectatorID != SPEC_FREEVIEW && (!m_apPlayers[pMsg->m_SpectatorID] || m_apPlayers[pMsg->m_SpectatorID]->GetTeam() == TEAM_SPECTATORS))
				SendChatTarget(ClientID, "Invalid spectator id used");
			else
				pPlayer->m_SpectatorID = pMsg->m_SpectatorID;
		}
		else if (MsgID == NETMSGTYPE_CL_CHANGEINFO)
		{
			if(g_Config.m_SvSpamprotection && pPlayer->m_LastChangeInfo && pPlayer->m_LastChangeInfo+Server()->TickSpeed()*5 > Server()->Tick())
				return;

			CNetMsg_Cl_ChangeInfo *pMsg = (CNetMsg_Cl_ChangeInfo *)pRawMsg;
			pPlayer->m_LastChangeInfo = Server()->Tick();

			// set infos
			char aOldName[MAX_NAME_LENGTH];
			str_copy(aOldName, Server()->ClientName(ClientID), sizeof(aOldName));
			Server()->SetClientName(ClientID, pMsg->m_pName);
			if(str_comp(aOldName, Server()->ClientName(ClientID)) != 0)
			{
				char aChatText[256];
				str_format(aChatText, sizeof(aChatText), "'%s' changed name to '%s'", aOldName, Server()->ClientName(ClientID));
				SendChat(-1, CGameContext::CHAT_ALL, aChatText);
			}
			Server()->SetClientClan(ClientID, pMsg->m_pClan);
			Server()->SetClientCountry(ClientID, pMsg->m_Country);
			str_copy(pPlayer->m_TeeInfos.m_TopperName, pMsg->m_pTopper, sizeof(pPlayer->m_TeeInfos.m_TopperName));
			str_copy(pPlayer->m_TeeInfos.m_EyeName, pMsg->m_pEye, sizeof(pPlayer->m_TeeInfos.m_EyeName));
			pPlayer->m_TeeInfos.m_Body = pMsg->m_Body;
			pPlayer->m_TeeInfos.m_ColorBody = pMsg->m_ColorBody;
			pPlayer->m_TeeInfos.m_ColorFeet = pMsg->m_ColorFeet;
			pPlayer->m_TeeInfos.m_ColorTopper = pMsg->m_ColorTopper;
			pPlayer->m_TeeInfos.m_BloodColor = pMsg->m_BloodColor;
			pPlayer->m_TeeInfos.m_ColorSkin = pMsg->m_ColorSkin;
			m_pController->OnPlayerInfoChange(pPlayer);
		}
		else if (MsgID == NETMSGTYPE_CL_EMOTICON && !m_World.m_Paused)
		{
			CNetMsg_Cl_Emoticon *pMsg = (CNetMsg_Cl_Emoticon *)pRawMsg;

			if(g_Config.m_SvSpamprotection && pPlayer->m_LastEmote && pPlayer->m_LastEmote+Server()->TickSpeed()*1 > Server()->Tick())
				return;

			pPlayer->m_LastEmote = Server()->Tick();
			
			if ((pMsg->m_Emoticon == EMOTICON_EYES || pMsg->m_Emoticon == EMOTICON_HEARTS) && pPlayer->GetCharacter())
				pPlayer->GetCharacter()->SetEmote(EMOTE_HAPPY, Server()->Tick() + Server()->TickSpeed());
			
			if ((pMsg->m_Emoticon == EMOTICON_SPLATTEE || pMsg->m_Emoticon == EMOTICON_DEVILTEE || pMsg->m_Emoticon == EMOTICON_ZOMG) && pPlayer->GetCharacter())
				pPlayer->GetCharacter()->SetEmote(EMOTE_ANGRY, Server()->Tick() + Server()->TickSpeed());
			
			SendEmoticon(ClientID, pMsg->m_Emoticon);
		}
		else if (MsgID == NETMSGTYPE_CL_DROPWEAPON && !m_World.m_Paused)
		{
			pPlayer->DropWeapon();
		}
		else if (MsgID == NETMSGTYPE_CL_SELECTITEM && !m_World.m_Paused)
		{
			CNetMsg_Cl_SelectItem *pMsg = (CNetMsg_Cl_SelectItem *)pRawMsg;
			pPlayer->SelectItem(pMsg->m_Item);
		}
		else if (MsgID == NETMSGTYPE_CL_INVENTORYACTION)
		{
			CNetMsg_Cl_InventoryAction *pMsg = (CNetMsg_Cl_InventoryAction *)pRawMsg;
			switch (pMsg->m_Type)
			{
				case INVENTORYACTION_DROP: pPlayer->DropItem(pMsg->m_Slot, vec2(pMsg->m_Item1, pMsg->m_Item2)); break;
				case INVENTORYACTION_SWAP: pPlayer->SwapItem(pMsg->m_Item1, pMsg->m_Item2); break;
				case INVENTORYACTION_COMBINE: pPlayer->CombineItem(pMsg->m_Item1, pMsg->m_Item2); break;
				case INVENTORYACTION_TAKEPART: pPlayer->TakePart(pMsg->m_Item1, pMsg->m_Slot, pMsg->m_Item2); break;
				default: return;
			};
		}
		else if (MsgID == NETMSGTYPE_CL_USEKIT && !m_World.m_Paused)
		{
			CNetMsg_Cl_UseKit *pMsg = (CNetMsg_Cl_UseKit *)pRawMsg;
			pPlayer->UseKit(pMsg->m_Kit, vec2(pMsg->m_X, pMsg->m_Y));
		}
		else if (MsgID == NETMSGTYPE_CL_KILL && !m_World.m_Paused)
		{
			if(pPlayer->m_LastKill && pPlayer->m_LastKill+Server()->TickSpeed()*1 > Server()->Tick())
				return;

			//pPlayer->m_LastKill = Server()->Tick();
			//pPlayer->KillCharacter(WEAPON_SELF);
		}
	}
	else
	{
		// bots skip sending this info
		if(MsgID == NETMSGTYPE_CL_STARTINFO)
		{
			// limit players to 4 in invasion
			if (m_pController->IsCoop() && m_pController->CountHumans() > 16)
				Server()->Kick(ClientID, "Server full - max 16 players in co-op modes");
			
			if(pPlayer->m_IsReady)
				return;

			CNetMsg_Cl_StartInfo *pMsg = (CNetMsg_Cl_StartInfo *)pRawMsg;
			pPlayer->m_LastChangeInfo = Server()->Tick();

			// set start infos
			Server()->SetClientName(ClientID, pMsg->m_pName);
			Server()->SetClientClan(ClientID, pMsg->m_pClan);
			Server()->SetClientCountry(ClientID, pMsg->m_Country);
			str_copy(pPlayer->m_TeeInfos.m_TopperName, pMsg->m_pTopper, sizeof(pPlayer->m_TeeInfos.m_TopperName));
			str_copy(pPlayer->m_TeeInfos.m_EyeName, pMsg->m_pEye, sizeof(pPlayer->m_TeeInfos.m_EyeName));
			pPlayer->m_TeeInfos.m_Body = pMsg->m_Body;
			pPlayer->m_TeeInfos.m_ColorBody = pMsg->m_ColorBody;
			pPlayer->m_TeeInfos.m_ColorFeet = pMsg->m_ColorFeet;
			pPlayer->m_TeeInfos.m_ColorTopper = pMsg->m_ColorTopper;
			pPlayer->m_TeeInfos.m_ColorSkin = pMsg->m_ColorSkin;
			pPlayer->m_TeeInfos.m_BloodColor = pMsg->m_BloodColor;
			m_pController->OnPlayerInfoChange(pPlayer);

			// send vote options
			CNetMsg_Sv_VoteClearOptions ClearMsg;
			Server()->SendPackMsg(&ClearMsg, MSGFLAG_VITAL, ClientID);

			CNetMsg_Sv_VoteOptionListAdd OptionMsg;
			int NumOptions = 0;
			OptionMsg.m_pDescription0 = "";
			OptionMsg.m_pDescription1 = "";
			OptionMsg.m_pDescription2 = "";
			OptionMsg.m_pDescription3 = "";
			OptionMsg.m_pDescription4 = "";
			OptionMsg.m_pDescription5 = "";
			OptionMsg.m_pDescription6 = "";
			OptionMsg.m_pDescription7 = "";
			OptionMsg.m_pDescription8 = "";
			OptionMsg.m_pDescription9 = "";
			OptionMsg.m_pDescription10 = "";
			OptionMsg.m_pDescription11 = "";
			OptionMsg.m_pDescription12 = "";
			OptionMsg.m_pDescription13 = "";
			OptionMsg.m_pDescription14 = "";
			CVoteOptionServer *pCurrent = m_pVoteOptionFirst;
			while(pCurrent)
			{
				switch(NumOptions++)
				{
				case 0: OptionMsg.m_pDescription0 = pCurrent->m_aDescription; break;
				case 1: OptionMsg.m_pDescription1 = pCurrent->m_aDescription; break;
				case 2: OptionMsg.m_pDescription2 = pCurrent->m_aDescription; break;
				case 3: OptionMsg.m_pDescription3 = pCurrent->m_aDescription; break;
				case 4: OptionMsg.m_pDescription4 = pCurrent->m_aDescription; break;
				case 5: OptionMsg.m_pDescription5 = pCurrent->m_aDescription; break;
				case 6: OptionMsg.m_pDescription6 = pCurrent->m_aDescription; break;
				case 7: OptionMsg.m_pDescription7 = pCurrent->m_aDescription; break;
				case 8: OptionMsg.m_pDescription8 = pCurrent->m_aDescription; break;
				case 9: OptionMsg.m_pDescription9 = pCurrent->m_aDescription; break;
				case 10: OptionMsg.m_pDescription10 = pCurrent->m_aDescription; break;
				case 11: OptionMsg.m_pDescription11 = pCurrent->m_aDescription; break;
				case 12: OptionMsg.m_pDescription12 = pCurrent->m_aDescription; break;
				case 13: OptionMsg.m_pDescription13 = pCurrent->m_aDescription; break;
				case 14:
					{
						OptionMsg.m_pDescription14 = pCurrent->m_aDescription;
						OptionMsg.m_NumOptions = NumOptions;
						Server()->SendPackMsg(&OptionMsg, MSGFLAG_VITAL, ClientID);
						OptionMsg = CNetMsg_Sv_VoteOptionListAdd();
						NumOptions = 0;
						OptionMsg.m_pDescription1 = "";
						OptionMsg.m_pDescription2 = "";
						OptionMsg.m_pDescription3 = "";
						OptionMsg.m_pDescription4 = "";
						OptionMsg.m_pDescription5 = "";
						OptionMsg.m_pDescription6 = "";
						OptionMsg.m_pDescription7 = "";
						OptionMsg.m_pDescription8 = "";
						OptionMsg.m_pDescription9 = "";
						OptionMsg.m_pDescription10 = "";
						OptionMsg.m_pDescription11 = "";
						OptionMsg.m_pDescription12 = "";
						OptionMsg.m_pDescription13 = "";
						OptionMsg.m_pDescription14 = "";
					}
				}
				pCurrent = pCurrent->m_pNext;
			}
			if(NumOptions > 0)
			{
				OptionMsg.m_NumOptions = NumOptions;
				Server()->SendPackMsg(&OptionMsg, MSGFLAG_VITAL, ClientID);
			}

			// send tuning parameters to client
			SendTuningParams(ClientID);

			// client is ready to enter
			pPlayer->m_IsReady = true;
			CNetMsg_Sv_ReadyToEnter m;
			Server()->SendPackMsg(&m, MSGFLAG_VITAL|MSGFLAG_FLUSH, ClientID);
		}
	}
}

void CGameContext::ConTuneParam(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	const char *pParamName = pResult->GetString(0);
	float NewValue = pResult->GetFloat(1);

	if(pSelf->Tuning()->Set(pParamName, NewValue))
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "%s changed to %.2f", pParamName, NewValue);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", aBuf);
		pSelf->SendTuningParams(-1);
	}
	else
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", "No such tuning parameter");
}

void CGameContext::ConTuneReset(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CTuningParams TuningParams;
	*pSelf->Tuning() = TuningParams;
	pSelf->SendTuningParams(-1);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", "Tuning reset");
}

void CGameContext::ConTuneDump(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	char aBuf[256];
	for(int i = 0; i < pSelf->Tuning()->Num(); i++)
	{
		float v;
		pSelf->Tuning()->Get(i, &v);
		str_format(aBuf, sizeof(aBuf), "%s %.2f", pSelf->Tuning()->m_apNames[i], v);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", aBuf);
	}
}

void CGameContext::ConPause(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	if(pSelf->m_pController->IsGameOver())
		return;

	pSelf->m_World.m_Paused ^= 1;
}

void CGameContext::ConChangeMap(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->m_pController->ChangeMap(pResult->NumArguments() ? pResult->GetString(0) : "");
}

void CGameContext::ConRestart(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(pResult->NumArguments())
		pSelf->m_pController->DoWarmup(pResult->GetInteger(0));
	else
		pSelf->m_pController->StartRound();
}

void CGameContext::ConBroadcast(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->SendBroadcast(pResult->GetString(0), -1);
}

void CGameContext::ConSay(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->SendChat(-1, CGameContext::CHAT_ALL, pResult->GetString(0));
}

void CGameContext::ConSetTeam(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int ClientID = clamp(pResult->GetInteger(0), 0, (int)MAX_CLIENTS-1);
	int Team = clamp(pResult->GetInteger(1), -1, 1);
	int Delay = pResult->NumArguments()>2 ? pResult->GetInteger(2) : 0;
	if(!pSelf->m_apPlayers[ClientID])
		return;

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "moved client %d to team %d", ClientID, Team);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);

	pSelf->m_apPlayers[ClientID]->m_TeamChangeTick = pSelf->Server()->Tick()+pSelf->Server()->TickSpeed()*Delay*60;
	pSelf->m_apPlayers[ClientID]->SetTeam(Team);
	(void)pSelf->m_pController->CheckTeamBalance();
}

void CGameContext::ConSetTeamAll(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Team = clamp(pResult->GetInteger(0), -1, 1);

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "All players were moved to the %s", pSelf->m_pController->GetTeamName(Team));
	pSelf->SendChat(-1, CGameContext::CHAT_ALL, aBuf);

	for(int i = 0; i < MAX_CLIENTS; ++i)
		if(pSelf->m_apPlayers[i])
			pSelf->m_apPlayers[i]->SetTeam(Team, false);

	(void)pSelf->m_pController->CheckTeamBalance();
}

void CGameContext::ConSwapTeams(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->SwapTeams();
}

void CGameContext::ConShuffleTeams(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!pSelf->m_pController->IsTeamplay() || pSelf->m_pController->IsInfection())
		return;

	int PlayerTeam = 0;
	for(int i = 0; i < MAX_CLIENTS; ++i)
		if(pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
			++PlayerTeam;
	PlayerTeam = (PlayerTeam+1)/2;
	
	pSelf->SendChat(-1, CGameContext::CHAT_ALL, "Teams were shuffled");

	/*
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
		{
			if(CounterRed == PlayerTeam)
				pSelf->m_apPlayers[i]->SetTeam(TEAM_BLUE, false);
			else if(CounterBlue == PlayerTeam)
				pSelf->m_apPlayers[i]->SetTeam(TEAM_RED, false);
			else
			{	
				if(rand() % 2)
				{
					pSelf->m_apPlayers[i]->SetTeam(TEAM_BLUE, false);
					++CounterBlue;
				}
				else
				{
					pSelf->m_apPlayers[i]->SetTeam(TEAM_RED, false);
					++CounterRed;
				}
			}
		}
	}
	*/

	(void)pSelf->m_pController->CheckTeamBalance();
}

void CGameContext::ConLockTeams(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->m_LockTeams ^= 1;
	if(pSelf->m_LockTeams)
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "Teams were locked");
	else
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "Teams were unlocked");
}

void CGameContext::ConAddVote(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	const char *pDescription = pResult->GetString(0);
	const char *pCommand = pResult->GetString(1);

	if(pSelf->m_NumVoteOptions == MAX_VOTE_OPTIONS)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "maximum number of vote options reached");
		return;
	}

	// check for valid option
	if(!pSelf->Console()->LineIsValid(pCommand) || str_length(pCommand) >= VOTE_CMD_LENGTH)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "skipped invalid command '%s'", pCommand);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
		return;
	}
	while(*pDescription && *pDescription == ' ')
		pDescription++;
	if(str_length(pDescription) >= VOTE_DESC_LENGTH || *pDescription == 0)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "skipped invalid option '%s'", pDescription);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
		return;
	}

	// check for duplicate entry
	CVoteOptionServer *pOption = pSelf->m_pVoteOptionFirst;
	while(pOption)
	{
		if(str_comp_nocase(pDescription, pOption->m_aDescription) == 0)
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "option '%s' already exists", pDescription);
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
			return;
		}
		pOption = pOption->m_pNext;
	}

	// add the option
	++pSelf->m_NumVoteOptions;
	int Len = str_length(pCommand);

	pOption = (CVoteOptionServer *)pSelf->m_pVoteOptionHeap->Allocate(sizeof(CVoteOptionServer) + Len);
	pOption->m_pNext = 0;
	pOption->m_pPrev = pSelf->m_pVoteOptionLast;
	if(pOption->m_pPrev)
		pOption->m_pPrev->m_pNext = pOption;
	pSelf->m_pVoteOptionLast = pOption;
	if(!pSelf->m_pVoteOptionFirst)
		pSelf->m_pVoteOptionFirst = pOption;

	str_copy(pOption->m_aDescription, pDescription, sizeof(pOption->m_aDescription));
	mem_copy(pOption->m_aCommand, pCommand, Len+1);
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "added option '%s' '%s'", pOption->m_aDescription, pOption->m_aCommand);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);

	// inform clients about added option
	CNetMsg_Sv_VoteOptionAdd OptionMsg;
	OptionMsg.m_pDescription = pOption->m_aDescription;
	pSelf->Server()->SendPackMsg(&OptionMsg, MSGFLAG_VITAL, -1);
}

void CGameContext::ConRemoveVote(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	const char *pDescription = pResult->GetString(0);

	// check for valid option
	CVoteOptionServer *pOption = pSelf->m_pVoteOptionFirst;
	while(pOption)
	{
		if(str_comp_nocase(pDescription, pOption->m_aDescription) == 0)
			break;
		pOption = pOption->m_pNext;
	}
	if(!pOption)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "option '%s' does not exist", pDescription);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
		return;
	}

	// inform clients about removed option
	CNetMsg_Sv_VoteOptionRemove OptionMsg;
	OptionMsg.m_pDescription = pOption->m_aDescription;
	pSelf->Server()->SendPackMsg(&OptionMsg, MSGFLAG_VITAL, -1);

	// TODO: improve this
	// remove the option
	--pSelf->m_NumVoteOptions;
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "removed option '%s' '%s'", pOption->m_aDescription, pOption->m_aCommand);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);

	CHeap *pVoteOptionHeap = new CHeap();
	CVoteOptionServer *pVoteOptionFirst = 0;
	CVoteOptionServer *pVoteOptionLast = 0;
	int NumVoteOptions = pSelf->m_NumVoteOptions;
	for(CVoteOptionServer *pSrc = pSelf->m_pVoteOptionFirst; pSrc; pSrc = pSrc->m_pNext)
	{
		if(pSrc == pOption)
			continue;

		// copy option
		int Len = str_length(pSrc->m_aCommand);
		CVoteOptionServer *pDst = (CVoteOptionServer *)pVoteOptionHeap->Allocate(sizeof(CVoteOptionServer) + Len);
		pDst->m_pNext = 0;
		pDst->m_pPrev = pVoteOptionLast;
		if(pDst->m_pPrev)
			pDst->m_pPrev->m_pNext = pDst;
		pVoteOptionLast = pDst;
		if(!pVoteOptionFirst)
			pVoteOptionFirst = pDst;

		str_copy(pDst->m_aDescription, pSrc->m_aDescription, sizeof(pDst->m_aDescription));
		mem_copy(pDst->m_aCommand, pSrc->m_aCommand, Len+1);
	}

	// clean up
	delete pSelf->m_pVoteOptionHeap;
	pSelf->m_pVoteOptionHeap = pVoteOptionHeap;
	pSelf->m_pVoteOptionFirst = pVoteOptionFirst;
	pSelf->m_pVoteOptionLast = pVoteOptionLast;
	pSelf->m_NumVoteOptions = NumVoteOptions;
}

void CGameContext::ConForceVote(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	const char *pType = pResult->GetString(0);
	const char *pValue = pResult->GetString(1);
	const char *pReason = pResult->NumArguments() > 2 && pResult->GetString(2)[0] ? pResult->GetString(2) : "No reason given";
	char aBuf[128] = {0};

	if(str_comp_nocase(pType, "option") == 0)
	{
		CVoteOptionServer *pOption = pSelf->m_pVoteOptionFirst;
		while(pOption)
		{
			if(str_comp_nocase(pValue, pOption->m_aDescription) == 0)
			{
				str_format(aBuf, sizeof(aBuf), "admin forced server option '%s' (%s)", pValue, pReason);
				pSelf->SendChatTarget(-1, aBuf);
				pSelf->Console()->ExecuteLine(pOption->m_aCommand);
				break;
			}

			pOption = pOption->m_pNext;
		}

		if(!pOption)
		{
			str_format(aBuf, sizeof(aBuf), "'%s' isn't an option on this server", pValue);
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
			return;
		}
	}
	else if(str_comp_nocase(pType, "kick") == 0)
	{
		int KickID = str_toint(pValue);
		if(KickID < 0 || KickID >= MAX_CLIENTS || !pSelf->m_apPlayers[KickID])
		{
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "Invalid client id to kick");
			return;
		}

		if (!g_Config.m_SvVoteKickBantime)
		{
			str_format(aBuf, sizeof(aBuf), "kick %d %s", KickID, pReason);
			pSelf->Console()->ExecuteLine(aBuf);
		}
		else
		{
			char aAddrStr[NETADDR_MAXSTRSIZE] = {0};
			pSelf->Server()->GetClientAddr(KickID, aAddrStr, sizeof(aAddrStr));
			str_format(aBuf, sizeof(aBuf), "ban %s %d %s", aAddrStr, g_Config.m_SvVoteKickBantime, pReason);
			pSelf->Console()->ExecuteLine(aBuf);
		}
	}
	else if(str_comp_nocase(pType, "spectate") == 0)
	{
		int SpectateID = str_toint(pValue);
		if(SpectateID < 0 || SpectateID >= MAX_CLIENTS || !pSelf->m_apPlayers[SpectateID] || pSelf->m_apPlayers[SpectateID]->GetTeam() == TEAM_SPECTATORS)
		{
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "Invalid client id to move");
			return;
		}

		str_format(aBuf, sizeof(aBuf), "admin moved '%s' to spectator (%s)", pSelf->Server()->ClientName(SpectateID), pReason);
		pSelf->SendChatTarget(-1, aBuf);
		str_format(aBuf, sizeof(aBuf), "set_team %d -1 %d", SpectateID, g_Config.m_SvVoteSpectateRejoindelay);
		pSelf->Console()->ExecuteLine(aBuf);
	}
}

void CGameContext::ReloadMap()
{
	Console()->ExecuteLine("reload");
}



void CGameContext::ConClearVotes(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "cleared votes");
	CNetMsg_Sv_VoteClearOptions VoteClearOptionsMsg;
	pSelf->Server()->SendPackMsg(&VoteClearOptionsMsg, MSGFLAG_VITAL, -1);
	pSelf->m_pVoteOptionHeap->Reset();
	pSelf->m_pVoteOptionFirst = 0;
	pSelf->m_pVoteOptionLast = 0;
	pSelf->m_NumVoteOptions = 0;
}

void CGameContext::ConVote(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	// check if there is a vote running
	if(!pSelf->m_VoteCloseTime)
		return;

	if(str_comp_nocase(pResult->GetString(0), "yes") == 0)
		pSelf->m_VoteEnforce = CGameContext::VOTE_ENFORCE_YES;
	else if(str_comp_nocase(pResult->GetString(0), "no") == 0)
		pSelf->m_VoteEnforce = CGameContext::VOTE_ENFORCE_NO;
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "admin forced vote %s", pResult->GetString(0));
	pSelf->SendChatTarget(-1, aBuf);
	str_format(aBuf, sizeof(aBuf), "forcing vote %s", pResult->GetString(0));
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
}

void CGameContext::ConchainSpecialMotdupdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->NumArguments())
	{
		CNetMsg_Sv_Motd Msg;
		Msg.m_pMessage = g_Config.m_SvMotd;
		CGameContext *pSelf = (CGameContext *)pUserData;
		for(int i = 0; i < MAX_CLIENTS; ++i)
			if(pSelf->m_apPlayers[i] && !pSelf->IsBot(i))
				pSelf->Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, i);
	}
}

void CGameContext::OnConsoleInit()
{
	m_pServer = Kernel()->RequestInterface<IServer>();
	m_pConsole = Kernel()->RequestInterface<IConsole>();

	Console()->Register("tune", "si", CFGFLAG_SERVER, ConTuneParam, this, "Tune variable to value");
	Console()->Register("tune_reset", "", CFGFLAG_SERVER, ConTuneReset, this, "Reset tuning");
	Console()->Register("tune_dump", "", CFGFLAG_SERVER, ConTuneDump, this, "Dump tuning");

	Console()->Register("pause", "", CFGFLAG_SERVER, ConPause, this, "Pause/unpause game");
	Console()->Register("change_map", "?r", CFGFLAG_SERVER|CFGFLAG_STORE, ConChangeMap, this, "Change map");
	Console()->Register("restart", "?i", CFGFLAG_SERVER|CFGFLAG_STORE, ConRestart, this, "Restart in x seconds (0 = abort)");
	Console()->Register("broadcast", "r", CFGFLAG_SERVER, ConBroadcast, this, "Broadcast message");
	Console()->Register("say", "r", CFGFLAG_SERVER, ConSay, this, "Say in chat");
	Console()->Register("set_team", "ii?i", CFGFLAG_SERVER, ConSetTeam, this, "Set team of player to team");
	Console()->Register("set_team_all", "i", CFGFLAG_SERVER, ConSetTeamAll, this, "Set team of all players to team");
	Console()->Register("swap_teams", "", CFGFLAG_SERVER, ConSwapTeams, this, "Swap the current teams");
	Console()->Register("shuffle_teams", "", CFGFLAG_SERVER, ConShuffleTeams, this, "Shuffle the current teams");
	Console()->Register("lock_teams", "", CFGFLAG_SERVER, ConLockTeams, this, "Lock/unlock teams");

	Console()->Register("add_vote", "sr", CFGFLAG_SERVER, ConAddVote, this, "Add a voting option");
	Console()->Register("remove_vote", "s", CFGFLAG_SERVER, ConRemoveVote, this, "remove a voting option");
	Console()->Register("force_vote", "ss?r", CFGFLAG_SERVER, ConForceVote, this, "Force a voting option");
	Console()->Register("clear_votes", "", CFGFLAG_SERVER, ConClearVotes, this, "Clears the voting options");
	Console()->Register("vote", "r", CFGFLAG_SERVER, ConVote, this, "Force a vote to yes/no");

	Console()->Chain("sv_motd", ConchainSpecialMotdupdate, this);
}

void CGameContext::OnInit(/*class IKernel *pKernel*/)
{
	m_pServer = Kernel()->RequestInterface<IServer>();
	m_pConsole = Kernel()->RequestInterface<IConsole>();
	m_pStorage = Kernel()->RequestInterface<IStorage>(); // MapGen
	m_World.SetGameServer(this);
	m_Events.SetGameServer(this);

	//if(!data) // only load once
		//data = load_data_from_memory(internal_data);

	for(int i = 0; i < NUM_NETOBJTYPES; i++)
		Server()->SnapSetStaticsize(i, m_NetObjHandler.GetObjSize(i));

	m_Layers.Init(Kernel());
	m_Collision.Init(&m_Layers);
	m_MapGen.Init(&m_Layers, &m_Collision, m_pStorage); // MapGen

	// reset everything here
	//world = new GAMEWORLD;
	//players = new CPlayer[MAX_CLIENTS];

	// select gametype
	if(str_comp(g_Config.m_SvGametype, "ctf") == 0)
		m_pController = new CGameControllerCTF(this);
	else if(str_comp(g_Config.m_SvGametype, "tdm") == 0)
		m_pController = new CGameControllerTDM(this);
	else if(str_comp(g_Config.m_SvGametype, "inf") == 0)
		m_pController = new CGameControllerTexasRun(this);
	else if(str_comp(g_Config.m_SvGametype, "base") == 0)
		m_pController = new CGameControllerBase(this);
	else if(str_comp(g_Config.m_SvGametype, "coop") == 0)
		m_pController = new CGameControllerCoop(this);
	else
		m_pController = new CGameControllerDM(this);
	
	if (str_comp(g_Config.m_SvGametype, "coop") != 0)
		Server()->ResetPlayerData();
		

	// MapGen
	//if (str_comp(g_Config.m_SvGametype, "coop") == 0 && g_Config.m_SvMapGen && !m_pServer->m_MapGenerated)
	if (g_Config.m_SvMapGen && !m_pServer->m_MapGenerated)
	{
		m_MapGen.FillMap();
		SaveMap("");
		
		str_copy(g_Config.m_SvMap, "generated", sizeof(g_Config.m_SvMap));
		m_pServer->m_MapGenerated = true;
	}

	// setup core world
	//for(int i = 0; i < MAX_CLIENTS; i++)
	//	game.players[i].core.world = &game.world.core;

	// create all entities from the game layer
	CMapItemLayerTilemap *pTileMap = m_Layers.GameLayer();
	CTile *pTiles = (CTile *)Kernel()->RequestInterface<IMap>()->GetData(pTileMap->m_Data);




	/*
	num_spawn_points[0] = 0;
	num_spawn_points[1] = 0;
	num_spawn_points[2] = 0;
	*/

	for(int y = 0; y < pTileMap->m_Height; y++)
	{
		for(int x = 0; x < pTileMap->m_Width; x++)
		{
			int Index = pTiles[y*pTileMap->m_Width+x].m_Index;

			if(Index >= ENTITY_OFFSET)
			{
				vec2 Pos(x*32.0f+16.0f, y*32.0f+16.0f);
				m_pController->OnEntity(Index-ENTITY_OFFSET, Pos);
			}
		}
	}
	

	//game.world.insert_entity(game.Controller);

	//SetupVotes(-1);
	
	/*
	CNetMsg_Sv_VoteClearOptions VoteClearOptionsMsg;
	Server()->SendPackMsg(&VoteClearOptionsMsg, MSGFLAG_VITAL, -1);
	
	m_pVoteOptionHeap->Reset();
	m_pVoteOptionFirst = 0;
	m_pVoteOptionLast = 0;
	m_NumVoteOptions = 0;
	*/
}





void CGameContext::OnShutdown()
{
	KickBots();
	
	if (str_comp(m_pController->m_pGameType, "INV") == 0)
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
			if (m_apPlayers[i])
				m_apPlayers[i]->SaveData();
	}
	else
		Server()->ResetPlayerData();
		
	delete m_pController;
	m_pController = 0;
	Clear();
}

void CGameContext::OnSnap(int ClientID)
{
	// add tuning to demo
	CTuningParams StandardTuning;
	if(ClientID == -1 && Server()->DemoRecorder_IsRecording() && mem_comp(&StandardTuning, &m_Tuning, sizeof(CTuningParams)) != 0)
	{
		CMsgPacker Msg(NETMSGTYPE_SV_TUNEPARAMS);
		int *pParams = (int *)&m_Tuning;
		for(unsigned i = 0; i < sizeof(m_Tuning)/sizeof(int); i++)
			Msg.AddInt(pParams[i]);
		Server()->SendMsg(&Msg, MSGFLAG_RECORD|MSGFLAG_NOSEND, ClientID);
	}

	m_World.Snap(ClientID);
	m_pController->Snap(ClientID);
	m_Events.Snap(ClientID);

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
			m_apPlayers[i]->Snap(ClientID);
	}
}
void CGameContext::OnPreSnap() {}
void CGameContext::OnPostSnap()
{
	m_Events.Clear();
}

bool CGameContext::IsClientReady(int ClientID)
{
	return m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_IsReady ? true : false;
}

bool CGameContext::IsClientPlayer(int ClientID)
{
	return m_apPlayers[ClientID] && m_apPlayers[ClientID]->GetTeam() == TEAM_SPECTATORS ? false : true;
}

const char *CGameContext::GameType() { return m_pController && m_pController->m_pGameType ? m_pController->m_pGameType : ""; }
const char *CGameContext::Version() { return GAME_VERSION; }
const char *CGameContext::NetVersion() { return GAME_NETVERSION; }

IGameServer *CreateGameServer() { return new CGameContext; }



void CGameContext::KickBots()
{
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "engine", "Kicking bots...");

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(IsBot(i))
			Server()->Kick(i, "");
	}
}


void CGameContext::KickBot(int ClientID)
{
	if (ClientID < 0 || ClientID >= MAX_CLIENTS)
		return;
	
	if (m_apPlayers[ClientID]->GetCharacter())
		m_apPlayers[ClientID]->GetCharacter()->Die(ClientID, WEAPON_WORLD, true);
	
	if(IsBot(ClientID))
		Server()->Kick(ClientID, "");
}



void CGameContext::AddBot()
{
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "engine", "Adding a bot...");

	/*
	// find first free slot
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_apPlayers[i])
		{
			Server()->AddZombie(i);
			return;
		}
	}
	*/
	Server()->AddZombie();
}




int CGameContext::CountBots(bool SkipSpecialTees)
{
	int n = 0;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (IsBot(i))
		{
			if (SkipSpecialTees)
			{
				if (m_apPlayers[i]->m_pAI && m_apPlayers[i]->m_pAI->m_Special < 0)
					n++;
			}
			else
				n++;
		}
	}
	
	return n;
}

/*
int CGameContext::CountHumans()
{
	int n = 0;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (IsHuman(i))
			n++;
	}
	
	return n;
}
*/

int CGameContext::CountBotsAlive(bool SkipSpecialTees)
{
	int n = 0;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (IsBot(i) && m_apPlayers[i]->GetCharacter() && m_apPlayers[i]->GetCharacter()->IsAlive())
		{
			if (SkipSpecialTees)
			{
				if (m_apPlayers[i]->m_pAI && m_apPlayers[i]->m_pAI->m_Special < 0)
					n++;
			}
			else
				n++;
		}
	}
	
	return n;
}

int CGameContext::CountHumansAlive()
{
	int n = 0;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (IsHuman(i) && m_apPlayers[i]->GetCharacter() && m_apPlayers[i]->GetCharacter()->IsAlive())
			n++;
	}
	
	return n;
}





int CGameContext::DistanceToHuman(vec2 Pos)
{
	int MinDist = 10000;
		
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && !m_apPlayers[i]->m_IsBot && m_apPlayers[i]->GetCharacter())
		{
			if (distance(Pos, m_apPlayers[i]->GetCharacter()->m_Pos) < MinDist)
				MinDist = distance(Pos, m_apPlayers[i]->GetCharacter()->m_Pos);
		}		
	}
	
	return MinDist;
}


vec2 CGameContext::GetNearHumanSpawnPos(bool AllowVision)
{
	int n = 0;
	vec2 ReturnPos = Collision()->GetRandomWaypointPos();
	int Dist = 100000;
	
	
	while (n++ < 50)
	{
		vec2 Pos = Collision()->GetRandomWaypointPos();
		
		bool Valid = true;
		int MinDist = 10000;
		
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (m_apPlayers[i] && !m_apPlayers[i]->m_IsBot && m_apPlayers[i]->GetCharacter())
			{
				vec2 PlayerPos = m_apPlayers[i]->GetCharacter()->m_Pos;
				
				if (!AllowVision && abs(Pos.x - PlayerPos.x) < 1200 && abs(Pos.x - PlayerPos.x) < 900)
				{
					Valid = false;
					break;
				}
				else
				{
					
					if (distance(Pos, PlayerPos) < MinDist)
						MinDist = distance(Pos, PlayerPos);
				}
			}
			
		}
		
		if (Valid)
		{
			if (MinDist < 1800)
				return Pos;
			
			if (MinDist < Dist)
			{
				Dist = MinDist;
				ReturnPos = Pos;
			}
		}
	}
	return ReturnPos;
}



vec2 CGameContext::GetFarHumanSpawnPos(bool AllowVision)
{
	int n = 0;
	vec2 ReturnPos = Collision()->GetRandomWaypointPos();
	int Dist = 1;
	
	
	while (n++ < 50)
	{
		vec2 Pos = Collision()->GetRandomWaypointPos();
		
		bool Valid = true;
		int MaxDist = 1;
		
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (m_apPlayers[i] && !m_apPlayers[i]->m_IsBot && m_apPlayers[i]->GetCharacter())
			{
				vec2 PlayerPos = m_apPlayers[i]->GetCharacter()->m_Pos;
				
				if (!AllowVision && abs(Pos.x - PlayerPos.x) < 1200 && abs(Pos.x - PlayerPos.x) < 900)
				{
					Valid = false;
					break;
				}
				else
				{
					
					if (distance(Pos, PlayerPos) > MaxDist)
						MaxDist = distance(Pos, PlayerPos);
				}
			}
			
		}
		
		if (Valid)
		{
			if (MaxDist > 3000)
				return Pos;
			
			if (MaxDist > Dist)
			{
				Dist = MaxDist;
				ReturnPos = Pos;
			}
		}
	}
	return ReturnPos;
}


// MapGen
void CGameContext::SaveMap(const char *path)
{
    IMap *pMap = Layers()->Map();
    if (!pMap)
        return;

    CDataFileWriter fileWrite;
    char aMapFile[512];
	//str_format(aMapFile, sizeof(aMapFile), "maps/%s_%d.map", Server()->GetMapName(), g_Config.m_SvMapGenSeed);
	str_format(aMapFile, sizeof(aMapFile), "maps/generated.map");
		
	// Map will be saved to current dir, not to ~/.ninslash/maps or to data/maps, so we need to create a dir for it
	Storage()->CreateFolder("maps", IStorage::TYPE_SAVE);
		
    fileWrite.SaveMap(Storage(), pMap->GetFileReader(), aMapFile);

    char aBuf[128];
    str_format(aBuf, sizeof(aBuf), "Map saved in '%s'!", aMapFile);
    Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
}

