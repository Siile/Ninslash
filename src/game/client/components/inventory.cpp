#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <engine/keys.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/client/gameclient.h>
#include <game/gamecore.h> // get_angle
#include <game/weapons.h>
#include <game/buildables.h>
#include <game/client/ui.h>
#include <game/client/render.h>
#include <game/client/customstuff.h>
#include <game/client/components/controls.h>
#include <game/client/components/camera.h>
#include <game/client/components/effects.h>

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
	m_Mouse1Loaded = false;
	m_WasActive = false;
	m_Active = false;
	m_ResetMouse = true;
	m_Render = false;
	m_Mouse1 = false;
	m_MouseTrigger = false;
	m_DragItem = -1;
	m_DragPart = -1;
	m_DragSlot = -1;
	m_MoveStartPos = vec2(0, 0);
	m_Moved = false;
	m_MoveTrigger = false;
	m_Tab = 0;
	m_SelectedBuilding = -1;
	m_Minimized = false;
	m_Scale = 1.0f;
}

void CInventory::OnRelease()
{
	m_Active = false;
	m_ResetMouse = true;
	m_Render = false;
	m_Mouse1 = false;
	m_MouseTrigger = false;
	m_DragItem = -1;
	m_DragPart = -1;
	m_DragSlot = -1;
	m_MoveStartPos = vec2(0, 0);
	m_Moved = false;
	m_MoveTrigger = false;
	m_Minimized = false;
	m_Scale = 1.0f;
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



void CInventory::DrawCrafting(int Type, vec2 Pos, float Size)
{
	//
	return;
	//
	
	if (!IsModularWeapon(Type))
		return;	int Part1 = GetPart(Type, 0)-1;
	
	int Part2 = GetPart(Type, 1)-1;
	
	Pos.x -= Size / 4;
	
	if (Part1 >= 0)
	{
		RenderTools()->SelectSprite(SPRITE_WEAPON_PART1_BG_0+Part1);
		RenderTools()->DrawSprite(Pos.x, Pos.y, Size);
	}
	
	if (Part2 >= 0)
	{
		RenderTools()->SelectSprite(SPRITE_WEAPON_PART2_0+Part2);
		RenderTools()->DrawSprite(Pos.x+Size*(5.0f/8.4f), Pos.y, Size);
	}
	
	if (Part1 >= 0)
	{
		RenderTools()->SelectSprite(SPRITE_WEAPON_PART1_0+Part1);
		RenderTools()->DrawSprite(Pos.x, Pos.y, Size);
	}
}

static float s_Fade = 0.0f;
static float s_ItemEffectSelect[12];
static float s_ItemEffectPick[12];
static vec2 s_ItemOffset[12];
static vec2 s_DragOffset;

void CInventory::DrawInventory(vec2 Pos, vec2 Size)
{
	Size *= (m_Scale*0.75f + 0.25f);
	
	CUIRect Screen = *UI()->Screen();
	Pos += vec2(-Screen.w/5, Screen.h/5)*(1.0f - m_Scale);
	
	vec2 Tab1Pos = Pos + vec2(-Size.x*0.8f, -Size.y*1.125f);
	vec2 Tab2Pos = Pos + vec2(-Size.x*0.47f, -Size.y*1.125f);
	
	
	// tab icons (first one behind the frame)
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
	
	if (m_Tab == 1)
	{
		float s = 16 * s_Fade * (m_Scale*0.75f + 0.25f);
		int w = GetModularWeapon(1, 4);
		RenderTools()->SetShadersForWeapon(w);
		RenderTools()->RenderWeapon(w, Tab1Pos, vec2(1, 0), s, true);
		Graphics()->ShaderEnd();
	}
	
	if (m_Tab == 0)
	{
		float s = 16 * s_Fade * (m_Scale*0.75f + 0.25f);
		int w = GetStaticWeapon(SW_TOOL);
		RenderTools()->SetShadersForWeapon(w);
		RenderTools()->RenderWeapon(w, Tab2Pos, vec2(1, 0), s, true);
		Graphics()->ShaderEnd();
	}
	
	// main frame
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GUI_WINDOW1].m_Id);
	Graphics()->ShaderBegin(SHADER_ELECTRIC, 0.2f);
	Graphics()->QuadsBegin();
	vec3 c = vec3(0.0f, 1.0f, 1.0f);
	Graphics()->SetColor(c.r, c.g, c.b, s_Fade*0.5f);
	DrawLayer(Pos, Size);
	
	// inventory tab frame
	if (m_Tab == 0)
		Graphics()->SetColor(c.r, c.g, c.b, s_Fade*0.5f);
	else
		Graphics()->SetColor(c.r*0.5f, c.g*0.5f, c.b*0.5f, s_Fade*0.5f);
	
	DrawLayer(Tab1Pos, Size/6.0f);
	
	// building tab frame
	if (m_Tab == 1)
		Graphics()->SetColor(c.r, c.g, c.b, s_Fade*0.5f);
	else
		Graphics()->SetColor(c.r*0.5f, c.g*0.5f, c.b*0.5f, s_Fade*0.5f);
	
	DrawLayer(Tab2Pos, Size/6.0f);
	
	Graphics()->QuadsEnd();
	Graphics()->ShaderEnd();
	
	// tab icon over the frame
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
	
	if (m_Tab == 0)
	{
		float s = 16 * s_Fade * (m_Scale*0.75f + 0.25f);
		int w = GetModularWeapon(1, 4);
		RenderTools()->SetShadersForWeapon(w);
		RenderTools()->RenderWeapon(w, Tab1Pos, vec2(1, 0), s, true);
		Graphics()->ShaderEnd();
	}
	
	if (m_Tab == 1)
	{
		float s = 16 * s_Fade * (m_Scale*0.75f + 0.25f);
		int w = GetStaticWeapon(SW_TOOL);
		RenderTools()->SetShadersForWeapon(w);
		RenderTools()->RenderWeapon(w, Tab2Pos, vec2(1, 0), s, true);
		Graphics()->ShaderEnd();
	}
	
	
	// frame continues...
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GUI_WINDOW1].m_Id);
	Graphics()->ShaderBegin(SHADER_ELECTRIC, 0.2f);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(c.r, c.g, c.b, s_Fade*0.5f);
	
	if (m_Minimized && m_Scale < 0.2f)
	{
		if (abs(m_SelectorMouse.x - Pos.x) < Size.x && abs(m_SelectorMouse.y - Pos.y) < Size.y)
		{
			m_Minimized = false;
			m_pClient->m_pControls->m_SelectedBuilding = 0;
			m_SelectedBuilding = -1;
		
			// gui open sound
		
		}
	}
	
	int Selected = -1;
	int SelectedPart = -1;
	
	// grid
	for (int x = 0; x < 4; x++)
		for (int y = 0; y < 3; y++)
		{
			if (y == 0 && x == CustomStuff()->m_WeaponSlot && m_Tab == 0)
				Graphics()->SetColor(0.3f, 1.0f, 0.3f, s_Fade*0.7f);
			else
				Graphics()->SetColor(c.r, c.g, c.b, s_Fade*0.5f);
			
			float s = 112 * s_Fade * (m_Scale*0.75f + 0.25f);
			float s2 = s * (1.0f+s_ItemEffectSelect[x+y*4]*0.5f);
			vec2 GSize = Size - vec2(8, 8);
			vec2 p = Pos-GSize + vec2(x+0.5f, y+0.5f)*GSize/vec2(4, 3)*2;
			RenderTools()->SelectSprite(SPRITE_GUI_SELECT3);
			RenderTools()->DrawSprite(p.x, p.y, s2);
			
			s2 = 0.35f * (1.0f+s_ItemEffectSelect[x+y*4]);
			
			if (y == 0)
			{
				/*
				Graphics()->SetColor(0.0f, 1.0f, 0.5f, s_Fade*0.5f);
				RenderTools()->SelectSprite(SPRITE_GUI_NUM1+x);
				RenderTools()->DrawSprite(p.x-s*0.35f, p.y-s*0.35f, s*s2);
				Graphics()->SetColor(c.r, c.g, c.b, s_Fade*0.5f);
				*/
			}
			
			if (abs(m_SelectorMouse.x - p.x) < s*0.65f && abs(m_SelectorMouse.y - p.y) < s*0.65f)
				Selected = x+y*4;
		}
	
	Graphics()->QuadsEnd();
	Graphics()->ShaderEnd();
	
	vec2 aPos[12];
	
	// items & weapons
	if (m_Tab == 0)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		
		for (int x = 0; x < 4; x++)
		{
			for (int y = 0; y < 3; y++)
			{
				float s = 16 * s_Fade * m_Scale;
				float s2 = 16 * s_Fade * m_Scale;
				
				if (x+y*4 == Selected)
					s *= 1.5f;
				
				vec2 GSize = Size - vec2(8, 8);
				vec2 p = Pos-GSize + vec2(x+0.5f, y+0.5f)*GSize/vec2(4, 3)*2;
				vec2 p2 = p;
				
				aPos[x+y*4] = p;
				
				p -= s_ItemOffset[x+y*4];
				
				int w = CustomStuff()->m_aItem[x+y*4];
				
				// item slot numbers
				if (y == 0)
				{
					//Graphics()->SetColor(0.0f, 1.0f, 0.5f, s_Fade*0.5f);
					Graphics()->QuadsBegin();
					Graphics()->SetColor(0.0f, 1.0f, 0.5f, s_Fade*0.5f);
					RenderTools()->SelectSprite(SPRITE_GUINUMBER_0+x+1);
					RenderTools()->DrawSprite(p2.x-s2*2.5f, p2.y-s2*2.5f, s2*2.8f);
					Graphics()->QuadsEnd();
					//Graphics()->SetColor(c.r, c.g, c.b, s_Fade*0.5f);
				}
				
				if (w >= 0 && (m_DragItem != x+y*4))
				{
					RenderTools()->SetShadersForWeapon(w);
					RenderTools()->RenderWeapon(w, p, vec2(1, 0), s, true);
					Graphics()->ShaderEnd();
				}
			}
		}
	}
	
	
	// building
	if (m_Tab == 1)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
		Graphics()->QuadsBegin();
		
		for (int x = 0; x < 4; x++)
		{
			for (int y = 0; y < 3; y++)
			{
				float s = 16 * s_Fade * (m_Scale*0.75f + 0.25f);
				
				if (x+y*4 == Selected)
					s *= 1.5f;
				
				vec2 GSize = Size - vec2(8, 8);
				vec2 p = Pos-GSize + vec2(x+0.5f, y+0.5f)*GSize/vec2(4, 3)*2;
				

				if (x+y*4 < NUM_BUILDABLES)
				{
					p.y += BuildableOffset[x+y*4] * 0.01f;
					
					// outlines
					if (x+y*4 == m_SelectedBuilding)
					{
						Graphics()->QuadsEnd();
						Graphics()->ShaderBegin(SHADER_GRAYSCALE, 0.0f);
						Graphics()->QuadsBegin();
						
						RenderTools()->SelectSprite(SPRITE_KIT_BARREL+x+y*4);
						RenderTools()->DrawSprite(p.x-1, p.y-1, s*5.0f);
						RenderTools()->DrawSprite(p.x+1, p.y-1, s*5.0f);
						RenderTools()->DrawSprite(p.x+1, p.y+1, s*5.0f);
						RenderTools()->DrawSprite(p.x-1, p.y+1, s*5.0f);
					
						Graphics()->QuadsEnd();
						Graphics()->ShaderEnd();
						Graphics()->QuadsBegin();
					}
					
					RenderTools()->SelectSprite(SPRITE_KIT_BARREL+x+y*4);
					RenderTools()->DrawSprite(p.x, p.y, s*5.0f);
				}
			}
		}
		
		Graphics()->QuadsEnd();
		
		// kit costs
		int LocalKits = clamp(CustomStuff()->m_LocalKits ,0, 9);

		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		Graphics()->QuadsBegin();
		
		for (int x = 0; x < 4; x++)
		{
			for (int y = 0; y < 3; y++)
			{
				float s = 16 * s_Fade * m_Scale;
				
				vec2 GSize = Size - vec2(8, 8);
				vec2 p = Pos-GSize + vec2(x+0.5f, y+0.5f)*GSize/vec2(4, 3)*2;
				

				if (x+y*4 < NUM_BUILDABLES)
				{
					p += vec2(s*2.0f, s*2.0f);
					
					RenderTools()->SelectSprite(SPRITE_PICKUP_KIT);
					RenderTools()->DrawSprite(p.x, p.y, s*3.0f);
					
					int Cost = BuildableCost[x+y*4];
					
					if (x+y*4 == Selected)
					{
						Graphics()->QuadsEnd();
						Graphics()->ShaderBegin(SHADER_GRAYSCALE, 0.0f);
						Graphics()->QuadsBegin();
						
						Graphics()->SetColor(0.0f, 0.0f, 0.0f, 1.0f);
						RenderTools()->SelectSprite(SPRITE_GUINUMBER_0+Cost);
						RenderTools()->DrawSprite(p.x-1, p.y-1, s*2.0f);
						RenderTools()->DrawSprite(p.x+1, p.y-1, s*2.0f);
						RenderTools()->DrawSprite(p.x+1, p.y+1, s*2.0f);
						RenderTools()->DrawSprite(p.x-1, p.y+1, s*2.0f);
					
						Graphics()->QuadsEnd();
						Graphics()->ShaderEnd();
						Graphics()->QuadsBegin();
					}
					
					if (x+y*4 == Selected)
					{
						if (LocalKits < Cost)
							Graphics()->SetColor(0.9f, 0.1f, 0.1f, 1.0f);
						else
							Graphics()->SetColor(0.1f, 0.9f, 0.1f, 1.0f);
					}
					else
						Graphics()->SetColor(0.9f, 0.9f, 0.9f, 1.0f);
					
					RenderTools()->SelectSprite(SPRITE_GUINUMBER_0+Cost);
					RenderTools()->DrawSprite(p.x, p.y, s*2.0f);
				}
				
				Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
			}
		}
		
		Graphics()->QuadsEnd();
	}

	
	// selected weapon / crafting
	/*
	int Part1 = 0;
	int Part2 = 0;
	*/
	int Part = 0;
	vec2 pp1;
	vec2 pp2;
	
	// this works, but crafting is kinda useless feature so it's disabled
	/*
	if (CustomStuff()->m_WeaponSlot >= 0 && CustomStuff()->m_WeaponSlot < 4 && IsModularWeapon(CustomStuff()->m_aItem[CustomStuff()->m_WeaponSlot]))
	{
		vec2 p = Pos-vec2(0, Size.y*1.35f);
		float s = Size.x/7.0f;
		int Type = CustomStuff()->m_aItem[CustomStuff()->m_WeaponSlot];
		
		RenderTools()->SetShadersForWeapon(Type);
		Graphics()->QuadsBegin();
		
		int Drag1 = -1;
		int Drag2 = -1;
	
		// 
		Part1 = GetPart(Type, 0)-1;
		Part2 = GetPart(Type, 1)-1;
		
		
		ivec2 WSize = GetWeaponVisualSize(Type);
		ivec2 WSize2 = GetWeaponVisualSize2(Type);
		
		if (Part2 < 5)
			p.x -= s * (2)/2;
		else
			p.x -= s*(8.0f/2.0f)/2;
		
		pp1 = p;
		pp2 = p;
		
		if (Part2 < 5)
			pp2.x += s*(WSize.x-1);
		else
			pp2.x += s*(7.0f/2.0f);
		
		//IGraphics::CQuadItem QuadItem2(Pos.x+Dir.x*Size*(WSize.x-1), Pos.y+Dir.y*Size*(WSize.x-1), Size*WSize2.x, Size*WSize2.y);
		
		// get mouse pos
		
		float ss = 0.35f;
				
		if (m_DragItem >= 0)
		{
			Drag1 = GetPart(CustomStuff()->m_aItem[m_DragItem], 0)-1;
			Drag2 = GetPart(CustomStuff()->m_aItem[m_DragItem], 1)-1;
				
			if (Drag1 >= 0 && Drag2 >= 0)
			{
				//if (distance(m_SelectorMouse, p+vec2(s/4, 0)) < s/2.5f)
				if (m_SelectorMouse.x > pp1.x - (s*WSize.x)*ss && m_SelectorMouse.x < pp1.x + (s*WSize.x)*ss &&
					m_SelectorMouse.y > pp1.y - (s*WSize.y)*ss && m_SelectorMouse.y < pp1.y + (s*WSize.y)*ss)
					Part = -1;
			}
			//else if (Drag1 >= 0 && distance(m_SelectorMouse, pp1) < s/4)
			else if (Drag1 >= 0 && m_SelectorMouse.x > pp1.x - (s*WSize.x)*ss && m_SelectorMouse.x < pp1.x + (s*WSize.x)*ss &&
					m_SelectorMouse.y > pp1.y - (s*WSize.y)*ss && m_SelectorMouse.y < pp1.y + (s*WSize.y)*ss)
				Part = 1;
			//else if (Drag2 >= 0 && distance(m_SelectorMouse, pp2) < s/4)
			else if (Drag2 >= 0 && m_SelectorMouse.x > pp2.x - (s*WSize2.x)*ss && m_SelectorMouse.x < pp2.x + (s*WSize2.x)*ss &&
					m_SelectorMouse.y > pp2.y - (s*WSize2.y)*ss && m_SelectorMouse.y < pp2.y + (s*WSize2.y)*ss)
				Part = 2;
		}
		
		// render
		
		// render weapon parts - drawcrafting(
		
		// part 1 bg
		if (Part1 >= 0)
		{
			if (m_DragSlot != 0)
			{
				RenderTools()->SelectSprite(SPRITE_WEAPON_PART1_BG_0+Part1);
				
				IGraphics::CQuadItem QuadItem(pp1.x, pp1.y, s*WSize.x, s*WSize.y);
				Graphics()->QuadsDraw(&QuadItem, 1);
				
				//RenderTools()->DrawSprite(pp1.x, pp1.y, s);
			}
		}
		
		if (Part2 >= 0)
		{
			if (m_DragSlot != 1)
			{
				RenderTools()->SelectSprite(SPRITE_WEAPON_PART2_0+Part2);
				//RenderTools()->DrawSprite(pp2.x, pp2.y, s);
				
				IGraphics::CQuadItem QuadItem(pp2.x, pp2.y, s*WSize2.x, s*WSize2.y);
				Graphics()->QuadsDraw(&QuadItem, 1);
				
				if (m_SelectorMouse.x > pp2.x - (s*WSize2.x)*ss && m_SelectorMouse.x < pp2.x + (s*WSize2.x)*ss &&
					m_SelectorMouse.y > pp2.y - (s*WSize2.y)*ss && m_SelectorMouse.y < pp2.y + (s*WSize2.y)*ss)
					SelectedPart = 1;
			}
			// dragging the part away
			//else
			if (SelectedPart == 1)
			{
				Graphics()->QuadsEnd();
				Graphics()->ShaderBegin(SHADER_ELECTRIC, 0.7f);
				Graphics()->QuadsBegin();
				
				RenderTools()->SelectSprite(SPRITE_WEAPON_PART2_0+Part2);
				//RenderTools()->DrawSprite(pp2.x, pp2.y, s);

				IGraphics::CQuadItem QuadItem(pp2.x, pp2.y, s*WSize2.x, s*WSize2.y);
				Graphics()->QuadsDraw(&QuadItem, 1);
				
				Graphics()->QuadsEnd();
				RenderTools()->SetShadersForWeapon(Type);
				Graphics()->QuadsBegin();
			}
		}
		
	
		// new part2 ghost
		if (Drag2 >= 0 || GetPart(m_DragPart, 1)-1 >= 0)
		{
			Graphics()->QuadsEnd();
			Graphics()->ShaderBegin(SHADER_SPAWN, 0.4f);
			Graphics()->QuadsBegin();
			
			int DPart = Drag2;
	
			if (m_DragPart > 0)
				DPart = GetPart(m_DragPart, 1)-1;
		
			if (DPart >= 0)
			{
				RenderTools()->SelectSprite(SPRITE_WEAPON_PART2_0+DPart);

				IGraphics::CQuadItem QuadItem(pp2.x, pp2.y, s*WSize2.x, s*WSize2.y);
				Graphics()->QuadsDraw(&QuadItem, 1);
			}
			
			Graphics()->QuadsEnd();
			RenderTools()->SetShadersForWeapon(Type);
			Graphics()->QuadsBegin();
		}
		
		
		if (Part1 >= 0)
		{
			if (m_DragSlot != 0)
			{
				RenderTools()->SelectSprite(SPRITE_WEAPON_PART1_0+Part1);
				//RenderTools()->DrawSprite(pp1.x, pp1.y, s);
				
				IGraphics::CQuadItem QuadItem(pp1.x, pp1.y, s*WSize.x, s*WSize.y);
				Graphics()->QuadsDraw(&QuadItem, 1);
				
				if (SelectedPart < 0 && m_SelectorMouse.x > pp1.x - (s*WSize.x)*ss && m_SelectorMouse.x < pp1.x + (s*WSize.x)*ss &&
					m_SelectorMouse.y > pp1.y - (s*WSize.y)*ss && m_SelectorMouse.y < pp1.y + (s*WSize.y)*ss)
					SelectedPart = 0;
			}
			// dragging the part away
			
			if (SelectedPart == 0)
			{
				Graphics()->QuadsEnd();
				Graphics()->ShaderBegin(SHADER_ELECTRIC, 0.75f);
				Graphics()->QuadsBegin();
				
				RenderTools()->SelectSprite(SPRITE_WEAPON_PART1_0+Part1);
				//RenderTools()->DrawSprite(pp1.x, pp1.y, s);
				
				IGraphics::CQuadItem QuadItem(pp1.x, pp1.y, s*WSize.x, s*WSize.y);
				Graphics()->QuadsDraw(&QuadItem, 1);
				
				Graphics()->QuadsEnd();
				Graphics()->ShaderEnd();
				Graphics()->QuadsBegin();
			}
		}
		
		
		// new part1 ghost
		if (Drag1 >= 0 || GetPart(m_DragPart, 0)-1 >= 0)
		{
			Graphics()->QuadsEnd();
			Graphics()->ShaderBegin(SHADER_SPAWN, 0.4f);
			Graphics()->QuadsBegin();
			int i = GetPart(CustomStuff()->m_aItem[m_DragItem], 0)-1;
			
			int DPart = Drag1;
	
			if (m_DragPart > 0)
				DPart = GetPart(m_DragPart, 0)-1;
		
			if (DPart >= 0)
			{
				RenderTools()->SelectSprite(SPRITE_WEAPON_PART1_0+DPart);
				//RenderTools()->DrawSprite(pp1.x, pp1.y, s);

				IGraphics::CQuadItem QuadItem(pp1.x, pp1.y, s*WSize.x*1.1f, s*WSize.y*1.1f);
				Graphics()->QuadsDraw(&QuadItem, 1);
			}
			
			Graphics()->QuadsEnd();
			Graphics()->ShaderEnd();
			Graphics()->QuadsBegin();
		}
		
		Graphics()->QuadsEnd();
		Graphics()->ShaderEnd();
	}
	*/
	
	
	// mouse press
	/*
	if (m_MouseTrigger && m_Mouse1 && Selected >= 0)
	{
		//if (m_DragItem != Selected)
		//	s_DragOffset = aPos[Selected] - m_SelectorMouse;
	
		s_DragOffset = vec2(0, 0);
		m_DragItem = Selected;
	}
	*/
	
	//if (!m_Mouse1)
	//	m_Mouse1Loaded = true;
		
		
	// building actions
	if (m_Tab == 1)
	{
		// mouse click
		if (m_MouseTrigger && !m_Moved && !m_Mouse1 && Selected >= 0)
		{
			int LocalKits = clamp(CustomStuff()->m_LocalKits ,0, 9);
			
			if (LocalKits >= BuildableCost[Selected])
			{
				m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_INV4, 0);
				m_pClient->m_pControls->m_SelectedBuilding = Selected+1;
				m_SelectedBuilding = Selected;
				m_Minimized = true;
				
				m_pClient->m_pControls->m_BuildMode = true;
			}
			else
			{
				// negative gui sound
				m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_GUI_DENIED1, 0);
			}
		}
		
		if (m_Minimized && m_Scale > 0.0f)
			m_Scale = max(0.0f, m_Scale - 0.05f);
		
		if (!m_Minimized && m_Scale < 1.0f)
			m_Scale = min(1.0f, m_Scale + 0.05f);
	}
	
	
	// tab actions
	if (m_Mouse1 && m_Mouse1Loaded)
	{
		if (abs(m_SelectorMouse.x - Tab1Pos.x) < Size.x/7.0f && abs(m_SelectorMouse.y - Tab1Pos.y) < Size.y/7.0f)
			m_Tab = 0;
		
		if (abs(m_SelectorMouse.x - Tab2Pos.x) < Size.x/7.0f && abs(m_SelectorMouse.y - Tab2Pos.y) < Size.y/7.0f)
			m_Tab = 1;
	}
	
	
	// item & weapon actions
	if (m_Tab == 0)
	{
		if (m_MoveTrigger && Selected >= 0)
		{
			if (m_DragItem != Selected)
				s_DragOffset = aPos[Selected] - m_SelectorMouse;
			
			m_DragItem = Selected;
			m_MoveTrigger = false;
		}
		
		if (m_MoveTrigger && SelectedPart >= 0)
		{
			s_DragOffset = (SelectedPart == 0 ? pp1 : vec2(0, 0)) + (SelectedPart == 1 ? pp2 : vec2(0, 0)) - m_SelectorMouse;
			
			int i = GetPart(CustomStuff()->m_aItem[CustomStuff()->m_WeaponSlot], SelectedPart);
			m_DragPart = GetModularWeapon(SelectedPart == 0 ? i : 0, SelectedPart == 1 ? i : 0);
			m_DragSlot = SelectedPart;
			m_MoveTrigger = false;
		}
		
		// mouse release
		if (m_MouseTrigger && !m_Mouse1 && m_DragItem >= 0)
		{
			if (Part)
			{
				if (m_DragItem != CustomStuff()->m_WeaponSlot)
					Combine(m_DragItem, CustomStuff()->m_WeaponSlot);
			}
			else
			{
				if (Selected >= 0)
				{
					Swap(m_DragItem, Selected);
					
					s_ItemOffset[Selected] = aPos[Selected] - m_SelectorMouse;
					s_ItemEffectSelect[Selected] = 0.5f;
					
					if (m_DragItem != Selected)
						s_ItemOffset[m_DragItem] = aPos[m_DragItem] - aPos[Selected];
				}
				else
					s_ItemOffset[m_DragItem] = aPos[m_DragItem] - m_SelectorMouse;
			}
				
			m_DragItem = -1;
		}
		
		// weapon part to inventory
		if (m_MouseTrigger && !m_Mouse1 && m_DragPart >= 0)
		{
			if (Selected >= 0)
			{
				TakePart(CustomStuff()->m_WeaponSlot, m_DragSlot, Selected);
			}
			else
			{
				
			}
				
			m_DragPart = -1;
			m_DragSlot = -1;
		}
		
		// mouse click
		if (m_MouseTrigger && !m_Moved && !m_Mouse1 && m_DragItem < 0 && Selected >= 0 && CustomStuff()->m_aItem[Selected] >= 0)
		{
			if (Selected < 4)
			{
				m_pClient->m_pControls->m_InputData.m_WantedWeapon = Selected+2;
				m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_INV4, 0);
			}
			else
			{
				int w = CustomStuff()->m_WeaponSlot;
				
				if (w >= 0 && w < 4)
				{
					s_ItemEffectSelect[w] = 1.0f;
					Swap(Selected, w);
					s_ItemOffset[Selected] = aPos[Selected] - aPos[w];
					s_ItemOffset[w] = aPos[w] - aPos[Selected];
				}
			}
			
			// effect trigger
			s_ItemEffectSelect[Selected] = 1.0f;
		}
	}
	
	m_MouseTrigger = false;
	
	
	//if (m_Mouse1)
	//	m_Mouse1Loaded = false;
}


void CInventory::DrawBuildMode()
{
	if (m_Tab != 1 || m_SelectedBuilding < 0 || m_SelectedBuilding >= NUM_BUILDABLES)
		return;
	
	//RenderTools()->SelectSprite(SPRITE_KIT_BARREL+Selected,
	//								(Selected == 2 && CustomStuff()->m_FlipBuilding) ? SPRITE_FLAG_FLIP_X : 0 + (FlipY ? SPRITE_FLAG_FLIP_Y : 0));

	// validity checks, snap & rotate
	vec2 Pos = m_SelectorMouse + m_pClient->m_pCamera->m_Center;

	
	int Selected = m_SelectedBuilding;
	int SnapRange = 128;
	
	bool Valid = false;
	bool FlipY = false;
	int Cost = 0; //BuildableCost[Selected];
	
	//m_pClient->m_pEffects->ChainsawSmoke(Pos);
	
	if (!Collision()->IsTileSolid(Pos.x, Pos.y))
		Valid = true;
	
	
			// snap y down
			if (Valid && Selected != BUILDABLE_FLAMETRAP)
			{
				vec2 To = Pos+vec2(0, SnapRange);
				if (Collision()->IntersectLine(Pos, To, 0x0, &To))
				{
					Pos = To;
					Pos.y += BuildableOffset[Selected];
				}
				else
					Valid = false;
				
				if (!Collision()->IsTileSolid(To.x - 22, To.y+2) || !Collision()->IsTileSolid(To.x + 22, To.y+2))
					Valid = false;

				if (Collision()->IsTileSolid(To.x , To.y-64))
					Valid = false;
				
				if (Collision()->IsForceTile(To.x, To.y+16))
					Valid = false;
			}
			
			// snap y up / turret stand
			if (!Valid && (Selected == BUILDABLE_TURRET || Selected == BUILDABLE_TESLACOIL))
			{
				vec2 To = Pos+vec2(0, -SnapRange);
				if (Collision()->IntersectLine(Pos, To, 0x0, &To))
				{
					Pos = To;
					Pos.y -= BuildableOffset[Selected];
					Valid = true;
				}
				else
					Valid = false;
				
				if (Valid)
					FlipY = true;
				
				if (!Collision()->IsTileSolid(To.x - 22, To.y-12) || !Collision()->IsTileSolid(To.x + 22, To.y-12))
					Valid = false;

				if (Collision()->IsTileSolid(To.x , To.y+64))
					Valid = false;
				
				if (Collision()->IsTileSolid(To.x , To.y+6))
					Valid = false;
			}
			
			// lightning wall line
			if (Valid && Selected == BUILDABLE_LIGHTNINGWALL)
			{
				vec2 To = Pos+vec2(0, -550);
				if (Collision()->IntersectLine(Pos, To, 0x0, &To))
				{
					Graphics()->TextureSet(-1);
					Graphics()->QuadsBegin();
					Graphics()->SetColor(0.0f, 1.0f, 0.0f, 0.3f);
				
					Pos -= m_pClient->m_pCamera->m_Center;
					To -= m_pClient->m_pCamera->m_Center;
				
					IGraphics::CFreeformItem FreeFormItem(
						Pos.x-4, Pos.y,
						Pos.x+4, Pos.y,
						To.x-4, To.y,
						To.x+4, To.y);
										
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				
					Graphics()->QuadsEnd();
					
					Pos += m_pClient->m_pCamera->m_Center;
				}
				else
					Valid = false;
			}
			
			// snap x
			if (Valid && Selected == BUILDABLE_FLAMETRAP)
			{
				vec2 To = Pos+vec2(SnapRange, 0);
				if (Collision()->IntersectLine(Pos, To, 0x0, &To))
				{
					Pos = To;
					CustomStuff()->m_FlipBuilding = true;
					Pos.x -= BuildableOffset[Selected];
				}
				else
					Valid = false;
				
				if (!Valid)
				{
					To = Pos+vec2(-SnapRange, 0);
					if (Collision()->IntersectLine(Pos, To, 0x0, &To))
					{
						Pos = To;
						Pos.x += BuildableOffset[Selected];
						Valid = true;
						CustomStuff()->m_FlipBuilding = false;
					}
				}
				
				int cx = CustomStuff()->m_FlipBuilding ? 16 : -16;
				
				if (!Collision()->IsTileSolid(To.x+cx, To.y-26) || !Collision()->IsTileSolid(To.x+cx, To.y+26))
					Valid = false;
				
				if (Collision()->IsTileSolid(To.x-cx, To.y-26) || Collision()->IsTileSolid(To.x-cx, To.y+26))
					Valid = false;
			}
			
			
			// final sanity checks
			if (Selected != 2 && Valid)
			{
				// ground on both sides
				if (Collision()->IsTileSolid(Pos.x - 12, Pos.y) || Collision()->IsTileSolid(Pos.x + 12, Pos.y))
					Valid = false;
			}
			
			// not too close to other buildings
			float Range = 48.0f;
			
			if (Selected == BUILDABLE_TESLACOIL)
				Range = 74.0f;
			
			if (m_pClient->BuildingNear(Pos, Range))
				Valid = false;
		
			// check for kits
			if (Cost > CustomStuff()->m_LocalKits)
				Valid = false;
	
	
	
	//
	if (Valid)
	{
		/*
		if (FlipY)
		{
			if (Selected == BUILDABLE_TURRET)
				CustomStuff()->m_BuildPos.y += BuildableOffset[Selected]-18;
			else if (Selected == BUILDABLE_TESLACOIL)
				CustomStuff()->m_BuildPos.y += BuildableOffset[Selected]-38;
		}
		*/
		
		CustomStuff()->m_BuildPos = Pos;
		CustomStuff()->m_BuildPosValid = true;
	}
	else
		CustomStuff()->m_BuildPosValid = false;
	
	
	
	// rendering
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	
	Pos -= m_pClient->m_pCamera->m_Center;
	
	// outlines
	Graphics()->ShaderBegin(SHADER_GRAYSCALE, 0.0f);
	Graphics()->QuadsBegin();
	if (Valid)	Graphics()->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
	else		Graphics()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
	RenderTools()->SelectSprite(SPRITE_KIT_BARREL+Selected,
		(Selected == 2 && CustomStuff()->m_FlipBuilding) ? SPRITE_FLAG_FLIP_X : 0 + (FlipY ? SPRITE_FLAG_FLIP_Y : 0));
	RenderTools()->DrawSprite(Pos.x-1, Pos.y-1, BuildableSize[Selected]);
	RenderTools()->DrawSprite(Pos.x+1, Pos.y-1, BuildableSize[Selected]);
	RenderTools()->DrawSprite(Pos.x-1, Pos.y+1, BuildableSize[Selected]);
	RenderTools()->DrawSprite(Pos.x+1, Pos.y+1, BuildableSize[Selected]);
	Graphics()->QuadsEnd();
	Graphics()->ShaderEnd();
	
	// building sprite
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	RenderTools()->SelectSprite(SPRITE_KIT_BARREL+Selected,
		(Selected == 2 && CustomStuff()->m_FlipBuilding) ? SPRITE_FLAG_FLIP_X : 0 + (FlipY ? SPRITE_FLAG_FLIP_Y : 0));
	RenderTools()->DrawSprite(Pos.x, Pos.y, BuildableSize[Selected]);
	Graphics()->QuadsEnd();
	
	//
	
	// building actions
	if (m_Tab == 1)
	{
		// mouse click
		if (m_Mouse1Loaded && m_Mouse1 && Valid && m_Minimized && m_Scale < 0.3f)
		{
			//m_Mouse1Loaded = false;
			Pos += m_pClient->m_pCamera->m_Center;
			
			if (FlipY)
			{
				if (Selected == BUILDABLE_TURRET)
					Pos.y += BuildableOffset[Selected]-18;
				else if (Selected == BUILDABLE_TESLACOIL)
					Pos.y += BuildableOffset[Selected]-38;
			}
			
			CNetMsg_Cl_UseKit Msg;
			Msg.m_Kit = m_SelectedBuilding;
			Msg.m_X = Pos.x;
			Msg.m_Y = Pos.y+18;
			Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
		}
	}
}


void CInventory::Swap(int Item1, int Item2)
{
	if (Item1 < 0 || Item2 < 0 || Item1 >= 12 || Item2 >= 12)
		return;
	
	int i1 = CustomStuff()->m_aItem[Item1];
	CustomStuff()->m_aItem[Item1] = CustomStuff()->m_aItem[Item2];
	CustomStuff()->m_aItem[Item2] = i1;
	
	CNetMsg_Cl_InventoryAction Msg;
	Msg.m_Type = INVENTORYACTION_SWAP;
	Msg.m_Slot = 0;
	Msg.m_Item1 = Item1;
	Msg.m_Item2 = Item2;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
	
	m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_INV1, 0);
}


void CInventory::TakePart(int Item1, int Slot, int Item2)
{
	if (Item1 < 0 || Item2 < 0 || Item1 >= 12 || Item2 >= 12 || Slot < 0 || Slot > 1)
		return;
	
	int w1 = CustomStuff()->m_aItem[Item1];
	int w2 = CustomStuff()->m_aItem[Item2];
	
	if (IsStaticWeapon(w1) || IsStaticWeapon(w2))
		return;
	
	int p1_1 = GetPart(w1, 0);
	int p1_2 = GetPart(w1, 1);
	int p2_1 = GetPart(w2, 0);
	int p2_2 = GetPart(w2, 1);
	
	if (Slot == 0)
	{
		CustomStuff()->m_aItem[Item1] = GetModularWeapon(p2_1, p1_2);
		CustomStuff()->m_aItem[Item2] = GetModularWeapon(p1_1, p2_2);
	}
	else if (Slot == 1)
	{
		CustomStuff()->m_aItem[Item1] = GetModularWeapon(p1_1, p2_2);
		CustomStuff()->m_aItem[Item2] = GetModularWeapon(p2_1, p1_2);
	}
	
	/*
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Weapon1 %d, %d", p1_1, p1_2);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Inventory", aBuf);
	}
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Weapon2 %d, %d", p2_1, p2_2);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Inventory", aBuf);
	}
	*/
	
	CNetMsg_Cl_InventoryAction Msg;
	Msg.m_Type = INVENTORYACTION_TAKEPART;
	Msg.m_Slot = Slot;
	Msg.m_Item1 = Item1;
	Msg.m_Item2 = Item2;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
	
	m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_INV3, 0);
}


void CInventory::Combine(int Item1, int Item2)
{
	if (Item1 < 0 || Item2 < 0 || Item1 >= 12 || Item2 >= 12)
		return;
	
	int w1 = CustomStuff()->m_aItem[Item1];
	int w2 = CustomStuff()->m_aItem[Item2];
	
	if (IsStaticWeapon(w1) || IsStaticWeapon(w2))
		return;
	
	int p1_1 = GetPart(w1, 0);
	int p1_2 = GetPart(w1, 1);
	int p2_1 = GetPart(w2, 0);
	int p2_2 = GetPart(w2, 1);
	
	if (!p1_1 || !p1_2)
	{
		if (p1_1)
		{
			int t = p2_1;
			p2_1 = p1_1;
			p1_1 = t;
		}
		else if (p1_2)
		{
			int t = p2_2;
			p2_2 = p1_2;
			p1_2 = t;
		}
	}
	else
	{
		if (!p2_1 && p1_1)
		{
			p2_1 = p1_1;
			p1_1 = 0;
		}
		
		if (!p2_2 && p1_2)
		{
			p2_2 = p1_2;
			p1_2 = 0;
		}
	}
	
	CustomStuff()->m_aItem[Item1] = GetModularWeapon(p1_1, p1_2);
	CustomStuff()->m_aItem[Item2] = GetModularWeapon(p2_1, p2_2);
	
	
	CNetMsg_Cl_InventoryAction Msg;
	Msg.m_Type = INVENTORYACTION_COMBINE;
	Msg.m_Slot = 0;
	Msg.m_Item1 = Item1;
	Msg.m_Item2 = Item2;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
	
	m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_INV2, 0);
}


void CInventory::RenderMouse()
{
	// drag drop item
	if (m_DragItem >= 0 && m_DragItem < 12)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		//Graphics()->QuadsBegin();

		int w = CustomStuff()->m_aItem[m_DragItem];
	
		if (w >= 0)
		{
			RenderTools()->SetShadersForWeapon(w);
			RenderTools()->RenderWeapon(w, m_SelectorMouse+s_DragOffset, vec2(1, 0), 10*1.5f, true);
			Graphics()->ShaderEnd();
		}
			
		//Graphics()->QuadsEnd();
	}
	else if (m_DragPart > 0)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		//Graphics()->QuadsBegin();

		int w = m_DragPart;
	
		if (w >= 0)
		{
			RenderTools()->SetShadersForWeapon(w);
			RenderTools()->RenderWeapon(w, m_SelectorMouse+s_DragOffset, vec2(1, 0), 10*1.5f, true);
			Graphics()->ShaderEnd();
		}
			
		//Graphics()->QuadsEnd();
	}
	
	// cursor
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CURSOR].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1,1,1,1);
	IGraphics::CQuadItem QuadItem(m_SelectorMouse.x, m_SelectorMouse.y, 32, 32);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();
}


void CInventory::Tick()
{
	for (int i = 0; i < 12; i++)
	{
		s_ItemOffset[i] -= s_ItemOffset[i]/8.0f;
		s_ItemEffectSelect[i] -= s_ItemEffectSelect[i] / 8.0f;
		s_ItemEffectPick[i] -= s_ItemEffectPick[i] / 8.0f;
	}
	
	s_DragOffset -= s_DragOffset / 8.0f;
}


void CInventory::MapscreenToGroup(float CenterX, float CenterY, CMapItemGroup *pGroup)
{
	float Points[4];
	RenderTools()->MapscreenToWorld(CenterX, CenterY, pGroup->m_ParallaxX/100.0f, pGroup->m_ParallaxY/100.0f,
		pGroup->m_OffsetX, pGroup->m_OffsetY, Graphics()->ScreenAspect(), 1.0f, Points);
	Graphics()->MapScreen(Points[0], Points[1], Points[2], Points[3]);
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
	/*
	m_SelectorMouse.x = clamp(m_SelectorMouse.x, 0.0f, Screen.w-16.0f);
	m_SelectorMouse.y = clamp(m_SelectorMouse.y, 0.0f, Screen.h-16.0f);
	*/
	
	m_SelectorMouse.x = clamp(m_SelectorMouse.x, 0.0f -Graphics()->ScreenWidth()/2, 0.0f + Graphics()->ScreenWidth()/2-16.0f);
	m_SelectorMouse.y = clamp(m_SelectorMouse.y, 0.0f -Graphics()->ScreenHeight()/2, Graphics()->ScreenHeight()/2-16.0f);
	
	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);
	MapscreenToGroup(0, 0, Layers()->GameGroup());
	Graphics()->BlendNormal();
	

	if (!m_Mouse1)
		m_Mouse1Loaded = true;
	
	// gui
	DrawInventory(vec2(-Screen.w/3, Screen.h/4), vec2(Screen.w/4, Screen.h/3));
	DrawBuildMode();

	if (m_Mouse1)
		m_Mouse1Loaded = false;

	if (!m_Render)
		return;

	// mouse cursor
	RenderMouse();
	
	CustomStuff()->m_Inventory = true;
}