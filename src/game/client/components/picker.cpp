#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>
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
	pSelf->m_ItemSelected = -1;
}

void CPicker::ConKeyPicker(IConsole::IResult *pResult, void *pUserData)
{
	CPicker *pSelf = (CPicker *)pUserData;
	
	if (!pSelf->m_pClient->m_pControls->m_BuildMode)
	{
		if(!pSelf->m_pClient->m_Snap.m_SpecInfo.m_Active && pSelf->Client()->State() != IClient::STATE_DEMOPLAYBACK)
			pSelf->m_Active = pResult->GetInteger(0) != 0;
		
		pSelf->m_PickerType = PICKER_WEAPON;
	}
	else
	{
		if(!pSelf->m_pClient->m_Snap.m_SpecInfo.m_Active && pSelf->Client()->State() != IClient::STATE_DEMOPLAYBACK)
			pSelf->m_Active = pResult->GetInteger(0) != 0;
		
		pSelf->m_PickerType = PICKER_TOOL;
		pSelf->m_ItemSelected = -1;
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

void CPicker::ConSwitchGroup(IConsole::IResult *pResult, void *pUserData)
{
	((CPicker *)pUserData)->SwitchGroup();
}


void CPicker::OnConsoleInit()
{
	//Console()->Register("+itempicker", "", CFGFLAG_CLIENT, ConKeyItemPicker, this, "Open item selector");
	//Console()->Register("+gamepaditempicker", "", CFGFLAG_CLIENT, ConKeyItemPicker, this, "Open item selector");
	
	Console()->Register("+switch", "", CFGFLAG_CLIENT, ConSwitchGroup, this, "Switch between weapon groups");
	Console()->Register("+gamepaddropweapon", "", CFGFLAG_CLIENT, ConDropWeapon, this, "Drop weapon");
	Console()->Register("+dropweapon", "", CFGFLAG_CLIENT, ConDropWeapon, this, "Drop weapon");
	Console()->Register("+lastweapon", "", CFGFLAG_CLIENT, ConLastWeaponpick, this, "Select last picked weapon");
	Console()->Register("+gamepadlastweapon", "", CFGFLAG_CLIENT, ConLastWeaponpick, this, "Select last picked weapon");
	//Console()->Register("+picker", "", CFGFLAG_CLIENT, ConKeyPicker, this, "Open weapon selector");
	Console()->Register("+gamepadpicker", "", CFGFLAG_CLIENT, ConKeyPicker, this, "Open weapon selector");
	Console()->Register("weaponpick", "i", CFGFLAG_CLIENT, ConWeaponpick, this, "Use weapon");
	
	Console()->Register("+gamepademote", "", CFGFLAG_CLIENT, ConKeyEmote, this, "Open emote selector");
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
	
	Input()->SetMouseModes(IInput::MOUSE_MODE_WARP_CENTER);

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
}




void CPicker::DrawKit()
{
	/*
	// reset mouse to active weapon
	if (m_ResetMouse)
	{
		//m_SelectorMouse = vec2(0, 0);
		m_ResetMouse = false;

		m_Selected = -1;
	}
	
	CUIRect Screen = *UI()->Screen();
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();

	bool Unselect = true;
	
	for (int i = 0; i < NUM_KITS; i++)
	{
		float Angle = -pi/2.0f + 2*pi*i/NUM_KITS;
		if (Angle > pi)
			Angle -= 2*pi;

		bool Selected = m_Selected == i;

		float Size = Selected ? 1.2f : 1.0f;

		float NudgeX = 135.0f * cosf(Angle);
		float NudgeY = 135.0f * sinf(Angle);
		RenderTools()->SelectSprite(SPRITE_KIT_BARREL+i);
		
		vec2 Pos = vec2(Screen.w/2 + NudgeX, Screen.h/2 + NudgeY);
		RenderTools()->DrawSprite(Pos.x, Pos.y, 70 * Size);
		
		if (distance(m_SelectorMouse+vec2(Screen.w/2, Screen.h/2), Pos) < 40 && 
		length(m_SelectorMouse) > 100.0f)
		{
			if (m_Selected != i)
				m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_UI_PICK, 0);
			
			m_Selected = i;
			Unselect = false;
		}
	}
	
	Graphics()->QuadsEnd();
	
	if (Unselect)
		m_Selected = -1;
	
	// render number of kits to mid
	vec2 KitPos = vec2(0, 60);
	
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1, 1, 1, 0.5f);
	RenderTools()->SelectSprite(SPRITE_PICKUP_KIT);
	RenderTools()->DrawSprite(Screen.w/2+KitPos.x, Screen.h/2+KitPos.y, 64);
	Graphics()->QuadsEnd();
	
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%d", clamp(CustomStuff()->m_LocalKits ,0, 9));
	TextRender()->TextColor(0.2f, 0.7f, 0.2f, 1);
	TextRender()->Text(0, Screen.w/2+KitPos.x+25, Screen.h/2+KitPos.y+10, 16, aBuf, -1);
	TextRender()->TextColor(1, 1, 1, 1);
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
				if (m_PickerType == PICKER_TOOL)
					m_pClient->m_pControls->m_SelectedBuilding = m_Selected+1;
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

	//float SelectedAngle = GetAngle(m_SelectorMouse) + 2*pi/24 +pi/2.0f;
	float SelectedAngle = GetAngle(m_SelectorMouse) + 3*pi/24 +pi/2.0f;
	if (SelectedAngle < 0)
		SelectedAngle += 2*pi;

	if (length(m_SelectorMouse) > 100.0f)
	{
		if (m_PickerType == PICKER_EMOTICON)
			m_Selected = (int)(SelectedAngle / (2*pi) * NUM_EMOTICONS);
	}
	
	// items in the middle
	else if (m_PickerType == PICKER_WEAPON)
	{
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
		case PICKER_TOOL:
			DrawKit();
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


void CPicker::UseKit(int Kit)
{
	/*
	if (Kit < 0 || Kit >= NUM_KITS)
		return;

	if (CustomStuff()->m_LocalKits < 1)
	{
		m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_UI_NEGATIVE, 0);
		return;
	}
	
	m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_UI_POSITIVE, 0);

	CNetMsg_Cl_UseKit Msg;
	Msg.m_Kit = Kit;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
	*/
}


void CPicker::Itempick(int Item)
{

}


void CPicker::LastWeaponpick()
{
	if (CustomStuff()->m_WeaponpickTimer > 0.0f)
	{
		Weaponpick(CustomStuff()->m_WeaponpickWeapon);
		CustomStuff()->m_LastWeaponPicked = true;
	}
}


	
void CPicker::Weaponpick(int Weapon)
{
	if (Weapon < 0 || Weapon >= NUM_WEAPONS)
		return;

	/*
	int w = CustomStuff()->m_LocalWeapons;
	if (!(w & (1<<(Weapon+1))))
	{
		m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_UI_NEGATIVE, 0);
		return;
	}
	*/
	
	m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_UI_POSITIVE, 0);

	m_pClient->m_pControls->m_PickedWeapon = Weapon+1;
}

void CPicker::DropWeapon()
{
	if (CustomStuff()->m_WeaponDropTick > CustomStuff()->LocalTick() - 20)
	{
		CustomStuff()->m_WeaponDropTick = CustomStuff()->LocalTick();
		return;
	}
	
	CustomStuff()->m_WeaponDropTick = CustomStuff()->LocalTick();
	
	CNetMsg_Cl_DropWeapon Msg;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
	
	m_pClient->m_pControls->m_InputData.m_WantedWeapon = 0;
}


void CPicker::SwitchGroup()
{
	//m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_UI_POSITIVE, 0);
	
	if (CustomStuff()->m_SwitchTick > CustomStuff()->LocalTick() - 20)
	{
		CustomStuff()->m_SwitchTick = CustomStuff()->LocalTick();
		return;
	}
	
	CustomStuff()->m_SwitchTick = CustomStuff()->LocalTick();
	
	if (CustomStuff()->m_SelectedGroup == 1)
		Console()->ExecuteLine("+weapon2");
	else if (CustomStuff()->m_SelectedGroup == 2)
		Console()->ExecuteLine("+weapon3");
	else if (CustomStuff()->m_SelectedGroup == 3)
		Console()->ExecuteLine("+weapon1");
	
	/*
	CNetMsg_Cl_SwitchGroup Msg;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
	*/
}


void CPicker::Emote(int Emoticon)
{
	CNetMsg_Cl_Emoticon Msg;
	Msg.m_Emoticon = Emoticon;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}
