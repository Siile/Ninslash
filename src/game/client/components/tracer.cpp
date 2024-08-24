#include <base/math.h>
#include <engine/graphics.h>
#include <engine/demo.h>
#include <engine/shared/config.h>

#include <game/generated/game_data.h>
#include <game/client/render.h>
#include <game/client/gameclient.h>
#include <game/client/customstuff.h>
#include "items.h"
#include "tracer.h"


inline vec2 RandomDir() { return normalize(vec2(frandom()-0.5f, frandom()-0.5f)); }


CTracer::CTracer()
{
	OnReset();
	m_RenderTracers.m_pParts = this;
	m_RenderSpriteTracers.m_pParts = this;
}


void CTracer::OnReset()
{
	// reset blood
	for(int i = 0; i < MAX_TRACERS; i++)
	{
		m_aTracer[i].m_PrevPart = i-1;
		m_aTracer[i].m_NextPart = i+1;
	}

	m_aTracer[0].m_PrevPart = 0;
	m_aTracer[MAX_TRACERS-1].m_NextPart = -1;
	m_FirstFree = 0;

	for(int i = 0; i < NUM_GROUPS; i++)
		m_aFirstPart[i] = -1;
}


void CTracer::UpdatePos(int ItemID, vec2 Pos)
{
	int i = m_aFirstPart[GROUP_SPRITETRACERS];
	while(i != -1)
	{
		if (m_aTracer[i].m_ItemID == ItemID)
		{
			m_aTracer[i].m_Pos[0] = Pos;
			return;
		}
		
		i = m_aTracer[i].m_NextPart;
	}
}
	
	
void CTracer::Add(int Type, int ItemID, vec2 Pos, vec2 StartPos, int StartTick, int Weapon, vec2 Vel)
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

	int Group = GROUP_TRACERS;
	
	if (Type < 0)
		Group = GROUP_SPRITETRACERS;
	
	int i = m_aFirstPart[Group];
	while(i != -1)
	{
		if (abs(m_aTracer[i].m_StartTick - StartTick) < 2 && m_aTracer[i].m_ItemID == ItemID && m_aTracer[i].m_Type == Type)
		{
			m_aTracer[i].Set(ItemID, Type, Pos, StartPos, StartTick, Weapon, Vel);
			return;
		}
		
		i = m_aTracer[i].m_NextPart;
	}
	
	
	// ...or add new one
	if (m_FirstFree == -1)
		return;

	CTrace Trace;
	Trace.Set(ItemID, Type, Pos, StartPos, StartTick, Weapon, Vel);
	
	// remove from the free list
	int Id = m_FirstFree;
	m_FirstFree = m_aTracer[Id].m_NextPart;
	if(m_FirstFree != -1)
		m_aTracer[m_FirstFree].m_PrevPart = -1;

	// copy data
	m_aTracer[Id] = Trace;

	// insert to the group list
	m_aTracer[Id].m_PrevPart = -1;
	m_aTracer[Id].m_NextPart = m_aFirstPart[Group];
	if(m_aFirstPart[Group] != -1)
		m_aTracer[m_aFirstPart[Group]].m_PrevPart = Id;
	m_aFirstPart[Group] = Id;

	// set some parameters
	m_aTracer[Id].m_Life = 0;
}


void CTracer::Update(float TimePassed)
{
	for(int g = 0; g < NUM_GROUPS; g++)
	{
		int i = m_aFirstPart[g];
		while(i != -1)
		{
			int Next = m_aTracer[i].m_NextPart;
			//m_aTracer[i].vel += flow_get(m_aTracer[i].pos)*time_passed * m_aTracer[i].flow_affected;

			m_aTracer[i].m_Life += TimePassed;

			// check timeout
			if(m_aTracer[i].m_Life > m_aTracer[i].m_LifeSpan)
			{
				// remove it from the group list
				if(m_aTracer[i].m_PrevPart != -1)
					m_aTracer[m_aTracer[i].m_PrevPart].m_NextPart = m_aTracer[i].m_NextPart;
				else
					m_aFirstPart[g] = m_aTracer[i].m_NextPart;

				if(m_aTracer[i].m_NextPart != -1)
					m_aTracer[m_aTracer[i].m_NextPart].m_PrevPart = m_aTracer[i].m_PrevPart;

				// insert to the free list
				if(m_FirstFree != -1)
					m_aTracer[m_FirstFree].m_PrevPart = i;
				m_aTracer[i].m_PrevPart = -1;
				m_aTracer[i].m_NextPart = m_FirstFree;
				m_FirstFree = i;
			}

			i = Next;
		}
	}
}

void CTracer::Tick()
{
	for(int g = 0; g < NUM_GROUPS; g++)
	{
		int i = m_aFirstPart[g];
		while(i != -1)
		{
			int Next = m_aTracer[i].m_NextPart;

			//m_aTracer[i].m_DelayPos += (m_aTracer[i].m_Pos - m_aTracer[i].m_DelayPos) / m_aTracer[i].m_Speed;
			
			for (int p = 1; p < m_aTracer[i].m_Parts; p++)
			{
				m_aTracer[i].m_Pos[p] += (m_aTracer[i].m_Pos[p-1] - m_aTracer[i].m_Pos[p]) / m_aTracer[i].m_Speed;
				m_aTracer[i].m_Vel[p] += (m_aTracer[i].m_Vel[p-1] - m_aTracer[i].m_Vel[p]) / m_aTracer[i].m_Speed;
				m_aTracer[i].m_Pos[p] += m_aTracer[i].m_Vel[p];
			}

			i = Next;
		}
	}
}

void CTracer::OnRender()
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


void CTracer::RenderGroup(int Group)
{
	if (Group == GROUP_TRACERS)
	{
		m_pClient->m_pItems->UpdateTraces();
		
		// bullet tracers
		vec2 Out, Border;
		
		Graphics()->BlendNormal();
		//Graphics()->BlendAdditive();
		Graphics()->TextureSet(-1);	
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			vec2 p1 = m_aTracer[i].m_Pos[0];
			vec2 TrailDir = normalize(p1 - m_aTracer[i].m_Pos[1]);
			float o1 = m_aTracer[i].m_Size1;
			vec2 Out1 = vec2(TrailDir.y, -TrailDir.x) * o1*0.7f;
				
			for (int t = 1; t < m_aTracer[i].m_Parts; t++)
			{
						
				vec2 p2 = m_aTracer[i].m_Pos[t];
				
				TrailDir = normalize(p1 - p2);
				
				float t1 = (t-1)/float(m_aTracer[i].m_Parts-1);
				float t2 = t/float(m_aTracer[i].m_Parts-1);

				o1 = mix(m_aTracer[i].m_Size1, m_aTracer[i].m_Size2, t1);
				float o2 = mix(m_aTracer[i].m_Size1, m_aTracer[i].m_Size2, t2);
				
				//vec2 Out1 = vec2(TrailDir.y, -TrailDir.x) * o1*0.7f;
				vec2 Out2 = vec2(TrailDir.y, -TrailDir.x) * o2*0.7f;
				
				float a1 = 0.9f-t1;
				float a2 = 0.9f-t2;
				
				float d = distance(p1, p2);
				
				if (d > 1.0f && d < 200.0f)
				{
					IGraphics::CColorVertex aColors[4] = {
						IGraphics::CColorVertex(0, m_aTracer[i].m_Color.r, m_aTracer[i].m_Color.g, m_aTracer[i].m_Color.b, m_aTracer[i].m_Color.a*a1),
						IGraphics::CColorVertex(1, m_aTracer[i].m_Color.r, m_aTracer[i].m_Color.g, m_aTracer[i].m_Color.b, m_aTracer[i].m_Color.a*a2),
						IGraphics::CColorVertex(2, m_aTracer[i].m_Color.r, m_aTracer[i].m_Color.g, m_aTracer[i].m_Color.b, m_aTracer[i].m_Color.a*a1),
						IGraphics::CColorVertex(3, m_aTracer[i].m_Color.r, m_aTracer[i].m_Color.g, m_aTracer[i].m_Color.b, m_aTracer[i].m_Color.a*a2)};
					Graphics()->SetColorVertex(aColors, 4);
				
					IGraphics::CFreeformItem Freeform1(
						p1.x-Out1.x, p1.y-Out1.y,
						p2.x-Out2.x, p2.y-Out2.y,
						p1.x+Out1.x, p1.y+Out1.y,
						p2.x+Out2.x, p2.y+Out2.y);
					
					Graphics()->QuadsDrawFreeform(&Freeform1, 1);
				}
				
				p1 = p2;
				Out1 = Out2;
			}
	
			
			i = m_aTracer[i].m_NextPart;
		}
	
		Graphics()->QuadsEnd();
		Graphics()->ShaderEnd();
	}
	else if (Group == GROUP_SPRITETRACERS)
	{
		// bullet tracers
		vec2 Out, Border;
		
		Graphics()->BlendNormal();
		//Graphics()->BlendAdditive();
		//Graphics()->TextureSet(-1);	
		//Graphics()->TextureSet(g_pData->m_aImages[IMAGE_SPARKS].m_Id);
		//Graphics()->ShaderBegin(SHADER_COLORSWAP, 1.0f, 0.0f, CustomStuff()->ChargeIntensity(70));
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_TRAILS].m_Id);
		Graphics()->QuadsBegin();
		
		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aTracer[i].m_Life / m_aTracer[i].m_LifeSpan;
			
			/*
			IGraphics::CColorVertex aColors[4] = {
				IGraphics::CColorVertex(0, m_aTracer[i].m_Color.r, m_aTracer[i].m_Color.g, m_aTracer[i].m_Color.b, m_aTracer[i].m_Color.a),
				IGraphics::CColorVertex(1, m_aTracer[i].m_Color.r, m_aTracer[i].m_Color.g, m_aTracer[i].m_Color.b, m_aTracer[i].m_Color.a),
				IGraphics::CColorVertex(2, m_aTracer[i].m_Color.r, m_aTracer[i].m_Color.g, m_aTracer[i].m_Color.b, 0.0f),
				IGraphics::CColorVertex(3, m_aTracer[i].m_Color.r, m_aTracer[i].m_Color.g, m_aTracer[i].m_Color.b, 0.0f)};
			Graphics()->SetColorVertex(aColors, 4);
			
			vec2 TrailDir = normalize(m_aTracer[i].m_Pos - m_aTracer[i].m_DelayPos);
			vec2 Out = vec2(TrailDir.y, -TrailDir.x) * 3.0f;
			
			IGraphics::CFreeformItem Freeform1(
				m_aTracer[i].m_Pos.x-Out.x, m_aTracer[i].m_Pos.y-Out.y,
				m_aTracer[i].m_Pos.x+Out.x, m_aTracer[i].m_Pos.y+Out.y,
				m_aTracer[i].m_DelayPos.x-Out.x, m_aTracer[i].m_DelayPos.y-Out.y,
				m_aTracer[i].m_DelayPos.x+Out.x, m_aTracer[i].m_DelayPos.y+Out.y);
			
			Graphics()->QuadsDrawFreeform(&Freeform1, 1);
			*/
	
	
			vec2 p1 = m_aTracer[i].m_Pos[0];
			vec2 TrailDir = normalize(p1 - m_aTracer[i].m_Pos[1]);
			float o1 = m_aTracer[i].m_Size1;
			vec2 Out1 = vec2(TrailDir.y, -TrailDir.x) * o1*0.7f;
				
			for (int t = 1; t < m_aTracer[i].m_Parts; t++)
			{
						
				vec2 p2 = m_aTracer[i].m_Pos[t];
				
				TrailDir = normalize(p1 - p2);
				
				float t1 = (t-1)/float(m_aTracer[i].m_Parts-1);
				float t2 = t/float(m_aTracer[i].m_Parts-1);
				
				//RenderTools()->SelectSprite(SPRITE_FX_CHAINSAW1+(CustomStuff()->GetSpriteFrame(6, 4)+m_aTracer[i].m_ItemID)%4,  + (false ? SPRITE_FLAG_FLIP_Y : 0), 0, 0, 1-t1, 1-t2);
				
				float tt = CustomStuff()->m_SawbladeAngle*m_aTracer[i].m_RotSpeed;
				
				RenderTools()->SelectSprite(SPRITE_TRAIL1+m_aTracer[i].m_Sprite, 0, 0, 0, (1-t1)*m_aTracer[i].m_Scale + tt, (1-t2)*m_aTracer[i].m_Scale + tt);
				
				o1 = mix(m_aTracer[i].m_Size1, m_aTracer[i].m_Size2, t1);
				float o2 = mix(m_aTracer[i].m_Size1, m_aTracer[i].m_Size2, t2);
				
				//vec2 Out1 = vec2(TrailDir.y, -TrailDir.x) * o1*0.7f;
				vec2 Out2 = vec2(TrailDir.y, -TrailDir.x) * o2*0.7f;
				
				float a1 = 0.9f-t1;
				float a2 = 0.9f-t2;
				
				a1 *= 1.0f-a;
				a2 *= 1.0f-a;
				
				//a1 = 1.0f;
				//a2 = 1.0f;
				
				float d = distance(p1, p2);
				
				if (d > 1.0f && d < 200.0f)
				{
					IGraphics::CColorVertex aColors[4] = {
						IGraphics::CColorVertex(0, m_aTracer[i].m_Color.r, m_aTracer[i].m_Color.g, m_aTracer[i].m_Color.b, m_aTracer[i].m_Color.a*a1),
						IGraphics::CColorVertex(1, m_aTracer[i].m_Color.r, m_aTracer[i].m_Color.g, m_aTracer[i].m_Color.b, m_aTracer[i].m_Color.a*a1),
						IGraphics::CColorVertex(2, m_aTracer[i].m_Color.r, m_aTracer[i].m_Color.g, m_aTracer[i].m_Color.b, m_aTracer[i].m_Color.a*a2),
						IGraphics::CColorVertex(3, m_aTracer[i].m_Color.r, m_aTracer[i].m_Color.g, m_aTracer[i].m_Color.b, m_aTracer[i].m_Color.a*a2)};
					Graphics()->SetColorVertex(aColors, 4);
				
					IGraphics::CFreeformItem Freeform1(
						p1.x-Out1.x, p1.y-Out1.y,
						p2.x-Out2.x, p2.y-Out2.y,
						p1.x+Out1.x, p1.y+Out1.y,
						p2.x+Out2.x, p2.y+Out2.y);
					
					Graphics()->QuadsDrawFreeform(&Freeform1, 1);
				}
				
				p1 = p2;
				Out1 = Out2;
			}

			i = m_aTracer[i].m_NextPart;
		}
	
		Graphics()->QuadsEnd();
		Graphics()->ShaderEnd();
	}
}















