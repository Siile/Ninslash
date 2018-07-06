#include <engine/demo.h>
#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/gamecore.h> // get_angle
#include <game/client/gameclient.h>
#include <game/client/ui.h>
#include <game/client/render.h>

#include <game/client/components/effects.h>
#include <game/client/components/tracer.h>
#include <game/client/components/sounds.h>
#include <game/client/components/controls.h>

#include "ball.h"


inline float NormalizeAngular(float f)
{
	return fmod(f+pi*2, pi*2);
}

inline float AngularMixDirection (float Src, float Dst) { return sinf(Dst-Src) >0?1:-1; }
inline float AngularDistance(float Src, float Dst) { return asinf(sinf(Dst-Src)); }

inline float AngularApproach(float Src, float Dst, float Amount)
{
	float d = AngularMixDirection (Src, Dst);
	float n = Src + Amount*d;
	if(AngularMixDirection (n, Dst) != d)
		return Dst;
	return n;
}



void CBalls::RenderBall(const CNetObj_Ball *pPrevBall, const CNetObj_Ball *pBall)
{
	CNetObj_Ball Prev;
	CNetObj_Ball Ball;
	Prev = *pPrevBall;
	Ball = *pBall;
	
	bool NewTick = m_pClient->m_NewTick;
	float IntraTick = Client()->IntraGameTick();
	
	if(g_Config.m_ClPredict && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		m_pClient->m_PredictedBall.Write(&Ball);
		m_pClient->m_PredictedPrevBall.Write(&Prev);
		IntraTick = Client()->PredIntraGameTick();
		NewTick = m_pClient->m_NewPredictedTick;
	}
	
	float Angle = mix(Prev.m_Angle, Ball.m_Angle, IntraTick) / 256.0f;
	
	if (Ball.m_Angle > (256.0f * pi) && Prev.m_Angle < 0)
	{
		float ca = Ball.m_Angle - 256.0f * 2 * pi;
		Angle = mix((float)Prev.m_Angle, ca, IntraTick) / 256.0f;
	}
	else if (Ball.m_Angle < 0 && Prev.m_Angle > (256.0f * pi))
	{
		float ca = Ball.m_Angle + 256.0f * 2 * pi;
		Angle = mix((float)Prev.m_Angle, ca, IntraTick) / 256.0f;
	}

	
	vec2 Position = mix(vec2(Prev.m_X, Prev.m_Y), vec2(Ball.m_X, Ball.m_Y), IntraTick);
	
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BALL].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->QuadsSetRotation(Angle);
	IGraphics::CQuadItem QuadItem(Position.x, Position.y, 84, 84);
	//IGraphics::CQuadItem QuadItem(Ball.m_X, Ball.m_Y, 48, 48);
	Graphics()->QuadsDraw(&QuadItem, 1);
	Graphics()->QuadsEnd();
	
	m_pClient->m_pTracers->Add(8, -609, Position, Position, 0, 0);
	
	/*
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_EMOTICONS].m_Id);
		Graphics()->QuadsBegin();
		RenderTools()->SelectSprite(SPRITE_DOTDOT);
		IGraphics::CQuadItem QuadItem(Position.x + 24, Position.y - 84, 64,64);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();
	*/
}


void CBalls::OnRender()
{
	if (!m_pClient->m_Snap.m_Ball.m_Active)
		return;
	
	//const void *pPrevInfo = Client()->SnapFindItem(IClient::SNAP_PREV, NETOBJTYPE_BALL, 0);
	//const void *pInfo = Client()->SnapFindItem(IClient::SNAP_CURRENT, NETOBJTYPE_BALL, 0);
			
	//if (pPrevInfo && pInfo)
	{
		CNetObj_Ball PrevBall = m_pClient->m_Snap.m_Ball.m_Prev;
		CNetObj_Ball CurBall = m_pClient->m_Snap.m_Ball.m_Cur;

		RenderBall(
			&PrevBall,
			&CurBall);
	}
}
