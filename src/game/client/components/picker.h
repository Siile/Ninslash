#ifndef GAME_CLIENT_COMPONENTS_PICKER_H
#define GAME_CLIENT_COMPONENTS_PICKER_H
#include <base/vmath.h>
#include <game/client/component.h>

class CPicker : public CComponent
{
	void DrawCircle(float x, float y, float r, int Segments);
	void DrawEmoticons();
	void DrawWeapons();
	void DrawItems();
	void DrawKit();

	bool m_WasActive;
	bool m_Active;

	vec2 m_SelectorMouse;
	int m_Selected;
	int m_ItemSelected;

	static void ConKeyItemPicker(IConsole::IResult *pResult, void *pUserData);
	static void ConKeyPicker(IConsole::IResult *pResult, void *pUserData);
	static void ConItempick(IConsole::IResult *pResult, void *pUserData);
	static void ConLastWeaponpick(IConsole::IResult *pResult, void *pUserData);
	static void ConWeaponpick(IConsole::IResult *pResult, void *pUserData);
	static void ConDropWeapon(IConsole::IResult *pResult, void *pUserData);
	static void ConSwitchGroup(IConsole::IResult *pResult, void *pUserData);
	static void ConKeyEmote(IConsole::IResult *pResult, void *pUserData);
	static void ConEmote(IConsole::IResult *pResult, void *pUserData);
	
	int m_PickerType;
	bool m_ResetMouse;

public:
	CPicker();

	virtual void OnReset();
	virtual void OnConsoleInit();
	virtual void OnRender();
	virtual void OnRelease();
	virtual void OnMessage(int MsgType, void *pRawMsg);
	virtual bool OnMouseMove(float x, float y);

	void LastWeaponpick();
	void Weaponpick(int Weapon);
	void Itempick(int Item);
	void UseKit(int Kit);
	void Emote(int Emoticon);
	void DropWeapon();
	void SwitchGroup();
};

#endif
