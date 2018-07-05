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
	
	
	float Angle = mix(pPrevBall->m_Angle, pBall->m_Angle, Client()->IntraGameTick()) / 256.0f;
	
	if (pBall->m_Angle > (256.0f * pi) && pPrevBall->m_Angle < 0)
	{
		float ca = pBall->m_Angle - 256.0f * 2 * pi;
		Angle = mix((float)pPrevBall->m_Angle, ca, Client()->IntraGameTick()) / 256.0f;
	}
	else if (pBall->m_Angle < 0 && pPrevBall->m_Angle > (256.0f * pi))
	{
		float ca = pBall->m_Angle + 256.0f * 2 * pi;
		Angle = mix((float)pPrevBall->m_Angle, ca, Client()->IntraGameTick()) / 256.0f;
	}
	
	
	// use preditect players if needed
	/*
	if(pInfo.m_Local && g_Config.m_ClPredict && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		if(!m_pClient->m_Snap.m_pLocalCharacter || (m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER))
		{
		}
		else
		{
			// m_PredictedChar.Write causes crash on some conditions when joining the game! todo: fix somehow 
			// apply predicted results
			m_pClient->m_PredictedChar.Write(&Player);
			m_pClient->m_PredictedPrevChar.Write(&Prev);
			IntraTick = Client()->PredIntraGameTick();
			NewTick = m_pClient->m_NewPredictedTick;
		}
	}

	vec2 Direction = GetDirection((int)(Angle*256.0f));
	vec2 Position = mix(vec2(Prev.m_X, Prev.m_Y), vec2(Ball.m_X, Ball.m_Y), IntraTick);
	vec2 Vel = mix(vec2(Prev.m_VelX/256.0f, Prev.m_VelY/256.0f), vec2(Ball.m_VelX/256.0f, Ball.m_VelY/256.0f), IntraTick);
	*/
	
	vec2 Position = mix(vec2(Prev.m_X, Prev.m_Y), vec2(Ball.m_X, Ball.m_Y), IntraTick);
	
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BALL].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->QuadsSetRotation(Angle);
	IGraphics::CQuadItem QuadItem(Position.x, Position.y, 84, 84);
	//IGraphics::CQuadItem QuadItem(Ball.m_X, Ball.m_Y, 48, 48);
	Graphics()->QuadsDraw(&QuadItem, 1);
	Graphics()->QuadsEnd();
		
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
