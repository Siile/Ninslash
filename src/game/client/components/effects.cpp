

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
#include <game/client/components/splatter.h>
#include <game/client/components/spark.h>

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


void CEffects::Blood(vec2 Pos, vec2 Dir)
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
	
	//b.m_Gravity = 1600.0f;
	b.m_Friction = 0.85f+frandom()*0.075f;
	b.m_Friction *= 0.95f;
	float c = frandom()*0.3f + 0.7f;
	b.m_Color = vec4(c, c, c, 1.0f);
	m_pClient->m_pBlood->Add(CBlood::GROUP_BLOOD, &b);
}



void CEffects::Splatter(vec2 Pos, float Angle, float Size)
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
	b.m_Color = vec4(0.65f, 0, 0, 1.0f);
	m_pClient->m_pSplatter->Add(CSplatter::GROUP_SPLATTER, &b);
}


void CEffects::Spark(vec2 Pos)
{
	CSinglespark b;
	b.SetDefault();
	b.m_Pos = Pos;
	b.m_LifeSpan = 0.1f + frandom()*0.2f;
	b.m_Rotspeed = frandom()*12.0f - frandom()*12.0f;
	b.m_Vel = RandomDir() * ((frandom()+0.165f)*500.0f);
	b.m_Rot = GetAngle(b.m_Vel);
	
	b.m_Color = vec4(1.0f, 0.2f + frandom()*0.8f, 0.0f, 1.0f);
	m_pClient->m_pSpark->Add(CSpark::GROUP_SPARKS, &b);
}


void CEffects::DamageIndicator(vec2 Pos, vec2 Dir)
{
	m_pClient->m_pDamageind->Create(Pos, Dir);
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
	p.m_Gravity = frandom()*-500.0f;
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

void CEffects::Flame(vec2 Pos, vec2 Vel, float Alpha)
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
	p.m_LifeSpan = 0.5f + frandom()*0.75f;
	p.m_StartSize = 16.0f + frandom()*20;
	p.m_Rot = frandom()*600.0f;
	p.m_EndSize = 0;
	p.m_Friction = 0.7f;
	p.m_Gravity = frandom()*-500.0f;
	p.m_Color = vec4(1.0f, frandom()*1.0, 0, Alpha);
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

void CEffects::BulletTrail(vec2 Start, vec2 End, vec4 Color)
{
	if(!m_Add100hz)
		return;

	CParticle p;
	p.SetDefault();
	
	p.m_StartPos = Start;
	p.m_EndPos = End;
	p.m_TrailDir = normalize(Start - End);
	
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
	p.m_Spr = SPRITE_DEATH1;
	p.m_Frames = 8;
	p.m_Pos = Pos + vec2(0, -24);
	p.m_LifeSpan = 0.3f;
	p.m_StartSize = 100;
	p.m_EndSize = 100;
	p.m_Rot = frandom()*pi*2;
	m_pClient->m_pParticles->Add(CParticles::GROUP_DEATH, &p);
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


void CEffects::Electrospark(vec2 Pos, float Size)
{
	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_SPARK1_1;
	if (frandom() < 0.5f)
		p.m_Spr = SPRITE_SPARK2_1;
	p.m_Frames = 3;
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
		
	case FX_ELECTRIC:
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_ELECTRIC1;
			p.m_Frames = 8;
			p.m_Pos = Pos;
			p.m_LifeSpan = 0.25f;
			p.m_StartSize = 80;
			p.m_EndSize = 80;
			p.m_Rot = frandom()*pi*2;
			m_pClient->m_pParticles->Add(CParticles::GROUP_ELECTRIC, &p);
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
			p.m_LifeSpan = 0.3f;
			p.m_StartSize = 120;
			p.m_EndSize = 120;
			p.m_Rot = frandom()*pi*2;
			m_pClient->m_pParticles->Add(CParticles::GROUP_BLOODFX, &p);
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_HAMMER_HIT, 1.0f, Pos);
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



void CEffects::Explosion(vec2 Pos)
{
	// add to flow
	for(int y = -8; y <= 8; y++)
		for(int x = -8; x <= 8; x++)
		{
			if(x == 0 && y == 0)
				continue;

			float a = 1 - (length(vec2(x,y)) / length(vec2(8,8)));
			m_pClient->m_pFlow->Add(Pos+vec2(x,y)*16, normalize(vec2(x,y))*5000.0f*a, 10.0f);
		}


	// add the explosion
	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_EXPLOSION1_1;
	p.m_Frames = 8;
	p.m_Pos = Pos;
	p.m_LifeSpan = 0.35f;
	p.m_StartSize = 200;
	p.m_EndSize = 200;
	p.m_Rot = frandom()*pi*2;
	m_pClient->m_pParticles->Add(CParticles::GROUP_EXPLOSIONS, &p);

	//Swordtracer(Pos, frandom()*pi*2);
	
	// add the smoke
	for(int i = 0; i < 20; i++)
	{
		Spark(Pos);
		Spark(Pos);
		
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_SMOKE;
		p.m_Pos = Pos;
		p.m_Vel = RandomDir() * ((1.0f + frandom()*0.2f) * 1000.0f);
		p.m_LifeSpan = 0.5f + frandom()*0.4f;
		p.m_StartSize = 32.0f + frandom()*8;
		p.m_EndSize = 0;
		p.m_Gravity = frandom()*-800.0f;
		p.m_Friction = 0.4f;
		p.m_Color = mix(vec4(0.75f,0.75f,0.75f,1.0f), vec4(0.5f,0.5f,0.5f,1.0f), frandom());
		m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
		
		
		//Triangle(Pos, RandomDir()*600);
	}
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

void CEffects::SwordHit(vec2 Pos, float Angle, bool Flip)
{
	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_SWORDHIT1;
	p.m_Frames = 3;
	p.m_Pos = Pos;
	p.m_LifeSpan = 0.15f;
	p.m_StartSize = 170;
	p.m_EndSize = 170;
	p.m_Rot = Angle;
	p.m_Flip = Flip;
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
