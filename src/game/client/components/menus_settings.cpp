


#include <base/math.h>
#include <cstring>

#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/storage.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <engine/shared/linereader.h>

#include <generated/protocol.h>
#include <generated/game_data.h>

#include <game/client/components/sounds.h>
#include <game/client/ui.h>
#include <game/client/render.h>
#include <game/client/gameclient.h>
#include <game/localization.h>

#include "binds.h"
#include "countryflags.h"
#include "menus.h"
#include "skins.h"

CMenusKeyBinder CMenus::m_Binder;

CMenusKeyBinder::CMenusKeyBinder()
{
	m_TakeKey = false;
	m_GotKey = false;
}

bool CMenusKeyBinder::OnInput(IInput::CEvent Event)
{
	if(m_TakeKey)
	{
		if(Event.m_Flags&IInput::FLAG_PRESS)
		{
			m_Key = Event;
			m_GotKey = true;
			m_TakeKey = false;
		}
		return true;
	}

	return false;
}

void CMenus::RenderSettingsGeneral(CUIRect MainView)
{
	char aBuf[128];
	CUIRect Label, Button, Left, Right, Game, Client;
	MainView.HSplitTop(150.0f, &Game, &Client);

	// game
	{
		// headline
		Game.HSplitTop(30.0f, &Label, &Game);
		UI()->DoLabelScaled(&Label, Localize("Game"), 20.0f, -1);
		Game.Margin(5.0f, &Game);
		Game.VSplitMid(&Left, &Right);
		Left.VSplitRight(5.0f, &Left, 0);
		Right.VMargin(5.0f, &Right);

		// dynamic camera
		Left.HSplitTop(20.0f, &Button, &Left);
		static int s_DynamicCameraButton = 0;
		if(DoButton_CheckBox(&s_DynamicCameraButton, Localize("Dynamic Camera"), g_Config.m_ClMouseDeadzone != 0, &Button))
		{
			if(g_Config.m_ClMouseDeadzone)
			{
				g_Config.m_ClMouseFollowfactor = 0;
				g_Config.m_ClMouseMaxDistance = 400;
				g_Config.m_ClMouseDeadzone = 0;
			}
			else
			{
				g_Config.m_ClMouseFollowfactor = 60;
				g_Config.m_ClMouseMaxDistance = 1000;
				g_Config.m_ClMouseDeadzone = 300;
			}
		}

		// weapon pickup
		Left.HSplitTop(5.0f, 0, &Left);
		Left.HSplitTop(20.0f, &Button, &Left);
		if(DoButton_CheckBox(&g_Config.m_ClAutoswitchWeapons, Localize("Switch weapon on pickup"), g_Config.m_ClAutoswitchWeapons, &Button))
			g_Config.m_ClAutoswitchWeapons ^= 1;

		// show hud
		Left.HSplitTop(5.0f, 0, &Left);
		Left.HSplitTop(20.0f, &Button, &Left);
		if(DoButton_CheckBox(&g_Config.m_ClShowhud, Localize("Show ingame HUD"), g_Config.m_ClShowhud, &Button))
			g_Config.m_ClShowhud ^= 1;

		// chat messages
		Left.HSplitTop(5.0f, 0, &Left);
		Left.HSplitTop(20.0f, &Button, &Left);
		if(DoButton_CheckBox(&g_Config.m_ClShowChatFriends, Localize("Show only chat messages from friends"), g_Config.m_ClShowChatFriends, &Button))
			g_Config.m_ClShowChatFriends ^= 1;

		// name plates
		Right.HSplitTop(20.0f, &Button, &Right);
		if(DoButton_CheckBox(&g_Config.m_ClNameplates, Localize("Show name plates"), g_Config.m_ClNameplates, &Button))
			g_Config.m_ClNameplates ^= 1;

		if(g_Config.m_ClNameplates)
		{
			Right.HSplitTop(2.5f, 0, &Right);
			Right.VSplitLeft(30.0f, 0, &Right);
			Right.HSplitTop(20.0f, &Button, &Right);
			if(DoButton_CheckBox(&g_Config.m_ClNameplatesAlways, Localize("Always show name plates"), g_Config.m_ClNameplatesAlways, &Button))
				g_Config.m_ClNameplatesAlways ^= 1;

			Right.HSplitTop(2.5f, 0, &Right);
			Right.HSplitTop(20.0f, &Label, &Right);
			Right.HSplitTop(20.0f, &Button, &Right);
			str_format(aBuf, sizeof(aBuf), "%s: %i", Localize("Name plates size"), g_Config.m_ClNameplatesSize);
			UI()->DoLabelScaled(&Label, aBuf, 13.0f, -1);
			Button.HMargin(2.0f, &Button);
			g_Config.m_ClNameplatesSize = (int)(DoScrollbarH(&g_Config.m_ClNameplatesSize, &Button, g_Config.m_ClNameplatesSize/100.0f)*100.0f+0.1f);

			Right.HSplitTop(20.0f, &Button, &Right);
			if(DoButton_CheckBox(&g_Config.m_ClNameplatesTeamcolors, Localize("Use team colors for name plates"), g_Config.m_ClNameplatesTeamcolors, &Button))
				g_Config.m_ClNameplatesTeamcolors ^= 1;
		}
	}

	// client
	/*
	{
		// headline
		Client.HSplitTop(30.0f, &Label, &Client);
		UI()->DoLabelScaled(&Label, Localize("Client"), 20.0f, -1);
		Client.Margin(5.0f, &Client);
		Client.VSplitMid(&Left, &Right);
		Left.VSplitRight(5.0f, &Left, 0);
		Right.VMargin(5.0f, &Right);

		// auto demo settings
		{
			Left.HSplitTop(20.0f, &Button, &Left);
			if(DoButton_CheckBox(&g_Config.m_ClAutoDemoRecord, Localize("Automatically record demos"), g_Config.m_ClAutoDemoRecord, &Button))
				g_Config.m_ClAutoDemoRecord ^= 1;

			Right.HSplitTop(20.0f, &Button, &Right);
			if(DoButton_CheckBox(&g_Config.m_ClAutoScreenshot, Localize("Automatically take game over screenshot"), g_Config.m_ClAutoScreenshot, &Button))
				g_Config.m_ClAutoScreenshot ^= 1;

			Left.HSplitTop(10.0f, 0, &Left);
			Left.VSplitLeft(20.0f, 0, &Left);
			Left.HSplitTop(20.0f, &Label, &Left);
			Button.VSplitRight(20.0f, &Button, 0);
			char aBuf[64];
			if(g_Config.m_ClAutoDemoMax)
				str_format(aBuf, sizeof(aBuf), "%s: %i", Localize("Max demos"), g_Config.m_ClAutoDemoMax);
			else
				str_format(aBuf, sizeof(aBuf), "%s: %s", Localize("Max demos"), Localize("no limit"));
			UI()->DoLabelScaled(&Label, aBuf, 13.0f, -1);
			Left.HSplitTop(20.0f, &Button, 0);
			Button.HMargin(2.0f, &Button);
			g_Config.m_ClAutoDemoMax = static_cast<int>(DoScrollbarH(&g_Config.m_ClAutoDemoMax, &Button, g_Config.m_ClAutoDemoMax/1000.0f)*1000.0f+0.1f);

			Right.HSplitTop(10.0f, 0, &Right);
			Right.VSplitLeft(20.0f, 0, &Right);
			Right.HSplitTop(20.0f, &Label, &Right);
			Button.VSplitRight(20.0f, &Button, 0);
			if(g_Config.m_ClAutoScreenshotMax)
				str_format(aBuf, sizeof(aBuf), "%s: %i", Localize("Max Screenshots"), g_Config.m_ClAutoScreenshotMax);
			else
				str_format(aBuf, sizeof(aBuf), "%s: %s", Localize("Max Screenshots"), Localize("no limit"));
			UI()->DoLabelScaled(&Label, aBuf, 13.0f, -1);
			Right.HSplitTop(20.0f, &Button, 0);
			Button.HMargin(2.0f, &Button);
			g_Config.m_ClAutoScreenshotMax = static_cast<int>(DoScrollbarH(&g_Config.m_ClAutoScreenshotMax, &Button, g_Config.m_ClAutoScreenshotMax/1000.0f)*1000.0f+0.1f);
		}
	}
	*/
}

void CMenus::RenderSettingsPlayer(CUIRect MainView)
{
	CUIRect Button, Label;
	MainView.HSplitTop(10.0f, 0, &MainView);

	// player name
	MainView.HSplitTop(20.0f, &Button, &MainView);
	Button.VSplitLeft(80.0f, &Label, &Button);
	Button.VSplitLeft(150.0f, &Button, 0);
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s:", Localize("Name"));
	UI()->DoLabelScaled(&Label, aBuf, 14.0, -1);
	static float s_OffsetName = 0.0f;
	if(DoEditBox(g_Config.m_PlayerName, &Button, g_Config.m_PlayerName, sizeof(g_Config.m_PlayerName), 14.0f, &s_OffsetName))
		m_NeedSendinfo = true;

	// player clan
	MainView.HSplitTop(5.0f, 0, &MainView);
	MainView.HSplitTop(20.0f, &Button, &MainView);
	Button.VSplitLeft(80.0f, &Label, &Button);
	Button.VSplitLeft(150.0f, &Button, 0);
	str_format(aBuf, sizeof(aBuf), "%s:", Localize("Clan"));
	UI()->DoLabelScaled(&Label, aBuf, 14.0, -1);
	static float s_OffsetClan = 0.0f;
	if(DoEditBox(g_Config.m_PlayerClan, &Button, g_Config.m_PlayerClan, sizeof(g_Config.m_PlayerClan), 14.0f, &s_OffsetClan))
		m_NeedSendinfo = true;

	// country flag selector
	MainView.HSplitTop(20.0f, 0, &MainView);
	static float s_ScrollValue = 0.0f;
	int OldSelected = -1;
	UiDoListboxStart(&s_ScrollValue, &MainView, 50.0f, Localize("Country"), "", m_pClient->m_pCountryFlags->Num(), 6, OldSelected, s_ScrollValue);

	for(int i = 0; i < m_pClient->m_pCountryFlags->Num(); ++i)
	{
		const CCountryFlags::CCountryFlag *pEntry = m_pClient->m_pCountryFlags->GetByIndex(i);
		if(pEntry->m_CountryCode == g_Config.m_PlayerCountry)
			OldSelected = i;

		CListboxItem Item = UiDoListboxNextItem(&pEntry->m_CountryCode, OldSelected == i);
		if(Item.m_Visible)
		{
			CUIRect Label;
			Item.m_Rect.Margin(5.0f, &Item.m_Rect);
			Item.m_Rect.HSplitBottom(10.0f, &Item.m_Rect, &Label);
			float OldWidth = Item.m_Rect.w;
			Item.m_Rect.w = Item.m_Rect.h*2;
			Item.m_Rect.x += (OldWidth-Item.m_Rect.w)/ 2.0f;
			vec4 Color(1.0f, 1.0f, 1.0f, 1.0f);
			m_pClient->m_pCountryFlags->Render(pEntry->m_CountryCode, &Color, Item.m_Rect.x, Item.m_Rect.y, Item.m_Rect.w, Item.m_Rect.h);
			if(pEntry->m_Texture != -1)
				UI()->DoLabel(&Label, pEntry->m_aCountryCodeString, 10.0f, 0);
		}
	}

	const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
	if(OldSelected != NewSelected)
	{
		g_Config.m_PlayerCountry = m_pClient->m_pCountryFlags->GetByIndex(NewSelected)->m_CountryCode;
		m_NeedSendinfo = true;
	}
}

void CMenus::RenderCustomization(CUIRect MainView)
{
	
	// back to menu button
	/*
	CUIRect BackButton;
	MainView.HSplitTop(30, &BackButton, &MainView);
	
	BackButton.VSplitLeft(300, NULL, &BackButton);
	BackButton.VSplitRight(300, &BackButton, NULL);
	
	static int s_FrontPageButton=0;
	if(DoButton_Menu(&s_FrontPageButton, Localize("Back to main menu"), 0, &BackButton))
		g_Config.m_UiPage = PAGE_FRONT;
	
	*/
	MainView.HSplitTop(20.0f, 0, &MainView);
	
	
	CUIRect LeftView;
	MainView.VSplitMid(&LeftView, &MainView);
	MainView.VSplitRight(20, &MainView, NULL);
	LeftView.VSplitLeft(20, NULL, &LeftView);
	
	
	// color select
	static int s_CustomizationColor = 0;
	const char *aColor[] = {Localize("Body"), Localize("Feet"), Localize("Skin"), Localize("Hair / hat")};
	int NumColors = (int)(sizeof(aColor)/sizeof(*aColor));
	
	CUIRect ColorRect, L;
	LeftView.HSplitTop(0.0f, &L, 0);
	
	UI()->DoLabelScaled(&L, Localize("Change color of"), 14.0f, -1);
	
	LeftView.HSplitTop(20.0f, 0, &ColorRect);
	ColorRect.HSplitTop(20.0f, &ColorRect, 0);
	
	for(int i = 0; i < NumColors; i++)
	{
		CUIRect Button;
		ColorRect.VSplitLeft(110.0f, &Button, &ColorRect);
		if(DoButton_MenuTab(&aColor[i], aColor[i], s_CustomizationColor == i, &Button, CUI::CORNER_BR))
			s_CustomizationColor = i;
	}
	
	
	CUIRect Slider;
	CUIRect Button, Label;
	
	
	LeftView.HSplitTop(5.0f, 0, &LeftView);
	LeftView.HSplitTop(82.5f, &Label, &LeftView);


	int *pColors;
	if (s_CustomizationColor == 0)
		pColors = &g_Config.m_PlayerColorBody;
	else if (s_CustomizationColor == 1)
		pColors = &g_Config.m_PlayerColorFeet;
	else if (s_CustomizationColor == 2)
		pColors = &g_Config.m_PlayerColorSkin;
	else if (s_CustomizationColor == 3)
		pColors = &g_Config.m_PlayerColorTopper;
	else
		pColors = &g_Config.m_PlayerColorBody;
	
	const char *paLabels[] = {
		Localize("Hue"),
		Localize("Sat."),
		Localize("Lht.")};
	static int s_aColorSlider[3] = {0};

	LeftView.HSplitTop(20.0f, 0, &Slider);
	LeftView.VSplitRight(150, &Slider, 0);

	int PrevColor = *pColors;
	
	// color sliders
	int Color = 0;
	for(int s = 0; s < 3; s++)
	{
		Slider.HSplitTop(20.0f, &Label, &Slider);
		Label.VSplitLeft(100.0f, &Label, &Button);
		Button.HMargin(2.0f, &Button);

		float k = ((PrevColor>>((2-s)*8))&0xff) / 255.0f;
		k = DoScrollbarH(&s_aColorSlider[s], &Button, k);
		Color <<= 8;
		Color += clamp((int)(k*255), 0, 255);
		UI()->DoLabelScaled(&Label, paLabels[s], 14.0f, -1);
	}

	if(PrevColor != Color)
		m_NeedSendinfo = true;

	*pColors = Color;
			
	
	LeftView.HSplitTop(5.0f, 0, &LeftView);
	LeftView.HSplitTop(82.5f, &Label, &LeftView);
		
	// blood color select
	const char *aBlood[] = {Localize("Red"), Localize("Green"), Localize("Black")};
	int NumBloods = (int)(sizeof(aBlood)/sizeof(*aBlood));
	
	CUIRect BloodRect, B;
	LeftView.HSplitTop(0.0f, &B, 0);
	
	UI()->DoLabelScaled(&B, Localize("Blood color"), 14.0f, -1);
	
	LeftView.HSplitTop(20.0f, 0, &BloodRect);
	BloodRect.HSplitTop(20.0f, &BloodRect, 0);
	
	for(int i = 0; i < NumBloods; i++)
	{
		CUIRect Button;
		BloodRect.VSplitLeft(110.0f, &Button, &BloodRect);
		if(DoButton_MenuTab(&aBlood[i], aBlood[i], g_Config.m_PlayerBloodColor == i, &Button, CUI::CORNER_ALL))
		{
			g_Config.m_PlayerBloodColor = i;
		}
	}
	
	
	
	LeftView.HSplitTop(5.0f, 0, &LeftView);
	LeftView.HSplitTop(82.5f, &Label, &LeftView);
	
	
	// render player
	{
		CTeeRenderInfo Info;
		Info.m_ColorBody = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorBody);
		Info.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorFeet);
		Info.m_Body = 0; //g_Config.m_PlayerBody;
		Info.m_TopperTexture = m_pClient->m_pSkins->GetTopper(m_pClient->m_pSkins->FindTopper(g_Config.m_PlayerTopper))->m_Texture;
		Info.m_EyeTexture = m_pClient->m_pSkins->GetEye(m_pClient->m_pSkins->FindEye(g_Config.m_PlayerEye))->m_Texture;
		Info.m_HeadTexture = m_pClient->m_pSkins->GetHead(m_pClient->m_pSkins->FindHead(g_Config.m_PlayerHead))->m_Texture;
		Info.m_BodyTexture = m_pClient->m_pSkins->GetBody(m_pClient->m_pSkins->FindBody(g_Config.m_PlayerBody))->m_Texture;
		Info.m_HandTexture = m_pClient->m_pSkins->GetHand(m_pClient->m_pSkins->FindHand(g_Config.m_PlayerHand))->m_Texture;
		Info.m_FootTexture = m_pClient->m_pSkins->GetFoot(m_pClient->m_pSkins->FindFoot(g_Config.m_PlayerFoot))->m_Texture;
		Info.m_ColorTopper = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorTopper);
		Info.m_ColorSkin = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorSkin);
		Info.m_Size = UI()->Scale()*60.0f;
		
		RenderTools()->RenderStaticPlayer(&Info, vec2(LeftView.x+LeftView.w*0.825f, LeftView.y-LeftView.h*0.1f));
	}
	
	
	// skin type select
	static int s_SkinType = 0;
	const char *aSkinType[] = {Localize("Head"), Localize("Hair / hat"), Localize("Eyes"), Localize("Body"), Localize("Hands"), Localize("Feet")};
	int NumSkinTypes = (int)(sizeof(aSkinType)/sizeof(*aSkinType));
	

	CUIRect SkinTypeLabel, SkinSelect;
	MainView.HSplitTop(0.0f, &SkinTypeLabel, 0);
	
	UI()->DoLabelScaled(&SkinTypeLabel, Localize("Change skin of"), 14.0f, -1);
	
		
	// saving skins, helper for creating bot skins
	if(Input()->KeyDown(KEY_S) && (Input()->KeyPressed(KEY_LCTRL) || Input()->KeyPressed(KEY_RCTRL)))
	{
		SaveSkin();
	}
	
	MainView.HSplitTop(20.0f, 0, &SkinSelect);
	SkinSelect.HSplitTop(20.0f, &SkinSelect, 0);
	
	for(int i = 0; i < NumSkinTypes; i++)
	{
		CUIRect Button;
		SkinSelect.VSplitLeft(80.0f, &Button, &SkinSelect);
		if(DoButton_MenuTab(&aSkinType[i], aSkinType[i], s_SkinType == i, &Button, CUI::CORNER_BR))
			s_SkinType = i;
	}

	
	MainView.HSplitTop(5.0f, 0, &MainView);
	MainView.HSplitTop(20.5f, &Label, &MainView);

	// eye selector
	if (s_SkinType == 2)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumEyes(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetEye(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Eyes"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerEye) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_EyeTexture = s->m_Texture;
				Info.m_Size = UI()->Scale()*50.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderEye(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerEye, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerEye));
			m_NeedSendinfo = true;
		}
	}
	
	// topper selector
	if (s_SkinType == 1)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumToppers(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetTopper(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Hair / hat"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerTopper) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_TopperTexture = s->m_Texture;
				Info.m_ColorTopper = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorTopper);
				Info.m_Size = UI()->Scale()*80.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderTopper(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerTopper, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerTopper));
			m_NeedSendinfo = true;
		}
	}
	
	// head selector
	if (s_SkinType == 0)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumHeads(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetHead(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Head"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerHead) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_HeadTexture = s->m_Texture;
				//Info.m_ColorHead = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorSkin);
				Info.m_Size = UI()->Scale()*80.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderHead(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerHead, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerHead));
			m_NeedSendinfo = true;
		}
	}
	
	// hand selector
	if (s_SkinType == 4)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumHands(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetHand(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Hands"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerHand) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_HandTexture = s->m_Texture;
				//Info.m_ColorHead = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorSkin);
				Info.m_Size = UI()->Scale()*80.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderHand(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerHand, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerHand));
			m_NeedSendinfo = true;
		}
	}
	
	// foot selector
	if (s_SkinType == 5)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumFeet(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetFoot(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Feet"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerFoot) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_FootTexture = s->m_Texture;
				//Info.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorSkin);
				Info.m_Size = UI()->Scale()*80.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderFoot(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerFoot, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerFoot));
			m_NeedSendinfo = true;
		}
	}
	
	
	// body selector
	if (s_SkinType == 3)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumBodies(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetBody(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Body"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerBody) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_BodyTexture = s->m_Texture;
				//Info.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorSkin);
				Info.m_Size = UI()->Scale()*80.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderBody(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerBody, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerBody));
			m_NeedSendinfo = true;
		}
	}
	
	return;
	
	
	/*
	
	MainView.HSplitTop(10.0f, 0, &MainView);
	

	// color select
	static int s_CustomizationColor = 0;
	const char *aColor[] = {Localize("Body"), Localize("Feet"), Localize("Skin"), Localize("Hair / hat")};
	int NumColors = (int)(sizeof(aColor)/sizeof(*aColor));
	
	CUIRect ColorRect, L;
	MainView.HSplitTop(0.0f, &L, 0);
	
	UI()->DoLabelScaled(&L, Localize("Change color of"), 14.0f, -1);
	
	MainView.HSplitTop(20.0f, 0, &ColorRect);
	ColorRect.HSplitTop(20.0f, &ColorRect, 0);
	
	for(int i = 0; i < NumColors; i++)
	{
		CUIRect Button;
		ColorRect.VSplitLeft(80.0f, &Button, &ColorRect);
		if(DoButton_MenuTab(&aColor[i], aColor[i], s_CustomizationColor == i, &Button, CUI::CORNER_BR))
			s_CustomizationColor = i;
	}
	
	
	CUIRect Slider;
	CUIRect Button, Label;
	
	
	MainView.HSplitTop(5.0f, 0, &MainView);
	MainView.HSplitTop(82.5f, &Label, &MainView);


	int *pColors;
	if (s_CustomizationColor == 0)
		pColors = &g_Config.m_PlayerColorBody;
	else if (s_CustomizationColor == 1)
		pColors = &g_Config.m_PlayerColorFeet;
	else if (s_CustomizationColor == 2)
		pColors = &g_Config.m_PlayerColorSkin;
	else if (s_CustomizationColor == 3)
		pColors = &g_Config.m_PlayerColorTopper;
	else
		pColors = &g_Config.m_PlayerColorBody;
	
	const char *paLabels[] = {
		Localize("Hue"),
		Localize("Sat."),
		Localize("Lht.")};
	static int s_aColorSlider[3] = {0};

	MainView.HSplitTop(20.0f, 0, &Slider);
	MainView.VSplitMid(&Slider, 0);

	int PrevColor = *pColors;
	
	// color sliders
	int Color = 0;
	for(int s = 0; s < 3; s++)
	{
		Slider.HSplitTop(20.0f, &Label, &Slider);
		Label.VSplitLeft(100.0f, &Label, &Button);
		Button.HMargin(2.0f, &Button);

		float k = ((PrevColor>>((2-s)*8))&0xff) / 255.0f;
		k = DoScrollbarH(&s_aColorSlider[s], &Button, k);
		Color <<= 8;
		Color += clamp((int)(k*255), 0, 255);
		UI()->DoLabelScaled(&Label, paLabels[s], 14.0f, -1);
	}

	if(PrevColor != Color)
		m_NeedSendinfo = true;

	*pColors = Color;
			
	
	MainView.HSplitTop(5.0f, 0, &MainView);
	MainView.HSplitTop(82.5f, &Label, &MainView);
	
	
	// render player
	{
		CTeeRenderInfo Info;
		Info.m_ColorBody = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorBody);
		Info.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorFeet);
		Info.m_TopperTexture = m_pClient->m_pSkins->GetTopper(m_pClient->m_pSkins->FindTopper(g_Config.m_PlayerTopper))->m_Texture;
		Info.m_EyeTexture = m_pClient->m_pSkins->GetEye(m_pClient->m_pSkins->FindEye(g_Config.m_PlayerEye))->m_Texture;
		Info.m_HeadTexture = m_pClient->m_pSkins->GetHead(m_pClient->m_pSkins->FindHead(g_Config.m_PlayerHead))->m_Texture;
		Info.m_BodyTexture = m_pClient->m_pSkins->GetBody(m_pClient->m_pSkins->FindBody(g_Config.m_PlayerBody))->m_Texture;
		Info.m_HandTexture = m_pClient->m_pSkins->GetHand(m_pClient->m_pSkins->FindHand(g_Config.m_PlayerHand))->m_Texture;
		Info.m_FootTexture = m_pClient->m_pSkins->GetFoot(m_pClient->m_pSkins->FindFoot(g_Config.m_PlayerFoot))->m_Texture;
		Info.m_ColorTopper = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorTopper);
		Info.m_ColorSkin = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorSkin);
		Info.m_Size = UI()->Scale()*50.0f;
		
		RenderTools()->RenderStaticPlayer(&Info, vec2(600, 200));
	}
	
	
	// skin type select
	static int s_SkinType = 0;
	const char *aSkinType[] = {Localize("Head"), Localize("Hair / hat"), Localize("Eyes"), Localize("Body"), Localize("Hands"), Localize("Feet")};
	int NumSkinTypes = (int)(sizeof(aSkinType)/sizeof(*aSkinType));
	

	CUIRect SkinTypeLabel, SkinSelect;
	MainView.HSplitTop(0.0f, &SkinTypeLabel, 0);
	
	UI()->DoLabelScaled(&SkinTypeLabel, Localize("Change skin of"), 14.0f, -1);

	// saving skins, helper for creating bot skins
	if(Input()->KeyDown(KEY_S) && (Input()->KeyPressed(KEY_LCTRL) || Input()->KeyPressed(KEY_RCTRL)))
	{
		SaveSkin();
	}
	
	MainView.HSplitTop(20.0f, 0, &SkinSelect);
	SkinSelect.HSplitTop(20.0f, &SkinSelect, 0);
	
	for(int i = 0; i < NumSkinTypes; i++)
	{
		CUIRect Button;
		SkinSelect.VSplitLeft(80.0f, &Button, &SkinSelect);
		if(DoButton_MenuTab(&aSkinType[i], aSkinType[i], s_SkinType == i, &Button, CUI::CORNER_BR))
			s_SkinType = i;
	}

	
	MainView.HSplitTop(5.0f, 0, &MainView);
	MainView.HSplitTop(20.5f, &Label, &MainView);

	// eye selector
	if (s_SkinType == 1)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumEyes(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetEye(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Eyes"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerEye) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_EyeTexture = s->m_Texture;
				Info.m_Size = UI()->Scale()*50.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderEye(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerEye, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerEye));
			m_NeedSendinfo = true;
		}
	}
	
	// topper selector
	if (s_SkinType == 2)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumToppers(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetTopper(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Hair / hat"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerTopper) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_TopperTexture = s->m_Texture;
				Info.m_ColorTopper = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorTopper);
				Info.m_Size = UI()->Scale()*80.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderTopper(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerTopper, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerTopper));
			m_NeedSendinfo = true;
		}
	}
	
		// Head selector
	if (s_SkinType == 0)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumHeads(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetHead(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Heads"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerHead) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_HeadTexture = s->m_Texture;
				Info.m_Size = UI()->Scale()*50.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderHead(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerHead, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerHead));
			m_NeedSendinfo = true;
		}
	}
	
		// Body selector
	if (s_SkinType == 3)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumBodies(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetBody(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Bodies"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerBody) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_BodyTexture = s->m_Texture;
				Info.m_Size = UI()->Scale()*50.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderBody(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerBody, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerBody));
			m_NeedSendinfo = true;
		}
	}
	
		// Hand selector
	if (s_SkinType == 4)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumHands(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetHand(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Hands"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerHand) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_HandTexture = s->m_Texture;
				Info.m_Size = UI()->Scale()*50.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderHand(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerHand, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerHand));
			m_NeedSendinfo = true;
		}
	}
	
		// Foot selector
	if (s_SkinType == 5)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumFeet(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetFoot(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Feet"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerFoot) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_FootTexture = s->m_Texture;
				Info.m_Size = UI()->Scale()*50.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderFoot(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerFoot, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerFoot));
			m_NeedSendinfo = true;
		}
	}

	// body selector
	if (s_SkinType == 0)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static float s_ScrollValue = 0.0f;
		static bool s_InitSkinlist = false;

		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Body"), "", NUM_BODIES, 4, OldSelected, s_ScrollValue);

		const int s[NUM_BODIES] = {0, 1, 2, 3, 4, 5};
		
		for (int i = 0; i < NUM_BODIES; i++)
		{
			//if (i == 0)
			//	continue;
			
			if (i == g_Config.m_PlayerBody)
				OldSelected = i;
			
			CListboxItem Item = UiDoListboxNextItem(&s[i], OldSelected == i);
			if (Item.m_Visible)
			{
				Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BODIES].m_Id);
				Graphics()->QuadsBegin();
				Graphics()->QuadsSetRotation(0);
				Graphics()->SetColor(1, 1, 1, 1);

				RenderTools()->SelectSprite(SPRITE_BODY1+i);
				IGraphics::CQuadItem QuadItem(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2, UI()->Scale()*40.0f, UI()->Scale()*40.0f);
				Graphics()->QuadsDraw(&QuadItem, 1);
				
				Graphics()->QuadsEnd();
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			//mem_copy(g_Config.m_PlayerTopper, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerTopper));
			g_Config.m_PlayerBody = NewSelected;
			m_NeedSendinfo = true;
			
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "new selected: '%d'", NewSelected);
			Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "skin", aBuf);
		}
	}
	*/
	
	
	
	// skin selector
	/*
	MainView.HSplitTop(20.0f, 0, &MainView);
	static bool s_InitSkinlist = true;
	static sorted_array<const CSkins::CSkin *> s_paSkinList;
	static float s_ScrollValue = 0.0f;
	if(s_InitSkinlist)
	{
		s_paSkinList.clear();
		for(int i = 0; i < m_pClient->m_pSkins->Num(); ++i)
		{
			const CSkins::CSkin *s = m_pClient->m_pSkins->Get(i);
			// no special skins
			if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
				continue;
			s_paSkinList.add(s);
		}
		s_InitSkinlist = false;
	}

	//MainView.HSplitTop(82.5f, &Label, &MainView);
	
	int OldSelected = -1;
	UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Skins"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

	for(int i = 0; i < s_paSkinList.size(); ++i)
	{
		const CSkins::CSkin *s = s_paSkinList[i];
		if(s == 0)
			continue;

		if(str_comp(s->m_aName, g_Config.m_PlayerSkin) == 0)
			OldSelected = i;

		CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
		if(Item.m_Visible)
		{
			CTeeRenderInfo Info;
			if(g_Config.m_PlayerUseCustomColor)
			{
				Info.m_Texture = s->m_ColorTexture;
				Info.m_ColorBody = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorBody);
				Info.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorFeet);
			}
			else
			{
				Info.m_Texture = s->m_OrgTexture;
				Info.m_ColorBody = vec4(1.0f, 1.0f, 1.0f, 1.0f);
				Info.m_ColorFeet = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			}

			Info.m_Size = UI()->Scale()*50.0f;
			Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
			//RenderTools()->RenderTee(CAnimState::GetIdle(), &Info, 0, vec2(1.0f, 0.0f), vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));

			if(g_Config.m_Debug)
			{
				vec3 BloodColor = g_Config.m_PlayerUseCustomColor ? m_pClient->m_pSkins->GetColorV3(g_Config.m_PlayerColorBody) : s->m_BloodColor;
				Graphics()->TextureSet(-1);
				Graphics()->QuadsBegin();
				Graphics()->SetColor(BloodColor.r, BloodColor.g, BloodColor.b, 1.0f);
				IGraphics::CQuadItem QuadItem(Item.m_Rect.x, Item.m_Rect.y, 12.0f, 12.0f);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
		}
	}

	const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
	if(OldSelected != NewSelected)
	{
		mem_copy(g_Config.m_PlayerSkin, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerSkin));
		m_NeedSendinfo = true;
	}
	*/
}

void CMenus::SaveSkin()
{
	Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "skin", "Saving skin");
	
	char aPath[512];
	//str_format(aPath, sizeof(aPath), "%s%s", "server/pvp_bots", "test.npc");
	str_format(aPath, sizeof(aPath), "%s%s%s", "data/server/pvp_bots/", g_Config.m_PlayerName, ".npc");
	
	
	/*
	# example npc profile
	name: Tigrrr
	head: default
	body: simple
	hand: default
	foot: default
	topper: tigger
	eye: sleepy

	# write /color in in-game chat to see your current colors
	skin-color: 2293760
	topper-color: 1835007
	body-color: 2345472
	foot-color: 13238016

	# red, black or green
	blood-color: red
	*/
	
	IOHANDLE file = io_open(aPath, IOFLAG_WRITE);
	
	char buf[1024];
	str_format(buf, sizeof(buf), "name: %s", g_Config.m_PlayerName);
	io_write(file, buf, strlen(buf)); io_write_newline(file);
	
	if (g_Config.m_PlayerBloodColor == 0){
		str_format(buf, sizeof(buf), "blood-color: %s", "red");	io_write(file, buf, strlen(buf)); io_write_newline(file); io_write_newline(file);}
	if (g_Config.m_PlayerBloodColor == 1){
		str_format(buf, sizeof(buf), "blood-color: %s", "green");	io_write(file, buf, strlen(buf)); io_write_newline(file); io_write_newline(file);}
	if (g_Config.m_PlayerBloodColor == 2){
		str_format(buf, sizeof(buf), "blood-color: %s", "black");	io_write(file, buf, strlen(buf)); io_write_newline(file); io_write_newline(file);}
	
	str_format(buf, sizeof(buf), "head: %s", g_Config.m_PlayerHead);
	io_write(file, buf, strlen(buf)); io_write_newline(file);
	str_format(buf, sizeof(buf), "body: %s", g_Config.m_PlayerBody);
	io_write(file, buf, strlen(buf)); io_write_newline(file);
	str_format(buf, sizeof(buf), "hand: %s", g_Config.m_PlayerHand);
	io_write(file, buf, strlen(buf)); io_write_newline(file);
	str_format(buf, sizeof(buf), "foot: %s", g_Config.m_PlayerFoot);
	io_write(file, buf, strlen(buf)); io_write_newline(file);
	str_format(buf, sizeof(buf), "topper: %s", g_Config.m_PlayerTopper);
	io_write(file, buf, strlen(buf)); io_write_newline(file);
	str_format(buf, sizeof(buf), "eye: %s", g_Config.m_PlayerEye);
	io_write(file, buf, strlen(buf)); io_write_newline(file); io_write_newline(file);
	
	
	str_format(buf, sizeof(buf), "skin-color: %u", g_Config.m_PlayerColorSkin);
	io_write(file, buf, strlen(buf)); io_write_newline(file);
	str_format(buf, sizeof(buf), "topper-color: %u", g_Config.m_PlayerColorTopper);
	io_write(file, buf, strlen(buf)); io_write_newline(file);
	str_format(buf, sizeof(buf), "body-color: %u", g_Config.m_PlayerColorBody);
	io_write(file, buf, strlen(buf)); io_write_newline(file);
	str_format(buf, sizeof(buf), "foot-color: %u", g_Config.m_PlayerColorFeet);
	io_write(file, buf, strlen(buf)); io_write_newline(file);
	
	
	io_close(file);
	
	Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "skin", "Skin saved");
}


typedef void (*pfnAssignFuncCallback)(CConfiguration *pConfig, int Value);

typedef struct
{
	CLocConstString m_Name;
	const char *m_pCommand;
	int m_KeyId;
} CKeyInfo;

enum ControlSettings
{
	START_MOVEMENT = 0,
	START_WEAPONS = 6,
	START_VOTING = 16,
	START_CHAT = 18,
	START_MISC = 21,
	END_CONTROL = 29,
};

static CKeyInfo gs_aKeys[] =
{
	// movement
	{ "Move left", "+left", 0},		// Localize - these strings are localized within CLocConstString
	{ "Move right", "+right", 0 },
	{ "Slide / down", "+down", 0 },
	{ "Jump", "+jump", 0 },
	{ "Fire", "+fire", 0 },
	//{ "Charge", "+charge", 0 },
	{ "Hook", "+turbo", 0 },
	//{ "Build", "+build", 0 },
	
	// weapons
	//{ "Tool", "+weapon1", 0 },
	{ "Item slot 1", "+weapon2", 0 },
	{ "Item slot 2", "+weapon3", 0 },
	{ "Item slot 3", "+weapon4", 0 },
	{ "Item slot 4", "+weapon5", 0 },
	
	{ "Inventory", "+inventory", 0 },
	{ "Build menu", "+buildmenu", 0 },
	
	//{ "Fast weapon change", "+lastweapon", 0 },
	//{ "Switch weapon", "+switch", 0 },
	{ "Next weapon", "+nextweapon", 0 },
	{ "Prev. weapon", "+prevweapon", 0 },
	{ "Drop weapon", "+dropweapon", 0 },
	{ "Inventory roll", "+inventoryroll", 0 },
	
	// voting
	{ "Vote yes", "vote yes", 0 },
	{ "Vote no", "vote no", 0 },
	
	// chat
	{ "Chat", "chat all", 0 },
	{ "Team chat", "chat team", 0 },
	{ "Show chat", "+show_chat", 0 },
	
	// misc
	//{ "Item picker", "+itempicker", 0 },
	{ "Emoticon", "+emote", 0 },
	{ "Spectator mode", "+spectate", 0 },
	{ "Spectate next", "spectate_next", 0 },
	{ "Spectate previous", "spectate_previous", 0 },
	{ "Console", "toggle_local_console", 0 },
	{ "Remote console", "toggle_remote_console", 0 },
	{ "Screenshot", "screenshot", 0 },
	{ "Scoreboard", "+scoreboard", 0 },
};

/*	This is for scripts/update_localization.py to work, don't remove!
	Localize("Move left");Localize("Move right");Localize("Jump");Localize("Fire");Localize("Hook");Localize("Hammer");
	Localize("Pistol");Localize("Shotgun");Localize("Grenade");Localize("Rifle");Localize("Next weapon");Localize("Prev. weapon");
	Localize("Vote yes");Localize("Vote no");Localize("Chat");Localize("Team chat");Localize("Show chat");Localize("Emoticon");
	Localize("Spectator mode");Localize("Spectate next");Localize("Spectate previous");Localize("Console");Localize("Remote console");
	Localize("Screenshot");Localize("Scoreboard");Localize("Respawn");Localize("Slide / down");Localize("Drop weapon");
	Localize("Item slot 1");Localize("Item slot 2");Localize("Item slot 3");Localize("Item slot 4");Localize("Inventory");Localize("Inventory roll");
	Localize("Build menu");
*/

const int g_KeyCount = sizeof(gs_aKeys) / sizeof(CKeyInfo);

void CMenus::UiDoGetButtons(int Start, int Stop, CUIRect View)
{
	for (int i = Start; i < Stop; i++)
	{
		CKeyInfo &Key = gs_aKeys[i];
		CUIRect Button, Label;
		View.HSplitTop(20.0f, &Button, &View);
		Button.VSplitLeft(135.0f, &Label, &Button);

		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "%s:", (const char *)Key.m_Name);

		UI()->DoLabelScaled(&Label, aBuf, 13.0f, -1);
		int OldId = Key.m_KeyId;
		int NewId = DoKeyReader((void *)&gs_aKeys[i].m_Name, &Button, OldId);
		if(NewId != OldId)
		{
			if(OldId != 0 || NewId == 0)
				m_pClient->m_pBinds->Bind(OldId, "");
			if(NewId != 0)
				m_pClient->m_pBinds->Bind(NewId, gs_aKeys[i].m_pCommand);
		}
		View.HSplitTop(5.0f, 0, &View);
	}
}

void CMenus::RenderSettingsControls(CUIRect MainView)
{
	// this is kinda slow, but whatever
	for(int i = 0; i < g_KeyCount; i++)
		gs_aKeys[i].m_KeyId = 0;

	for(int KeyId = 0; KeyId < KEY_LAST; KeyId++)
	{
		const char *pBind = m_pClient->m_pBinds->Get(KeyId);
		if(!pBind[0])
			continue;

		for(int i = 0; i < g_KeyCount; i++)
			if(str_comp(pBind, gs_aKeys[i].m_pCommand) == 0)
			{
				gs_aKeys[i].m_KeyId = KeyId;
				break;
			}
	}
	
	
	//MainView.HSplitTop(10.0f, 0, &MainView);
	

	// color select
	static int s_ControlMenu = 0;
	
	CUIRect Tab1, Tab2;

	
	MainView.HSplitTop(25.0f, 0, &Tab1);
	Tab1.HSplitTop(25.0f, &Tab1, 0);
	
	Tab2 = Tab1;
	
	{
		CUIRect Button;
		//TabRect.VSplitMid(&Button, &TabRect);
		Tab1.VSplitMid(&Tab1, &Button);
		Tab1.VSplitLeft(200.0f, &Tab1, &Button);
		
		static int s_MovementButton=0;
		if(DoButton_MenuTab(&s_MovementButton, Localize("Movement & Weapons"), s_ControlMenu == 0, &Button, CUI::CORNER_L))
			s_ControlMenu = 0;
	}

	
	{
		CUIRect Button;
		Tab2.VSplitMid(&Button, &Tab2);
		Tab2.VSplitRight(200.0f, &Button, &Tab2);
		
		static int s_ChatButton=0;
		if(DoButton_MenuTab(&s_ChatButton, Localize("Chat & Misc."), s_ControlMenu == 1, &Button, CUI::CORNER_R))
			s_ControlMenu = 1;
	}
	
	

	CUIRect MovementSettings, WeaponSettings, VotingSettings, ChatSettings, MiscSettings, ResetButton;
	
	MainView.HSplitBottom(50.0f, &MainView, &ResetButton);
	
	
	// movement & weapons
	if (s_ControlMenu == 0)
	{
		MainView.HSplitTop(80.0f, 0, &MovementSettings);
		MainView.HSplitTop(80.0f, 0, &WeaponSettings);
		
		MovementSettings.VSplitMid(&MovementSettings, 0);
		WeaponSettings.VSplitMid(0, &WeaponSettings);
		
		// movement settings
		{
			MovementSettings.VMargin(5.0f, &MovementSettings);
			//MovementSettings.HSplitTop(MainView.h/3+60.0f, &MovementSettings, &WeaponSettings);
			RenderTools()->DrawUIRect(&MovementSettings, vec4(1,1,1,0.25f), CUI::CORNER_ALL, 10.0f);
			MovementSettings.Margin(10.0f, &MovementSettings);

			TextRender()->Text(0, MovementSettings.x, MovementSettings.y, 14.0f*UI()->Scale(), Localize("Movement"), -1);

			MovementSettings.HSplitTop(14.0f+5.0f+10.0f, 0, &MovementSettings);

			{
				CUIRect Button, Label;
				MovementSettings.HSplitTop(20.0f, &Button, &MovementSettings);
				Button.VSplitLeft(135.0f, &Label, &Button);
				UI()->DoLabel(&Label, Localize("Mouse sens."), 14.0f*UI()->Scale(), -1);
				Button.HMargin(2.0f, &Button);
				g_Config.m_InpMousesens = (int)(DoScrollbarH(&g_Config.m_InpMousesens, &Button, (g_Config.m_InpMousesens-5)/500.0f)*500.0f)+5;
				//*key.key = ui_do_key_reader(key.key, &Button, *key.key);
				MovementSettings.HSplitTop(20.0f, 0, &MovementSettings);
			}

			UiDoGetButtons(START_MOVEMENT, START_WEAPONS, MovementSettings);

		}

		// weapon settings
		{
			WeaponSettings.VMargin(5.0f, &WeaponSettings);
			
			RenderTools()->DrawUIRect(&WeaponSettings, vec4(1,1,1,0.25f), CUI::CORNER_ALL, 10.0f);
			WeaponSettings.Margin(10.0f, &WeaponSettings);

			TextRender()->Text(0, WeaponSettings.x, WeaponSettings.y, 14.0f*UI()->Scale(), Localize("Weapons & Items"), -1);

			WeaponSettings.HSplitTop(14.0f+5.0f+10.0f, 0, &WeaponSettings);
			UiDoGetButtons(START_WEAPONS, START_VOTING, WeaponSettings);
		}
	}
	else
	
	// chat & misc
	if (s_ControlMenu == 1)
	{
		MainView.HSplitTop(80.0f, 0, &VotingSettings);
		MainView.HSplitTop(80.0f, 0, &MiscSettings);
		
		VotingSettings.VSplitMid(&VotingSettings, 0);
		MiscSettings.VSplitMid(0, &MiscSettings);
		
		// voting settings
		{
			VotingSettings.VMargin(5.0f, &VotingSettings);
			VotingSettings.HSplitTop(MainView.h/3-50.0f, &VotingSettings, &ChatSettings);
			RenderTools()->DrawUIRect(&VotingSettings, vec4(1,1,1,0.25f), CUI::CORNER_ALL, 10.0f);
			VotingSettings.Margin(10.0f, &VotingSettings);

			TextRender()->Text(0, VotingSettings.x, VotingSettings.y, 14.0f*UI()->Scale(), Localize("Voting"), -1);

			VotingSettings.HSplitTop(14.0f+5.0f+10.0f, 0, &VotingSettings);
			UiDoGetButtons(START_VOTING, START_CHAT, VotingSettings);
		}

		// chat settings
		{
			ChatSettings.HSplitTop(10.0f, 0, &ChatSettings);
			ChatSettings.HSplitTop(MainView.h/3, &ChatSettings, 0);
			RenderTools()->DrawUIRect(&ChatSettings, vec4(1,1,1,0.25f), CUI::CORNER_ALL, 10.0f);
			ChatSettings.Margin(10.0f, &ChatSettings);

			TextRender()->Text(0, ChatSettings.x, ChatSettings.y, 14.0f*UI()->Scale(), Localize("Chat"), -1);

			ChatSettings.HSplitTop(14.0f+5.0f+10.0f, 0, &ChatSettings);
			UiDoGetButtons(START_CHAT, START_MISC, ChatSettings);
		}

		// misc settings
		{
			MiscSettings.VMargin(5.0f, &MiscSettings);
			//MiscSettings.HSplitTop(10.0f, 0, &MiscSettings);
			RenderTools()->DrawUIRect(&MiscSettings, vec4(1,1,1,0.25f), CUI::CORNER_ALL, 10.0f);
			MiscSettings.Margin(10.0f, &MiscSettings);

			TextRender()->Text(0, MiscSettings.x, MiscSettings.y, 14.0f*UI()->Scale(), Localize("Miscellaneous"), -1);

			MiscSettings.HSplitTop(14.0f+5.0f+10.0f, 0, &MiscSettings);
			UiDoGetButtons(START_MISC, END_CONTROL, MiscSettings);
		}
	}
	

	// reset to defaults
	{
		ResetButton.VSplitLeft(250.0f, 0, &ResetButton);
		ResetButton.VSplitRight(250.0f, &ResetButton, 0);
		
		ResetButton.HSplitTop(10.0f, 0, &ResetButton);
		RenderTools()->DrawUIRect(&ResetButton, vec4(1,1,1,0.25f), CUI::CORNER_ALL, 10.0f);
		ResetButton.HMargin(10.0f, &ResetButton);
		ResetButton.VMargin(30.0f, &ResetButton);
		ResetButton.HSplitTop(20.0f, &ResetButton, 0);
		static int s_DefaultButton = 0;
		if(DoButton_Menu((void*)&s_DefaultButton, Localize("Reset to defaults"), 0, &ResetButton))
			m_pClient->m_pBinds->SetDefaults();
	}
}

void CMenus::RenderSettingsGraphics(CUIRect MainView)
{
	CUIRect Button;
	char aBuf[128];
	bool CheckSettings = false;

	static const int MAX_RESOLUTIONS = 256;
	static CVideoMode s_aModes[MAX_RESOLUTIONS];
	static int s_NumNodes = Graphics()->GetVideoModes(s_aModes, MAX_RESOLUTIONS, g_Config.m_GfxScreen);
	static int s_GfxScreenWidth = g_Config.m_GfxScreenWidth;
	static int s_GfxScreenHeight = g_Config.m_GfxScreenHeight;
	static int s_GfxColorDepth = g_Config.m_GfxColorDepth;
	static int s_GfxBorderless = g_Config.m_GfxBorderless;
	static int s_GfxFullscreen = g_Config.m_GfxFullscreen;
	static int s_GfxVsync = g_Config.m_GfxVsync;
	static int s_GfxFsaaSamples = g_Config.m_GfxFsaaSamples;
	static int s_GfxTextureQuality = g_Config.m_GfxTextureQuality;
	static int s_GfxTextureCompression = g_Config.m_GfxTextureCompression;
	static int s_GfxThreaded = g_Config.m_GfxThreaded;

	CUIRect ModeList;
	MainView.VSplitLeft(300.0f, &MainView, &ModeList);

	// draw allmodes switch
	ModeList.HSplitTop(20, &Button, &ModeList);
	if(DoButton_CheckBox(&g_Config.m_GfxDisplayAllModes, Localize("Show only supported"), g_Config.m_GfxDisplayAllModes^1, &Button))
	{
		g_Config.m_GfxDisplayAllModes ^= 1;
		s_NumNodes = Graphics()->GetVideoModes(s_aModes, MAX_RESOLUTIONS, g_Config.m_GfxScreen);
	}

	// display mode list
	static float s_ScrollValue = 0;
	int OldSelected = -1;
	int G = gcd(s_GfxScreenWidth, s_GfxScreenHeight);
	str_format(aBuf, sizeof(aBuf), "%s: %dx%d %d bit (%d:%d)", Localize("Current"), s_GfxScreenWidth, s_GfxScreenHeight, s_GfxColorDepth, s_GfxScreenWidth/G, s_GfxScreenHeight/G);
	UiDoListboxStart(&s_NumNodes , &ModeList, 24.0f, Localize("Display Modes"), aBuf, s_NumNodes, 1, OldSelected, s_ScrollValue);

	for(int i = 0; i < s_NumNodes; ++i)
	{
		const int Depth = s_aModes[i].m_Red+s_aModes[i].m_Green+s_aModes[i].m_Blue > 16 ? 24 : 16;
		if(g_Config.m_GfxColorDepth == Depth &&
			g_Config.m_GfxScreenWidth == s_aModes[i].m_Width &&
			g_Config.m_GfxScreenHeight == s_aModes[i].m_Height)
		{
			OldSelected = i;
		}

		CListboxItem Item = UiDoListboxNextItem(&s_aModes[i], OldSelected == i);
		if(Item.m_Visible)
		{
			int G = gcd(s_aModes[i].m_Width, s_aModes[i].m_Height);
			str_format(aBuf, sizeof(aBuf), " %dx%d %d bit (%d:%d)", s_aModes[i].m_Width, s_aModes[i].m_Height, Depth, s_aModes[i].m_Width/G, s_aModes[i].m_Height/G);
			UI()->DoLabelScaled(&Item.m_Rect, aBuf, 16.0f, -1);
		}
	}

	const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
	if(OldSelected != NewSelected)
	{
		const int Depth = s_aModes[NewSelected].m_Red+s_aModes[NewSelected].m_Green+s_aModes[NewSelected].m_Blue > 16 ? 24 : 16;
		g_Config.m_GfxColorDepth = Depth;
		g_Config.m_GfxScreenWidth = s_aModes[NewSelected].m_Width;
		g_Config.m_GfxScreenHeight = s_aModes[NewSelected].m_Height;
		CheckSettings = true;
	}

	// switches
	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GfxBorderless, Localize("Borderless window"), g_Config.m_GfxBorderless, &Button))
	{
		g_Config.m_GfxBorderless ^= 1;
		if(g_Config.m_GfxBorderless && g_Config.m_GfxFullscreen)
			g_Config.m_GfxFullscreen = 0;
		CheckSettings = true;
	}

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GfxFullscreen, Localize("Fullscreen"), g_Config.m_GfxFullscreen, &Button))
	{
		g_Config.m_GfxFullscreen ^= 1;
		if(g_Config.m_GfxFullscreen && g_Config.m_GfxBorderless)
			g_Config.m_GfxBorderless = 0;
		CheckSettings = true;
	}

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GfxVsync, Localize("V-Sync"), g_Config.m_GfxVsync, &Button))
	{
		g_Config.m_GfxVsync ^= 1;
		CheckSettings = true;
	}

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox_Number(&g_Config.m_GfxFsaaSamples, Localize("FSAA samples"), g_Config.m_GfxFsaaSamples, &Button))
	{
		g_Config.m_GfxFsaaSamples = (g_Config.m_GfxFsaaSamples+1)%17;
		CheckSettings = true;
	}
	
	/*
	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GfxThreaded, Localize("Threaded rendering"), g_Config.m_GfxThreaded, &Button))
	{
		g_Config.m_GfxThreaded ^= 1;
		CheckSettings = true;
	}
	*/

	MainView.HSplitTop(20.0f, &Button, &MainView);
	//if(g_Config.m_GfxThreaded)
	{
		Button.VSplitLeft(20.0f, 0, &Button);
		if(DoButton_CheckBox(&g_Config.m_GfxAsyncRender, Localize("Handle rendering async from updates"), g_Config.m_GfxAsyncRender, &Button))
		{
			g_Config.m_GfxAsyncRender ^= 1;
			CheckSettings = true;
		}
	}
		
	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GfxTextureQuality, Localize("Quality Textures"), g_Config.m_GfxTextureQuality, &Button))
	{
		g_Config.m_GfxTextureQuality ^= 1;
		CheckSettings = true;
	}

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GfxTextureCompression, Localize("Texture Compression"), g_Config.m_GfxTextureCompression, &Button))
	{
		g_Config.m_GfxTextureCompression ^= 1;
		CheckSettings = true;
	}

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GfxHighDetail, Localize("High Detail"), g_Config.m_GfxHighDetail, &Button))
		g_Config.m_GfxHighDetail ^= 1;
	
	MainView.HSplitTop(20.0f, &Button, &MainView);
	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GfxMultiBuffering, Localize("Multi framebuffering (requires restart)"), g_Config.m_GfxMultiBuffering, &Button))
	{
		g_Config.m_GfxMultiBuffering ^= 1;
		if (!g_Config.m_GfxMultiBuffering)
			g_Config.m_ClLighting = 0;
		else
			g_Config.m_ClLighting = 1;
	}
	
	MainView.HSplitTop(20.0f, &Button, &MainView);
	Button.VSplitLeft(20.0f, 0, &Button);
	if(DoButton_CheckBox(&g_Config.m_ClLighting, Localize("Dynamic lighting"), g_Config.m_ClLighting, &Button))
		g_Config.m_ClLighting ^= 1;

	// check if the new settings require a restart
	if(CheckSettings)
	{
		if(s_GfxScreenWidth == g_Config.m_GfxScreenWidth &&
			s_GfxScreenHeight == g_Config.m_GfxScreenHeight &&
			s_GfxColorDepth == g_Config.m_GfxColorDepth &&
			s_GfxBorderless == g_Config.m_GfxBorderless &&
			s_GfxFullscreen == g_Config.m_GfxFullscreen &&
			s_GfxVsync == g_Config.m_GfxVsync &&
			s_GfxFsaaSamples == g_Config.m_GfxFsaaSamples &&
			s_GfxTextureQuality == g_Config.m_GfxTextureQuality &&
			s_GfxTextureCompression == g_Config.m_GfxTextureCompression &&
			s_GfxThreaded == g_Config.m_GfxThreaded)
			m_NeedRestartGraphics = false;
		else
			m_NeedRestartGraphics = true;
	}

	MainView.HSplitTop(20.0f, &Button, &MainView);

	// blood slider
	{
		CUIRect Button, Label;
		MainView.HSplitTop(5.0f, &Button, &MainView);
		MainView.HSplitTop(20.0f, &Button, &MainView);
		Button.VSplitLeft(190.0f, &Label, &Button);
		Button.HMargin(2.0f, &Button);
		UI()->DoLabelScaled(&Label, Localize("Blood amount"), 14.0f, -1);
		MainView.HSplitTop(20.0f, &Button, &MainView);
		g_Config.m_GoreBlood = (int)(DoScrollbarH(&g_Config.m_GoreBlood, &Button, g_Config.m_GoreBlood/100.0f)*100.0f);
		MainView.HSplitTop(20.0f, 0, &MainView);
	}


	//

	CUIRect Text;
	MainView.HSplitTop(20.0f, 0, &MainView);
	MainView.HSplitTop(20.0f, &Text, &MainView);
	//text.VSplitLeft(15.0f, 0, &text);
	
	/*
	UI()->DoLabelScaled(&Text, Localize("UI Color"), 14.0f, -1);

	const char *paLabels[] = {
		Localize("Hue"),
		Localize("Sat."),
		Localize("Lht."),
		Localize("Alpha")};
	int *pColorSlider[4] = {&g_Config.m_UiColorHue, &g_Config.m_UiColorSat, &g_Config.m_UiColorLht, &g_Config.m_UiColorAlpha};
	for(int s = 0; s < 4; s++)
	{
		CUIRect Text;
		MainView.HSplitTop(19.0f, &Button, &MainView);
		Button.VMargin(15.0f, &Button);
		Button.VSplitLeft(100.0f, &Text, &Button);
		//Button.VSplitRight(5.0f, &Button, 0);
		Button.HSplitTop(4.0f, 0, &Button);

		float k = (*pColorSlider[s]) / 255.0f;
		k = DoScrollbarH(pColorSlider[s], &Button, k);
		*pColorSlider[s] = (int)(k*255.0f);
		UI()->DoLabelScaled(&Text, paLabels[s], 15.0f, -1);
	}
	*/
}

void CMenus::RenderSettingsSound(CUIRect MainView)
{
	CUIRect Button;
	MainView.VSplitMid(&MainView, 0);
	static int s_SndEnable = g_Config.m_SndEnable;
	static int s_SndRate = g_Config.m_SndRate;

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_SndEnable, Localize("Use sounds"), g_Config.m_SndEnable, &Button))
	{
		g_Config.m_SndEnable ^= 1;
		if(g_Config.m_SndEnable)
		{
			if(g_Config.m_SndMusic)
				m_pClient->m_pSounds->Play(CSounds::CHN_MUSIC, SOUND_MENU, 1.0f);
		}
		else
			m_pClient->m_pSounds->Stop(SOUND_MENU);
		m_NeedRestartSound = g_Config.m_SndEnable && (!s_SndEnable || s_SndRate != g_Config.m_SndRate);
	}

	if(!g_Config.m_SndEnable)
		return;

	MainView.HSplitTop(20.0f, &Button, &MainView);
	/*if(DoButton_CheckBox(&g_Config.m_SndEnvironmental, Localize("Environmental sounds"), g_Config.m_SndEnvironmental, &Button))
		g_Config.m_SndEnvironmental ^= 1;*/
	
	MainView.HSplitTop(20.0f, &Button, &MainView);
	/*if(DoButton_CheckBox(&g_Config.m_SndMusic, Localize("Play background music"), g_Config.m_SndMusic, &Button))
	{
		g_Config.m_SndMusic ^= 1;
		if(Client()->State() == IClient::STATE_OFFLINE)
		{
			if(g_Config.m_SndMusic)
				m_pClient->m_pSounds->Play(CSounds::CHN_MUSIC, SOUND_MENU, 1.0f);
			else
				m_pClient->m_pSounds->Stop(SOUND_MENU);
		}
	}
	*/

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_SndNonactiveMute, Localize("Mute when not active"), g_Config.m_SndNonactiveMute, &Button))
		g_Config.m_SndNonactiveMute ^= 1;

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_ClThreadsoundloading, Localize("Threaded sound loading"), g_Config.m_ClThreadsoundloading, &Button))
		g_Config.m_ClThreadsoundloading ^= 1;

	// sample rate box
	{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "%d", g_Config.m_SndRate);
		MainView.HSplitTop(20.0f, &Button, &MainView);
		UI()->DoLabelScaled(&Button, Localize("Sample rate"), 14.0f, -1);
		Button.VSplitLeft(190.0f, 0, &Button);
		static float Offset = 0.0f;
		DoEditBox(&g_Config.m_SndRate, &Button, aBuf, sizeof(aBuf), 14.0f, &Offset);
		g_Config.m_SndRate = max(1, str_toint(aBuf));
		m_NeedRestartSound = !s_SndEnable || s_SndRate != g_Config.m_SndRate;
	}

	// volume slider
	{
		CUIRect Button, Label;
		MainView.HSplitTop(5.0f, &Button, &MainView);
		MainView.HSplitTop(20.0f, &Button, &MainView);
		Button.VSplitLeft(190.0f, &Label, &Button);
		Button.HMargin(2.0f, &Button);
		UI()->DoLabelScaled(&Label, Localize("Sound volume"), 14.0f, -1);
		g_Config.m_SndVolume = (int)(DoScrollbarH(&g_Config.m_SndVolume, &Button, g_Config.m_SndVolume/100.0f)*100.0f);
		MainView.HSplitTop(20.0f, 0, &MainView);
	}
}




// custom menu for the client
void CMenus::RenderSettingsGamepad(CUIRect MainView)
{
	/*
	CUIRect Button;
	MainView.VSplitMid(&MainView, 0);
	
	
	MainView.HSplitTop(24.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GoreGamepad, Localize("xbox 360 controller support"), g_Config.m_GoreGamepad, &Button))
		g_Config.m_GoreGamepad ^= 1;
	
	MainView.HSplitTop(24.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GoreGamepadFlipMove, Localize("Flip move & aim axis"), g_Config.m_GoreGamepadFlipMove, &Button))
		g_Config.m_GoreGamepadFlipMove ^= 1;
	
	MainView.HSplitTop(24.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GoreGamepadFlipAttack, Localize("Flip attack & hook"), g_Config.m_GoreGamepadFlipAttack, &Button))
		g_Config.m_GoreGamepadFlipAttack ^= 1;
	
	// gamepad sensitivity slider
	{
		CUIRect Button, Label;
		MainView.HSplitTop(25.0f, &Button, &MainView);
		MainView.HSplitTop(24.0f, &Button, &MainView);
		Button.VSplitLeft(190.0f, &Label, &Button);
		Button.HMargin(2.0f, &Button);
		UI()->DoLabelScaled(&Label, "Gamepad analog stick sensitivity", 14.0f, -1);
		MainView.HSplitTop(24.0f, &Button, &MainView);
		g_Config.m_GoreGamepadSensitivity = (int)(DoScrollbarH(&g_Config.m_GoreGamepadSensitivity, &Button, g_Config.m_GoreGamepadSensitivity/100.0f)*100.0f);

		MainView.HSplitTop(20.0f, 0, &MainView);
	}
	*/
}


// custom menu for the client
void CMenus::RenderSettingsCustom(CUIRect MainView)
{
}







class CLanguage
{
public:
	CLanguage() {}
	CLanguage(const char *n, const char *f, int Code) : m_Name(n), m_FileName(f), m_CountryCode(Code) {}

	string m_Name;
	string m_FileName;
	int m_CountryCode;

	bool operator<(const CLanguage &Other) { return m_Name < Other.m_Name; }
};

void LoadLanguageIndexfile(IStorage *pStorage, IConsole *pConsole, sorted_array<CLanguage> *pLanguages)
{
	IOHANDLE File = pStorage->OpenFile("languages/index.txt", IOFLAG_READ, IStorage::TYPE_ALL);
	if(!File)
	{
		pConsole->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "localization", "couldn't open index file");
		return;
	}

	char aOrigin[128];
	char aReplacement[128];
	CLineReader LineReader;
	LineReader.Init(File);
	char *pLine;
	while((pLine = LineReader.Get()))
	{
		if(!str_length(pLine) || pLine[0] == '#') // skip empty lines and comments
			continue;

		str_copy(aOrigin, pLine, sizeof(aOrigin));

		pLine = LineReader.Get();
		if(!pLine)
		{
			pConsole->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "localization", "unexpected end of index file");
			break;
		}

		if(pLine[0] != '=' || pLine[1] != '=' || pLine[2] != ' ')
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "malform replacement for index '%s'", aOrigin);
			pConsole->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "localization", aBuf);
			(void)LineReader.Get();
			continue;
		}
		str_copy(aReplacement, pLine+3, sizeof(aReplacement));

		pLine = LineReader.Get();
		if(!pLine)
		{
			pConsole->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "localization", "unexpected end of index file");
			break;
		}

		if(pLine[0] != '=' || pLine[1] != '=' || pLine[2] != ' ')
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "malform replacement for index '%s'", aOrigin);
			pConsole->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "localization", aBuf);
			continue;
		}

		char aFileName[128];
		str_format(aFileName, sizeof(aFileName), "languages/%s.txt", aOrigin);
		pLanguages->add(CLanguage(aReplacement, aFileName, str_toint(pLine+3)));
	}
	io_close(File);
}

void CMenus::RenderLanguageSelection(CUIRect MainView)
{
	static int s_LanguageList = 0;
	static int s_SelectedLanguage = 0;
	static sorted_array<CLanguage> s_Languages;
	static float s_ScrollValue = 0;

	if(s_Languages.size() == 0)
	{
		s_Languages.add(CLanguage("English", "", 826));
		LoadLanguageIndexfile(Storage(), Console(), &s_Languages);
		for(int i = 0; i < s_Languages.size(); i++)
			if(str_comp(s_Languages[i].m_FileName, g_Config.m_ClLanguagefile) == 0)
			{
				s_SelectedLanguage = i;
				break;
			}
	}

	int OldSelected = s_SelectedLanguage;

	UiDoListboxStart(&s_LanguageList , &MainView, 24.0f, Localize("Language"), "", s_Languages.size(), 1, s_SelectedLanguage, s_ScrollValue);

	for(sorted_array<CLanguage>::range r = s_Languages.all(); !r.empty(); r.pop_front())
	{
		CListboxItem Item = UiDoListboxNextItem(&r.front());

		if(Item.m_Visible)
		{
			CUIRect Rect;
			Item.m_Rect.VSplitLeft(Item.m_Rect.h*2.0f, &Rect, &Item.m_Rect);
			Rect.VMargin(6.0f, &Rect);
			Rect.HMargin(3.0f, &Rect);
			vec4 Color(1.0f, 1.0f, 1.0f, 1.0f);
			m_pClient->m_pCountryFlags->Render(r.front().m_CountryCode, &Color, Rect.x, Rect.y, Rect.w, Rect.h);
			Item.m_Rect.HSplitTop(2.0f, 0, &Item.m_Rect);
 			UI()->DoLabelScaled(&Item.m_Rect, r.front().m_Name, 16.0f, -1);
		}
	}

	s_SelectedLanguage = UiDoListboxEnd(&s_ScrollValue, 0);

	if(OldSelected != s_SelectedLanguage)
	{
		str_copy(g_Config.m_ClLanguagefile, s_Languages[s_SelectedLanguage].m_FileName, sizeof(g_Config.m_ClLanguagefile));
		g_Localization.Load(s_Languages[s_SelectedLanguage].m_FileName, Storage(), Console());
	}
}


	
void CMenus::RenderCustomize(CUIRect MainView)
{
	/*
	CUIRect Temp, TabBar, RestartWarning;
	MainView.HSplitBottom(15.0f, &MainView, &RestartWarning);
	MainView.VSplitRight(120.0f, &MainView, &TabBar);
	RenderTools()->DrawUIRect(&MainView, ms_ColorTabbarActive, CUI::CORNER_B|CUI::CORNER_TL, 10.0f);
	TabBar.HSplitTop(50.0f, &Temp, &TabBar);
	RenderTools()->DrawUIRect(&Temp, ms_ColorTabbarActive, CUI::CORNER_R, 10.0f);

	MainView.HSplitTop(10.0f, 0, &MainView);
	*/
	
	//MainView.HSplitTop(10.0f, 0, &MainView);
	

	// back to menu button
	CUIRect BackButton;
	MainView.HSplitTop(30, &BackButton, &MainView);
	
	BackButton.VSplitLeft(300, NULL, &BackButton);
	BackButton.VSplitRight(300, &BackButton, NULL);
	
	static int s_FrontPageButton=0;
	if(DoButton_Menu(&s_FrontPageButton, Localize("Back to main menu"), 0, &BackButton))
		g_Config.m_UiPage = PAGE_FRONT;
	
	
	MainView.HSplitTop(20.0f, 0, &MainView);
	
	
	CUIRect LeftView;
	MainView.VSplitMid(&LeftView, &MainView);
	MainView.VSplitRight(20, &MainView, NULL);
	LeftView.VSplitLeft(20, NULL, &LeftView);
	
	
	// color select
	static int s_CustomizationColor = 0;
	const char *aColor[] = {Localize("Body"), Localize("Feet"), Localize("Skin"), Localize("Hair / hat")};
	int NumColors = (int)(sizeof(aColor)/sizeof(*aColor));
	
	CUIRect ColorRect, L;
	LeftView.HSplitTop(0.0f, &L, 0);
	
	UI()->DoLabelScaled(&L, Localize("Change color of"), 14.0f, -1);
	
	LeftView.HSplitTop(20.0f, 0, &ColorRect);
	ColorRect.HSplitTop(20.0f, &ColorRect, 0);
	
	for(int i = 0; i < NumColors; i++)
	{
		CUIRect Button;
		ColorRect.VSplitLeft(110.0f, &Button, &ColorRect);
		if(DoButton_MenuTab(&aColor[i], aColor[i], s_CustomizationColor == i, &Button, CUI::CORNER_BR))
			s_CustomizationColor = i;
	}
	
	
	CUIRect Slider;
	CUIRect Button, Label;
	
	
	LeftView.HSplitTop(5.0f, 0, &LeftView);
	LeftView.HSplitTop(82.5f, &Label, &LeftView);


	int *pColors;
	if (s_CustomizationColor == 0)
		pColors = &g_Config.m_PlayerColorBody;
	else if (s_CustomizationColor == 1)
		pColors = &g_Config.m_PlayerColorFeet;
	else if (s_CustomizationColor == 2)
		pColors = &g_Config.m_PlayerColorSkin;
	else if (s_CustomizationColor == 3)
		pColors = &g_Config.m_PlayerColorTopper;
	else
		pColors = &g_Config.m_PlayerColorBody;
	
	const char *paLabels[] = {
		Localize("Hue"),
		Localize("Sat."),
		Localize("Lht.")};
	static int s_aColorSlider[3] = {0};

	LeftView.HSplitTop(20.0f, 0, &Slider);
	LeftView.VSplitRight(150, &Slider, 0);

	int PrevColor = *pColors;
	
	// color sliders
	int Color = 0;
	for(int s = 0; s < 3; s++)
	{
		Slider.HSplitTop(20.0f, &Label, &Slider);
		Label.VSplitLeft(100.0f, &Label, &Button);
		Button.HMargin(2.0f, &Button);

		float k = ((PrevColor>>((2-s)*8))&0xff) / 255.0f;
		k = DoScrollbarH(&s_aColorSlider[s], &Button, k);
		Color <<= 8;
		Color += clamp((int)(k*255), 0, 255);
		UI()->DoLabelScaled(&Label, paLabels[s], 14.0f, -1);
	}

	if(PrevColor != Color)
		m_NeedSendinfo = true;

	*pColors = Color;
			
	
	LeftView.HSplitTop(5.0f, 0, &LeftView);
	LeftView.HSplitTop(82.5f, &Label, &LeftView);
		
	// blood color select
	const char *aBlood[] = {Localize("Red"), Localize("Green"), Localize("Black")};
	int NumBloods = (int)(sizeof(aBlood)/sizeof(*aBlood));
	
	CUIRect BloodRect, B;
	LeftView.HSplitTop(0.0f, &B, 0);
	
	UI()->DoLabelScaled(&B, Localize("Blood color"), 14.0f, -1);
	
	LeftView.HSplitTop(20.0f, 0, &BloodRect);
	BloodRect.HSplitTop(20.0f, &BloodRect, 0);
	
	for(int i = 0; i < NumBloods; i++)
	{
		CUIRect Button;
		BloodRect.VSplitLeft(110.0f, &Button, &BloodRect);
		if(DoButton_MenuTab(&aBlood[i], aBlood[i], g_Config.m_PlayerBloodColor == i, &Button, CUI::CORNER_ALL))
		{
			g_Config.m_PlayerBloodColor = i;
		}
	}
	
	
	
	LeftView.HSplitTop(5.0f, 0, &LeftView);
	LeftView.HSplitTop(82.5f, &Label, &LeftView);
	
	
	// render player
	{
		CTeeRenderInfo Info;
		Info.m_ColorBody = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorBody);
		Info.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorFeet);
		Info.m_Body = 0; //g_Config.m_PlayerBody;
		Info.m_TopperTexture = m_pClient->m_pSkins->GetTopper(m_pClient->m_pSkins->FindTopper(g_Config.m_PlayerTopper))->m_Texture;
		Info.m_EyeTexture = m_pClient->m_pSkins->GetEye(m_pClient->m_pSkins->FindEye(g_Config.m_PlayerEye))->m_Texture;
		Info.m_HeadTexture = m_pClient->m_pSkins->GetHead(m_pClient->m_pSkins->FindHead(g_Config.m_PlayerHead))->m_Texture;
		Info.m_BodyTexture = m_pClient->m_pSkins->GetBody(m_pClient->m_pSkins->FindBody(g_Config.m_PlayerBody))->m_Texture;
		Info.m_HandTexture = m_pClient->m_pSkins->GetHand(m_pClient->m_pSkins->FindHand(g_Config.m_PlayerHand))->m_Texture;
		Info.m_FootTexture = m_pClient->m_pSkins->GetFoot(m_pClient->m_pSkins->FindFoot(g_Config.m_PlayerFoot))->m_Texture;
		Info.m_ColorTopper = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorTopper);
		Info.m_ColorSkin = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorSkin);
		Info.m_Size = UI()->Scale()*60.0f;
		
		RenderTools()->RenderStaticPlayer(&Info, vec2(LeftView.x+LeftView.w*0.825f, LeftView.y-LeftView.h*0.1f));
	}
	
	
	// skin type select
	static int s_SkinType = 0;
	const char *aSkinType[] = {Localize("Head"), Localize("Hair / hat"), Localize("Eyes"), Localize("Body"), Localize("Hands"), Localize("Feet")};
	int NumSkinTypes = (int)(sizeof(aSkinType)/sizeof(*aSkinType));
	

	CUIRect SkinTypeLabel, SkinSelect;
	MainView.HSplitTop(0.0f, &SkinTypeLabel, 0);
	
	UI()->DoLabelScaled(&SkinTypeLabel, Localize("Change skin of"), 14.0f, -1);
	
		
	// saving skins, helper for creating bot skins
	if(Input()->KeyDown(KEY_S) && (Input()->KeyPressed(KEY_LCTRL) || Input()->KeyPressed(KEY_RCTRL)))
	{
		SaveSkin();
	}
	
	MainView.HSplitTop(20.0f, 0, &SkinSelect);
	SkinSelect.HSplitTop(20.0f, &SkinSelect, 0);
	
	for(int i = 0; i < NumSkinTypes; i++)
	{
		CUIRect Button;
		SkinSelect.VSplitLeft(80.0f, &Button, &SkinSelect);
		if(DoButton_MenuTab(&aSkinType[i], aSkinType[i], s_SkinType == i, &Button, CUI::CORNER_BR))
			s_SkinType = i;
	}

	
	MainView.HSplitTop(5.0f, 0, &MainView);
	MainView.HSplitTop(20.5f, &Label, &MainView);

	// eye selector
	if (s_SkinType == 2)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumEyes(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetEye(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Eyes"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerEye) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_EyeTexture = s->m_Texture;
				Info.m_Size = UI()->Scale()*50.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderEye(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerEye, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerEye));
			m_NeedSendinfo = true;
		}
	}
	
	// topper selector
	if (s_SkinType == 1)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumToppers(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetTopper(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Hair / hat"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerTopper) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_TopperTexture = s->m_Texture;
				Info.m_ColorTopper = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorTopper);
				Info.m_Size = UI()->Scale()*80.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderTopper(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerTopper, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerTopper));
			m_NeedSendinfo = true;
		}
	}
	
	// head selector
	if (s_SkinType == 0)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumHeads(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetHead(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Head"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerHead) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_HeadTexture = s->m_Texture;
				//Info.m_ColorHead = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorSkin);
				Info.m_Size = UI()->Scale()*80.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderHead(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerHead, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerHead));
			m_NeedSendinfo = true;
		}
	}
	
	// hand selector
	if (s_SkinType == 4)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumHands(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetHand(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Hands"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerHand) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_HandTexture = s->m_Texture;
				//Info.m_ColorHead = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorSkin);
				Info.m_Size = UI()->Scale()*80.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderHand(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerHand, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerHand));
			m_NeedSendinfo = true;
		}
	}
	
	// foot selector
	if (s_SkinType == 5)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumFeet(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetFoot(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Feet"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerFoot) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_FootTexture = s->m_Texture;
				//Info.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorSkin);
				Info.m_Size = UI()->Scale()*80.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderFoot(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerFoot, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerFoot));
			m_NeedSendinfo = true;
		}
	}
	
	
	// body selector
	if (s_SkinType == 3)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		static bool s_InitSkinlist = true;
		static sorted_array<const CSkins::CSkinPart *> s_paSkinList;
		static float s_ScrollValue = 0.0f;
		if(s_InitSkinlist)
		{
			s_paSkinList.clear();
			for(int i = 0; i < m_pClient->m_pSkins->NumBodies(); ++i)
			{
				const CSkins::CSkinPart *s = m_pClient->m_pSkins->GetBody(i);
				// no special toppers
				if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
					continue;
				s_paSkinList.add(s);
			}
			s_InitSkinlist = false;
		}
		
		int OldSelected = -1;
		UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Body"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

		for(int i = 0; i < s_paSkinList.size(); ++i)
		{
			const CSkins::CSkinPart *s = s_paSkinList[i];
			if(s == 0)
				continue;

			if(str_comp(s->m_aName, g_Config.m_PlayerBody) == 0)
				OldSelected = i;

			CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
			if(Item.m_Visible)
			{
				CTeeRenderInfo Info;
				Info.m_BodyTexture = s->m_Texture;
				//Info.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorSkin);
				Info.m_Size = UI()->Scale()*80.0f;
				Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
				
				RenderTools()->RenderBody(&Info, vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));
			}
		}

		const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
		if(OldSelected != NewSelected)
		{
			mem_copy(g_Config.m_PlayerBody, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerBody));
			m_NeedSendinfo = true;
		}
	}
	
	
	
	if (m_EscapePressed)
		g_Config.m_UiPage = PAGE_FRONT;
}

	
void CMenus::RenderSettings(CUIRect MainView)
{
	static int s_SettingsPage = 0;

	// render background
	CUIRect Temp, TabBar, RestartWarning;
	MainView.HSplitBottom(15.0f, &MainView, &RestartWarning);
	MainView.VSplitRight(120.0f, &MainView, &TabBar);
	RenderTools()->DrawUIRect(&MainView, ms_ColorTabbarActive, CUI::CORNER_B|CUI::CORNER_TL, 10.0f);
	TabBar.HSplitTop(50.0f, &Temp, &TabBar);
	RenderTools()->DrawUIRect(&Temp, ms_ColorTabbarActive, CUI::CORNER_R, 10.0f);

	MainView.HSplitTop(10.0f, 0, &MainView);

	CUIRect Button;

	const char *aTabs[] = {
		Localize("Language"),
		Localize("General"),
		Localize("Player"),
		Localize("Customize"),
		Localize("Controls"),
		Localize("Graphics"),
		Localize("Sound")};

	int NumTabs = (int)(sizeof(aTabs)/sizeof(*aTabs));

	for(int i = 0; i < NumTabs; i++)
	{
		TabBar.HSplitTop(10, &Button, &TabBar);
		TabBar.HSplitTop(26, &Button, &TabBar);
		if(DoButton_MenuTab(aTabs[i], aTabs[i], s_SettingsPage == i, &Button, CUI::CORNER_R))
			s_SettingsPage = i;
	}

	MainView.Margin(10.0f, &MainView);

	if(s_SettingsPage == 0)
		RenderLanguageSelection(MainView);
	else if(s_SettingsPage == 1)
		RenderSettingsGeneral(MainView);
	else if(s_SettingsPage == 2)
		RenderSettingsPlayer(MainView);
	else if(s_SettingsPage == 3)
		RenderCustomization(MainView);
	else if(s_SettingsPage == 4)
		RenderSettingsControls(MainView);
	else if(s_SettingsPage == 5)
		RenderSettingsGraphics(MainView);
	else if(s_SettingsPage == 6)
		RenderSettingsSound(MainView);

	if(m_NeedRestartGraphics || m_NeedRestartSound)
		UI()->DoLabel(&RestartWarning, Localize("You must restart the game for all settings to take effect."), 15.0f, -1);
}
