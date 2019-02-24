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

#include "weapons.h"

void CWeapons::OnReset()
{
	
}




void CWeapons::RenderWeapon(const CNetObj_Weapon *pPrev, const CNetObj_Weapon *pCurrent)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
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
	
	vec2 Vel = vec2(pCurrent->m_X, pCurrent->m_Y) - vec2(pPrev->m_X, pPrev->m_Y);
	m_pClient->AddFluidForce(Pos, Vel);

	
	int Phase1Tick = (Client()->GameTick() - pCurrent->m_AttackTick);
	float ChargeLevel = min(Phase1Tick*0.013f, 1.0f);
	
	if (ChargeLevel == 1.0f)
		ChargeLevel = 0.7f+cos(Phase1Tick*0.4f)*0.3f;
	
	//if (GetStaticType(pCurrent->m_WeaponType) == SW_SHURIKEN)
	if (GetProjectileTraceType(pCurrent->m_WeaponType) != 0)
	{
		if (length(Vel) >= GetWeaponTraceThreshold(pCurrent->m_WeaponType))
			m_pClient->m_pTracers->Add(GetProjectileTraceType(pCurrent->m_WeaponType), pCurrent->m_AttackTick, Pos, Pos, pCurrent->m_AttackTick, pCurrent->m_WeaponType);
		else if (length(Vel) > GetWeaponTraceThreshold(pCurrent->m_WeaponType) / 5.0f)
			m_pClient->m_pTracers->UpdatePos(pCurrent->m_AttackTick, Pos);
		
		if (GetStaticType(pCurrent->m_WeaponType) == SW_SHURIKEN)
			ChargeLevel = 0.0f;
	}
	
	if (GetStaticType(pCurrent->m_WeaponType) == SW_CLUSTER && GetWeaponCharge(pCurrent->m_WeaponType) == 15)
		ChargeLevel = 0.0f;
	
	if (GetStaticType(pCurrent->m_WeaponType) == SW_BOMB)
	{
		ChargeLevel = min(Phase1Tick*0.0011f, 1.0f);
	
		if (ChargeLevel == 1.0f)
			ChargeLevel = 0.7f+cos(Phase1Tick*0.5f)*0.3f;
	}
	
	if (GetStaticType(pCurrent->m_WeaponType) == SW_BALL)
		ChargeLevel = 0;
		
	//Graphics()->ShaderBegin(SHADER_COLORSWAP, 1.0f, 0.0f, ChargeLevel);
	RenderTools()->SetShadersForWeapon(pCurrent->m_WeaponType, ChargeLevel);
	Graphics()->QuadsBegin();
	Graphics()->QuadsSetRotation(Angle);

	static float s_Time = 0.0f;
	static float s_LastLocalTime = Client()->LocalTime();
	//float Offset = Pos.y/32.0f + Pos.x/32.0f;
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
	
	
	s_LastLocalTime = Client()->LocalTime();
	
	RenderTools()->RenderWeapon(pCurrent->m_WeaponType, Pos, vec2(1, 0), WEAPON_GAME_SIZE);
	
	Graphics()->QuadsEnd();
	Graphics()->ShaderEnd();
	
	
	if (GetStaticType(pCurrent->m_WeaponType) == SW_AREASHIELD)
	{
		float c = cos(CustomStuff()->m_SawbladeAngle*0.25f+(pCurrent->m_X/17)%30*0.1f)*0.3f + 0.7f;
		
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GENERATOR_SHIELD].m_Id);
		Graphics()->QuadsBegin();
	Graphics()->QuadsSetRotation(Angle);
		
		/*
		//team color
		if (m_pClient->m_Snap.m_pGameInfoObj)
		{
			int Flags = m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags;
			int Team = pCurrent->m_Team;
		
			if ((Flags & GAMEFLAG_TEAMS) && !(Flags & GAMEFLAG_INFECTION))
			{
				if (Team == TEAM_RED)
					Graphics()->SetColor(1, 0.5f + c*0.5f, 0.5f, 0.5f);
				else if (Team == TEAM_BLUE)
					Graphics()->SetColor(0.5f, 0.5f + c*0.5f, 1, 0.5f);
			}
			else if (Team == TEAM_RED)
			{
				vec4 pc = CustomStuff()->m_LocalColor;
				Graphics()->SetColor(0.5f+pc.r*0.5f, 0.5f+pc.g*0.5f, 0.5f+pc.b*0.5f, 0.5f);
			}
			else
				Graphics()->SetColor(0.0f, 0.5f+c*0.5f, 1, 0.5f);
		}
		else
			*/
			Graphics()->SetColor(0.5f, 0.5f+c*0.5f, 1, 0.5f);
		
		
		//Graphics()->SetColor(0, 0.5f+c*0.5f, 1, 0.5f);
		RenderTools()->SelectSprite(SPRITE_GENERATOR_SHIELD);
		RenderTools()->DrawSprite(Pos.x, Pos.y, 512+40.0f*c);
		Graphics()->QuadsEnd();
	}
}







void CWeapons::OnRender()
{
	if(Client()->State() < IClient::STATE_ONLINE)
		return;
	
	int Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
	for(int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);

		if(Item.m_Type == NETOBJTYPE_WEAPON)
		{
			const void *pPrev = Client()->SnapFindItem(IClient::SNAP_PREV, Item.m_Type, Item.m_ID);
			if(pPrev)
				RenderWeapon((const CNetObj_Weapon *)pPrev, (const CNetObj_Weapon *)pData);
			else
				RenderWeapon((const CNetObj_Weapon *)pData, (const CNetObj_Weapon *)pData);
		}
	}
}
