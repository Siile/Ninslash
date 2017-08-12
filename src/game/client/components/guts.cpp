#include <base/math.h>
#include <engine/graphics.h>
#include <engine/demo.h>
#include <engine/shared/config.h>

#include <game/generated/client_data.h>
#include <game/client/render.h>
#include <game/client/gameclient.h>
#include <game/client/customstuff.h>
#include <game/gamecore.h>
#include "guts.h"


inline vec2 RandomDir() { return normalize(vec2(frandom()-0.5f, frandom()-0.5f)); }


CGuts::CGuts()
{
	OnReset();
	m_RenderGuts.m_pParts = this;
}


void CGuts::OnReset()
{
	// reset blood
	for(int i = 0; i < MAX_GUTS; i++)
	{
		m_aGuts[i].m_PrevPart = i-1;
		m_aGuts[i].m_NextPart = i+1;
	}

	m_aGuts[0].m_PrevPart = 0;
	m_aGuts[MAX_GUTS-1].m_NextPart = -1;
	m_FirstFree = 0;

	for(int i = 0; i < NUM_GROUPS; i++)
		m_aFirstPart[i] = -1;
}

void CGuts::Add(int Group, CGutSpill *pPart)
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
	m_FirstFree = m_aGuts[Id].m_NextPart;
	if(m_FirstFree != -1)
		m_aGuts[m_FirstFree].m_PrevPart = -1;

	// copy data
	m_aGuts[Id] = *pPart;

	// insert to the group list
	m_aGuts[Id].m_PrevPart = -1;
	m_aGuts[Id].m_NextPart = m_aFirstPart[Group];
	if(m_aFirstPart[Group] != -1)
		m_aGuts[m_aFirstPart[Group]].m_PrevPart = Id;
	m_aFirstPart[Group] = Id;

	// set some parameters
	m_aGuts[Id].m_Life = 0;
}


void CGuts::Bounce(vec2 Pos, vec2 Dir, int Group, vec4 Color)
{
	/*
	CGutSpill b;
	b.SetDefault();
	b.m_Pos = Pos;
	
	b.m_Spr = SPRITE_BLOOD01;
	b.m_LifeSpan = 2.0f + frandom()*2.0f;
	b.m_Rotspeed = 0.0f;
	b.m_StartSize = (30.0f + frandom()*24) / 1.75f;
	b.m_EndSize = 16.0f / 1.75f;
	b.m_Gravity = 1500.0f + frandom()*400;
	b.m_Friction = 0.7f+frandom()*0.075f;

	if (g_Config.m_GfxMultiBuffering)
	{
		b.m_Rotspeed = 0.0f;
		//b.m_StartSize *= 1.5f;
		b.m_StartSize = 42.0f + frandom()*16;
		b.m_EndSize = 4.0f;
		b.m_LifeSpan = 4.0f;
		b.m_Friction = 0.85f+frandom()*0.075f;
	}
	
	b.m_Vel = Dir * ((frandom()+0.15f)*700.0f);
	b.m_Rot = GetAngle(b.m_Vel);
	
	//float c = frandom()*0.3f + 0.7f;
	//b.m_Color = vec4(c, c, c, 1.0f);
	b.m_Color = Color;
	m_pClient->m_pGuts->Add(Group, &b);
	
	if (g_Config.m_GfxMultiBuffering && frandom()*10 < 3 && Group == GROUP_GUTS)
		m_pClient->m_pEffects->Splatter(b.m_Pos + Dir*frandom()*48.0f - Dir*frandom()*16.0f, b.m_Rot, b.m_StartSize * 2, Color);
	*/
}


void CGuts::Update(float TimePassed)
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
			int Next = m_aGuts[i].m_NextPart;
			//m_aGuts[i].vel += flow_get(m_aGuts[i].pos)*time_passed * m_aGuts[i].flow_affected;
			m_aGuts[i].m_Vel.y += m_aGuts[i].m_Gravity*TimePassed;

			
			// ugly way to force tiles to blood
			int OnForceTile = Collision()->IsForceTile(m_aGuts[i].m_Pos.x, m_aGuts[i].m_Pos.y+4);
			

			for(int f = 0; f < FrictionCount; f++) // apply friction
				m_aGuts[i].m_Vel *= m_aGuts[i].m_Friction;
			
			if (OnForceTile != 0)
				//m_aGuts[i].m_Vel.x = (m_aGuts[i].m_Vel.x + OnForceTile*400) / 2.0f;
				m_aGuts[i].m_Vel.x = OnForceTile*250;
				
			vec2 Force = m_aGuts[i].m_Vel;
				
			if (CustomStuff()->Impact(m_aGuts[i].m_Pos, &Force))
			{
				m_aGuts[i].m_Pos += Force*10.0f;
				m_aGuts[i].m_Vel += Force*(700.0f+frandom()*700);
				
				if (frandom()*20 < 2)
					m_pClient->AddPlayerSplatter(m_aGuts[i].m_Pos, m_aGuts[i].m_Color);
			}
				
			// move the point
			vec2 Vel = m_aGuts[i].m_Vel*TimePassed;
			
			vec2 OldVel = Vel;
			//Vel.x += OnForceTile*1;
			
			if (OnForceTile != 0)
				Collision()->MovePoint(&m_aGuts[i].m_Pos, &Vel, 0.99f, NULL);
			else
				Collision()->MovePoint(&m_aGuts[i].m_Pos, &Vel, 0.2f+0.5f*frandom(), NULL);

			/*
			// break big blood splats into smaller ones
			if (m_aGuts[i].m_Spr != SPRITE_BLOOD01 && m_aGuts[i].m_Spr < SPRITE_BONE01)
			{
				if ((Vel.y < 0 && OldVel.y > 0) || (Vel.y > 0 && OldVel.y < 0) ||
					(Vel.x < 0 && OldVel.x > 0) || (Vel.x > 0 && OldVel.x < 0))
				{
					m_aGuts[i].m_Spr = SPRITE_BLOOD01;
					m_aGuts[i].m_StartSize /= 1.75f;
					m_aGuts[i].m_EndSize /= 1.75f;
					
					Bounce(m_aGuts[i].m_Pos, RandomDir(), g, m_aGuts[i].m_Color);
				}
			}
			*/

			// stick to walls and ceiling
			vec2 P = m_aGuts[i].m_Pos;
			
			/*
			// not too close the floor
			if (!Collision()->IsTileSolid(P.x, P.y+16))
			{
				if (Collision()->IsTileSolid(P.x-16, P.y))
				{
					float Dist = abs(P.x-int((P.x-16)/32)*32.0f);
					
					Vel.x -= Dist*0.1f;
				}
				else
				if (Collision()->IsTileSolid(P.x+16, P.y))
				{
					float Dist = abs(P.x-int((P.x+16)/32)*32.0f);
					
					Vel.x += Dist*0.1f;
				}
			}
			*/
			
			m_aGuts[i].m_Vel = Vel* (1.0f/TimePassed);

			m_aGuts[i].m_Life += TimePassed;
			
			
			if (abs(m_aGuts[i].m_Vel.x) + abs(m_aGuts[i].m_Vel.y) > 60.0f)
			{
				if (m_aGuts[i].m_Rotspeed == 0.0f)
					m_aGuts[i].m_Rot = GetAngle(m_aGuts[i].m_Vel);
				else
				{
					m_aGuts[i].m_Rot += TimePassed * m_aGuts[i].m_Rotspeed;
				}
			}
			else
			{	
			}
				
			// check blood death
			if(m_aGuts[i].m_Life > m_aGuts[i].m_LifeSpan)
			{
				// remove it from the group list
				if(m_aGuts[i].m_PrevPart != -1)
					m_aGuts[m_aGuts[i].m_PrevPart].m_NextPart = m_aGuts[i].m_NextPart;
				else
					m_aFirstPart[g] = m_aGuts[i].m_NextPart;

				if(m_aGuts[i].m_NextPart != -1)
					m_aGuts[m_aGuts[i].m_NextPart].m_PrevPart = m_aGuts[i].m_PrevPart;

				// insert to the free list
				if(m_FirstFree != -1)
					m_aGuts[m_FirstFree].m_PrevPart = i;
				m_aGuts[i].m_PrevPart = -1;
				m_aGuts[i].m_NextPart = m_FirstFree;
				m_FirstFree = i;
			}

			i = Next;
		}
	}
}

void CGuts::OnRender()
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


void CGuts::RenderGroup(int Group)
{
	if (Group == GROUP_GUTS)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GUTS].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aGuts[i].m_Life / m_aGuts[i].m_LifeSpan;
			vec2 p = m_aGuts[i].m_Pos;

			float Size = mix(m_aGuts[i].m_StartSize, m_aGuts[i].m_EndSize*1.0f, a);
			//RenderTools()->SelectSprite(m_aGuts[i].m_Spr + a*m_aGuts[i].m_Frames);
			Graphics()->QuadsSetRotation(m_aGuts[i].m_Rot);
			//Graphics()->SetColor(m_aGuts[i].m_Color.r, m_aGuts[i].m_Color.g, m_aGuts[i].m_Color.b, 1);
			Graphics()->SetColor(1, 1, 1, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);

			i = m_aGuts[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
}















