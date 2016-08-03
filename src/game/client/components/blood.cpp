#include <base/math.h>
#include <engine/graphics.h>
#include <engine/demo.h>

#include <game/generated/client_data.h>
#include <game/client/render.h>
#include <game/client/gameclient.h>
#include <game/gamecore.h>
#include "blood.h"


inline vec2 RandomDir() { return normalize(vec2(frandom()-0.5f, frandom()-0.5f)); }


CBlood::CBlood()
{
	OnReset();
	m_RenderBlood.m_pParts = this;
}


void CBlood::OnReset()
{
	// reset blood
	for(int i = 0; i < MAX_BLOOD; i++)
	{
		m_aBlood[i].m_PrevPart = i-1;
		m_aBlood[i].m_NextPart = i+1;
	}

	m_aBlood[0].m_PrevPart = 0;
	m_aBlood[MAX_BLOOD-1].m_NextPart = -1;
	m_FirstFree = 0;

	for(int i = 0; i < NUM_GROUPS; i++)
		m_aFirstPart[i] = -1;
}

void CBlood::Add(int Group, CBlooddrop *pPart)
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
		if(pInfo->m_Paused)
			return;
	}
	else
	{
		if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED)
			return;
	}

	if (m_FirstFree == -1)
		return;

	// remove from the free list
	int Id = m_FirstFree;
	m_FirstFree = m_aBlood[Id].m_NextPart;
	if(m_FirstFree != -1)
		m_aBlood[m_FirstFree].m_PrevPart = -1;

	// copy data
	m_aBlood[Id] = *pPart;

	// insert to the group list
	m_aBlood[Id].m_PrevPart = -1;
	m_aBlood[Id].m_NextPart = m_aFirstPart[Group];
	if(m_aFirstPart[Group] != -1)
		m_aBlood[m_aFirstPart[Group]].m_PrevPart = Id;
	m_aFirstPart[Group] = Id;

	// set some parameters
	m_aBlood[Id].m_Life = 0;
}


void CBlood::Bounce(vec2 Pos, vec2 Dir)
{
	CBlooddrop b;
	b.SetDefault();
	b.m_Pos = Pos;
	
	b.m_Spr = SPRITE_BLOOD01;
	b.m_LifeSpan = 2.0f + frandom()*2.0f;
	b.m_Rotspeed = 0.0f;
	b.m_StartSize = (30.0f + frandom()*24) / 1.75f;
	b.m_EndSize = 16.0f / 1.75f;
	b.m_Gravity = 1400.0f + frandom()*300;
	
	
	b.m_Vel = Dir * ((frandom()+0.05f)*700.0f);
	b.m_Rot = GetAngle(b.m_Vel);
	
	b.m_Friction = 0.85f+frandom()*0.075f;
	float c = frandom()*0.3f + 0.7f;
	b.m_Color = vec4(c, c, c, 1.0f);
	m_pClient->m_pBlood->Add(GROUP_BLOOD, &b);
	
	if (frandom()*10 < 2)
		m_pClient->m_pEffects->Splatter(b.m_Pos + Dir*frandom()*16.0f, b.m_Rot, b.m_StartSize * 2);
}


void CBlood::Update(float TimePassed)
{
	static float FrictionFraction = 0;
	FrictionFraction += TimePassed;

	if(FrictionFraction > 2.0f) // safty messure
		FrictionFraction = 0;

	int FrictionCount = 0;
	while(FrictionFraction > 0.05f)
	{
		FrictionCount++;
		//FrictionFraction -= 0.05f;
		FrictionFraction -= 0.075f;
	}

	for(int g = 0; g < NUM_GROUPS; g++)
	{
		int i = m_aFirstPart[g];
		while(i != -1)
		{
			int Next = m_aBlood[i].m_NextPart;
			//m_aBlood[i].vel += flow_get(m_aBlood[i].pos)*time_passed * m_aBlood[i].flow_affected;
			m_aBlood[i].m_Vel.y += m_aBlood[i].m_Gravity*TimePassed;

			for(int f = 0; f < FrictionCount; f++) // apply friction
				m_aBlood[i].m_Vel *= m_aBlood[i].m_Friction;

			// move the point
			vec2 Vel = m_aBlood[i].m_Vel*TimePassed;
			
			vec2 OldVel = Vel;
			
			if (m_aBlood[i].m_Spr >= SPRITE_BONE01)
				Collision()->MovePoint(&m_aBlood[i].m_Pos, &Vel, 0.5f+0.5f*frandom(), NULL);
			else
			if (m_aBlood[i].m_Spr > SPRITE_BLOOD01)
				Collision()->MovePoint(&m_aBlood[i].m_Pos, &Vel, 0.6f+0.6f*frandom(), NULL);
			else
				Collision()->MovePoint(&m_aBlood[i].m_Pos, &Vel, 0.1f+0.8f*frandom(), NULL);
			
			// break big blood splats into smaller ones
			if (m_aBlood[i].m_Spr != SPRITE_BLOOD01 && m_aBlood[i].m_Spr < SPRITE_BONE01)
			{
				if ((Vel.y < 0 && OldVel.y > 0) || (Vel.y > 0 && OldVel.y < 0) ||
					(Vel.x < 0 && OldVel.x > 0) || (Vel.x > 0 && OldVel.x < 0))
				{
					m_aBlood[i].m_Spr = SPRITE_BLOOD01;
					m_aBlood[i].m_StartSize /= 1.75f;
					m_aBlood[i].m_EndSize /= 1.75f;
					
					Bounce(m_aBlood[i].m_Pos, RandomDir());
				}
			}
			
			m_aBlood[i].m_Vel = Vel* (1.0f/TimePassed);

			m_aBlood[i].m_Life += TimePassed;
			
			
			if (abs(m_aBlood[i].m_Vel.x) + abs(m_aBlood[i].m_Vel.y) > 60.0f)
			{
				if (m_aBlood[i].m_Rotspeed == 0.0f)
					m_aBlood[i].m_Rot = GetAngle(m_aBlood[i].m_Vel);
				else
				{
					m_aBlood[i].m_Rot += TimePassed * m_aBlood[i].m_Rotspeed;
				}
			}
				
			// check blood death
			if(m_aBlood[i].m_Life > m_aBlood[i].m_LifeSpan)
			{
				// remove it from the group list
				if(m_aBlood[i].m_PrevPart != -1)
					m_aBlood[m_aBlood[i].m_PrevPart].m_NextPart = m_aBlood[i].m_NextPart;
				else
					m_aFirstPart[g] = m_aBlood[i].m_NextPart;

				if(m_aBlood[i].m_NextPart != -1)
					m_aBlood[m_aBlood[i].m_NextPart].m_PrevPart = m_aBlood[i].m_PrevPart;

				// insert to the free list
				if(m_FirstFree != -1)
					m_aBlood[m_FirstFree].m_PrevPart = i;
				m_aBlood[i].m_PrevPart = -1;
				m_aBlood[i].m_NextPart = m_FirstFree;
				m_FirstFree = i;
			}

			i = Next;
		}
	}
}

void CBlood::OnRender()
{
	if(Client()->State() < IClient::STATE_ONLINE)
		return;

	static int64 LastTime = 0;
	int64 t = time_get();

	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
		if(!pInfo->m_Paused)
			Update((float)((t-LastTime)/(double)time_freq())*pInfo->m_Speed);
	}
	else
	{
		if(m_pClient->m_Snap.m_pGameInfoObj && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
			Update((float)((t-LastTime)/(double)time_freq()));
	}

	LastTime = t;
}

void CBlood::RenderGroup(int Group)
{
	Graphics()->BlendNormal();
	//gfx_blend_additive();
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GORE].m_Id);
	Graphics()->QuadsBegin();

	int i = m_aFirstPart[Group];
	while(i != -1)
	{
		RenderTools()->SelectSprite(m_aBlood[i].m_Spr);
		float a = m_aBlood[i].m_Life / m_aBlood[i].m_LifeSpan;
		vec2 p = m_aBlood[i].m_Pos;
		float Size = mix(m_aBlood[i].m_StartSize, m_aBlood[i].m_EndSize, a);

		Graphics()->QuadsSetRotation(m_aBlood[i].m_Rot);

		Graphics()->SetColor(
			m_aBlood[i].m_Color.r,
			m_aBlood[i].m_Color.g,
			m_aBlood[i].m_Color.b,
			1.2f-a); // pow(a, 0.75f) *

		IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size / 2.0f);
		Graphics()->QuadsDraw(&QuadItem, 1);

		i = m_aBlood[i].m_NextPart;
	}
	Graphics()->QuadsEnd();
	Graphics()->BlendNormal();
}
