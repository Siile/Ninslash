

#include <engine/graphics.h>
#include <engine/textrender.h>
#include <generated/protocol.h>
#include <generated/game_data.h>

#include <game/client/gameclient.h>
#include "killmessages.h"

void CKillMessages::OnReset()
{
	m_KillmsgCurrent = 0;
	for(int i = 0; i < MAX_KILLMSGS; i++)
		m_aKillmsgs[i].m_Tick = -100000;
}

void CKillMessages::OnMessage(int MsgType, void *pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_KILLMSG)
	{
		CNetMsg_Sv_KillMsg *pMsg = (CNetMsg_Sv_KillMsg *)pRawMsg;

		// unpack messages
		CKillMsg Kill;
		Kill.m_VictimID = pMsg->m_Victim;
		Kill.m_VictimTeam = m_pClient->m_aClients[Kill.m_VictimID].m_Team;
		str_copy(Kill.m_aVictimName, m_pClient->m_aClients[Kill.m_VictimID].m_aName, sizeof(Kill.m_aVictimName));
		Kill.m_VictimRenderInfo = m_pClient->m_aClients[Kill.m_VictimID].m_RenderInfo;
		Kill.m_KillerID = pMsg->m_Killer;
		Kill.m_KillerTeam = m_pClient->m_aClients[Kill.m_KillerID].m_Team;
		str_copy(Kill.m_aKillerName, m_pClient->m_aClients[Kill.m_KillerID].m_aName, sizeof(Kill.m_aKillerName));
		Kill.m_KillerRenderInfo = m_pClient->m_aClients[Kill.m_KillerID].m_RenderInfo;
		Kill.m_Weapon = pMsg->m_Weapon;
		Kill.m_ModeSpecial = pMsg->m_ModeSpecial;
		Kill.m_Tick = Client()->GameTick();
		
		// hide bot names in invasion
		if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_COOP)
		{
			if (m_pClient->m_aClients[Kill.m_KillerID].m_IsBot)
				str_copy(Kill.m_aKillerName, "", sizeof(""));
			
			if (m_pClient->m_aClients[Kill.m_VictimID].m_IsBot)
				str_copy(Kill.m_aVictimName, "", sizeof(""));
		}
		
		// add the message
		m_KillmsgCurrent = (m_KillmsgCurrent+1)%MAX_KILLMSGS;
		m_aKillmsgs[m_KillmsgCurrent] = Kill;
	}
}

void CKillMessages::OnRender()
{
	float Width = 400*3.0f*Graphics()->ScreenAspect();
	float Height = 400*3.0f;

	Graphics()->MapScreen(0, 0, Width*1.5f, Height*1.5f);
	float StartX = Width*1.5f-10.0f;
	float y = 20.0f;

	for(int i = 1; i <= MAX_KILLMSGS; i++)
	{
		int r = (m_KillmsgCurrent+i)%MAX_KILLMSGS;
		if(Client()->GameTick() > m_aKillmsgs[r].m_Tick+50*10)
			continue;

		float FontSize = 36.0f;
		float KillerNameW = TextRender()->TextWidth(0, FontSize, m_aKillmsgs[r].m_aKillerName, -1);
		float VictimNameW = TextRender()->TextWidth(0, FontSize, m_aKillmsgs[r].m_aVictimName, -1);

		float x = StartX;

		
		int Flags = m_pClient->m_Snap.m_pGameInfoObj ? m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags : 0;
		
		
		if (Flags & GAMEFLAG_TEAMS && !(Flags & GAMEFLAG_INFECTION))
		{
			if (m_aKillmsgs[r].m_VictimTeam == TEAM_RED)
				TextRender()->TextColor(250/255.0f, 100/255.0f, 0, 1.0f);
				
			if (m_aKillmsgs[r].m_VictimTeam == TEAM_BLUE)
				TextRender()->TextColor(0/255.0f, 100/255.0f, 230/255.0f, 1.0f);
		}
		
		// render victim name
		x -= VictimNameW;
		
		TextRender()->Text(0, x, y, FontSize, m_aKillmsgs[r].m_aVictimName, -1);

		// render victim tee
		x -= 32.0f;

		if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_FLAGS)
		{
			if(m_aKillmsgs[r].m_ModeSpecial&1)
			{
				Graphics()->BlendNormal();
				Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
				Graphics()->QuadsBegin();

				if(m_aKillmsgs[r].m_VictimTeam == TEAM_RED)
					RenderTools()->SelectSprite(SPRITE_FLAG_BLUE);
				else
					RenderTools()->SelectSprite(SPRITE_FLAG_RED);

				float Size = 56.0f;
				IGraphics::CQuadItem QuadItem(x, y-16, Size/2, Size);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
		}

		//RenderTools()->RenderTee(CAnimState::GetIdle(), &m_aKillmsgs[r].m_VictimRenderInfo, EMOTE_PAIN, vec2(-1,0), vec2(x, y+28));
		RenderTools()->RenderPortrait(&m_aKillmsgs[r].m_VictimRenderInfo, vec2(x, y+86), 3);
		
		x -= 36.0f;

		// render weapon
		x -= 48.0f;
		
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		RenderTools()->SetShadersForWeapon(m_aKillmsgs[r].m_Weapon);
		
		RenderTools()->RenderWeapon(m_aKillmsgs[r].m_Weapon, vec2(x+5, y+30), vec2(1, 0), 16, true, 0, 1.0f, true);
		Graphics()->ShaderEnd();
		
		/*
		Graphics()->QuadsBegin();
		RenderTools()->RenderWeapon(m_aKillmsgs[r].m_Weapon, vec2(x+5, y+30), vec2(1, 0), 16);
		Graphics()->QuadsEnd();
		Graphics()->ShaderEnd();
		*/
		
		/*
		if (IsTurret(m_aKillmsgs[r].m_Weapon))
		{
			Graphics()->QuadsBegin();
			RenderTools()->SelectSprite(SPRITE_WEAPON_TURRET);
			
			Graphics()->QuadsEnd();
		}
		*/
		
		/*
		if (m_aKillmsgs[r].m_Weapon >= 0)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_DEATHTYPES].m_Id);
			Graphics()->QuadsBegin();
			//RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[m_aKillmsgs[r].m_Weapon].m_pSpriteBody);
			RenderTools()->SelectSprite(SPRITE_DEATHTYPE1 + m_aKillmsgs[r].m_Weapon);
			//RenderTools()->DrawSprite(x, y+28, 64);
			IGraphics::CQuadItem QuadItem(x, y+28, 96, 96/2);
			Graphics()->QuadsDraw(&QuadItem, 1);
			Graphics()->QuadsEnd();
		}
		if (m_aKillmsgs[r].m_IsTurret)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_DEATHTYPES].m_Id);
			Graphics()->QuadsBegin();
			RenderTools()->SelectSprite(SPRITE_DEATHTYPE19);
			IGraphics::CQuadItem QuadItem(x, y+28, 96, 96/2);
			Graphics()->QuadsDraw(&QuadItem, 1);
			Graphics()->QuadsEnd();
		}
		*/
		x -= 56.0f;

		if(m_aKillmsgs[r].m_VictimID != m_aKillmsgs[r].m_KillerID)
		{
			if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_FLAGS)
			{
				if(m_aKillmsgs[r].m_ModeSpecial&2)
				{
					Graphics()->BlendNormal();
					Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
					Graphics()->QuadsBegin();

					if(m_aKillmsgs[r].m_KillerTeam == TEAM_RED)
						RenderTools()->SelectSprite(SPRITE_FLAG_BLUE, SPRITE_FLAG_FLIP_X);
					else
						RenderTools()->SelectSprite(SPRITE_FLAG_RED, SPRITE_FLAG_FLIP_X);

					float Size = 56.0f;
					IGraphics::CQuadItem QuadItem(x-56, y-16, Size/2, Size);
					Graphics()->QuadsDrawTL(&QuadItem, 1);
					Graphics()->QuadsEnd();
				}
			}

			if (Flags & GAMEFLAG_TEAMS && !(Flags & GAMEFLAG_INFECTION))
			{
				if (m_aKillmsgs[r].m_KillerTeam == TEAM_RED)
					TextRender()->TextColor(250/255.0f, 100/255.0f, 0, 1.0f);
				
				if (m_aKillmsgs[r].m_KillerTeam == TEAM_BLUE)
					TextRender()->TextColor(0/255.0f, 100/255.0f, 230/255.0f, 1.0f);
			}
			
			// render killer tee
			x -= 28.0f;
			//RenderTools()->RenderTee(CAnimState::GetIdle(), &m_aKillmsgs[r].m_KillerRenderInfo, EMOTE_ANGRY, vec2(1,0), vec2(x, y+28));
			RenderTools()->RenderPortrait(&m_aKillmsgs[r].m_KillerRenderInfo, vec2(x, y+86), 2);
			x -= 36.0f;

			// render killer name
			x -= KillerNameW;
			TextRender()->Text(0, x, y, FontSize, m_aKillmsgs[r].m_aKillerName, -1);
		}

		//y += 46.0f;
		y += 52.0f;
		
		TextRender()->TextColor(1, 1, 1, 1.0f);
	}
}
