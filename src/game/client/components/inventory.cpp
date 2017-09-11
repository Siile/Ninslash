#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <engine/keys.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/client/gameclient.h>
#include <game/gamecore.h> // get_angle
#include <game/weapons.h> // get_angle
#include <game/client/ui.h>
#include <game/client/render.h>
#include <game/client/customstuff.h>
#include <game/client/components/controls.h>

#include <game/client/components/controls.h>
#include <game/client/components/sounds.h>
#include "inventory.h"

CInventory::CInventory()
{
	OnReset();
	m_ResetMouse = true;
}

void CInventory::ConKeyInventory(IConsole::IResult *pResult, void *pUserData)
{
	CInventory *pSelf = (CInventory *)pUserData;
	
	if(!pSelf->m_pClient->m_Snap.m_SpecInfo.m_Active && pSelf->Client()->State() != IClient::STATE_DEMOPLAYBACK)
		pSelf->m_Active = pResult->GetInteger(0) != 0;
}


void CInventory::OnConsoleInit()
{
	//Console()->Register("+gamepaditempicker", "", CFGFLAG_CLIENT, ConKeyItemPicker, this, "Open item selector");
	Console()->Register("+inventory", "", CFGFLAG_CLIENT, ConKeyInventory, this, "Open inventory");
}

void CInventory::OnReset()
{
	m_WasActive = false;
	m_Active = false;
	m_ResetMouse = true;
	m_Render = false;
	m_Mouse1 = false;
	m_MouseTrigger = false;
	m_DragItem = -1;
}

void CInventory::OnRelease()
{
	m_Active = false;
	m_ResetMouse = true;
	m_Render = false;
	m_Mouse1 = false;
	m_MouseTrigger = false;
	m_DragItem = -1;
}

void CInventory::OnMessage(int MsgType, void *pRawMsg)
{
}

bool CInventory::OnMouseMove(float x, float y)
{
	if(!m_Render)
		return false;

	Input()->GetRelativePosition(&x, &y);
	m_SelectorMouse += vec2(x,y);
	return true;
}


bool CInventory::OnInput(IInput::CEvent Event)
{
	if(!m_Render)
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

void CInventory::DrawCircle(float x, float y, float r, int Segments)
{
	IGraphics::CFreeformItem Array[32];
	int NumItems = 0;
	float FSegments = (float)Segments;
	for(int i = 0; i < Segments; i+=2)
	{
		float a1 = i/FSegments * 2*pi;
		float a2 = (i+1)/FSegments * 2*pi;
		float a3 = (i+2)/FSegments * 2*pi;
		float Ca1 = cosf(a1);
		float Ca2 = cosf(a2);
		float Ca3 = cosf(a3);
		float Sa1 = sinf(a1);
		float Sa2 = sinf(a2);
		float Sa3 = sinf(a3);

		Array[NumItems++] = IGraphics::CFreeformItem(
			x, y,
			x+Ca1*r, y+Sa1*r,
			x+Ca3*r, y+Sa3*r,
			x+Ca2*r, y+Sa2*r);
		if(NumItems == 32)
		{
			m_pClient->Graphics()->QuadsDrawFreeform(Array, 32);
			NumItems = 0;
		}
	}
	if(NumItems)
		m_pClient->Graphics()->QuadsDrawFreeform(Array, NumItems);
}



void CInventory::DrawLayer(vec2 Pos, vec2 Size)
{
	vec2 p1 = Pos - Size;
	vec2 p2 = Pos + Size;
	
	vec2 b = vec2(min(32.0f, Size.x / 2.0f), min(32.0f, Size.y / 2.0f));
	
	vec2 aPos[4];
	
	aPos[0] = p1;
	aPos[1] = p1+b;
	aPos[2] = p2-b;
	aPos[3] = p2;
	
	for (int x = 1; x < 4; x++)
		for (int y = 1; y < 4; y++)
		{
			p1 = vec2(aPos[x-1].x, aPos[y-1].y);
			p2 = vec2(aPos[x].x, aPos[y].y);
			
			Graphics()->QuadsSetSubsetFree(	(x-1)/4.0f, (y-1)/4.0f,  
											(x)/4.0f, (y-1)/4.0f, 
											(x-1)/4.0f, (y)/4.0f, 
											(x)/4.0f, (y)/4.0f);
			
			IGraphics::CFreeformItem Freeform(
				p1.x, p1.y,
				p2.x, p1.y,
				p1.x, p2.y,
				p2.x, p2.y);
						
			Graphics()->QuadsDrawFreeform(&Freeform, 1);
		}
}

static float s_Fade = 0.0f;
static vec2 s_ItemOffset[12];
static vec2 s_DragOffset;

void CInventory::DrawInventory(vec2 Pos, vec2 Size)
{
	//Size *= s_Fade;
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GUI_WINDOW1].m_Id);
	Graphics()->ShaderBegin(SHADER_ELECTRIC, 0.2f);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1, 1, 1, s_Fade*0.5f);
	DrawLayer(Pos, Size);
	
	int Selected = -1;
	
	// grid
	for (int x = 0; x < 4; x++)
		for (int y = 0; y < 3; y++)
		{
			float s = 64 * s_Fade;
			vec2 GSize = Size - vec2(8, 8);
			vec2 p = Pos-GSize + vec2(x+0.5f, y+0.5f)*GSize/vec2(4, 3)*2;
			RenderTools()->SelectSprite(SPRITE_GUI_SELECT);
			RenderTools()->DrawSprite(p.x, p.y, s);
			
			if (abs(m_SelectorMouse.x - p.x) < s*0.7f && abs(m_SelectorMouse.y - p.y) < s*0.7f)
				Selected = x+y*4;
		}
	
	Graphics()->QuadsEnd();
	Graphics()->ShaderEnd();
	
	vec2 aPos[12];
	
	// items
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
	Graphics()->QuadsBegin();
	for (int x = 0; x < 4; x++)
		for (int y = 0; y < 3; y++)
		{
			float s = 48 * s_Fade;
			
			if (x+y*4 == Selected)
				s *= 1.5f;
			
			vec2 GSize = Size - vec2(8, 8);
			vec2 p = Pos-GSize + vec2(x+0.5f, y+0.5f)*GSize/vec2(4, 3)*2;
			
			aPos[x+y*4] = p;
			
			p -= s_ItemOffset[x+y*4];
			
			int w = CustomStuff()->m_aItem[x+y*4]-1;
			
			if (w >= 0 && w < NUM_WEAPONS && m_DragItem != x+y*4)
			{
				RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[w].m_pSpriteBody);
				RenderTools()->DrawSprite(p.x, p.y, s);
			}
		}
		
	Graphics()->QuadsEnd();
	
	// mouse press
	if (m_MouseTrigger && m_Mouse1 && Selected >= 0)
	{
		if (m_DragItem != Selected)
			s_DragOffset = aPos[Selected] - m_SelectorMouse;
		
		m_DragItem = Selected;
	}
	
	// mouse release
	if (m_MouseTrigger && !m_Mouse1 && m_DragItem >= 0)
	{
		if (Selected >= 0)
			Swap(m_DragItem, Selected);
		
		s_ItemOffset[Selected] = aPos[Selected] - m_SelectorMouse;
		s_ItemOffset[m_DragItem] = aPos[m_DragItem] - aPos[Selected];
		
		m_DragItem = -1;
	}
	
	m_MouseTrigger = false;
}


void CInventory::Swap(int Item1, int Item2)
{
	if (Item1 < 0 || Item2 < 0 || Item1 >= 12 || Item2 >= 12)
		return;
	
	int i1 = CustomStuff()->m_aItem[Item1];
	CustomStuff()->m_aItem[Item1] = CustomStuff()->m_aItem[Item2];
	CustomStuff()->m_aItem[Item2] = i1;
}


void CInventory::RenderMouse()
{
	// drag drop item
	if (m_DragItem >= 0 && m_DragItem < 12)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		Graphics()->QuadsBegin();

		int w = CustomStuff()->m_aItem[m_DragItem]-1;
	
		if (w >= 0 && w < NUM_WEAPONS)
		{
			RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[w].m_pSpriteBody);
			RenderTools()->DrawSprite(m_SelectorMouse.x+s_DragOffset.x, m_SelectorMouse.y+s_DragOffset.y, 48*1.5f);
		}
			
		Graphics()->QuadsEnd();
	}
	
	// cursor
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CURSOR].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1,1,1,1);
	IGraphics::CQuadItem QuadItem(m_SelectorMouse.x, m_SelectorMouse.y, 24, 24);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();
}


void CInventory::Tick()
{
	for (int i = 0; i < 12; i++)
		s_ItemOffset[i] -= s_ItemOffset[i]/4.0f;
	
	s_DragOffset -= s_DragOffset / 4.0f;
}

void CInventory::OnRender()
{
	//if(Client()->State() < IClient::STATE_ONLINE)
	//	return;
	CustomStuff()->m_Inventory = false;
	
	if(m_pClient->m_Snap.m_SpecInfo.m_Active)
	{
		m_Active = false;
		m_WasActive = false;
		m_Render = false;
		return;
	}

	if (m_Active && !m_WasActive)
	{
		m_Render = !m_Render;
		m_WasActive = m_Active;
	}
	
	if (!m_Active && m_WasActive)
		m_WasActive = m_Active;


	static int64 LastTime = 0;
	int64 t = time_get();
	s_Fade += ((float)((t-LastTime)/(double)time_freq())) * (m_Render ? 1.0f : -1.0f) * 4.0f;
	s_Fade = clamp(s_Fade, 0.0f, 1.0f);
	LastTime = t;
	
	
	if (!m_Render && s_Fade < 0.01f)
		return;
	
	CUIRect Screen = *UI()->Screen();

	// clamp mouse
	m_SelectorMouse.x = clamp(m_SelectorMouse.x, 0.0f, Screen.w-16.0f);
	m_SelectorMouse.y = clamp(m_SelectorMouse.y, 0.0f, Screen.h-16.0f);
	
	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);
	Graphics()->BlendNormal();
	
	// gui
	DrawInventory(vec2(Screen.w/4, Screen.h/2), vec2(Screen.w/6, Screen.h/4));

	if (!m_Render)
		return;

	// mouse cursor
	RenderMouse();
	
	CustomStuff()->m_Inventory = true;
}