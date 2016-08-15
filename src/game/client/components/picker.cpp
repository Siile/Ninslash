#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/gamecore.h> // get_angle
#include <game/weapons.h> // get_angle
#include <game/client/ui.h>
#include <game/client/render.h>
#include <game/client/customstuff.h>


#include <game/client/components/sounds.h>
#include "picker.h"

CPicker::CPicker()
{
	OnReset();
	m_PickerType = 0;
	m_ResetMouse = true;
}

void CPicker::ConKeyEmote(IConsole::IResult *pResult, void *pUserData)
{
	CPicker *pSelf = (CPicker *)pUserData;
	if(!pSelf->m_pClient->m_Snap.m_SpecInfo.m_Active && pSelf->Client()->State() != IClient::STATE_DEMOPLAYBACK)
		pSelf->m_Active = pResult->GetInteger(0) != 0;
	
	pSelf->m_PickerType = PICKER_EMOTICON;
}

void CPicker::ConKeyItemPicker(IConsole::IResult *pResult, void *pUserData)
{
	return;
	
	CPicker *pSelf = (CPicker *)pUserData;
	
	//if (pSelf->CustomStuff()->m_SelectedGroup < 3)
	//{
		if(!pSelf->m_pClient->m_Snap.m_SpecInfo.m_Active && pSelf->Client()->State() != IClient::STATE_DEMOPLAYBACK)
			pSelf->m_Active = pResult->GetInteger(0) != 0;
		
		pSelf->m_PickerType = PICKER_ITEM;
	//}
}

void CPicker::ConKeyPicker(IConsole::IResult *pResult, void *pUserData)
{
	CPicker *pSelf = (CPicker *)pUserData;
	
	if (pSelf->CustomStuff()->m_SelectedGroup < 3)
	{
		if(!pSelf->m_pClient->m_Snap.m_SpecInfo.m_Active && pSelf->Client()->State() != IClient::STATE_DEMOPLAYBACK)
			pSelf->m_Active = pResult->GetInteger(0) != 0;
		
		pSelf->m_PickerType = PICKER_WEAPON;
	}
}

void CPicker::ConLastWeaponpick(IConsole::IResult *pResult, void *pUserData)
{
	((CPicker *)pUserData)->LastWeaponpick();
}

void CPicker::ConWeaponpick(IConsole::IResult *pResult, void *pUserData)
{
	((CPicker *)pUserData)->Weaponpick(pResult->GetInteger(0));
}

void CPicker::ConItempick(IConsole::IResult *pResult, void *pUserData)
{
	((CPicker *)pUserData)->Itempick(pResult->GetInteger(0));
}

void CPicker::ConEmote(IConsole::IResult *pResult, void *pUserData)
{
	((CPicker *)pUserData)->Emote(pResult->GetInteger(0));
}

void CPicker::ConDropWeapon(IConsole::IResult *pResult, void *pUserData)
{
	((CPicker *)pUserData)->DropWeapon();
}


void CPicker::OnConsoleInit()
{
	//Console()->Register("+itempicker", "", CFGFLAG_CLIENT, ConKeyItemPicker, this, "Open item selector");
	//Console()->Register("+gamepaditempicker", "", CFGFLAG_CLIENT, ConKeyItemPicker, this, "Open item selector");
	
	Console()->Register("+dropweapon", "", CFGFLAG_CLIENT, ConDropWeapon, this, "Drop weapon");
	Console()->Register("+lastweapon", "", CFGFLAG_CLIENT, ConLastWeaponpick, this, "Select last picked weapon");
	Console()->Register("+picker", "", CFGFLAG_CLIENT, ConKeyPicker, this, "Open weapon selector");
	Console()->Register("+gamepadpicker", "", CFGFLAG_CLIENT, ConKeyPicker, this, "Open weapon selector");
	Console()->Register("weaponpick", "i", CFGFLAG_CLIENT, ConWeaponpick, this, "Use weapon");
	
	Console()->Register("+emote", "", CFGFLAG_CLIENT, ConKeyEmote, this, "Open emote selector");
	Console()->Register("emote", "i", CFGFLAG_CLIENT, ConEmote, this, "Use emote");
}

void CPicker::OnReset()
{
	m_WasActive = false;
	m_Active = false;
	m_Selected = -1;
	m_ItemSelected = -1;
	m_ResetMouse = true;
}

void CPicker::OnRelease()
{
	m_Active = false;
	m_ResetMouse = true;
}

void CPicker::OnMessage(int MsgType, void *pRawMsg)
{
}

bool CPicker::OnMouseMove(float x, float y)
{
	if(!m_Active)
		return false;

	Input()->GetRelativePosition(&x, &y);
	m_SelectorMouse += vec2(x,y);
	return true;
}

void CPicker::DrawCircle(float x, float y, float r, int Segments)
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


void CPicker::DrawEmoticons()
{
	CUIRect Screen = *UI()->Screen();
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_EMOTICONS].m_Id);
	Graphics()->QuadsBegin();

	for (int i = 0; i < NUM_EMOTICONS; i++)
	{
		float Angle = -pi/2.0f + 2*pi*i/NUM_EMOTICONS;
		if (Angle > pi)
			Angle -= 2*pi;

		bool Selected = m_Selected == i;

		float Size = Selected ? 80.0f : 50.0f;

		float NudgeX = 150.0f * cosf(Angle);
		float NudgeY = 150.0f * sinf(Angle);
		RenderTools()->SelectSprite(SPRITE_OOP + i);
		IGraphics::CQuadItem QuadItem(Screen.w/2 + NudgeX, Screen.h/2 + NudgeY, Size, Size);
		Graphics()->QuadsDraw(&QuadItem, 1);
	}

	Graphics()->QuadsEnd();
}


void CPicker::DrawWeapons()
{
	// reset mouse to active weapon
	if (m_ResetMouse)
	{
		float Angle = -pi/2.0f + 2*pi*(CustomStuff()->m_SelectedWeapon-1)/(NUM_WEAPONS-1);
		if (Angle > pi)
			Angle -= 2*pi;
		
		m_SelectorMouse = vec2(135.0f * cosf(Angle), 135.0f * sinf(Angle));
		m_ResetMouse = false;
		m_Selected = -1;
		m_ItemSelected = -1;
	}
	
	CUIRect Screen = *UI()->Screen();
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
	Graphics()->QuadsBegin();

	for (int i = 0; i < NUM_WEAPONS-1; i++)
	{
		int w = CustomStuff()->m_LocalWeapons;
		if (!(w & (1<<(i+1))))
			continue;
		
		float Angle = -pi/2.0f + 2*pi*i/(NUM_WEAPONS-1);
		if (Angle > pi)
			Angle -= 2*pi;

		bool Selected = m_Selected == i;

		float Size = Selected ? 1.25f : 1.0f;

		float NudgeX = 135.0f * cosf(Angle);
		float NudgeY = 135.0f * sinf(Angle);
		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[i+1].m_pSpriteBody);
		RenderTools()->DrawSprite(Screen.w/2 + NudgeX, Screen.h/2 + NudgeY, g_pData->m_Weapons.m_aId[i+1].m_VisualSize * Size);
	}

	Graphics()->QuadsEnd();
	
	// render mines
	if (CustomStuff()->m_aLocalItems[PLAYERITEM_LANDMINE] + CustomStuff()->m_aLocalItems[PLAYERITEM_ELECTROMINE] > 0)
	{		
		if (CustomStuff()->m_aLocalItems[PLAYERITEM_LANDMINE] > 0)
		{
			bool Selected = m_ItemSelected == PLAYERITEM_LANDMINE;
			
			Graphics()->TextureSet(-1);
			Graphics()->QuadsBegin();
			if (Selected)
				Graphics()->SetColor(0.2f, 1.0f, 0.2f, 0.5f);
			else
				Graphics()->SetColor(0.4f, .4f, 0.4f, 0.5f);
			DrawCircle(Screen.w/2-32, Screen.h/2, 28, 20);
			Graphics()->QuadsEnd();
			
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ITEMS].m_Id);
			Graphics()->QuadsBegin();
			Graphics()->SetColor(1,1,1,1);

			float Size = Selected ? 1.25f : 1.0f;
		
			RenderTools()->SelectSprite(SPRITE_ITEM1+PLAYERITEM_LANDMINE);
			RenderTools()->DrawSprite(Screen.w/2 - 32, Screen.h/2, 64 * Size);
			Graphics()->QuadsEnd();
			
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ITEMNUMBERS].m_Id);
			Graphics()->QuadsBegin();
			Graphics()->SetColor(0.7f, .7f, 0.7f, 1.0f);
			RenderTools()->SelectSprite(SPRITE_ITEMNUMBER_0+CustomStuff()->m_aLocalItems[PLAYERITEM_LANDMINE]);
			RenderTools()->DrawSprite(Screen.w/2 - 32+20, Screen.h/2+20, 26 * Size);
			Graphics()->QuadsEnd();
			
		}
		if (CustomStuff()->m_aLocalItems[PLAYERITEM_ELECTROMINE] > 0)
		{
			bool Selected = m_ItemSelected == PLAYERITEM_ELECTROMINE;
			
			Graphics()->TextureSet(-1);
			Graphics()->QuadsBegin();
			if (Selected)
				Graphics()->SetColor(0.2f, 1.0f, 0.2f, 0.5f);
			else
				Graphics()->SetColor(0.4f, .4f, 0.4f, 0.5f);
			DrawCircle(Screen.w/2+32, Screen.h/2, 28, 20);
			Graphics()->QuadsEnd();
			
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ITEMS].m_Id);
			Graphics()->QuadsBegin();
			Graphics()->SetColor(1,1,1,1);
			
			float Size = Selected ? 1.25f : 1.0f;
		
			RenderTools()->SelectSprite(SPRITE_ITEM1+PLAYERITEM_ELECTROMINE);
			RenderTools()->DrawSprite(Screen.w/2 + 32, Screen.h/2, 64 * Size);
			Graphics()->QuadsEnd();
			
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ITEMNUMBERS].m_Id);
			Graphics()->QuadsBegin();
			Graphics()->SetColor(0.7f, .7f, 0.7f, 1.0f);
			RenderTools()->SelectSprite(SPRITE_ITEMNUMBER_0+CustomStuff()->m_aLocalItems[PLAYERITEM_ELECTROMINE]);
			RenderTools()->DrawSprite(Screen.w/2 + 32+20, Screen.h/2+20, 26 * Size);
			Graphics()->QuadsEnd();
		}
	}
	
	/*
	if (m_Selected >= 0 && m_Selected < NUM_PLAYERITEMS)
	{
		float Size = 18;
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "(%d, %d)", int(m_SelectorMouse.x), int(m_SelectorMouse.y));
		TextRender()->Text(0, Screen.w/2-TextRender()->TextWidth(0, Size, aBuf, -1)/2, Screen.h/2-9, Size, aBuf, -1);
	}
	*/
}



void CPicker::DrawItems()
{
	// reset mouse to active weapon
	if (m_ResetMouse)
	{
		//m_SelectorMouse = vec2(0, 0);
		m_ResetMouse = false;

		m_Selected = -1;
	}
	
	CUIRect Screen = *UI()->Screen();
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ITEMS].m_Id);
	Graphics()->QuadsBegin();

	for (int i = 0; i < NUM_PLAYERITEMS; i++)
	{
		/*
		int w = CustomStuff()->m_LocalWeapons;
		if (!(w & (1<<(i+1))))
			continue;
		*/
		
		float Angle = -pi/2.0f + 2*pi*i/NUM_PLAYERITEMS;
		if (Angle > pi)
			Angle -= 2*pi;

		bool Selected = m_Selected == i;

		float Size = Selected ? 1.25f : 1.0f;

		float NudgeX = 135.0f * cosf(Angle);
		float NudgeY = 135.0f * sinf(Angle);
		RenderTools()->SelectSprite(SPRITE_ITEM1+i);
		RenderTools()->DrawSprite(Screen.w/2 + NudgeX, Screen.h/2 + NudgeY, 52 * Size);
	}
	Graphics()->QuadsEnd();
	
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ITEMNUMBERS].m_Id);
	Graphics()->QuadsBegin();
	for (int i = 0; i < NUM_PLAYERITEMS; i++)
	{
		/*
		int w = CustomStuff()->m_LocalWeapons;
		if (!(w & (1<<(i+1))))
			continue;
		*/
		int a = CustomStuff()->m_aLocalItems[i];
		if (a > 0)
			Graphics()->SetColor(0, 1, 0, 0.75f);
		else
			Graphics()->SetColor(1, 0, 0, 0.75f);
		
		float Angle = -pi/2.0f + 2*pi*i/NUM_PLAYERITEMS;
		if (Angle > pi)
			Angle -= 2*pi;

		bool Selected = m_Selected == i;

		float Size = Selected ? 1.25f : 1.0f;

		float NudgeX = 135.0f * cosf(Angle);
		float NudgeY = 135.0f * sinf(Angle);
		RenderTools()->SelectSprite(SPRITE_ITEMNUMBER_0+a);
		RenderTools()->DrawSprite(Screen.w/2 + NudgeX + 24, Screen.h/2 + NudgeY + 16, 32 * Size);
	}
	Graphics()->QuadsEnd();
	
	/*
	if (m_Selected >= 0 && m_Selected < NUM_PLAYERITEMS)
	{
		float Size = 18;
		TextRender()->Text(0, Screen.w/2-TextRender()->TextWidth(0,Size,aPlayerItemName[m_Selected], -1)/2, Screen.h/2-9, Size, aPlayerItemName[m_Selected], -1);
	}
	*/
}



void CPicker::OnRender()
{
	if (m_PickerType == PICKER_WEAPON && CustomStuff()->m_SelectedGroup > 2)
		m_Active = false;
	
	if(!m_Active)
	{
		m_ResetMouse = true;
		if(m_WasActive && (m_Selected != -1  || m_ItemSelected != -1))
		{
			if (m_ItemSelected != -1)
				Itempick(m_ItemSelected);
			else
			{
				if (m_PickerType == PICKER_WEAPON)
					Weaponpick(m_Selected);
				if (m_PickerType == PICKER_EMOTICON)
					Emote(m_Selected);
				if (m_PickerType == PICKER_ITEM)
					Itempick(m_Selected);
			}
		}
		m_WasActive = false;
		return;
	}

	if(m_pClient->m_Snap.m_SpecInfo.m_Active)
	{
		m_Active = false;
		m_WasActive = false;
		return;
	}

	m_WasActive = true;

	if (length(m_SelectorMouse) > 170.0f)
		m_SelectorMouse = normalize(m_SelectorMouse) * 170.0f;

	float SelectedAngle = GetAngle(m_SelectorMouse) + 2*pi/24 +pi/2.0f;
	if (SelectedAngle < 0)
		SelectedAngle += 2*pi;

	if (length(m_SelectorMouse) > 100.0f)
	{
		if (m_PickerType == PICKER_EMOTICON)
			m_Selected = (int)(SelectedAngle / (2*pi) * NUM_EMOTICONS);
		if (m_PickerType == PICKER_WEAPON)
		{
			m_ItemSelected = -1;
			
			int i = (int)(SelectedAngle / (2*pi) * (NUM_WEAPONS-1));
			int w = CustomStuff()->m_LocalWeapons;
			if (w & (1<<(i+1)))
			{
				if (m_Selected != i)
					m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_UI_PICK, 0);
				m_Selected = i;
			}
		}
		if (m_PickerType == PICKER_ITEM)
		{
			int i = (int)(SelectedAngle / (2*pi) * NUM_PLAYERITEMS);
			
			if (m_Selected != i)
				m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_UI_PICK, 0);
			
			m_Selected = i;
		}
	}
	// items in the middle
	else if (m_PickerType == PICKER_WEAPON)
	{
		CUIRect Screen = *UI()->Screen();
		
		m_Selected = -1;
			
		if (distance(m_SelectorMouse, vec2(-32, 0)) < 32 && CustomStuff()->m_aLocalItems[PLAYERITEM_LANDMINE] > 0)
		{
			if (m_ItemSelected != PLAYERITEM_LANDMINE)
				m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_UI_PICK, 0);
				
			m_ItemSelected = PLAYERITEM_LANDMINE;
		}
		else if (distance(m_SelectorMouse, vec2(+32, 0)) < 32 && CustomStuff()->m_aLocalItems[PLAYERITEM_ELECTROMINE] > 0)
		{
			if (m_ItemSelected != PLAYERITEM_ELECTROMINE)
				m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_UI_PICK, 0);
			
			m_ItemSelected = PLAYERITEM_ELECTROMINE;
		}
		else
			m_ItemSelected = -1;
	}

	CUIRect Screen = *UI()->Screen();

	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);

	Graphics()->BlendNormal();

	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0,0,0,0.3f);
	DrawCircle(Screen.w/2, Screen.h/2, 190.0f, 64);
	Graphics()->QuadsEnd();

	switch (m_PickerType)
	{
		case PICKER_EMOTICON:
			DrawEmoticons();
			break;
		case PICKER_WEAPON:
			DrawWeapons();
			break;
		case PICKER_ITEM:
			DrawItems();
			break;
	
		default:
			break;
	};

	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CURSOR].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1,1,1,1);
	IGraphics::CQuadItem QuadItem(m_SelectorMouse.x+Screen.w/2,m_SelectorMouse.y+Screen.h/2,24,24);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();
}


void CPicker::Itempick(int Item)
{
	if (Item < 0 || Item >= NUM_PLAYERITEMS)
		return;

	if (CustomStuff()->m_aLocalItems[Item] < 1)
	{
		m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_UI_NEGATIVE, 0);
		return;
	}
	
	m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_UI_POSITIVE, 0);

	CNetMsg_Cl_SelectItem Msg;
	Msg.m_Item = Item;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}


void CPicker::LastWeaponpick()
{
	if (CustomStuff()->m_WeaponpickTimer > 0.0f)
	{
		Weaponpick(CustomStuff()->m_WeaponpickWeapon-1);
		CustomStuff()->m_LastWeaponPicked = true;
	}
}
	
void CPicker::Weaponpick(int Weapon)
{
	int Group = CustomStuff()->m_SelectedGroup-1;
	
	if (Weapon < 0 || Weapon >= NUM_WEAPONS)
		return;
	
	if (Group < 0 || Group > 1)
		return;

	int w = CustomStuff()->m_LocalWeapons;
	if (!(w & (1<<(Weapon+1))))
	{
		m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_UI_NEGATIVE, 0);
		return;
	}
	
	m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_UI_POSITIVE, 0);
	
	CNetMsg_Cl_SelectWeapon Msg;
	Msg.m_Weapon = Weapon+1;
	Msg.m_Group = Group;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}

void CPicker::DropWeapon()
{
	//m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_UI_POSITIVE, 0);
	
	if (CustomStuff()->m_WeaponDropTick > CustomStuff()->LocalTick() - 30)
	{
		CustomStuff()->m_WeaponDropTick = CustomStuff()->LocalTick();
		return;
	}
	
	CustomStuff()->m_WeaponDropTick = CustomStuff()->LocalTick();
	
	CNetMsg_Cl_DropWeapon Msg;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}


void CPicker::Emote(int Emoticon)
{
	CNetMsg_Cl_Emoticon Msg;
	Msg.m_Emoticon = Emoticon;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}