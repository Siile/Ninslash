

#include <engine/graphics.h>
#include <engine/demo.h>
#include <engine/shared/config.h>

#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/gamecore.h> // get_angle
#include <game/client/gameclient.h>
#include <game/client/ui.h>
#include <game/client/render.h>
#include <game/client/customstuff.h>

#include <game/client/components/flow.h>
#include <game/client/components/effects.h>
#include <game/client/components/tracer.h>

#include "items.h"

void CItems::OnReset()
{
	m_NumExtraProjectiles = 0;
	
	
	// reset flag animations
	for (int i = 0; i < 2; i++)
	{
		m_LastUpdate[i] = 0;
		m_LastTiltUpdate[i] = 0;
		m_FlagFrame[i] = 0;
		
		m_FlagPos[i] = vec2(0, 0);
		m_FlagOldPos[i] = vec2(0, 0);
		
		m_FlagOffset[i] = vec2(0, 0);
		m_FlagTargetOffset[i] = vec2(0, 0);
		
		m_FlagTilt[i] = 0;
	}
}




void CItems::UpdateProjectileTrace(const CNetObj_Projectile *pCurrent, int ItemID)
{
	static float s_LastGameTickTime = Client()->GameTickTime();
	if(m_pClient->m_Snap.m_pGameInfoObj && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
		s_LastGameTickTime = Client()->GameTickTime();
	float Ct = (Client()->PrevGameTick()-pCurrent->m_StartTick)/(float)SERVER_TICK_SPEED + s_LastGameTickTime;
	if(Ct < 0)
	{
		return; // projectile haven't been shot yet
	}
		
	// get positions
	float Speed = GetProjectileSpeed(pCurrent->m_Type);
	float Curvature = GetProjectileCurvature(pCurrent->m_Type);
	
	vec2 StartPos(pCurrent->m_X, pCurrent->m_Y);
	vec2 StartVel(pCurrent->m_VelX/100.0f, pCurrent->m_VelY/100.0f);
	vec2 Vel2(pCurrent->m_Vel2X/10.0f, pCurrent->m_Vel2Y/10.0f);
	
	vec2 Pos = vec2(0, 0);
	
	if (WeaponProjectilePosType(pCurrent->m_Type) == 1)
		Pos = CalcLogPos(StartPos, StartVel, Vel2, Curvature, Speed, Ct);
	else if (WeaponProjectilePosType(pCurrent->m_Type) == 2)
		Pos = CalcRocketPos(StartPos, StartVel, Vel2, Curvature, Speed, Ct);
	else
		Pos = CalcPos(StartPos, StartVel, Vel2, Curvature, Speed, Ct);
	
	m_pClient->m_pTracers->Add(GetProjectileTraceType(pCurrent->m_Type), ItemID+100, Pos, Pos, pCurrent->m_StartTick, pCurrent->m_Type);
}
	

// render projectiles
void CItems::RenderProjectile(const CNetObj_Projectile *pCurrent, int ItemID)
{

	static float s_LastGameTickTime = Client()->GameTickTime();
	if(m_pClient->m_Snap.m_pGameInfoObj && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
		s_LastGameTickTime = Client()->GameTickTime();
	float Ct = (Client()->PrevGameTick()-pCurrent->m_StartTick)/(float)SERVER_TICK_SPEED + s_LastGameTickTime;
	if(Ct < 0)
	{
		return; // projectile havn't been shot yet
	}
		
	// get positions
	float Speed = GetProjectileSpeed(pCurrent->m_Type);
	float Curvature = GetProjectileCurvature(pCurrent->m_Type);
	
	vec2 StartPos(pCurrent->m_X, pCurrent->m_Y);
	vec2 StartVel(pCurrent->m_VelX/100.0f, pCurrent->m_VelY/100.0f);
	vec2 Vel2(pCurrent->m_Vel2X/10.0f, pCurrent->m_Vel2Y/10.0f);
	

	vec2 Pos;
	vec2 PrevPos;
	

	if (WeaponProjectilePosType(pCurrent->m_Type) == 1)
	{
		Pos = CalcLogPos(StartPos, StartVel, Vel2, Curvature, Speed, Ct);
		PrevPos = CalcLogPos(StartPos, StartVel, Vel2, Curvature, Speed, Ct-0.001f);
	}
	else if (WeaponProjectilePosType(pCurrent->m_Type) == 2)
	{
		Pos = CalcRocketPos(StartPos, StartVel, Vel2, Curvature, Speed, Ct);
		PrevPos = CalcRocketPos(StartPos, StartVel, Vel2, Curvature, Speed, Ct-0.001f);
	}
	else
	{
		Pos = CalcPos(StartPos, StartVel, Vel2, Curvature, Speed, Ct);
		PrevPos = CalcPos(StartPos, StartVel, Vel2, Curvature, Speed, Ct-0.001f);
	}
	
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_PROJECTILES].m_Id);
	Graphics()->QuadsBegin();

	RenderTools()->SelectSprite(SPRITE_PROJECTILE1_1 + GetProjectileSprite(pCurrent->m_Type));
	
	//	RenderTools()->SelectSprite(SPRITE_GREEN_CHARGE1 + CustomStuff()->GetSpriteFrame(6, 7));
	// else if (pCurrent->m_Type == WEAPON_RIFLE && pCurrent->m_PowerLevel > 1)
	
	
	vec2 Vel = Pos-PrevPos;
	
	
	// add force to fluids
	m_pClient->AddFluidForce(Pos, Vel*5);
	
	{
		bool Rot = true;
		
		if (Rot)
		{
			if(length(Vel) > 0.00001f)
				Graphics()->QuadsSetRotation(GetAngle(Vel));
			else
				Graphics()->QuadsSetRotation(0);
		}

	}

	vec2 Size = vec2(45, 30) * GetProjectileSize(pCurrent->m_Type);
	
	if (GetProjectileSprite(pCurrent->m_Type) == 3 || GetProjectileSprite(pCurrent->m_Type) == 7)
		Size *= 2.0f;
	
	if (GetStaticType(pCurrent->m_Type) == SW_BUBBLER)
	{
		m_pClient->m_pEffects->Flame(Pos, vec2(frandom()-frandom(), frandom()-frandom())*10.0f);
		m_pClient->m_pEffects->Flame(Pos, vec2(frandom()-frandom(), frandom()-frandom())*10.0f);
	}
	
	if (GetStaticType(pCurrent->m_Type) == SW_BAZOOKA)
	{
		m_pClient->m_pEffects->Flame(Pos, -Vel);
		m_pClient->m_pEffects->SmokeTrail(Pos, -Vel);
	}
	
	
	IGraphics::CQuadItem QuadItem(Pos.x, Pos.y, Size.x, Size.y);
	// draw projectile
	Graphics()->QuadsDraw(&QuadItem, 1);
	Graphics()->QuadsSetRotation(0);
	Graphics()->QuadsEnd();
	
	Graphics()->ShaderEnd();
}



void CItems::RenderPickup(const CNetObj_Pickup *pPrev, const CNetObj_Pickup *pCurrent)
{
	if (pCurrent->m_Type == POWERUP_WEAPON)
		RenderTools()->SetShadersForWeapon(pCurrent->m_Subtype);
	
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
	Graphics()->QuadsBegin();
	vec2 Pos = mix(vec2(pPrev->m_X, pPrev->m_Y), vec2(pCurrent->m_X, pCurrent->m_Y), Client()->IntraGameTick());
	float Angle = mix(pPrev->m_Angle, pCurrent->m_Angle, Client()->IntraGameTick()) / 256.0f;
	
	if (pCurrent->m_Angle > (256.0f * pi) && pPrev->m_Angle < 0)
	{
		float ca = pCurrent->m_Angle - 256.0f * 2 * pi;
		Angle = mix((float)pPrev->m_Angle, ca, Client()->IntraGameTick()) / 256.0f;
	}
	else if (pCurrent->m_Angle < 0 && pPrev->m_Angle > (256.0f * pi))
	{
		float ca = pCurrent->m_Angle + 256.0f * 2 * pi;
		Angle = mix((float)pPrev->m_Angle, ca, Client()->IntraGameTick()) / 256.0f;
	}
	
	int Flags = pCurrent->m_Mirror ? SPRITE_FLAG_FLIP_Y : 0;
	
	float Size = 64.0f;
	bool SkipOffset = false;
	
	if (Angle != 0.0f)
		SkipOffset = true;
	
	vec2 Vel = vec2(pCurrent->m_X, pCurrent->m_Y) - vec2(pPrev->m_X, pPrev->m_Y);
	m_pClient->AddFluidForce(Pos, Vel);
	
	if (pCurrent->m_Type == POWERUP_WEAPON)
	{
		//Angle = 0; //-pi/6;//-0.25f * pi * 2.0f;
	}
	else
	{
		//if (pCurrent->m_Type == POWERUP_AMMO || pCurrent->m_Type == POWERUP_KIT)
		//	SkipOffset = true;
		
		const int c[] = {
			SPRITE_PICKUP_HEALTH,
			SPRITE_PICKUP_AMMO,
			SPRITE_PICKUP_WEAPON,
			SPRITE_PICKUP_ARMOR,
			SPRITE_PICKUP_KIT
			};
		RenderTools()->SelectSprite(c[pCurrent->m_Type]);
	}

	Graphics()->QuadsSetRotation(Angle);
		
	static float s_Time = 0.0f;
	static float s_LastLocalTime = Client()->LocalTime();
	float Offset = Pos.y/32.0f + Pos.x/32.0f;
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
		if(!pInfo->m_Paused)
			s_Time += (Client()->LocalTime()-s_LastLocalTime)*pInfo->m_Speed;
	}
	else
	{
		if(m_pClient->m_Snap.m_pGameInfoObj && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
			s_Time += Client()->LocalTime()-s_LastLocalTime;
 	}
	
	if (!SkipOffset)
	{
		Pos.x += cosf(s_Time*2.0f+Offset)*2.5f;
		Pos.y += sinf(s_Time*2.0f+Offset)*2.5f;
	}
	
	s_LastLocalTime = Client()->LocalTime();
	
	if (pCurrent->m_Type == POWERUP_WEAPON)
		RenderTools()->RenderWeapon(pCurrent->m_Subtype, Pos, vec2(cos(Angle), sin(Angle)), WEAPON_GAME_SIZE, false, Flags, 1.0f, false, true);
	else
		RenderTools()->DrawSprite(Pos.x, Pos.y, Size);

	Graphics()->QuadsEnd();
	Graphics()->ShaderEnd();
}





void CItems::RenderFlag(const CNetObj_Flag *pPrev, const CNetObj_Flag *pCurrent, const CNetObj_GameData *pPrevGameData, const CNetObj_GameData *pCurGameData)
{
	float Size = 42.0f;
	
	
	Graphics()->BlendNormal();
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_FLAG].m_Id);
	Graphics()->QuadsBegin();
	
	int Team = 0;

	if(pCurrent->m_Team == TEAM_RED)
	{
		RenderTools()->SelectSprite(SPRITE_FLAG_RED1+m_FlagFrame[0], m_FlagMirror[0] ? SPRITE_FLAG_FLIP_X : 0);
	}
	else
	{
		RenderTools()->SelectSprite(SPRITE_FLAG_BLUE1+m_FlagFrame[1], m_FlagMirror[1] ? SPRITE_FLAG_FLIP_X : 0);
		Team = 1;
	}
		
	Graphics()->QuadsSetRotation(m_FlagTilt[Team] / 20.0f);
	
	vec2 Pos = mix(vec2(pPrev->m_X, pPrev->m_Y), vec2(pCurrent->m_X, pCurrent->m_Y), Client()->IntraGameTick());
	
	if(pCurGameData)
	{
		// make sure that the flag isn't interpolated between capture and return
		if(pPrevGameData &&
			((pCurrent->m_Team == TEAM_RED && pPrevGameData->m_FlagCarrierRed != pCurGameData->m_FlagCarrierRed) ||
			(pCurrent->m_Team == TEAM_BLUE && pPrevGameData->m_FlagCarrierBlue != pCurGameData->m_FlagCarrierBlue)))
			Pos = vec2(pCurrent->m_X, pCurrent->m_Y);
			
		// make sure to use predicted position if we are the carrier
		if(m_pClient->m_Snap.m_pLocalInfo &&
			((pCurrent->m_Team == TEAM_RED && pCurGameData->m_FlagCarrierRed == m_pClient->m_Snap.m_LocalClientID) ||
			(pCurrent->m_Team == TEAM_BLUE && pCurGameData->m_FlagCarrierBlue == m_pClient->m_Snap.m_LocalClientID)))
			Pos = m_pClient->m_LocalCharacterPos;
			
			
		if ((pCurrent->m_Team == TEAM_RED && pCurGameData->m_FlagCarrierRed >= 0) ||
			(pCurrent->m_Team == TEAM_BLUE && pCurGameData->m_FlagCarrierBlue >= 0))
			Pos += vec2(0, -42);
	}
	
	
	//m_pClient->m_pEffects->Light(Pos+vec2(0, -32), 256);


	
	int64 currentTime = time_get();
	if ((currentTime-m_LastUpdate[Team] > time_freq()) || (m_LastUpdate[Team] == 0))
		m_LastUpdate[Team] = currentTime;
	
	if ((currentTime-m_LastTiltUpdate[Team] > time_freq()) || (m_LastTiltUpdate[Team] == 0))
		m_LastTiltUpdate[Team] = currentTime;

	
	bool Once = true;
	
	int step = time_freq()/60;
	
	for (;m_LastTiltUpdate[Team] < currentTime; m_LastTiltUpdate[Team] += step)
	{
		m_FlagOffset[Team].x += (m_FlagTargetOffset[Team].x-m_FlagOffset[Team].x) / 3.0f;
		
		m_FlagTilt[Team] += (0-m_FlagTilt[Team]) / 3.0f;
		m_FlagTilt[Team] -= (m_FlagPos[Team].x - m_FlagOldPos[Team].x) / 20.0f;
		
		if (Once)
		{
			//m_FlagOldPos[Team] = m_FlagPos[Team];
			m_FlagOldPos[Team] = vec2(pPrev->m_X, pPrev->m_Y);
			m_FlagPos[Team] = vec2(pCurrent->m_X, pCurrent->m_Y);
			Once = false;
		}
	}

	
	float Speed = abs(m_FlagPos[Team].x - m_FlagOldPos[Team].x) / 4.0f;
	step = time_freq()/(6+Speed);
	
	for (;m_LastUpdate[Team] < currentTime; m_LastUpdate[Team] += step)
	{
		if (m_FlagPos[Team].x > m_FlagOldPos[Team].x)
		{
			m_FlagMirror[Team] = true;
			m_FlagTargetOffset[Team] = vec2(-16, 0);
		}
		if (m_FlagPos[Team].x < m_FlagOldPos[Team].x)
		{
			m_FlagMirror[Team] = false;
			m_FlagTargetOffset[Team] = vec2(16, 0);
		}
		

		if (Speed < 3)
		{
			if (m_FlagFrame[Team] > 1)
				m_FlagFrame[Team] = 1;
			else
				m_FlagFrame[Team] = 0;
		}
		else
		{
			if (++m_FlagFrame[Team] > 5)
				m_FlagFrame[Team] = 2;
		}
			
	}
	
	
	IGraphics::CQuadItem QuadItem(Pos.x+m_FlagOffset[Team].x + m_FlagTilt[Team] * 2, Pos.y-Size*0.75f+m_FlagOffset[Team].y + 10.0f, Size*1.2f, Size*2.4f);
	Graphics()->QuadsDraw(&QuadItem, 1);
	Graphics()->QuadsEnd();
}





void CItems::RenderLaser(const struct CNetObj_Laser *pCurrent)
{
	vec2 Pos = vec2(pCurrent->m_X, pCurrent->m_Y);
	vec2 From = vec2(pCurrent->m_FromX, pCurrent->m_FromY);
	vec2 Dir = normalize(Pos-From);

	float Ticks = Client()->GameTick() + Client()->IntraGameTick() - pCurrent->m_StartTick;
	float Ms = (Ticks/50.0f) * 1000.0f;
	float a = Ms / m_pClient->m_Tuning.m_LaserBounceDelay;
	a = clamp(a, 0.0f, 1.0f);
	float Ia = 1-a;
	
	Graphics()->ShaderBegin(SHADER_ELECTRIC, 1.0f);
		
	vec2 Out, Border;

	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();

	vec4 OuterColor(0.075f, 0.15f, 0.25f, 1.0f);
	vec4 InnerColor(0.5f, 0.75f, 1.0f, 1.0f);

	if (pCurrent->m_Charge >= 0)
	{
		Graphics()->SetColor(0.5f, 0.75f, 1, 0.3f+pCurrent->m_Charge*0.007f);
		
		int Steps = 1 + length(Pos - From) / 75;
		vec2 Step = (Pos - From) / Steps;
		Out = vec2(Dir.y, -Dir.x) * (1+pCurrent->m_Charge*0.1f*Ia); // 7
		
		vec2 p1 = From;
		vec2 s1 = Out * 0.1f;
		
		vec2 o1 = vec2(0, 0);
			
		for (int i = 0; i < Steps; i++)
		{
			vec2 p2 = p1 + Step;
			vec2 o2 = vec2(0, 0);
			
			if (i < Steps-1)
				o2 = vec2(frandom()-frandom(), frandom()-frandom()) * (pCurrent->m_Charge*0.15f); // 15
			
			vec2 s2 = Out * frandom()*4.0f;
			
			if (i == Steps -1)
				s2 *= 0.1f;
			
			IGraphics::CFreeformItem FreeFormItem(
				p1.x-s1.x+o1.x, p1.y-s1.y+o1.y,
				p1.x+s1.x+o1.x, p1.y+s1.y+o1.y,
				p2.x-s2.x+o2.x, p2.y-s2.y+o2.y,
				p2.x+s2.x+o2.x, p2.y+s2.y+o2.y);
								
			Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
		
			m_pClient->m_pEffects->BulletTrail(p1+o1, p2+o2, vec4(0.5f, 1.0f, 1.0f, 0.2f));
		
			s1 = s2;
			p1 = p2;
			o1 = o2;
		}
	}
	
	// lightning shotgun
	if (pCurrent->m_Charge == -1)
	{
		Graphics()->SetColor(0.5f+a*0.5f, 0.75f+a*0.25f, 1, 0.75f);
		
		int Steps = 1 + length(Pos - From) / 75;
		vec2 Step = (Pos - From) / Steps;
		Out = vec2(Dir.y, -Dir.x) * (8*Ia); // 7
		
		vec2 p1 = From;
		vec2 s1 = Out * 0.1f;
		
		vec2 o1 = vec2(0, 0);
			
		for (int i = 0; i < Steps; i++)
		{
			vec2 p2 = p1 + Step;
			vec2 o2 = vec2(0, 0);
			
			if (i < Steps-1)
				o2 = vec2(frandom()-frandom(), frandom()-frandom()) * ((i+1)*10.0f);
			
			vec2 s2 = Out * frandom()*2.0f;
			
			if (i == Steps -1)
				s2 *= 0.1f;
			
			IGraphics::CFreeformItem FreeFormItem(
				p1.x-s1.x+o1.x, p1.y-s1.y+o1.y,
				p1.x+s1.x+o1.x, p1.y+s1.y+o1.y,
				p2.x-s2.x+o2.x, p2.y-s2.y+o2.y,
				p2.x+s2.x+o2.x, p2.y+s2.y+o2.y);
								
			Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
		
			m_pClient->m_pEffects->BulletTrail(p1+o1, p2+o2, vec4(0.5f, 1.0f, 1.0f, 0.2f));
		
			s1 = s2;
			p1 = p2;
			o1 = o2;
		}
	}
	
	// lightning grenade
	if (pCurrent->m_Charge == -2)
	{
		Graphics()->SetColor(0.5f+a*0.5f, 0.75f+a*0.25f, 1, 0.75f);
		
		int Steps = 1 + length(Pos - From) / 35;
		vec2 Step = (Pos - From) / Steps;
		Out = vec2(Dir.y, -Dir.x) * (6*Ia); // 7
		
		vec2 p1 = From;
		vec2 s1 = Out * 0.1f;
		
		vec2 o1 = vec2(0, 0);
			
		for (int i = 0; i < Steps; i++)
		{
			vec2 p2 = p1 + Step;
			vec2 o2 = vec2(0, 0);
			
			if (i < Steps-1)
				o2 = vec2(frandom()-frandom(), frandom()-frandom()) * ((i+1)*30.0f);
			
			vec2 s2 = Out * frandom()*1.0f * (1.0f + i*0.4f);
			
			if (i == Steps -1)
				s2 *= 0.1f;
			
			IGraphics::CFreeformItem FreeFormItem(
				p1.x-s1.x+o1.x, p1.y-s1.y+o1.y,
				p1.x+s1.x+o1.x, p1.y+s1.y+o1.y,
				p2.x-s2.x+o2.x, p2.y-s2.y+o2.y,
				p2.x+s2.x+o2.x, p2.y+s2.y+o2.y);
								
			Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
		
			//m_pClient->m_pEffects->BulletTrail(p1+o1, p2+o2, vec4(0.5f, 1.0f, 1.0f, 0.2f));
		
			s1 = s2;
			p1 = p2;
			o1 = o2;
		}
	}
	
	Graphics()->QuadsEnd();
	
	
	// render head
	if (pCurrent->m_Charge != -2)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_PARTICLES].m_Id);
		Graphics()->QuadsBegin();

		int Sprites[] = {SPRITE_PART_SPLAT01, SPRITE_PART_SPLAT02, SPRITE_PART_SPLAT03};
		RenderTools()->SelectSprite(Sprites[Client()->GameTick()%3]);
		Graphics()->QuadsSetRotation(Client()->GameTick());
		Graphics()->SetColor(OuterColor.r, OuterColor.g, OuterColor.b, 1.0f);
		IGraphics::CQuadItem QuadItem(Pos.x, Pos.y, 24, 24);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->SetColor(InnerColor.r, InnerColor.g, InnerColor.b, 1.0f);
		QuadItem = IGraphics::CQuadItem(Pos.x, Pos.y, 20, 20);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();
	}

	Graphics()->ShaderEnd();
	Graphics()->BlendNormal();
}



void CItems::RenderLaserFail(const struct CNetObj_LaserFail *pCurrent)
{
	vec2 Pos = vec2(pCurrent->m_X, pCurrent->m_Y);
	vec2 From = vec2(pCurrent->m_FromX, pCurrent->m_FromY);
	vec2 Dir = normalize(Pos-From);

	float Ticks = Client()->GameTick() + Client()->IntraGameTick() - pCurrent->m_StartTick;
	float Ms = (Ticks/50.0f) * 1000.0f;
	float a = Ms / m_pClient->m_Tuning.m_LaserBounceDelay;
	a = clamp(a, 0.0f, 1.0f);
	float Ia = 1-a;

	Graphics()->ShaderBegin(SHADER_ELECTRIC, 1.0f);
	
	vec2 Out;

	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();

	vec4 OuterColor(0.075f, 0.075f, 0.25f, 1.0f);
	vec4 InnerColor(0.5f, 0.5f, 1.0f, 1.0f);

	Graphics()->SetColor(0.5f, 0.5f, 1, 1.0f);
	int Steps = 1 + length(Pos - From) / 75;
	vec2 Step = (Pos - From) / Steps;
	Out = vec2(Dir.y, -Dir.x) * (7.0f*Ia);
		
	vec2 p1 = From;
	vec2 s1 = Out * 0.1f;
		
	vec2 o1 = vec2(0, 0);
			
	for (int i = 0; i < Steps; i++)
	{
		vec2 p2 = p1 + Step;
		vec2 o2 = vec2(0, 0);
			
		if (i < Steps-1)
			o2 = vec2(frandom()-frandom(), frandom()-frandom()) * (15.0f + a*70.0f);
			
		vec2 s2 = Out * frandom()*4.0f;
			
		if (i == Steps -1)
			s2 *= 0.1f;
			
		IGraphics::CFreeformItem FreeFormItem(
			p1.x-s1.x+o1.x, p1.y-s1.y+o1.y,
			p1.x+s1.x+o1.x, p1.y+s1.y+o1.y,
			p2.x-s2.x+o2.x, p2.y-s2.y+o2.y,
			p2.x+s2.x+o2.x, p2.y+s2.y+o2.y);
								
		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
		
		m_pClient->m_pEffects->BulletTrail(p1+o1, p2+o2, vec4(0.5f, 0.5f, 1.0f, 0.2f));
		
		s1 = s2;
		p1 = p2;
		o1 = o2;
	}
	
	Graphics()->QuadsEnd();
	Graphics()->ShaderEnd();
	Graphics()->BlendNormal();
}

void CItems::UpdateTraces()
{
	if(Client()->State() < IClient::STATE_ONLINE)
		return;
	
	int Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
	for(int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);

		if(Item.m_Type == NETOBJTYPE_PROJECTILE)
			UpdateProjectileTrace((const CNetObj_Projectile *)pData, Item.m_ID);
	}
}


void CItems::OnRender()
{
	if(Client()->State() < IClient::STATE_ONLINE)
		return;
	
	int Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
	for(int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);

		if(Item.m_Type == NETOBJTYPE_PROJECTILE)
		{
			RenderProjectile((const CNetObj_Projectile *)pData, Item.m_ID);
		}
		else if(Item.m_Type == NETOBJTYPE_PICKUP)
		{
			const void *pPrev = Client()->SnapFindItem(IClient::SNAP_PREV, Item.m_Type, Item.m_ID);
			if(pPrev)
				RenderPickup((const CNetObj_Pickup *)pPrev, (const CNetObj_Pickup *)pData);
		}
		else if(Item.m_Type == NETOBJTYPE_LASER)
		{
			RenderLaser((const CNetObj_Laser *)pData);
		}
		else if(Item.m_Type == NETOBJTYPE_LASERFAIL)
		{
			RenderLaserFail((const CNetObj_LaserFail *)pData);
		}
	}

	// render flag
	for(int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);

		if(Item.m_Type == NETOBJTYPE_FLAG)
		{
			const void *pPrev = Client()->SnapFindItem(IClient::SNAP_PREV, Item.m_Type, Item.m_ID);
			if (pPrev)
			{
				const void *pPrevGameData = Client()->SnapFindItem(IClient::SNAP_PREV, NETOBJTYPE_GAMEDATA, m_pClient->m_Snap.m_GameDataSnapID);
				RenderFlag(static_cast<const CNetObj_Flag *>(pPrev), static_cast<const CNetObj_Flag *>(pData),
							static_cast<const CNetObj_GameData *>(pPrevGameData), m_pClient->m_Snap.m_pGameDataObj);
			}
		}
	}

	// render extra projectiles
	for(int i = 0; i < m_NumExtraProjectiles; i++)
	{
		if(m_aExtraProjectiles[i].m_StartTick < Client()->GameTick())
		{
			m_aExtraProjectiles[i] = m_aExtraProjectiles[m_NumExtraProjectiles-1];
			m_NumExtraProjectiles--;
		}
		else
			RenderProjectile(&m_aExtraProjectiles[i], 0);
	}
}

void CItems::AddExtraProjectile(CNetObj_Projectile *pProj)
{
	if(m_NumExtraProjectiles != MAX_EXTRA_PROJECTILES)
	{
		m_aExtraProjectiles[m_NumExtraProjectiles] = *pProj;
		m_NumExtraProjectiles++;
	}
}
