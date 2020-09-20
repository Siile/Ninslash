#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/keys.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/client/gameclient.h>

#include <game/client/components/motd.h>
#include <game/client/components/scoreboard.h>

#include "skins.h"

#include <game/client/customstuff.h>
#include <game/client/customstuff/playerinfo.h>

#include "gamevote.h"

void CGameVoteDisplay::OnReset()
{
	m_GameVoteCount = 0;
	
	for (int i = 0; i < 6; i++)
	{
		m_aGameVoteDetails[i].m_Valid = false;
		m_aGameVoteDetails[i].m_Votes = 0;
	}
	
	m_Mouse1Loaded = false;
	m_ResetMouse = true;
	m_Mouse1 = false;
	m_MouseTrigger = false;
	m_MoveStartPos = vec2(0, 0);
	m_Moved = false;
	m_MoveTrigger = false;
	m_SelectorMouse = vec2(150, 150);
	m_Selected = -1;
	m_TimeLeft = 0;
}

bool CGameVoteDisplay::OnInput(IInput::CEvent Event)
{
	if(!IsActive())
		return false;
	
	if(Event.m_Key == KEY_MOUSE_1)
	{
		bool M = m_Mouse1;
		m_Mouse1 = Event.m_Flags&IInput::FLAG_PRESS;
		if (M != m_Mouse1)
			m_MouseTrigger = true;
		return true;
	}

	return false;
}

bool CGameVoteDisplay::OnMouseMove(float x, float y)
{
	if(!IsActive())
		return false;

	Input()->SetMouseModes(IInput::MOUSE_MODE_WARP_CENTER);
	
	Input()->GetRelativePosition(&x, &y);
	m_SelectorMouse += vec2(x,y)*0.5f;
	
	/*
	if (!m_Mouse1)
	{
		m_MoveStartPos = m_SelectorMouse;
		m_Moved = false;
		m_MoveTrigger = false;
	}
	else if (!m_Moved)
	{
		if (abs(length(m_SelectorMouse-m_MoveStartPos)) > 16.0f)
		{
			m_Moved = true;
			m_MoveTrigger = true;
		}
	}
	*/

	return true;
}

void CGameVoteDisplay::RenderMouse()
{
	// cursor
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CURSOR].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1,1,1,1);
	IGraphics::CQuadItem QuadItem(m_SelectorMouse.x, m_SelectorMouse.y, 16, 16);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();
}

void CGameVoteDisplay::SendVote()
{
	CNetMsg_Cl_VoteGameMode Msg;
	Msg.m_Vote = m_Selected;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}

void CGameVoteDisplay::OnRender()
{
	if (m_GameVoteCount <= 0)
		return;
	
	/*
	if(m_pClient->m_pScoreboard->Active() || m_pClient->m_pMotd->IsActive())
		return;
	*/

	Graphics()->MapScreen(0, 0, 300*Graphics()->ScreenAspect(), 300);
	
	m_SelectorMouse.x = clamp(m_SelectorMouse.x, 3.0f, 300*Graphics()->ScreenAspect()-5.0f);
	m_SelectorMouse.y = clamp(m_SelectorMouse.y, 3.0f, 295.0f);
	
	
	Graphics()->BlendNormal();
	
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0.0, 0.0, 0.0, 1.0f);
			
	// the background frame
	for (int x = 0; x < 3; x++)
		for (int y = 0; y < 2; y++)
		{
			if (!m_aGameVoteDetails[x+y*3].m_Valid)
				continue;

			if (m_Selected == x+y*3)
				Graphics()->SetColor(0.4f, 0.3f, 0.0f, 1.0f);
			else
				Graphics()->SetColor(0.0, 0.0, 0.0, 1.0f);
	
	
			vec2 p = vec2((150-102)*Graphics()->ScreenAspect() + x*102*Graphics()->ScreenAspect(), 100 + y*100);
			vec2 s = vec2(84, 44);
			
			IGraphics::CFreeformItem FreeFormItem(
				p.x-s.x, p.y-s.y,
				p.x+s.x, p.y-s.y,
				p.x-s.x, p.y+s.y,
				p.x+s.x, p.y+s.y);
									
			Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
		}
	
	Graphics()->QuadsEnd();
		
	// the background image
	for (int x = 0; x < 3; x++)
		for (int y = 0; y < 2; y++)
		{
			if (!m_aGameVoteDetails[x+y*3].m_Valid)
				continue;

			Graphics()->TextureSet(m_aGameVoteDetails[x+y*3].m_Texture);
			Graphics()->QuadsBegin();
			
			Graphics()->SetColor(1.0, 1.0, 1.0, 1.0f);
	
			vec2 p = vec2((150-102)*Graphics()->ScreenAspect() + x*102*Graphics()->ScreenAspect(), 100 + y*100);
			vec2 s = vec2(80, 40);
			
			Graphics()->QuadsSetSubsetFree(	0, 0,
											1, 0,
											0, 1,
											1, 1);
			
			IGraphics::CFreeformItem FreeFormItem(
				p.x-s.x, p.y-s.y,
				p.x+s.x, p.y-s.y,
				p.x-s.x, p.y+s.y,
				p.x+s.x, p.y+s.y);
									
			Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
			
			Graphics()->QuadsEnd();
			
			// check mouse input
			if (m_MouseTrigger)
			{
				if (abs(m_SelectorMouse.x - p.x) < s.x && abs(m_SelectorMouse.y - p.y) < s.y)
				{
					if (m_Selected != x+y*3)
					{
						m_Selected = x+y*3;
						SendVote();
						
						m_MouseTrigger = false;
					}
				}
			}
		}

	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0.5f, 0.5f, 0.5f, 0.4f);
			
	// the overlay frames
	for (int x = 0; x < 3; x++)
		for (int y = 0; y < 2; y++)
		{
			if (!m_aGameVoteDetails[x+y*3].m_Valid)
				continue;

	
			// top
			{
				vec2 p = vec2((150-102)*Graphics()->ScreenAspect() + x*102*Graphics()->ScreenAspect(), 100 + y*100-30);
				vec2 s = vec2(80, 10);
				
				IGraphics::CFreeformItem FreeFormItem(
					p.x-s.x, p.y-s.y,
					p.x+s.x, p.y-s.y,
					p.x-s.x, p.y+s.y,
					p.x+s.x, p.y+s.y);
										
				Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
			}
			
			// topleft
			{
				vec2 p = vec2((150-102)*Graphics()->ScreenAspect() + x*102*Graphics()->ScreenAspect() - 70, 100 + y*100-30);
				vec2 s = vec2(10, 10);
				IGraphics::CFreeformItem FreeFormItem(
					p.x-s.x, p.y-s.y,
					p.x+s.x, p.y-s.y,
					p.x-s.x, p.y+s.y,
					p.x+s.x, p.y+s.y);
										
				Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
			}
		}
	
	Graphics()->QuadsEnd();
	
	// labels
	for (int x = 0; x < 3; x++)
		for (int y = 0; y < 2; y++)
		{
			if (!m_aGameVoteDetails[x+y*3].m_Valid)
				continue;
			
			{ // name
				CTextCursor Cursor;
				TextRender()->SetCursor(&Cursor, (150-102+x*102)*Graphics()->ScreenAspect()-m_aGameVoteDetails[x+y*3].m_NameWidth, 100 + y*100 - 40+3, 10.0f, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
				Cursor.m_LineWidth = 150;
				TextRender()->TextEx(&Cursor, m_aGameVoteDetails[x+y*3].m_aName, -1);
			}
			{ // description
				CTextCursor Cursor;
				TextRender()->SetCursor(&Cursor, (150-102+x*102)*Graphics()->ScreenAspect()-m_aGameVoteDetails[x+y*3].m_DescriptionWidth, 100 + y*100 + 31, 6.0f, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
				Cursor.m_LineWidth = 150;
				TextRender()->TextEx(&Cursor, m_aGameVoteDetails[x+y*3].m_aDescription, -1);
			}
			{ // amount of cast votes
				char aBuf[16];
				str_format(aBuf, sizeof(aBuf), "%d", m_aGameVoteDetails[x+y*3].m_Votes);
			
				CTextCursor Cursor;
				TextRender()->SetCursor(&Cursor, (150-102+x*102)*Graphics()->ScreenAspect()-m_aGameVoteDetails[x+y*3].m_VotesWidth - 70, 100 + y*100 - 40+3, 10.0f, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
				Cursor.m_LineWidth = 150;
				TextRender()->TextEx(&Cursor, aBuf, -1);
			}
		}

	//Time = (Client()->GameTick()-m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick)/Client()->GameTickSpeed();
	
	// time left to vote
	{
		char aBuf[64];
		
		int Time = m_TimeLeft + (m_TimeLeftTick-Client()->GameTick())/Client()->GameTickSpeed();
		if (Time < 0)
		{
			Time = 0;
			str_format(aBuf, sizeof(aBuf), "Server is changing map");
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "Vote ends in %d...", Time);
		}
			
		
		CTextCursor Cursor;
		TextRender()->SetCursor(&Cursor, (150-135)*Graphics()->ScreenAspect(), 20, 12.0f, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
		Cursor.m_LineWidth = 150;
		TextRender()->TextEx(&Cursor, aBuf, -1);
	}
	
	if (m_Mouse1)
		m_Mouse1Loaded = false;
	
	RenderMouse();
	m_MouseTrigger = false;
}

void CGameVoteDisplay::OnMessage(int MsgType, void *pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_VOTESTATUS)
	{
		CNetMsg_Sv_VoteStatus *pMsg = (CNetMsg_Sv_VoteStatus *)pRawMsg;
		if (pMsg->m_Type == 1)
		{
			int aVotes[6] = {pMsg->m_Yes, pMsg->m_No, pMsg->m_Pass, pMsg->m_Total, pMsg->m_Option5, pMsg->m_Option6};
			
			for (int i = 0; i < 6; i++)
				m_aGameVoteDetails[i].m_Votes = aVotes[i];
		}
	}
	
	if(MsgType == NETMSGTYPE_SV_GAMEVOTE)
	{
		CNetMsg_Sv_GameVote *pMsg = (CNetMsg_Sv_GameVote *)pRawMsg;
		
		int i = pMsg->m_Index;
		
		if (m_aGameVoteDetails[i].m_Valid)
			return;
		
		m_aGameVoteDetails[i].m_Valid = true;
		m_aGameVoteDetails[i].m_Texture = m_pClient->m_pSkins->GetGameVote(m_pClient->m_pSkins->FindGameVote(pMsg->m_pImage))->m_Texture;
		
		str_copy(m_aGameVoteDetails[i].m_aName, pMsg->m_pName, sizeof(m_aGameVoteDetails[i].m_aName));
		str_copy(m_aGameVoteDetails[i].m_aDescription, pMsg->m_pDescription, sizeof(m_aGameVoteDetails[i].m_aDescription));
		
		m_TimeLeft = pMsg->m_TimeLeft;
		m_TimeLeftTick = Client()->GameTick();
		
		m_GameVoteCount++;
		
		{
			CTextCursor Cursor;
			TextRender()->SetCursor(&Cursor, 0, 0, 10.0f, TEXTFLAG_STOP_AT_END);
			Cursor.m_LineWidth = 300*Graphics()->ScreenAspect();
			TextRender()->TextEx(&Cursor, m_aGameVoteDetails[i].m_aName, -1);
			m_aGameVoteDetails[i].m_NameWidth = Cursor.m_X/2;
		}
		{
			CTextCursor Cursor;
			TextRender()->SetCursor(&Cursor, 0, 0, 6.0f, TEXTFLAG_STOP_AT_END);
			Cursor.m_LineWidth = 300*Graphics()->ScreenAspect();
			TextRender()->TextEx(&Cursor, m_aGameVoteDetails[i].m_aDescription, -1);
			m_aGameVoteDetails[i].m_DescriptionWidth = Cursor.m_X/2;
		}
		{
			CTextCursor Cursor;
			TextRender()->SetCursor(&Cursor, 0, 0, 10.0f, TEXTFLAG_STOP_AT_END);
			Cursor.m_LineWidth = 300*Graphics()->ScreenAspect();
			TextRender()->TextEx(&Cursor, "0", -1);
			m_aGameVoteDetails[i].m_VotesWidth = Cursor.m_X/2;
		}
	}
}

