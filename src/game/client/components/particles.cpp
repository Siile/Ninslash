#include <base/math.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/demo.h>
#include <engine/shared/config.h>

#include <generated/game_data.h>
#include <game/client/render.h>
#include <game/client/gameclient.h>
#include <game/client/customstuff.h>
#include <game/gamecore.h>
#include "particles.h"

CParticles::CParticles()
{
	OnReset();
	m_RenderTrail.m_pParts = this;
	m_RenderColorTrail.m_pParts = this;
	m_RenderExplosions.m_pParts = this;
	m_RenderGreenExplosion.m_pParts = this;
	m_RenderHitEffects.m_pParts = this;
	m_RenderGeneral.m_pParts = this;
	m_RenderTriangles.m_pParts = this;
	m_RenderFlames.m_pParts = this;
	m_RenderFlame1.m_pParts = this;
	m_RenderSmoke1.m_pParts = this;
	m_RenderMine1.m_pParts = this;
	m_RenderMine2.m_pParts = this;
	m_RenderElectromine.m_pParts = this;
	m_RenderElectric.m_pParts = this;
	m_RenderSwordHits.m_pParts = this;
	m_RenderClawHits.m_pParts = this;
	m_RenderDeath.m_pParts = this;
	m_RenderLazer.m_pParts = this;
	m_RenderLazerload.m_pParts = this;
	m_RenderSparks.m_pParts = this;
	m_RenderTakeoff.m_pParts = this;
	m_RenderBloodFX.m_pParts = this;
	m_RenderPlayerSpawn.m_pParts = this;
	m_RenderMonsterSpawn.m_pParts = this;
	m_RenderCrafting.m_pParts = this;
	m_RenderDamageInd.m_pParts = this;
	m_RenderMeat.m_pParts = this;
	m_RenderEffect1.m_pParts = this;
}


void CParticles::OnReset()
{
	// reset particles
	for(int i = 0; i < MAX_PARTICLES; i++)
	{
		m_aParticles[i].m_PrevPart = i-1;
		m_aParticles[i].m_NextPart = i+1;
		m_aParticles[i].m_Pos = vec2(0,0);
	}

	m_aParticles[0].m_PrevPart = 0;
	m_aParticles[MAX_PARTICLES-1].m_NextPart = -1;
	m_FirstFree = 0;

	for(int i = 0; i < NUM_GROUPS; i++)
		m_aFirstPart[i] = -1;
}

void CParticles::Add(int Group, CParticle *pPart)
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
	m_FirstFree = m_aParticles[Id].m_NextPart;
	if(m_FirstFree != -1)
		m_aParticles[m_FirstFree].m_PrevPart = -1;

	// copy data
	m_aParticles[Id] = *pPart;

	// insert to the group list
	m_aParticles[Id].m_PrevPart = -1;
	m_aParticles[Id].m_NextPart = m_aFirstPart[Group];
	if(m_aFirstPart[Group] != -1)
		m_aParticles[m_aFirstPart[Group]].m_PrevPart = Id;
	m_aFirstPart[Group] = Id;

	// set some parameters
	m_aParticles[Id].m_Life = 0;
}

void CParticles::Update(float TimePassed)
{
	static float FrictionFraction = 0;
	FrictionFraction += TimePassed;

	if(FrictionFraction > 2.0f) // safty messure
		FrictionFraction = 0;

	int FrictionCount = 0;
	while(FrictionFraction > 0.05f)
	{
		FrictionCount++;
		FrictionFraction -= 0.05f;
	}

	for(int g = 0; g < NUM_GROUPS; g++)
	{
		int i = m_aFirstPart[g];
		while(i != -1)
		{
			int Next = m_aParticles[i].m_NextPart;
			//m_aParticles[i].vel += flow_get(m_aParticles[i].pos)*time_passed * m_aParticles[i].flow_affected;
			m_aParticles[i].m_Vel.y += m_aParticles[i].m_Gravity*TimePassed;

			
			for(int f = 0; f < FrictionCount; f++) // apply friction
				m_aParticles[i].m_Vel *= m_aParticles[i].m_Friction;

			
			if (m_aParticles[i].m_Chunk)
			{
				int OnForceTile = Collision()->IsForceTile(m_aParticles[i].m_Pos.x, m_aParticles[i].m_Pos.y+4);
				
				if (OnForceTile != 0)
				{
					m_aParticles[i].m_Vel.x = OnForceTile*250;
					m_aParticles[i].m_Pos.y -= 1.0f;
				}
				
				vec2 Force = m_aParticles[i].m_Vel;
				
				if (CustomStuff()->Impact(m_aParticles[i].m_Pos, &Force))
				{
					//m_aParticles[i].m_Pos.y -= 10.0f;
					//m_aParticles[i].m_Vel.y = -1000.0f-frandom()*400.0f;
					m_aParticles[i].m_Pos += Force*10.0f;
					m_aParticles[i].m_Vel += Force*(700.0f+frandom()*700);
				}
			}
				
			// move the point
			vec2 Vel = m_aParticles[i].m_Vel*TimePassed;
			bool Collided = Collision()->MovePoint(&m_aParticles[i].m_Pos, &Vel, 0.1f+0.9f*frandom(), NULL, m_aParticles[i].m_IgnoreCollision);
			m_aParticles[i].m_Vel = Vel* (1.0f/TimePassed);

			m_aParticles[i].m_Life += TimePassed;
			m_aParticles[i].m_Rot += TimePassed * m_aParticles[i].m_Rotspeed;

			// meat chunks' rotation
			if (m_aParticles[i].m_Chunk && Collided)
			{
				m_aParticles[i].m_Rotspeed += (m_aParticles[i].m_Vel.x / 40 - m_aParticles[i].m_Rotspeed) / 2.0f;
				if (g_Config.m_GoreBlood > 0 && abs(m_aParticles[i].m_Vel.x) + abs(m_aParticles[i].m_Vel.y) > 340)
				{
					vec4 Color = vec4(1, 0, 0, 1);
					
								
					switch (m_aParticles[i].m_Special)
					{
						case 1: Color = vec4(0, 1, 0, 1); break;
						case 2: Color = vec4(0, 0, 1, 1); break;
					};
					
					m_pClient->m_pEffects->Blood(m_aParticles[i].m_Pos, vec2(frandom()-frandom(), frandom()-frandom()) * 0.2f, Color);
				}
			}
			
			if (g == GROUP_EXPLOSIONS)
			{
				if (!m_aParticles[i].m_Special)
				{
					m_aParticles[i].m_Special++;
					
					float r = 25;
					vec2 p = m_aParticles[i].m_Pos + vec2(-50, -20);
					CustomStuff()->AddImpact(vec4(p.x-r, p.y-r, p.x+r, p.y+r), CCustomStuff::IMPACT_GRENADE, vec2(-0.6f, -0.6f));

					p = m_aParticles[i].m_Pos + vec2(0, -20);
					CustomStuff()->AddImpact(vec4(p.x-r, p.y-r, p.x+r, p.y+r), CCustomStuff::IMPACT_GRENADE, vec2(0, -1));

					p = m_aParticles[i].m_Pos + vec2(50, -20);
					CustomStuff()->AddImpact(vec4(p.x-r, p.y-r, p.x+r, p.y+r), CCustomStuff::IMPACT_GRENADE, vec2(0.6f, 0.6f));
				}
			}
			
			// check particle death
			if(m_aParticles[i].m_Life > m_aParticles[i].m_LifeSpan)
			{
				// remove it from the group list
				if(m_aParticles[i].m_PrevPart != -1)
					m_aParticles[m_aParticles[i].m_PrevPart].m_NextPart = m_aParticles[i].m_NextPart;
				else
					m_aFirstPart[g] = m_aParticles[i].m_NextPart;

				if(m_aParticles[i].m_NextPart != -1)
					m_aParticles[m_aParticles[i].m_NextPart].m_PrevPart = m_aParticles[i].m_PrevPart;

				// insert to the free list
				if(m_FirstFree != -1)
					m_aParticles[m_FirstFree].m_PrevPart = i;
				m_aParticles[i].m_PrevPart = -1;
				m_aParticles[i].m_NextPart = m_FirstFree;
				m_FirstFree = i;
			}

			i = Next;
		}
	}
}

void CParticles::OnRender()
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


void CParticles::RenderLights()
{
	{
		int i = m_aFirstPart[GROUP_EXPLOSIONS];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;
			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.5f, a)*1.5f;
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1-a);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);

			i = m_aParticles[i].m_NextPart;
		}
	}
}


void CParticles::RenderGroup(int Group)
{
	if (Group == GROUP_MINE1)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_MINE1].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_ELECTROMINE)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ELECTROMINE].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size/2, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_DAMAGEIND)
	{
			
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a*2.0f);

			char aBuf[8];
			str_format(aBuf, sizeof(aBuf), "%d", m_aParticles[i].m_Spr);
			
			vec4 c = m_aParticles[i].m_Color;
			
			TextRender()->TextColor(c.r, c.g, c.b, 1-a);
			TextRender()->Text(0, p.x, p.y, Size, aBuf, -1);
			
			i = m_aParticles[i].m_NextPart;
		}
		
		TextRender()->TextColor(1, 1, 1, 1);
	}
	else if (Group == GROUP_DEATH)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_DEATH].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_TAKEOFF)
	{
		Graphics()->BlendNormal();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_TAKEOFF].m_Id);
			Graphics()->QuadsBegin();
			
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);

			i = m_aParticles[i].m_NextPart;
			Graphics()->QuadsEnd();
			
			RenderTools()->RenderWalker(p+vec2(0, 62), -1, a/3, 1, 0, 0);
		}
	}
	else if (Group == GROUP_FLAME1)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_FLAME1].m_Id);
		Graphics()->QuadsBegin();

		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size/2);
			Graphics()->QuadsDraw(&QuadItem, 1);
			
			//m_pClient->m_pEffects->BoxLight(p, vec4(1.0f, 0.8f, 0.6f, 0.5f-a/2.0f), vec2(256, 64));

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_LAZERLOAD)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_LAZERLOAD].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);

			m_pClient->m_pEffects->SimpleLight(p, vec4(1.0f, 0.8f, 0.6f, 0.5f-a/2.0f), Size);

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_LAZER)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_LAZER].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y+m_aParticles[i].m_Height/2, Size, m_aParticles[i].m_Height);
			Graphics()->QuadsDraw(&QuadItem, 1);
			
			m_pClient->m_pEffects->BoxLight(vec2(p.x, p.y+m_aParticles[i].m_Height/2), vec4(1.0f, 0.75f, 0.5f, 1-a), vec2(70, m_aParticles[i].m_Height+70));
			
			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_CRAFTING)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_PICKUPS].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1.0f - a);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			
			RenderTools()->SelectSprite(SPRITE_REPAIR);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);
			
			if (m_aParticles[i].m_Spr != SPRITE_REPAIR)
			{
				int iw = clamp(m_aParticles[i].m_Spr, 0, NUM_WEAPONS-1);
				RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_pSpritePickup);
				IGraphics::CQuadItem QuadItem(p.x, p.y, Size/2, Size);
				Graphics()->QuadsDraw(&QuadItem, 1);
			}

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_SMOKE1)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_SMOKE1].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size/2);
			Graphics()->QuadsDraw(&QuadItem, 1);

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_SWORDHITS)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_SWORDHIT].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames, m_aParticles[i].m_Flip ? SPRITE_FLAG_FLIP_Y : 0);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_CLAWHITS)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CLAWHIT].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames, m_aParticles[i].m_Flip ? SPRITE_FLAG_FLIP_Y : 0);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_PLAYERSPAWN)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_PLAYERSPAWN].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames, m_aParticles[i].m_Flip ? SPRITE_FLAG_FLIP_Y : 0);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size*0.5f, Size*1.5f);
			Graphics()->QuadsDraw(&QuadItem, 1);

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_MONSTERSPAWN)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_MONSTERSPAWN].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames, m_aParticles[i].m_Flip ? SPRITE_FLAG_FLIP_Y : 0);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size*1.0f, Size*1.5f);
			Graphics()->QuadsDraw(&QuadItem, 1);

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_BLOODFX)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BLOODFX].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames, m_aParticles[i].m_Flip ? SPRITE_FLAG_FLIP_Y : 0);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_MINE2)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_MINE2].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_ELECTRIC)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ELECTRIC].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, m_aParticles[i].m_Color.a);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);

			m_pClient->m_pEffects->SimpleLight(m_aParticles[i].m_Pos, vec4(0.6f, 0.8f, 1.0f, 0.5f-a/2.0f), Size);

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_GREEN_EXPLOSION)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GREEN_EXPLOSION].m_Id);
		//Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ELECTRIC].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);
			
			m_pClient->m_pEffects->SimpleLight(m_aParticles[i].m_Pos, vec4(0.6f, 1.0f, 0.8f, 0.5f-a/2.0f), Size);

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_EFFECT1)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_FX_EFFECT1].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_SPARKS)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_SPARKS].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);
			
			m_pClient->m_pEffects->SimpleLight(m_aParticles[i].m_Pos, vec4(0.6f, 0.8f, 1.0f, 0.5f-a/2.0f), Size*1.5f);

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_MEAT)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_MEAT].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;
			float alpha = min(1.0f, (1.0f-a)*1.5f);

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, alpha);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1-a);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);
			
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr+1);
			
				
			switch (m_aParticles[i].m_Special)
			{
				case 1: Graphics()->SetColor(0, 1, 0, alpha); break;
				case 2: Graphics()->SetColor(0, 0, 0, alpha); break;
				default: Graphics()->SetColor(1, 0, 0, alpha); break;
			};
			
			//Graphics()->SetColor(1, 1, 1, 1-a);
			IGraphics::CQuadItem QuadItem2(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem2, 1);

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_HITEFFECTS)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ELECTROHIT].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*m_aParticles[i].m_Frames);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);

			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group == GROUP_EXPLOSIONS)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_EXPLOSION].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;

			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize*1.0f, a);
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr + a*8);
			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);
			Graphics()->SetColor(m_aParticles[i].m_Color.r, m_aParticles[i].m_Color.g, m_aParticles[i].m_Color.b, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);

			m_pClient->m_pEffects->SimpleLight(m_aParticles[i].m_Pos, vec4(1.0f, 0.9f, 0.8f, 1-a*0.7f), Size*1.2f);
			
			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
	}
	else if (Group != GROUP_COLORTRAIL)
	{
		Graphics()->BlendNormal();
		//gfx_blend_additive();

		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_PARTICLES].m_Id);
		
		// the others
		Graphics()->QuadsBegin();

		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			RenderTools()->SelectSprite(m_aParticles[i].m_Spr);
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			vec2 p = m_aParticles[i].m_Pos;
			float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize, a);

			Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);

			if (Group == GROUP_TRIANGLES || Group == GROUP_FLAMES)
			{
				float l = 0.3f - a*1.0f;
				const vec4 c = vec4(m_aParticles[i].m_Color.r,
					m_aParticles[i].m_Color.g+l,
					m_aParticles[i].m_Color.b+l,
					m_aParticles[i].m_Color.a * (1 - a));
					
				Graphics()->SetColor(c.r, c.g, c.b, c.a);
					
				m_pClient->m_pEffects->SimpleLight(p, c, Size*2);
					
				//if (Group == GROUP_FLAMES)
				//	m_pClient->m_pEffects->Light(m_aParticles[i].m_Pos, 32*(1.0f-m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan));
			}
			else
				Graphics()->SetColor(
					m_aParticles[i].m_Color.r,
					m_aParticles[i].m_Color.g,
					m_aParticles[i].m_Color.b,
					1 - a);

			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);


			i = m_aParticles[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
		Graphics()->BlendNormal();
	}
	else
	{
		// bullet tracers
		vec2 Out, Border;
		
		Graphics()->BlendNormal();
		Graphics()->TextureSet(-1);	
		//Graphics()->TextureSet(g_pData->m_aImages[IMAGE_PARTICLES].m_Id);
		Graphics()->QuadsBegin();
		
		//RenderTools()->SelectSprite(SPRITE_PART_BULLETTRACE);
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
			Graphics()->SetColor(m_aParticles[i].m_Color.x, m_aParticles[i].m_Color.y, m_aParticles[i].m_Color.z, m_aParticles[i].m_Color.w * (1.0f - a));
		
			Out = vec2(m_aParticles[i].m_TrailDir.y, -m_aParticles[i].m_TrailDir.x) * m_aParticles[i].m_StartSize;
			
			IGraphics::CFreeformItem Freeform1(
					m_aParticles[i].m_StartPos.x-Out.x, m_aParticles[i].m_StartPos.y-Out.y,
					m_aParticles[i].m_StartPos.x+Out.x, m_aParticles[i].m_StartPos.y+Out.y,
					m_aParticles[i].m_EndPos.x-Out.x, m_aParticles[i].m_EndPos.y-Out.y,
					m_aParticles[i].m_EndPos.x+Out.x, m_aParticles[i].m_EndPos.y+Out.y);
			Graphics()->QuadsDrawFreeform(&Freeform1, 1);
			
			i = m_aParticles[i].m_NextPart;
		}
	
		Graphics()->QuadsEnd();
		Graphics()->BlendNormal();
	}
}
