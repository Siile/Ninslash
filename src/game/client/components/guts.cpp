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
		
		if (g == GROUP_GUTS)
		{
			while(i != -1)
			{
				int Next = m_aGuts[i].m_NextPart;
				//m_aGuts[i].vel += flow_get(m_aGuts[i].pos)*time_passed * m_aGuts[i].flow_affected;
				
				int Parts = m_aGuts[i].m_Parts;
				
				for (int p = 0; p < Parts; p++)
				{
					m_aGuts[i].m_aVel[p].y += m_aGuts[i].m_Gravity*TimePassed;

					// ugly way to force tiles to blood
					int OnForceTile = Collision()->IsForceTile(m_aGuts[i].m_aPos[p].x, m_aGuts[i].m_aPos[p].y+4);
					
					for(int f = 0; f < FrictionCount; f++) // apply friction
						m_aGuts[i].m_aVel[p] *= m_aGuts[i].m_Friction;
					
					if (OnForceTile != 0)
						m_aGuts[i].m_aVel[p].x = OnForceTile*250;
						
					vec2 Force = m_aGuts[i].m_aVel[p];
						
					if (CustomStuff()->Impact(m_aGuts[i].m_aPos[p], &Force))
					{
						m_aGuts[i].m_aPos[p] += Force*10.0f;
						m_aGuts[i].m_aVel[p] += Force*(700.0f+frandom()*700);
						
						if (frandom()*20 < 2)
							m_pClient->AddPlayerSplatter(m_aGuts[i].m_aPos[p], m_aGuts[i].m_Color);
					}
					
					if (p == 0)
					{
						vec2 n = m_aGuts[i].m_aPos[p] - m_aGuts[i].m_aPos[p+1];
						float d = abs(length(n));
						
						if (d > m_aGuts[i].m_ControlDist)
							m_aGuts[i].m_aVel[p] -= n*0.5f;
						
						//if (frandom() < TimePassed*10 && m_aGuts[i].m_Life < 1.0f)
						//	m_pClient->m_pEffects->Blood(m_aGuts[i].m_aPos[p], normalize(n)*4.0f + m_aGuts[i].m_aVel[p]/2, m_aGuts[i].m_Color);
					}
					else if (p == Parts-1)
					{
						vec2 n = m_aGuts[i].m_aPos[p-1] - m_aGuts[i].m_aPos[p];
						float d = abs(length(n));
						
						if (d > m_aGuts[i].m_ControlDist)
							m_aGuts[i].m_aVel[p] += n*0.5f;
					}
					else
					{
						vec2 n = m_aGuts[i].m_aPos[p] - m_aGuts[i].m_aPos[p-1];
						float d = abs(length(n));
						
						if (d > m_aGuts[i].m_ControlDist)
							m_aGuts[i].m_aVel[p] -= n*0.5f;
						
						if (d < m_aGuts[i].m_ControlDist/2)
							m_aGuts[i].m_aVel[p] += n*0.3f;
						
						n = m_aGuts[i].m_aPos[p+1] - m_aGuts[i].m_aPos[p];
						d = abs(length(n));
						
						if (d > m_aGuts[i].m_ControlDist)
							m_aGuts[i].m_aVel[p] += n*0.5f;
						
						if (d < m_aGuts[i].m_ControlDist/2)
							m_aGuts[i].m_aVel[p] -= n*0.3f;
					}

					
					// move the point
					vec2 Vel = m_aGuts[i].m_aVel[p]*TimePassed;
					
					vec2 OldVel = Vel;
					//Vel.x += OnForceTile*1;

					if (OnForceTile != 0)
						Collision()->MoveBox(&m_aGuts[i].m_aPos[p], &Vel, vec2(6, 6), 0.99f, false);
					else
						Collision()->MoveBox(&m_aGuts[i].m_aPos[p], &Vel, vec2(6, 6), 0.85f, false);
					
					// stick to walls and ceiling
					vec2 P = m_aGuts[i].m_aPos[p];
					

					m_aGuts[i].m_aVel[p] = Vel* (1.0f/TimePassed);
				}

				m_aGuts[i].m_Life += TimePassed;

					
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
		//Graphics()->TextureSet(-1);
		Graphics()->QuadsBegin();
				
		Graphics()->QuadsSetRotation(0);
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aGuts[i].m_Life / m_aGuts[i].m_LifeSpan;
			a = min(1.0f, (1-a)*2.5f);
			//Graphics()->SetColor(1, 1, 1, 1-a);
			Graphics()->SetColor(m_aGuts[i].m_Color.r*0.6f + 0.4f, m_aGuts[i].m_Color.g*0.6f + 0.4f, m_aGuts[i].m_Color.b*0.7f + 0.3f, a);
			
			vec2 aPos[9];
			
			int Parts = m_aGuts[i].m_Parts;
			
			for (int f = 0; f < Parts; f++)
				aPos[f*2] = m_aGuts[i].m_aPos[f]+vec2(0, 6);
			
			for (int f = 0; f < Parts-1; f++)
				aPos[1+f*2] = (aPos[f*2] + aPos[2+f*2]) / 2;
			
			for (int f = 1; f < Parts-1; f++)
				aPos[f*2] = (aPos[f*2] + (aPos[f*2-1]+aPos[f*2+1])/2) / 2;
			
			
			
			vec2 p1 = aPos[0];
			vec2 TrailDir = normalize(p1 - aPos[1]);
			float s1 = 10.0f-m_aGuts[i].m_Spr*2.0f;
			vec2 Out1 = vec2(TrailDir.y, -TrailDir.x) * s1;
			
			float Frames = (Parts-1)*2;
			
			for (int t = 1; t < (Parts-1)*2+1; t++)
			{
						
				vec2 p2 = aPos[t];
				
				TrailDir = normalize(p1 - p2);
				
				Graphics()->QuadsSetSubsetFree(	(t-1)/Frames, m_aGuts[i].m_Spr*0.5f, 
												(t)/Frames, m_aGuts[i].m_Spr*0.5f, 
												(t-1)/Frames, m_aGuts[i].m_Spr*0.5f+0.5f, 
												(t)/Frames, m_aGuts[i].m_Spr*0.5f+0.5f);

				
				vec2 Out2 = vec2(TrailDir.y, -TrailDir.x) * s1;
								

				IGraphics::CFreeformItem Freeform1(
					p1.x-Out1.x, p1.y-Out1.y,
					p2.x-Out2.x, p2.y-Out2.y,
					p1.x+Out1.x, p1.y+Out1.y,
					p2.x+Out2.x, p2.y+Out2.y);
				
				Graphics()->QuadsDrawFreeform(&Freeform1, 1);
				
				p1 = p2;
				Out1 = Out2;
			}
			
			i = m_aGuts[i].m_NextPart;
		}
		
		Graphics()->QuadsEnd();
	}
}















