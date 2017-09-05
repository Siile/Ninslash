#include <base/math.h>
#include <engine/graphics.h>
#include <engine/demo.h>
#include <engine/shared/config.h>

#include <game/generated/client_data.h>
#include <game/client/render.h>
#include <game/client/gameclient.h>
#include <game/client/customstuff.h>
#include <game/gamecore.h>
#include "brains.h"


inline vec2 RandomDir() { return normalize(vec2(frandom()-0.5f, frandom()-0.5f)); }


CBrains::CBrains()
{
	OnReset();
	m_RenderBrains.m_pParts = this;
}


void CBrains::OnReset()
{
	// reset blood
	for(int i = 0; i < MAX_BRAINS; i++)
	{
		m_aBrains[i].m_PrevPart = i-1;
		m_aBrains[i].m_NextPart = i+1;
	}

	m_aBrains[0].m_PrevPart = 0;
	m_aBrains[MAX_BRAINS-1].m_NextPart = -1;
	m_FirstFree = 0;

	for(int i = 0; i < NUM_GROUPS; i++)
		m_aFirstPart[i] = -1;
}

void CBrains::Add(int Group, CBrainSpill *pPart)
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
	m_FirstFree = m_aBrains[Id].m_NextPart;
	if(m_FirstFree != -1)
		m_aBrains[m_FirstFree].m_PrevPart = -1;

	// copy data
	m_aBrains[Id] = *pPart;

	// insert to the group list
	m_aBrains[Id].m_PrevPart = -1;
	m_aBrains[Id].m_NextPart = m_aFirstPart[Group];
	if(m_aFirstPart[Group] != -1)
		m_aBrains[m_aFirstPart[Group]].m_PrevPart = Id;
	m_aFirstPart[Group] = Id;

	// set some parameters
	m_aBrains[Id].m_Life = 0;
}


void CBrains::Update(float TimePassed)
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
		
		if (g == GROUP_BRAINS)
		{
			while(i != -1)
			{
				int Next = m_aBrains[i].m_NextPart;
				//m_aBrains[i].vel += flow_get(m_aBrains[i].pos)*time_passed * m_aBrains[i].flow_affected;
				
				for (int p = 0; p < 9; p++)
				{
					//if (p == 0)
						m_aBrains[i].m_aVel[p].y += m_aBrains[i].m_Gravity*TimePassed;

					// ugly way to force tiles to blood
					int OnForceTile = Collision()->IsForceTile(m_aBrains[i].m_aPos[p].x, m_aBrains[i].m_aPos[p].y+4);
					
					for(int f = 0; f < FrictionCount; f++) // apply friction
						m_aBrains[i].m_aVel[p] *= m_aBrains[i].m_Friction;
					
					if (OnForceTile != 0)
						m_aBrains[i].m_aVel[p].x = OnForceTile*250;
						
					vec2 Force = m_aBrains[i].m_aVel[p];
						
					if (CustomStuff()->Impact(m_aBrains[i].m_aPos[p], &Force))
					{
						m_aBrains[i].m_aPos[p] += Force*10.0f;
						m_aBrains[i].m_aVel[p] += Force*(700.0f+frandom()*700);
						
						if (frandom()*20 < 2)
							m_pClient->AddPlayerSplatter(m_aBrains[i].m_aPos[p], m_aBrains[i].m_Color);
					}
					
					
					// hold the brain together
					if (p != 0)
					{
						float a = 2*pi / 8.0f * (p-1);
						float s = 24.0f;
						vec2 n = m_aBrains[i].m_aPos[p] - (m_aBrains[i].m_aPos[0]+vec2(sin(a), cos(a))*s);
						//float d = abs(length(n));
						
						/*if (d > 20)
							m_aBrains[i].m_aVel[p] -= n*0.5f;
						
						if (d < 10)*/
						
						//m_aBrains[i].m_aVel[p] *= 0.0f;
						//m_aBrains[i].m_aPos[p] = (m_aBrains[i].m_aPos[0]+vec2(sin(a), cos(a))*s);
						
						m_aBrains[i].m_aVel[p] *= 0.9f;
						m_aBrains[i].m_aVel[p] -= n*0.9f;
					}

					
					// move the point
					vec2 Vel = m_aBrains[i].m_aVel[p]*TimePassed;
					
					vec2 OldVel = Vel;
					//Vel.x += OnForceTile*1;

					if (OnForceTile != 0)
						Collision()->MoveBox(&m_aBrains[i].m_aPos[p], &Vel, vec2(6, 6), 0.99f, false);
					else
						Collision()->MoveBox(&m_aBrains[i].m_aPos[p], &Vel, vec2(6, 6), 0.9f, false);
					
					// stick to walls and ceiling
					vec2 P = m_aBrains[i].m_aPos[p];
					

					m_aBrains[i].m_aVel[p] = Vel* (1.0f/TimePassed);
				}

				m_aBrains[i].m_Life += TimePassed;

					
				// check blood death
				if(m_aBrains[i].m_Life > m_aBrains[i].m_LifeSpan)
				{
					// remove it from the group list
					if(m_aBrains[i].m_PrevPart != -1)
						m_aBrains[m_aBrains[i].m_PrevPart].m_NextPart = m_aBrains[i].m_NextPart;
					else
						m_aFirstPart[g] = m_aBrains[i].m_NextPart;

					if(m_aBrains[i].m_NextPart != -1)
						m_aBrains[m_aBrains[i].m_NextPart].m_PrevPart = m_aBrains[i].m_PrevPart;

					// insert to the free list
					if(m_FirstFree != -1)
						m_aBrains[m_FirstFree].m_PrevPart = i;
					m_aBrains[i].m_PrevPart = -1;
					m_aBrains[i].m_NextPart = m_FirstFree;
					m_FirstFree = i;
				}

				i = Next;
			}
		}
	}
}

void CBrains::OnRender()
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


void CBrains::RenderGroup(int Group)
{
	if (Group == GROUP_BRAINS)
	{
		/*
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GUTS].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aBrains[i].m_Life / m_aBrains[i].m_LifeSpan;
			vec2 p = m_aBrains[i].m_Pos;

			float Size = mix(m_aBrains[i].m_StartSize, m_aBrains[i].m_EndSize*1.0f, a);
			//RenderTools()->SelectSprite(m_aBrains[i].m_Spr + a*m_aBrains[i].m_Frames);
			Graphics()->QuadsSetRotation(m_aBrains[i].m_Rot);
			//Graphics()->SetColor(m_aBrains[i].m_Color.r, m_aBrains[i].m_Color.g, m_aBrains[i].m_Color.b, 1);
			Graphics()->SetColor(1, 1, 1, 1);
			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);

			i = m_aBrains[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
		*/
		
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BRAIN].m_Id);
		//Graphics()->TextureSet(-1);
		Graphics()->QuadsBegin();
				
		Graphics()->QuadsSetRotation(0);
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aBrains[i].m_Life / m_aBrains[i].m_LifeSpan;
			//Graphics()->SetColor(1, 1, 1, 1-a);
			Graphics()->SetColor(m_aBrains[i].m_Color.r*0.6f + 0.4f, m_aBrains[i].m_Color.g*0.6f + 0.4f, m_aBrains[i].m_Color.b*0.7f + 0.3f, 1-a);
			
			vec2 aPos[9];
			
			/*
			for (int f = 0; f < 5; f++)
				aPos[f*2] = m_aBrains[i].m_aPos[f]+vec2(0, 6);
			
			for (int f = 0; f < 4; f++)
				aPos[1+f*2] = (aPos[f*2] + aPos[2+f*2]) / 2;
			
			for (int f = 1; f < 4; f++)
				aPos[f*2] = (aPos[f*2] + (aPos[f*2-1]+aPos[f*2+1])/2) / 2;
			*/
				
			/*
			aPos[1] = (aPos[0] + aPos[2]) / 2;
			aPos[3] = (aPos[2] + aPos[4]) / 2;
			
			aPos[2] = (aPos[2] + (aPos[1]+aPos[3])/2) / 2;
			*/
			
			for (int f = 0; f < 9; f++)
				aPos[f] = m_aBrains[i].m_aPos[f]+vec2(0, 6);
			
			
			/*
			for (int x = 0; x < 3; x++)
				for (int y = 0; y < 3; y++)
					aPos[x+y*3] = m_aBrains[i].m_aPos[4] + vec2(x-1, y-1)*16;
				*/
					
			for (int x = 1; x < 9; x++)
			{
				vec2 p1 = aPos[0];
				vec2 p2 = aPos[0];
				vec2 p3 = aPos[x];
				vec2 p4 = aPos[x%8+1];
					
						
				float a1 = 2*pi / 8.0f * (x-1);
				float a2 = 2*pi / 8.0f * x;
				
				//float a1 = 2*pi / 9.0f * x;
				//float a2 = 2*pi / 9.0f * ((x+1)%9);
				
				Graphics()->QuadsSetSubsetFree(	0.5f, 0.5f, 
												0.5f, 0.5f, 
												0.5f+sin(a1)*0.5f, 0.5f+cos(a1)*0.5f, 
												0.5f+sin(a2)*0.5f, 0.5f+cos(a2)*0.5f);
												
				IGraphics::CFreeformItem FreeFormItem(
					p1.x, p1.y,
					p2.x, p2.y,
					p3.x, p3.y,
					p4.x, p4.y);
								
				Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
			}
			
			/*
			for (int x = 0; x < 2; x++)
			{
				for (int y = 0; y < 2; y++)
				{
					vec2 p1 = aPos[x+y*3];
					vec2 p2 = aPos[x+1+y*3];
					vec2 p3 = aPos[x+(y+1)*3];
					vec2 p4 = aPos[x+1+(y+1)*3];
					
					Graphics()->QuadsSetSubsetFree(	x/2.0f, y/2.0f, 
													(x+1)/2.0f, y/2.0f, 
													x/2.0f, (y+1)/2.0f, 
													(x+1)/2.0f, (y+1)/2.0f);
													
					IGraphics::CFreeformItem FreeFormItem(
						p1.x, p1.y,
						p2.x, p2.y,
						p3.x, p3.y,
						p4.x, p4.y);
								
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
			}
			*/
			
				/*
			vec2 p0 = aPos[0]-aPos[1];
			float a0 = atan2(p0.y, p0.x);
				
			float Frames = 3.0f;
				
			for (int f = 0; f < 3; f++)
			{
				p0 = aPos[f]-aPos[f+1];
				
				float a = atan2(p0.y, p0.x);
				
				float a1 = a0-pi/2.0f;
				float a2 = a-pi/2.0f;
				float a3 = a0+pi/2.0f;
				float a4 = a+pi/2.0f;
				
				float s1 = 20.0f;
						
				vec2 p1 = aPos[f]+vec2(cos(a1), sin(a1))*s1;
				vec2 p2 = aPos[f+1]+vec2(cos(a2), sin(a2))*s1;
				vec2 p3 = aPos[f]+vec2(cos(a3), sin(a3))*s1;
				vec2 p4 = aPos[f+1]+vec2(cos(a4), sin(a4))*s1;
			
				
				Graphics()->QuadsSetSubsetFree(	f/Frames, 0, 
												(f+1)/Frames, 0, 
												f/Frames, 1, 
												(f+1)/Frames, 1);
			
				IGraphics::CFreeformItem FreeFormItem(
					p1.x, p1.y,
					p2.x, p2.y,
					p3.x, p3.y,
					p4.x, p4.y);
							
				Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				
				a0 = a;
			}
			*/
			
			i = m_aBrains[i].m_NextPart;
		}
		
		Graphics()->QuadsEnd();
	}
}















