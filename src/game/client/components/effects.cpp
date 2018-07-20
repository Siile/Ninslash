

#include <engine/demo.h>
#include <engine/engine.h>

#include <engine/shared/config.h>

#include <game/generated/client_data.h>

#include <game/client/components/light.h>
#include <game/client/components/particles.h>
#include <game/client/components/skins.h>
#include <game/client/components/flow.h>
#include <game/client/components/damageind.h>
#include <game/client/components/sounds.h>
#include <game/client/gameclient.h>

#include "effects.h"

#include <game/client/components/blood.h>
#include <game/client/components/guts.h>
#include <game/client/components/brains.h>
#include <game/client/components/splatter.h>
#include <game/client/components/spark.h>

#include <game/client/customstuff.h>
#include <game/client/customstuff/playerinfo.h>

inline vec2 RandomDir() { return normalize(vec2(frandom()-0.5f, frandom()-0.5f)); }

CEffects::CEffects()
{
	m_Add50hz = false;
	m_Add100hz = false;
}

void CEffects::AirJump(vec2 Pos)
{
	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_AIRJUMP;
	p.m_Pos = Pos + vec2(-6.0f, 16.0f);
	p.m_Vel = vec2(0, -200);
	p.m_LifeSpan = 0.5f;
	p.m_StartSize = 48.0f;
	p.m_EndSize = 0;
	p.m_Rot = frandom()*pi*2;
	p.m_Rotspeed = pi*2;
	p.m_Gravity = 500;
	p.m_Friction = 0.7f;
	p.m_FlowAffected = 0.0f;
	m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);

	p.m_Pos = Pos + vec2(6.0f, 16.0f);
	m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);

	m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_PLAYER_AIRJUMP, 1.0f, Pos);
}


void CEffects::Blood(vec2 Pos, vec2 Dir, vec4 Color)
{
	CBlooddrop b;
	b.SetDefault();
	b.m_Pos = Pos;
	
	// bones
	if (frandom()*100 < 2)
	{
		b.m_Spr = SPRITE_BONE01 + (rand()%2);
		b.m_LifeSpan = 8.0f + frandom()*8.0f;
		b.m_Rotspeed = frandom()*12.0f - frandom()*12.0f;
		
		b.m_StartSize = 24.0f + frandom()*24;
		b.m_EndSize = b.m_StartSize;
		b.m_Gravity = 1600.0f;
	}
	else
	// blood
	{
		b.m_Spr = SPRITE_BLOOD01 + (rand()%6);
		b.m_LifeSpan = 6.0f + frandom()*6.0f;
		b.m_Rotspeed = 0.0f;
		b.m_StartSize = 32.0f + frandom()*32;
		b.m_EndSize = 24.0f;
		b.m_Gravity = 1400.0f + frandom()*300;
	}
	
	b.m_Friction = 0.85f+frandom()*0.075f;
	
	if (g_Config.m_GfxMultiBuffering)
	{
		b.m_Spr = SPRITE_BLOOD02;
		b.m_Rotspeed = 0.0f;
		//b.m_StartSize *= 0.75f;
		b.m_StartSize = 64.0f;
		b.m_EndSize = 4.0f;
		b.m_LifeSpan = 8.0f;
		b.m_StartSize = 42.0f + frandom()*16;
		b.m_Gravity = 1500.0f + frandom()*400;
		//b.m_Friction = 0.95f;
	}
	
	b.m_Vel = Dir * ((frandom()+0.50f)*1200.0f);

	b.m_Rot = GetAngle(b.m_Vel);
	
	//b.m_Gravity = 1600.0f;
	//b.m_Friction *= 0.95f;
	//float c = frandom()*0.3f + 0.7f;
	//b.m_Color = vec4(c, c, c, 1.0f);
	b.m_Color = Color;
	m_pClient->m_pBlood->Add(CBlood::GROUP_BLOOD, &b);
}

void CEffects::Guts(vec2 Pos, vec4 Color)
{
	// gut
	vec2 Dir = vec2((frandom()-frandom())*2.0f, -1-frandom()*1.7f);
	{
		CGutSpill b;
		b.SetDefault();
		
		b.m_Spr = 0;
		b.m_Parts = 5;
		b.m_LifeSpan = 4.0f + frandom()*2.0f;
		b.m_StartSize = 32.0f + frandom()*32;
		b.m_EndSize = 24.0f;
		b.m_Gravity = 1400.0f + frandom()*300;
		
		b.m_Friction = 0.85f;
		
		for (int i = 0; i < 5; i++)
		{
			b.m_aPos[i] = Pos + RandomDir()*(frandom()*4.0f);
			b.m_aVel[i] = (Dir + RandomDir()) * ((frandom()+0.50f)*250.0f);
		}
		
		b.m_Color = Color;
		m_pClient->m_pGuts->Add(CGuts::GROUP_GUTS, &b);
	}
	
	// something else
	Dir = vec2((frandom()-frandom())*2.0f, -1-frandom()*1.7f);
	{
		CGutSpill b;
		b.SetDefault();
		
		b.m_Spr = 1;
		b.m_Parts = 5;
		b.m_LifeSpan = 4.0f + frandom()*2.0f;
		b.m_StartSize = 32.0f + frandom()*32;
		b.m_EndSize = 24.0f;
		b.m_ControlDist = 12.0f;
		b.m_Gravity = 1400.0f + frandom()*300;
		
		b.m_Friction = 0.85f;
		
		vec2 Vel = (Dir + RandomDir()) * ((frandom()+0.50f)*250.0f);
		for (int i = 0; i < 5; i++)
		{
			b.m_aPos[i] = Pos + RandomDir()*(frandom()*4.0f);
			b.m_aVel[i] = Vel + RandomDir() * ((frandom()+0.50f)*150.0f);
		}
		
		b.m_Color = Color/3;
		b.m_Color.a = Color.a;
		m_pClient->m_pGuts->Add(CGuts::GROUP_GUTS, &b);
	}
	
	// brain
	Dir = vec2((frandom()-frandom())*4.0f, -1-frandom()*1.7f);
	{
		CBrainSpill b;
		b.SetDefault();
		
		b.m_LifeSpan = 4.0f + frandom()*2.0f;
		b.m_StartSize = 32.0f + frandom()*32;
		b.m_EndSize = 24.0f;
		b.m_Gravity = 1400.0f + frandom()*300;
		
		b.m_Friction = 0.85f;
		
		b.SetPosition(Pos);
		
		for (int i = 0; i < 9; i++)
		{
			//b.m_aPos[i] = Pos + RandomDir()*(frandom()*8.0f);
			b.m_aVel[i] = (Dir + RandomDir()) * ((frandom()+0.50f)*250.0f);
		}
		
		b.m_Color = Color/2;
		b.m_Color.g *= 0.7f;
		b.m_Color.b *= 0.7f;
		b.m_Color.a = Color.a;
		m_pClient->m_pBrains->Add(CBrains::GROUP_BRAINS, &b);
	}
}

void CEffects::Acid(vec2 Pos, vec2 Dir)
{
	CBlooddrop b;
	b.SetDefault();
	b.m_Pos = Pos;
	
	// bones
	if (frandom()*100 < 2)
	{
		b.m_Spr = SPRITE_BONE01 + (rand()%2);
		b.m_LifeSpan = 8.0f + frandom()*8.0f;
		b.m_Rotspeed = frandom()*12.0f - frandom()*12.0f;
		
		b.m_StartSize = 24.0f + frandom()*24;
		b.m_EndSize = b.m_StartSize;
		b.m_Gravity = 1600.0f;
	}
	else
	// blood
	{
		b.m_Spr = SPRITE_BLOOD01 + (rand()%6);
		b.m_LifeSpan = 6.0f + frandom()*6.0f;
		b.m_Rotspeed = 0.0f;
		b.m_StartSize = 32.0f + frandom()*32;
		b.m_EndSize = 24.0f;
		b.m_Gravity = 1400.0f + frandom()*300;
	}
	
	if (g_Config.m_GfxMultiBuffering)
	{
		b.m_Spr = SPRITE_BLOOD02;
		b.m_Rotspeed = 0.0f;
		b.m_StartSize *= 0.75f;
		b.m_EndSize = 0.0f;
		b.m_LifeSpan = 8.0f;
		b.m_StartSize = 32.0f + frandom()*32;
	}
	
	b.m_Vel = Dir * ((frandom()+0.50f)*1200.0f);

	b.m_Rot = GetAngle(b.m_Vel);

	b.m_Friction = 0.85f+frandom()*0.075f;
	//float c = frandom()*0.3f + 0.7f;
	//b.m_Color = vec4(c, c, c, 1.0f);
	b.m_Color = vec4(0, 1, 0, 1.0f);
	m_pClient->m_pBlood->Add(CBlood::GROUP_ACID, &b);
}



void CEffects::Splatter(vec2 Pos, float Angle, float Size, vec4 Color)
{
	CBloodspill b;
	b.SetDefault();
	b.m_Pos = Pos;
	

	b.m_Spr = SPRITE_SPLATTER01 + (rand()%4);
	b.m_LifeSpan = 24.0f + frandom()*8.0f;
	//b.m_LifeSpan = frandom()*8.0f;
	if (Size == -1)
		b.m_Size = 16.0f + frandom()*64;
	else
		b.m_Size = Size;
	
	b.m_Rot = Angle;
	
	//b.m_Color = vec4(frandom()*0.2f + 0.5f, 0, 0, 1.0f);
	//b.m_Color = vec4(0.65f, 0, 0, 1.0f);
	b.m_Color = Color;
	m_pClient->m_pSplatter->Add(CSplatter::GROUP_SPLATTER, &b);
}


void CEffects::Spark(vec2 Pos)
{
	CSinglespark b;
	b.SetDefault();
	b.m_Pos = Pos;
	b.m_Size = 0.5f + frandom();
	b.m_LifeSpan = 0.1f + frandom()*0.2f;
	b.m_Rotspeed = frandom()*12.0f - frandom()*12.0f;
	b.m_Vel = RandomDir() * ((frandom()+0.165f)*500.0f);
	b.m_Rot = GetAngle(b.m_Vel);
	
	b.m_Color = vec4(1.0f, 0.2f + frandom()*0.8f, 0.0f, 1.0f);
	m_pClient->m_pSpark->Add(CSpark::GROUP_SPARKS, &b);
}

void CEffects::Area1(vec2 Pos)
{
	CSinglespark b;
	b.SetDefault();
	b.m_Pos = Pos;
	b.m_Size = 512.0f;
	b.m_LifeSpan = 1.2f;
	b.m_Rotspeed = 0;
	b.m_Vel = vec2(0, 0);
	b.m_Rot = 0;
	
	b.m_Color = vec4(0.0f, 1.0f, 1.0f, 1.0f);
	m_pClient->m_pSpark->Add(CSpark::GROUP_AREA1, &b);
}

void CEffects::Muzzle(vec2 Pos, vec2 Dir, int Weapon)
{
	int s = GetMuzzleAmount(Weapon);
	for (int i = 0; i < s; i++)
	{
		CSinglespark b;
		b.SetDefault();
		b.m_Pos = Pos;
		b.m_Size = 0.5f + frandom();
		b.m_LifeSpan = 0.05f + frandom()*0.1f;
		b.m_Rotspeed = frandom()*12.0f - frandom()*12.0f;
		b.m_Vel = (Dir*2.0f + RandomDir()/3.0f) * ((frandom()+0.2f)*500.0f);
		b.m_Rot = GetAngle(b.m_Vel);
		
		switch (GetMuzzleType(Weapon))
		{
			case 1: b.m_Color = vec4(frandom()*0.5f, 0.5f + frandom()*0.5f, 1.0f, 1.0f); break;
			case 2: b.m_Color = vec4(frandom()*0.5f, 1.0f, frandom()*0.5f, 1.0f); break;
			default: b.m_Color = vec4(1.0f, 0.2f + frandom()*0.8f, 0.0f, 1.0f); break;
		};

		m_pClient->m_pSpark->Add(CSpark::GROUP_SPARKS, &b);
	}
}

void CEffects::GreenSpark(vec2 Pos)
{
	CSinglespark b;
	b.SetDefault();
	b.m_Pos = Pos;
	b.m_Size = 0.5f + frandom();
	b.m_LifeSpan = 0.1f + frandom()*0.1f;
	b.m_Rotspeed = frandom()*12.0f - frandom()*12.0f;
	b.m_Vel = RandomDir() * ((frandom()+0.165f)*600.0f);
	b.m_Rot = GetAngle(b.m_Vel);
	
	b.m_Color = vec4(frandom()*0.5f, 1.0f, frandom()*0.5f, 1.0f);
	m_pClient->m_pSpark->Add(CSpark::GROUP_SPARKS, &b);
}

void CEffects::BlueSpark(vec2 Pos)
{
	CSinglespark b;
	b.SetDefault();
	b.m_Pos = Pos;
	b.m_Size = 0.5f + frandom();
	b.m_LifeSpan = 0.1f + frandom()*0.1f;
	b.m_Rotspeed = frandom()*12.0f - frandom()*12.0f;
	b.m_Vel = RandomDir() * ((frandom()+0.165f)*600.0f);
	b.m_Rot = GetAngle(b.m_Vel);
	
	b.m_Color = vec4(0.25f + frandom()*0.25f, 0.5f + frandom()*0.5f, 1.0f, 1.0f);
	m_pClient->m_pSpark->Add(CSpark::GROUP_SPARKS, &b);
}


void CEffects::DamageIndicator(vec2 Pos, vec2 Dir)
{
	m_pClient->m_pDamageind->Create(Pos, Dir);
}

void CEffects::DamageInd(vec2 Pos, vec2 Dir, int Damage, vec4 Color)
{
	if(!m_Add50hz)
		return;

	CParticle p;
	p.SetDefault();
	p.m_Spr = Damage;
	p.m_Pos = Pos;
	p.m_Vel = Dir * 50 * (1 + min(Damage, 60)*0.025f);
	p.m_LifeSpan = 0.5f+min(Damage, 60)*0.025f;
	p.m_StartSize = 16.0f+min(Damage, 60)*0.7f;
	p.m_EndSize = 16.0f+min(Damage, 60)*0.5f;
	p.m_Color = Color;
	p.m_Rot = 0;
	p.m_Rotspeed = 0;
	p.m_Gravity = 0;
	p.m_Friction = 0.9f;
	p.m_FlowAffected = 0.0f;
	p.m_IgnoreCollision = true;
	m_pClient->m_pParticles->Add(CParticles::GROUP_DAMAGEIND, &p);
}

void CEffects::PowerupShine(vec2 Pos, vec2 size)
{
	if(!m_Add50hz)
		return;

	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_SLICE;
	p.m_Pos = Pos + vec2((frandom()-0.5f)*size.x, (frandom()-0.5f)*size.y);
	p.m_Vel = vec2(0, 0);
	p.m_LifeSpan = 0.5f;
	p.m_StartSize = 16.0f;
	p.m_EndSize = 0;
	p.m_Rot = frandom()*pi*2;
	p.m_Rotspeed = pi*2;
	p.m_Gravity = 500;
	p.m_Friction = 0.9f;
	p.m_FlowAffected = 0.0f;
	m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
}

void CEffects::Repair(vec2 Pos)
{
	if(!m_Add50hz)
		return;

	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_REPAIR;
	p.m_Pos = Pos;
	p.m_Vel = vec2((frandom()-frandom())*600, -1000);
	p.m_LifeSpan = 1.5f;
	p.m_StartSize = 32.0f;
	p.m_EndSize = 48.0f;
	p.m_Rot = 0;
	p.m_Rotspeed = 0;
	p.m_Gravity = -1600;
	p.m_Friction = 0;
	m_pClient->m_pParticles->Add(CParticles::GROUP_CRAFTING, &p);
}

void CEffects::AmmoFill(vec2 Pos, int Weapon)
{
	if(!m_Add50hz)
		return;

	CParticle p;
	p.SetDefault();
	p.m_Spr = Weapon;
	p.m_Pos = Pos;
	p.m_Vel = vec2((frandom()-frandom())*600, -1000);
	p.m_LifeSpan = 1.5f;
	p.m_StartSize = 32.0f;
	p.m_EndSize = 48.0f;
	p.m_Rot = 0;
	p.m_Rotspeed = 0;
	p.m_Gravity = -1600;
	p.m_Friction = 0;
	m_pClient->m_pParticles->Add(CParticles::GROUP_CRAFTING, &p);
}

void CEffects::Light(vec2 Pos, float Size)
{
	if(!m_Add50hz)
		return;

	CLightsource l;
	l.SetDefault();
	l.m_Pos = Pos;
	l.m_Size = Size;
	m_pClient->m_pLight->Add(CLight::GROUP_LIGHTSOURCE, &l);
}

void CEffects::SmokeTrail(vec2 Pos, vec2 Vel)
{
	if(!m_Add50hz)
		return;

	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_SMOKE;
	p.m_Pos = Pos;
	p.m_Vel = Vel + RandomDir()*50.0f;
	p.m_LifeSpan = 0.5f + frandom()*0.5f;
	p.m_StartSize = 12.0f + frandom()*8;
	p.m_EndSize = 0;
	p.m_Friction = 0.7f;
	p.m_Gravity = frandom()*-300.0f;
	p.m_Color = vec4(1,1,1, 0.75f);
	m_pClient->m_pParticles->Add(CParticles::GROUP_PROJECTILE_TRAIL, &p);
}


void CEffects::SkidTrail(vec2 Pos, vec2 Vel)
{
	if(!m_Add100hz)
		return;

	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_SMOKE;
	p.m_Pos = Pos;
	p.m_Vel = Vel + RandomDir()*50.0f;
	p.m_LifeSpan = 0.5f + frandom()*0.5f;
	p.m_StartSize = 24.0f + frandom()*12;
	p.m_EndSize = 0;
	p.m_Friction = 0.7f;
	p.m_Gravity = frandom()*-500.0f;
	p.m_Color = vec4(0.75f,0.75f,0.75f,1.0f);
	m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
}

void CEffects::Triangle(vec2 Pos, vec2 Vel)
{
	if(!m_Add100hz)
		return;

	CParticle p;
	p.SetDefault();
	//p.m_Spr = SPRITE_PART_PARTICLE1 + (rand()%3);
	p.m_Spr = SPRITE_PART_SPLAT01 + (rand()%2);
	p.m_Pos = Pos;
	//p.m_Vel = Vel + RandomDir()*750.0f;
	p.m_Vel = Vel + RandomDir()*50.0f;
	p.m_LifeSpan = 0.5f + frandom()*0.5f;
	p.m_StartSize = 12.0f + frandom()*12;
	p.m_Rot = frandom()*600.0f;
	p.m_EndSize = 0;
	p.m_Friction = 0.7f;
	p.m_Gravity = frandom()*-500.0f;
	p.m_Color = vec4(1.0f, frandom()*0.8, 0, 1.0f);
	m_pClient->m_pParticles->Add(CParticles::GROUP_TRIANGLES, &p);
}

void CEffects::Flame(vec2 Pos, vec2 Vel, float Alpha, bool IgnoreCollision)
{
	if(!m_Add100hz)
		return;

	CParticle p;
	p.SetDefault();
	//p.m_Spr = SPRITE_PART_PARTICLE1 + (rand()%3);
	p.m_Spr = SPRITE_PART_SPLAT01 + (rand()%3);
	p.m_Pos = Pos;
	//p.m_Vel = Vel + RandomDir()*750.0f;
	p.m_Vel = Vel + RandomDir()*50.0f;
	p.m_LifeSpan = 0.5f + frandom()*0.75f;
	p.m_StartSize = 12.0f + frandom()*20;
	p.m_IgnoreCollision = IgnoreCollision;
	p.m_Rot = frandom()*600.0f;
	p.m_EndSize = 0;
	p.m_Friction = 0.7f;
	p.m_Gravity = frandom()*-500.0f;
	p.m_Color = vec4(1.0f, frandom()*0.8f, 0, Alpha);
	m_pClient->m_pParticles->Add(CParticles::GROUP_FLAMES, &p);
}

void CEffects::BulletTrail(vec2 Pos)
{
	if(!m_Add100hz)
		return;

	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_BALL;
	p.m_Pos = Pos;
	p.m_LifeSpan = 0.15f + frandom()*0.15f;
	p.m_StartSize = 8.0f;
	p.m_EndSize = 0;
	p.m_Friction = 0.7f;
	p.m_Color = vec4(0, 0.5f, 1.0f, 0.5f);
	m_pClient->m_pParticles->Add(CParticles::GROUP_PROJECTILE_TRAIL, &p);
}

void CEffects::BulletTrail(vec2 Start, vec2 End, vec4 Color, float Size)
{
	if(!m_Add100hz)
		return;

	CParticle p;
	p.SetDefault();
	
	p.m_StartPos = Start;
	p.m_EndPos = End;
	p.m_TrailDir = normalize(Start - End);
	p.m_StartSize = Size;
	
	p.m_LifeSpan = 0.2f;
	p.m_Color = Color;
	
	m_pClient->m_pParticles->Add(CParticles::GROUP_COLORTRAIL, &p);
}

void CEffects::PlayerSpawn(vec2 Pos)
{
	/*
	for(int i = 0; i < 32; i++)
	{
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_SHELL;
		p.m_Pos = Pos;
		p.m_Vel = RandomDir() * (powf(frandom(), 3)*600.0f);
		p.m_LifeSpan = 0.3f + frandom()*0.3f;
		p.m_StartSize = 64.0f + frandom()*32;
		p.m_EndSize = 0;
		p.m_Rot = frandom()*pi*2;
		p.m_Rotspeed = frandom();
		p.m_Gravity = frandom()*-400.0f;
		p.m_Friction = 0.7f;
		p.m_Color = vec4(0xb5/255.0f, 0x50/255.0f, 0xcb/255.0f, 1.0f);
		m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);

	}
	m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_PLAYER_SPAWN, 1.0f, Pos);
	*/
	
	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PLAYERSPAWN1;
	p.m_Frames = 11;
	p.m_Pos = Pos+vec2(0, -16);
	p.m_LifeSpan = 0.5f;
	p.m_StartSize = 110;
	p.m_EndSize = 110;
	p.m_Rot = 0;
	m_pClient->m_pParticles->Add(CParticles::GROUP_PLAYERSPAWN, &p);
	m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_SPAWN, 1.0f, Pos);
}

void CEffects::PlayerDeath(vec2 Pos, int ClientID)
{
	CParticle p;
	p.SetDefault();
	//p.m_Special = CustomStuff()->m_aPlayerInfo[ClientID].m_RenderInfo.m_BloodColor;
	p.m_Spr = SPRITE_DEATH1;
	p.m_Frames = 8;
	p.m_Pos = Pos + vec2(0, -24);
	p.m_LifeSpan = 0.3f;
	p.m_StartSize = 100;
	p.m_EndSize = 100;
	p.m_Rot = frandom()*pi*2;
	
	switch (CustomStuff()->m_aPlayerInfo[ClientID].m_RenderInfo.m_BloodColor)
	{
		case 1: p.m_Color = vec4(0, 1, 0, 1); break;
		case 2: p.m_Color = vec4(0, 0, 0.1f, 1); break;
		default: p.m_Color = vec4(1, 0, 0, 1); break;
	};
	
	m_pClient->m_pParticles->Add(CParticles::GROUP_DEATH, &p);
	
	if (ClientID < 0 || ClientID >= MAX_CLIENTS)
		return;
	
	Guts(Pos, p.m_Color);
	//Guts(Pos, vec2((frandom()-frandom())*1.5f, -1-frandom()*1.7f), p.m_Color);
	
	// body
	for (int i = 0; i < 4; i++)
	{
		CParticle p;
		p.SetDefault();
		p.m_Special = CustomStuff()->m_aPlayerInfo[ClientID].m_RenderInfo.m_BloodColor;
		p.m_Spr = SPRITE_MEAT1+2*rand()%6;
		p.m_Pos = Pos + vec2(0, -14) + vec2(frandom()-frandom(), frandom()-frandom())*12;
		p.m_Vel = RandomDir()*(frandom()+0.2f)*600.0f + vec2(0, -100);
		p.m_LifeSpan = 4.0f+frandom();
		p.m_StartSize = 32;
		p.m_EndSize = p.m_StartSize;
		p.m_Rot = frandom()*pi*2;
		p.m_Rotspeed = p.m_Vel.x / 10;
		p.m_Friction = 0.9f;
		p.m_Chunk = true;
		p.m_Gravity = 1200.0f + frandom()*400;
		p.m_Color = CustomStuff()->m_aPlayerInfo[ClientID].m_RenderInfo.m_ColorBody;
		m_pClient->m_pParticles->Add(CParticles::GROUP_MEAT, &p);
	}
	
	// feet
	for (int i = 0; i < 4; i++)
	{
		CParticle p;
		p.SetDefault();
		p.m_Special = CustomStuff()->m_aPlayerInfo[ClientID].m_RenderInfo.m_BloodColor;
		p.m_Spr = SPRITE_MEAT1+2*rand()%6;
		p.m_Pos = Pos + vec2(0, +4) + vec2(frandom()-frandom(), frandom()-frandom())*12;
		p.m_Vel = RandomDir()*(frandom()+0.2f)*400.0f;
		p.m_LifeSpan = 4.0f+frandom();
		p.m_StartSize = 32;
		p.m_EndSize = p.m_StartSize;
		p.m_Rot = frandom()*pi*2;
		p.m_Rotspeed = p.m_Vel.x / 10;
		p.m_Friction = 0.9f;
		p.m_Chunk = true;
		p.m_Gravity = 1200.0f + frandom()*400;
		p.m_Color = CustomStuff()->m_aPlayerInfo[ClientID].m_RenderInfo.m_ColorFeet;
		m_pClient->m_pParticles->Add(CParticles::GROUP_MEAT, &p);
	}
	
	// skin (head)
	for (int i = 0; i < 7; i++)
	{
		CParticle p;
		p.SetDefault();
		p.m_Special = CustomStuff()->m_aPlayerInfo[ClientID].m_RenderInfo.m_BloodColor;
		p.m_Spr = SPRITE_MEAT7+2*rand()%2;
		p.m_Pos = Pos + vec2(0, -36) + vec2(frandom()-frandom(), frandom()-frandom())*12;
		p.m_Vel = RandomDir()*(frandom()+0.2f)*700.0f + vec2(0, -100);
		p.m_LifeSpan = 4.0f+frandom();
		p.m_StartSize = 32;
		p.m_EndSize = p.m_StartSize;
		p.m_Rot = frandom()*pi*2;
		p.m_Rotspeed = p.m_Vel.x / 10;
		p.m_Friction = 0.9f;
		p.m_Chunk = true;
		p.m_Gravity = 1200.0f + frandom()*400;
		p.m_Color = CustomStuff()->m_aPlayerInfo[ClientID].m_RenderInfo.m_ColorSkin;
		m_pClient->m_pParticles->Add(CParticles::GROUP_MEAT, &p);
	}
	
	// eye
	/*
	{
		CParticle p;
		p.SetDefault();
		p.m_Special = CustomStuff()->m_aPlayerInfo[ClientID].m_RenderInfo.m_BloodColor;
		p.m_Spr = SPRITE_MEAT6;
		p.m_Pos = Pos + vec2(0, -36);
		p.m_Vel = RandomDir()*(frandom()+0.2f)*700.0f;
		p.m_LifeSpan = 3.0f+frandom();
		p.m_StartSize = 20;
		p.m_EndSize = 20;
		p.m_Rot = frandom()*pi*2;
		p.m_Rotspeed = p.m_Vel.x / 10;
		p.m_Friction = 0.95f;
		p.m_Chunk = true;
		p.m_Gravity = 1200.0f + frandom()*400;
		p.m_Color = vec4(1, 1, 1, 1);
		m_pClient->m_pParticles->Add(CParticles::GROUP_MEAT, &p);
	}
	*/
}


void CEffects::Lazer(vec2 Pos, int Height)
{
	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_LAZER1;
	p.m_Frames = 16;
	p.m_Pos = Pos;
	p.m_LifeSpan = 0.45f;
	p.m_StartSize = 70;
	p.m_EndSize = 70;
	p.m_Height = Height;
	p.m_Rot = 0;
	m_pClient->m_pParticles->Add(CParticles::GROUP_LAZER, &p);
}


void CEffects::Electrospark(vec2 Pos, float Size, vec2 Vel)
{
	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_SPARK1_1;
	if (frandom() < 0.5f)
		p.m_Spr = SPRITE_SPARK2_1;
	p.m_Frames = 3;
	p.m_Vel = Vel;
	p.m_Gravity = 0;
	p.m_Friction = 0.8f;
	
	p.m_Pos = Pos;
	p.m_LifeSpan = 0.2f;
	p.m_StartSize = Size;
	p.m_EndSize = Size;
	p.m_Rot = frandom()*pi*2;;
	m_pClient->m_pParticles->Add(CParticles::GROUP_SPARKS, &p);
}


void CEffects::SpriteSheet(int FX, vec2 Pos)
{
	switch (FX)
	{
	case FX_MINE:
		{ // smoke
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_MINE1_1;
			p.m_Frames = 8;
			p.m_Pos = Pos+vec2(35, -45);
			p.m_LifeSpan = 0.35f;
			p.m_StartSize = 110;
			p.m_EndSize = 110;
			p.m_Rot = 0;
			m_pClient->m_pParticles->Add(CParticles::GROUP_MINE1, &p);
		}{ // explosion
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_MINE2_1;
			p.m_Frames = 4;
			p.m_Pos = Pos+vec2(10, -45);
			p.m_LifeSpan = 0.25f;
			p.m_StartSize = 120;
			p.m_EndSize = 120;
			p.m_Rot = 0;
			m_pClient->m_pParticles->Add(CParticles::GROUP_MINE2, &p);
		}
		break;
		
	case FX_ELECTROMINE:
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_ELECTROMINE1;
			p.m_Frames = 12;
			p.m_Pos = Pos+vec2(0, -20);
			p.m_LifeSpan = 0.35f;
			p.m_StartSize = 180;
			p.m_EndSize = 180;
			p.m_Rot = -pi/2;
			m_pClient->m_pParticles->Add(CParticles::GROUP_ELECTROMINE, &p);
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_ELECTROMINE, 1, Pos);
		}
		break;
		
	case FX_SMALLELECTRIC:
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_ELECTRIC1;
			p.m_Frames = 8;
			p.m_Pos = Pos;
			p.m_LifeSpan = 0.20f;
			p.m_StartSize = 70;
			p.m_EndSize = 70;
			p.m_Rot = frandom()*pi*2;
			m_pClient->m_pParticles->Add(CParticles::GROUP_ELECTRIC, &p);
		}
		break;
		
	case FX_ELECTRIC:
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_ELECTRIC1;
			p.m_Frames = 8;
			p.m_Pos = Pos;
			p.m_LifeSpan = 0.20f;
			p.m_StartSize = 150;
			p.m_EndSize = 150;
			p.m_Rot = frandom()*pi*2;
			m_pClient->m_pParticles->Add(CParticles::GROUP_ELECTRIC, &p);
		}
		break;
		
	case FX_SUPERELECTRIC:
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_ELECTRIC1;
			p.m_Frames = 8;
			p.m_Pos = Pos;
			p.m_LifeSpan = 0.20f;
			p.m_StartSize = 210;
			p.m_EndSize = 210;
			p.m_Rot = frandom()*pi*2;
			m_pClient->m_pParticles->Add(CParticles::GROUP_ELECTRIC, &p);
			
			for (int i = 0; i < 4; i++)
				Electrospark(Pos+vec2(frandom()-frandom(), frandom()-frandom())*120.0f, 64);
		}
		break;
		
	case FX_GREEN_EXPLOSION:
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_GREEN_EXPLOSION1;
			p.m_Frames = 8;
			p.m_Pos = Pos;
			p.m_LifeSpan = 0.26f;
			p.m_StartSize = 140;
			p.m_EndSize = 160;
			p.m_Rot = frandom()*pi*2;
			m_pClient->m_pParticles->Add(CParticles::GROUP_GREEN_EXPLOSION, &p);
			
			for (int i = 0; i < 24; i++)
				GreenSpark(Pos);
		}
		break;
		
	case FX_ROLLDASH:
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_FX_DASH1;
			p.m_Frames = 8;
			p.m_Pos = Pos;
			p.m_LifeSpan = 0.22f;
			p.m_StartSize = 200;
			p.m_EndSize = 200;
			p.m_Rot = 0;
			m_pClient->m_pParticles->Add(CParticles::GROUP_EFFECT1, &p);
			
			for (int i = 0; i < 24; i++)
				GreenSpark(Pos);
		}
		break;
		
	case FX_LAZERLOAD:
		m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_DEATHRAY, 1.0f, Pos);
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_LAZERLOAD1;
			p.m_Frames = 8;
			p.m_Pos = Pos;
			p.m_LifeSpan = 0.3f;
			p.m_StartSize = 60;
			p.m_EndSize = 60;
			p.m_Rot = 0;
			m_pClient->m_pParticles->Add(CParticles::GROUP_LAZERLOAD, &p);
		}
		break;
		
	case FX_TAKEOFF:
		m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_WALKER_TAKEOFF, 1.0f, Pos);
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_TAKEOFF1;
			p.m_Frames = 8;
			p.m_Pos = Pos;
			p.m_LifeSpan = 0.4f;
			p.m_StartSize = 70;
			p.m_EndSize = 80;
			p.m_Rot = 0;
			m_pClient->m_pParticles->Add(CParticles::GROUP_TAKEOFF, &p);
		}
		break;
		
	case FX_FLAME1:
		//m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_WALKER_TAKEOFF, 1.0f, Pos);
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_FLAME1_1;
			p.m_Frames = 8;
			p.m_Pos = Pos;
			p.m_LifeSpan = 0.3f;
			p.m_StartSize = 90;
			p.m_EndSize = 90;
			p.m_Rot = 0;
			m_pClient->m_pParticles->Add(CParticles::GROUP_FLAME1, &p);
		}
		break;
		
	case FX_BARREL:
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_SMOKE1_1;
			p.m_Frames = 11;
			p.m_Pos = Pos+vec2(0, -20);
			p.m_LifeSpan = 0.35f;
			p.m_StartSize = 200;
			p.m_EndSize = 300;
			p.m_Rot = 0;
			m_pClient->m_pParticles->Add(CParticles::GROUP_SMOKE1, &p);
		}
		break;
		
	case FX_ELECTROHIT:
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_ELECTROHIT1;
			p.m_Frames = 4;
			p.m_Pos = Pos;
			p.m_LifeSpan = 0.3f;
			p.m_StartSize = 60;
			p.m_EndSize = 60;
			p.m_Rot = frandom()*pi*2;
			m_pClient->m_pParticles->Add(CParticles::GROUP_HITEFFECTS, &p);
			
			Electrospark(Pos, 64);
		}
		break;
		
	case FX_SHIELDHIT:
		{
			Electrospark(Pos, 64);
			
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_ELECTROHIT1;
			p.m_Frames = 4;
			p.m_Pos = Pos;
			p.m_LifeSpan = 0.3f;
			p.m_StartSize = 60;
			p.m_EndSize = 60;
			p.m_Rot = frandom()*pi*2;
			m_pClient->m_pParticles->Add(CParticles::GROUP_HITEFFECTS, &p);

			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_SHIELD_HIT, 1.0f, Pos);
		}
		break;
		
	// sword hit
	case FX_BLOOD1:
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_BLOODFX1_1;
			p.m_Frames = 6;
			p.m_Pos = Pos;
			p.m_LifeSpan = 0.25f;
			p.m_StartSize = 110;
			p.m_EndSize = 110;
			p.m_Rot = frandom()*pi*2;
			m_pClient->m_pParticles->Add(CParticles::GROUP_BLOODFX, &p);
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_HAMMER_HIT, 1.0f, Pos);
		}
		break;
		
	// chainsaw
	case FX_BLOOD2:
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_BLOODFX1_1;
			p.m_Frames = 6;
			p.m_Pos = Pos;
			p.m_LifeSpan = 0.20f;
			p.m_StartSize = 60;
			p.m_EndSize = 60;
			p.m_Rot = frandom()*pi*2;
			m_pClient->m_pParticles->Add(CParticles::GROUP_BLOODFX, &p);
			//m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_HAMMER_HIT, 1.0f, Pos);
		}
		break;

	// hammer hit
	case FX_BLOOD3:
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_BLOODFX1_1;
			p.m_Frames = 6;
			p.m_Pos = Pos;
			p.m_LifeSpan = 0.25f;
			p.m_StartSize = 110;
			p.m_EndSize = 110;
			p.m_Rot = frandom()*pi*2;
			m_pClient->m_pParticles->Add(CParticles::GROUP_BLOODFX, &p);
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_KICKHIT, 1.0f, Pos);
		}
		break;
		
	case FX_MONSTERDEATH:
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_DEATH1;
			p.m_Frames = 8;
			p.m_Pos = Pos + vec2(0, -24);
			p.m_LifeSpan = 0.3f;
			p.m_StartSize = 75;
			p.m_EndSize = 75;
			p.m_Rot = frandom()*pi*2;
			m_pClient->m_pParticles->Add(CParticles::GROUP_DEATH, &p);
			//m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_HAMMER_HIT, 1.0f, Pos);
		}
		break;		
		
	case FX_MONSTERSPAWN:
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_MONSTERSPAWN1;
			p.m_Frames = 11;
			p.m_Pos = Pos+vec2(0, -16);
			p.m_LifeSpan = 0.5f;
			p.m_StartSize = 90;
			p.m_EndSize = 90;
			p.m_Rot = 0;
			m_pClient->m_pParticles->Add(CParticles::GROUP_MONSTERSPAWN, &p);
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_SPAWN, 1.0f, Pos);
		}
		break;
		
	default:;
	};


}



void CEffects::DashEffect(vec2 Pos, int Angle)
{
	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_FX_DASH1;
	p.m_Frames = 8;
	p.m_Pos = Pos;
	p.m_Vel = -vec2(cosf(Angle/256.0f), sinf(Angle/256.0f)) * 400.0f;
	p.m_LifeSpan = 0.5f;
	p.m_IgnoreCollision = true;
	p.m_Friction = 0.8f;
	p.m_StartSize = 80;
	p.m_EndSize = 100;
	p.m_Rot = Angle/256.0f-pi/2;
	m_pClient->m_pParticles->Add(CParticles::GROUP_EFFECT1, &p);
	
	m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_DASH, 1.0f, Pos);
}


void CEffects::SpriteExplosion(vec2 Pos, float Size, int Sprite)
{
	// add explosion
	CParticle p;
	p.SetDefault();
			
	p.m_Spr = Sprite;
			
	p.m_Frames = 8;
	p.m_Pos = Pos;
	p.m_LifeSpan = 0.3f;
	p.m_StartSize = Size;
	p.m_EndSize = Size*1.1f;
	p.m_Rot = frandom()*pi*2;
	m_pClient->m_pParticles->Add(CParticles::GROUP_EXPLOSIONS, &p);
			
	// add smoke
	for(int i = 0; i < Size / 15; i++)
	{
		Spark(Pos);
		Spark(Pos);
			
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_SMOKE;
		p.m_Pos = Pos;
		p.m_Vel = RandomDir() * ((1.0f + frandom()*0.2f) * 1000.0f);
		p.m_LifeSpan = 0.5f + frandom()*0.4f;
		p.m_StartSize = (32.0f + frandom()*8);
		p.m_EndSize = 0;
		p.m_Gravity = frandom()*-800.0f;
		p.m_Friction = 0.4f;
		p.m_Color = mix(vec4(0.75f,0.75f,0.75f,1.0f), vec4(0.5f,0.5f,0.5f,1.0f), frandom());
		m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
	}
}



void CEffects::SpriteSmoke(vec2 Pos, float Size, vec4 Color)
{
	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_SMOKE;
	p.m_Pos = Pos;
	p.m_Vel = RandomDir() * ((1.0f + frandom()*0.2f) * 20.0f * Size);
	p.m_LifeSpan = 0.5f + frandom()*0.4f;
	p.m_StartSize = (Size + frandom()*8);
	p.m_EndSize = 0;
	p.m_Gravity = frandom()*-20.0f * Size;
	p.m_Friction = 0.4f;
	p.m_Color = mix(Color, vec4(Color.r/2, Color.g/2, Color.b/2, Color.a), frandom());
	m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
}

void CEffects::Explosion(vec2 Pos, int Weapon)
{
	// add to flow
	if (GetExplosionSprite(Weapon))
	{
		for(int y = -8; y <= 8; y++)
			for(int x = -8; x <= 8; x++)
			{
				if(x == 0 && y == 0)
					continue;

				float a = 1 - (length(vec2(x,y)) / length(vec2(8,8)));
				m_pClient->m_pFlow->Add(Pos+vec2(x,y)*16, normalize(vec2(x,y))*5000.0f*a, 10.0f);
			}
	}

	
	if (IsDroid(Weapon))
	{
		if (GetExplosionSprite(Weapon))
			SpriteExplosion(Pos, GetExplosionSize(Weapon), GetExplosionSprite(Weapon));
		else
		{
			Electrospark(Pos, 64 * GetProjectileSize(Weapon));
			
			// add sparks
			for(int i = 0; i < 6*GetProjectileSize(Weapon); i++)
				BlueSpark(Pos);
				
			if (GetDroidType(Weapon) == DROIDTYPE_STAR)
			{
				if (g_Config.m_GfxShaders)
				{
					CSinglespark b;
					b.SetDefault();
					b.m_Pos = Pos;
					b.m_Size = 84.0f * GetProjectileSize(Weapon);
					b.m_LifeSpan = 0.35f;
					b.m_Rotspeed = 0;
					b.m_Vel = vec2(0, 0);
					b.m_Rot = 0;
					
					b.m_Color = vec4(0.0f, 1.0f, 1.0f, 1.0f);
					m_pClient->m_pSpark->Add(CSpark::GROUP_AREA1, &b);
				}
				else
				{
					CParticle p;
					p.SetDefault();
					p.m_Spr = SPRITE_ELECTRIC1;
					p.m_Frames = 8;
					p.m_Pos = Pos;
					p.m_LifeSpan = 0.20f;
					p.m_StartSize = 70 * GetProjectileSize(Weapon);
					p.m_EndSize = p.m_StartSize;
					p.m_Rot = frandom()*pi*2;
					m_pClient->m_pParticles->Add(CParticles::GROUP_ELECTRIC, &p);
				}
			}
		}
		
		return;
	}
	
	// buildings
	if (IsBuilding(Weapon))
	{
		if (GetBuildingType(Weapon) == BUILDING_STAND)
		{
			for(int i = 0; i < 9; i++)
				Spark(Pos+vec2(0, frandom()-frandom())*10.0f);
			
		}
		
		if (GetBuildingType(Weapon) == BUILDING_TURRET)
		{
			SpriteExplosion(Pos, GetExplosionSize(Weapon), GetExplosionSprite(Weapon));
		}
		
		if (GetBuildingType(Weapon) == BUILDING_TESLACOIL)
		{
			SpriteExplosion(Pos, GetExplosionSize(Weapon), GetExplosionSprite(Weapon));
		}
		
		if (GetBuildingType(Weapon) == BUILDING_FLAMETRAP)
		{
			SpriteExplosion(Pos, GetExplosionSize(Weapon), GetExplosionSprite(Weapon));
		}
		
		if (GetBuildingType(Weapon) == BUILDING_REACTOR)
		{
			SpriteExplosion(Pos, GetExplosionSize(Weapon), GetExplosionSprite(Weapon));
		}
		
		
		if (GetBuildingType(Weapon) == BUILDING_BARREL || GetBuildingType(Weapon) == BUILDING_POWERBARREL)
			SpriteExplosion(Pos, GetExplosionSize(Weapon), GetExplosionSprite(Weapon));
		
		return;
	}
	
	if (IsStaticWeapon(Weapon))
	{
		switch (GetStaticType(Weapon))
		{
			case SW_GUN1:
			{
				for(int i = 0; i < 3; i++)
					Spark(Pos);
				
				SpriteSmoke(Pos, 16, vec4(1.0f, 1.0f, 1.0f, 1.0f));
				break;
			}
			
			case SW_SHURIKEN:
			{
				for(int i = 0; i < 5; i++)
					Spark(Pos);
				
				break;
			}
			
			case SW_BUBBLER:
			{
				// add smoke
				for(int i = 0; i < 5; i++)
				{
					Spark(Pos);
					Spark(Pos);
					Spark(Pos);
					
					CParticle p;
					p.SetDefault();
					p.m_Spr = SPRITE_PART_SMOKE;
					p.m_Pos = Pos;
					p.m_Vel = RandomDir() * ((1.0f + frandom()*0.2f) * (1000.0f*GetProjectileSize(Weapon)));
					p.m_LifeSpan = 0.5f + frandom()*0.4f;
					p.m_StartSize = (32.0f + frandom()*8)*GetProjectileSize(Weapon);
					p.m_EndSize = 0;
					p.m_Gravity = frandom()*-800.0f;
					p.m_Friction = 0.4f;
					p.m_Color = mix(vec4(0.75f,0.75f,0.75f,1.0f), vec4(0.5f,0.5f,0.5f,1.0f), frandom());
					m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
				}
				
				break;
			};

			case SW_GRENADE1:
			{
				// add explosion
				CParticle p;
				p.SetDefault();
				p.m_Spr = GetExplosionSprite(Weapon);
				
				p.m_Frames = 8;
				p.m_Pos = Pos;
				p.m_LifeSpan = 0.3f;
				p.m_StartSize = GetExplosionSize(Weapon);
				p.m_EndSize = GetExplosionSize(Weapon)*1.1f;
				p.m_Rot = frandom()*pi*2;
				m_pClient->m_pParticles->Add(CParticles::GROUP_EXPLOSIONS, &p);
				
				// add smoke
				for(int i = 0; i < 20*GetProjectileSize(Weapon); i++)
				{
					Spark(Pos);
					Spark(Pos);
					
					CParticle p;
					p.SetDefault();
					p.m_Spr = SPRITE_PART_SMOKE;
					p.m_Pos = Pos;
					p.m_Vel = RandomDir() * ((1.0f + frandom()*0.2f) * (1000.0f*GetProjectileSize(Weapon)));
					p.m_LifeSpan = 0.5f + frandom()*0.4f;
					p.m_StartSize = (32.0f + frandom()*8)*GetProjectileSize(Weapon);
					p.m_EndSize = 0;
					p.m_Gravity = frandom()*-800.0f;
					p.m_Friction = 0.4f;
					p.m_Color = mix(vec4(0.75f,0.75f,0.75f,1.0f), vec4(0.5f,0.5f,0.5f,1.0f), frandom());
					m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
				}
			} break;
			
			case SW_GRENADE2:
			{
				Electrospark(Pos, 96);
				Electrospark(Pos, 96);
				
				if (g_Config.m_GfxShaders)
				{
					for (int i = 0; i < 2; i++)
					{
						CSinglespark b;
						b.SetDefault();
						b.m_Pos = Pos;
						b.m_Size = 256.0f+i*128;
						b.m_LifeSpan = 0.30f-i*0.05f;
						b.m_Rotspeed = 0;
						b.m_Vel = vec2(0, 0);
						b.m_Rot = 0;
						
						b.m_Color = vec4(0.0f, 0.5f+i*0.5f, 1.0f, 1.0f);
						m_pClient->m_pSpark->Add(CSpark::GROUP_AREA1, &b);
					}
				}
				else
				{
					CParticle p;
					p.SetDefault();
					p.m_Spr = SPRITE_ELECTRIC1;
					p.m_Frames = 8;
					p.m_Pos = Pos;
					p.m_LifeSpan = 0.20f;
					p.m_StartSize = 160;
					p.m_EndSize = p.m_StartSize;
					p.m_Rot = frandom()*pi*2;
					m_pClient->m_pParticles->Add(CParticles::GROUP_ELECTRIC, &p);
				}
				
			} break;
			
			case SW_GRENADE3:
			{
				Electrospark(Pos, 96);
				Electrospark(Pos, 96);
			} break;
			
			case SW_BOUNCER:
			{
				SpriteSheet(FX_GREEN_EXPLOSION, Pos);
				break;
			}
			
			default:
			{
				if (GetExplosionSprite(Weapon))
					SpriteExplosion(Pos, GetExplosionSize(Weapon), GetExplosionSprite(Weapon));
				
				break;
			}
		};
		
		return;
	}
	
	// weapons
	int Part1 = GetPart(Weapon, 0);
	//int Part2 = GetPart(Weapon, 1);

	// launcher
	if (Part1 == 2)
	{
		// add explosion
		CParticle p;
		p.SetDefault();
		p.m_Spr = GetExplosionSprite(Weapon);
		
		p.m_Frames = 8;
		p.m_Pos = Pos;
		p.m_LifeSpan = 0.3f;
		p.m_StartSize = GetExplosionSize(Weapon);
		p.m_EndSize = GetExplosionSize(Weapon)*1.1f;
		p.m_Rot = frandom()*pi*2;
		m_pClient->m_pParticles->Add(CParticles::GROUP_EXPLOSIONS, &p);
		
		// add smoke
		for(int i = 0; i < 10*GetProjectileSize(Weapon); i++)
		{
			Spark(Pos);
			Spark(Pos);
			
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_PART_SMOKE;
			p.m_Pos = Pos;
			p.m_Vel = RandomDir() * ((1.0f + frandom()*0.2f) * (1000.0f*GetProjectileSize(Weapon)));
			p.m_LifeSpan = 0.5f + frandom()*0.4f;
			p.m_StartSize = (32.0f + frandom()*8)*GetProjectileSize(Weapon);
			p.m_EndSize = 0;
			p.m_Gravity = frandom()*-800.0f;
			p.m_Friction = 0.4f;
			p.m_Color = mix(vec4(0.75f,0.75f,0.75f,1.0f), vec4(0.5f,0.5f,0.5f,1.0f), frandom());
			m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
		}
	}
	
	// basic / rifle
	if (Part1 == 1)
	{
		// add sparks
		for(int i = 0; i < 6*GetProjectileSize(Weapon); i++)
			Spark(Pos);
		
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_SMOKE;
		p.m_Pos = Pos;
		p.m_Vel = RandomDir() * ((1.0f + frandom()*0.2f) * (400.0f*GetProjectileSize(Weapon)));
		p.m_LifeSpan = 0.5f + frandom()*0.4f;
		p.m_StartSize = (32.0f + frandom()*8)*GetProjectileSize(Weapon);
		p.m_EndSize = 0;
		p.m_Gravity = frandom()*-800.0f;
		p.m_Friction = 0.4f;
		p.m_Color = mix(vec4(0.75f,0.75f,0.75f,1.0f), vec4(0.5f,0.5f,0.5f,1.0f), frandom());
		m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
	}
	
	// sniper
	if (Part1 == 4)
	{
		for(int i = 0; i < 4*GetProjectileSize(Weapon); i++)
		{
			GreenSpark(Pos);
			
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_PART_SMOKE;
			p.m_Pos = Pos;
			p.m_Vel = RandomDir() * ((1.0f + frandom()*0.2f) * (300.0f*GetProjectileSize(Weapon)));
			p.m_LifeSpan = 0.4f + frandom()*0.2f;
			p.m_StartSize = (16.0f + frandom()*8)*GetProjectileSize(Weapon);
			p.m_EndSize = 0;
			p.m_Gravity = frandom()*-800.0f;
			p.m_Friction = 0.4f;
			p.m_Color = mix(vec4(0.1f,0.6f,0.1f,0.5f), vec4(0.2f, 0.8f,0.2f,0.5f), frandom());
			m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
		}
	}
	
	// electro
	if (Part1 == 3)
	{
		// add sparks
		for(int i = 0; i < 6*GetProjectileSize(Weapon); i++)
			BlueSpark(Pos);
		
		//Area1(Pos);'
		
		Electrospark(Pos, 64 * GetProjectileSize(Weapon));
		
		
		//if (g_Config.m_GfxShaders)
		{
			CSinglespark b;
			b.SetDefault();
			b.m_Pos = Pos;
			b.m_Size = 84.0f * GetProjectileSize(Weapon);
			b.m_LifeSpan = 0.35f;
			b.m_Rotspeed = 0;
			b.m_Vel = vec2(0, 0);
			b.m_Rot = 0;
			
			b.m_Color = vec4(0.0f, 1.0f, 1.0f, 1.0f);
			m_pClient->m_pSpark->Add(CSpark::GROUP_AREA1, &b);
		}
		/*
		else
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_ELECTRIC1;
			p.m_Frames = 8;
			p.m_Pos = Pos;
			p.m_LifeSpan = 0.20f;
			p.m_StartSize = 70 * GetProjectileSize(Weapon);
			p.m_EndSize = p.m_StartSize;
			p.m_Rot = frandom()*pi*2;
			m_pClient->m_pParticles->Add(CParticles::GROUP_ELECTRIC, &p);
		}
		*/
		
		/*
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_SMOKE;
		p.m_Pos = Pos;
		p.m_Vel = RandomDir() * ((1.0f + frandom()*0.2f) * (400.0f*GetProjectileSize(Weapon)));
		p.m_LifeSpan = 0.5f + frandom()*0.4f;
		p.m_StartSize = (32.0f + frandom()*8)*GetProjectileSize(Weapon);
		p.m_EndSize = 0;
		p.m_Gravity = frandom()*-800.0f;
		p.m_Friction = 0.4f;
		p.m_Color = mix(vec4(0.75f,0.75f,0.75f,1.0f), vec4(0.5f,0.5f,0.5f,1.0f), frandom());
		m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
		*/
	}
}


void CEffects::ChainsawSmoke(vec2 Pos)
{
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_SMOKE;
		p.m_Pos = Pos;
		p.m_Vel = vec2((frandom()-frandom()) * 0.5f, -1.0f) * ((1.0f + frandom()*0.2f) * 350.0f);
		p.m_LifeSpan = 0.2f + frandom()*0.2f;
		p.m_StartSize = 14.0f + frandom()*4;
		p.m_EndSize = 0;
		p.m_Gravity = frandom()*-700.0f;
		p.m_Friction = 0.4f;
		p.m_Color = mix(vec4(0.75f,0.75f,0.75f,0.3f), vec4(0.5f,0.5f,0.5f,0.3f), frandom());
		m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
}

	
void CEffects::FlameExplosion(vec2 Pos)
{
	// add the smoke and flame
	for(int i = 0; i < 9; i++)
	{
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_SMOKE;
		p.m_Pos = Pos;
		p.m_Vel = RandomDir() * ((1.0f + frandom()*0.2f) * 300.0f);
		p.m_LifeSpan = 0.5f + frandom()*0.4f;
		p.m_StartSize = 32.0f + frandom()*8;
		p.m_EndSize = 0;
		p.m_Gravity = frandom()*-800.0f;
		p.m_Friction = 0.4f;
		p.m_Color = mix(vec4(0.75f,0.75f,0.75f,1.0f), vec4(0.5f,0.5f,0.5f,1.0f), frandom());
		m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
		
		for (int f = 0; f < 3; f++)
			Flame(Pos+vec2(frandom()-frandom(), frandom()-frandom())*5.0f, RandomDir()*(frandom()*200));
	}
}


void CEffects::HammerHit(vec2 Pos)
{
	// add the explosion
	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_HIT01;
	p.m_Pos = Pos;
	p.m_LifeSpan = 0.2f;
	p.m_StartSize = 120.0f;
	p.m_EndSize = 0;
	p.m_Rot = frandom()*pi*2;
	m_pClient->m_pParticles->Add(CParticles::GROUP_EXPLOSIONS, &p);
	m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_HAMMER_HIT, 1.0f, Pos);
}

void CEffects::SwordHit(vec2 Pos, float Angle, bool Flip, float Charge)
{
	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_SWORDHIT1;
	p.m_Frames = 3;
	p.m_Pos = Pos;
	p.m_LifeSpan = 0.12f;
	p.m_StartSize = 170+Charge*40.0f;
	p.m_EndSize = 170+Charge*40.0f;
	p.m_Rot = Angle;
	p.m_Flip = Flip;
	p.m_Color = vec4(0.5f+Charge*0.25f, 1.0f, 0.5f+Charge*0.5f, 1.0f);
	m_pClient->m_pParticles->Add(CParticles::GROUP_SWORDHITS, &p);
}

void CEffects::OnRender()
{
	static int64 LastUpdate100hz = 0;
	static int64 LastUpdate50hz = 0;

	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();

		if(time_get()-LastUpdate100hz > time_freq()/(100*pInfo->m_Speed))
		{
			m_Add100hz = true;
			LastUpdate100hz = time_get();
		}
		else
			m_Add100hz = false;

		if(time_get()-LastUpdate50hz > time_freq()/(100*pInfo->m_Speed))
		{
			m_Add50hz = true;
			LastUpdate50hz = time_get();
		}
		else
			m_Add50hz = false;

		if(m_Add50hz)
			m_pClient->m_pFlow->Update();

		return;
	}

	if(time_get()-LastUpdate100hz > time_freq()/100)
	{
		m_Add100hz = true;
		LastUpdate100hz = time_get();
	}
	else
		m_Add100hz = false;

	if(time_get()-LastUpdate50hz > time_freq()/100)
	{
		m_Add50hz = true;
		LastUpdate50hz = time_get();
	}
	else
		m_Add50hz = false;

	if(m_Add50hz)
		m_pClient->m_pFlow->Update();
}
