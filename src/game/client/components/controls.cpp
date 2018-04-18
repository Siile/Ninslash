

#include <base/math.h>

#include <engine/shared/config.h>

#include <game/collision.h>
#include <game/client/gameclient.h>
#include <game/client/component.h>
#include <game/client/components/inventory.h>
#include <game/client/components/chat.h>
#include <game/client/components/menus.h>
#include <game/client/components/scoreboard.h>

#include <game/client/customstuff.h>

#include "controls.h"

CControls::CControls()
{
	mem_zero(&m_LastData, sizeof(m_LastData));
}

void CControls::OnReset()
{
	m_LastData.m_Direction = 0;
	m_LastData.m_Hook = 0;
	m_LastData.m_Down = 0;
	m_LastData.m_Charge = 0;
	// simulate releasing the fire button
	if((m_LastData.m_Fire&1) != 0)
		m_LastData.m_Fire++;
	m_LastData.m_Fire &= INPUT_STATE_MASK;
	m_LastData.m_Jump = 0;
	m_InputData = m_LastData;

	m_InputDirectionLeft = 0;
	m_InputDirectionRight = 0;
	
	m_PickedWeapon = -1;
	m_SignalWeapon = -1;
	
	m_Build = 0;
	m_BuildReleased = true;
	m_BuildMode = false;
	m_LastWeapon = 1;
	m_SelectedBuilding = -1;
}

void CControls::OnRelease()
{
	OnReset();
}

void CControls::OnPlayerDeath()
{
	m_LastData.m_WantedWeapon = m_InputData.m_WantedWeapon = 0;
}

static void ConKeyInputState(IConsole::IResult *pResult, void *pUserData)
{
	((int *)pUserData)[0] = pResult->GetInteger(0);
}

static void ConKeyInputCounter(IConsole::IResult *pResult, void *pUserData)
{
	int *v = (int *)pUserData;
	if(((*v)&1) != pResult->GetInteger(0))
		(*v)++;
	*v &= INPUT_STATE_MASK;
}

struct CInputSet
{
	CControls *m_pControls;
	int *m_pVariable;
	int *m_pVariable2; // for building
	int m_Value;
};

static void ConKeyInputSet(IConsole::IResult *pResult, void *pUserData)
{
	CInputSet *pSet = (CInputSet *)pUserData;
	if(pResult->GetInteger(0))
	{
		*pSet->m_pVariable = pSet->m_Value;
		*pSet->m_pVariable2 = pSet->m_Value-1; // ugly!
	}
}

static void ConKeyInputNextPrevWeapon(IConsole::IResult *pResult, void *pUserData)
{
	CInputSet *pSet = (CInputSet *)pUserData;
	ConKeyInputCounter(pResult, pSet->m_pVariable);
	pSet->m_pControls->m_InputData.m_WantedWeapon = 0;
}

void CControls::OnConsoleInit()
{
	// game commands
	Console()->Register("+left", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputDirectionLeft, "Move left");
	Console()->Register("+right", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputDirectionRight, "Move right");
	Console()->Register("+down", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Down, "Slide / down");
	Console()->Register("+jump", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Jump, "Jump");
	Console()->Register("+turbo", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Hook, "Turbo");
	Console()->Register("+charge", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Charge, "Charge");
	Console()->Register("+fire", "", CFGFLAG_CLIENT, ConKeyInputCounter, &m_InputData.m_Fire, "Fire");
	//Console()->Register("+build", "", CFGFLAG_CLIENT, ConKeyInputCounter, &m_Build, "Build");

	// gamepad
	Console()->Register("+gamepadleft", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputDirectionLeft, "Move left");
	Console()->Register("+gamepadright", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputDirectionRight, "Move right");
	Console()->Register("+gamepaddown", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Down, "Slide");
	Console()->Register("+gamepadjump", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Jump, "Jump");
	Console()->Register("+gamepadturbo", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Hook, "Turbo");
	Console()->Register("+gamepadfire", "", CFGFLAG_CLIENT, ConKeyInputCounter, &m_InputData.m_Fire, "Fire");
	Console()->Register("+gamepadbuild", "", CFGFLAG_CLIENT, ConKeyInputCounter, &m_Build, "Build");

	{ static CInputSet s_Set = {this, &m_InputData.m_NextWeapon, 0}; Console()->Register("+gamepadnextweapon", "", CFGFLAG_CLIENT, ConKeyInputNextPrevWeapon, (void *)&s_Set, "Switch to next weapon"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_PrevWeapon, 0}; Console()->Register("+gamepadprevweapon", "", CFGFLAG_CLIENT, ConKeyInputNextPrevWeapon, (void *)&s_Set, "Switch to previous weapon"); }

	
	
	// can't pick tool except with build key
	//{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 1}; Console()->Register("+weapon1", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon1"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 2}; Console()->Register("+weapon2", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon2"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 3}; Console()->Register("+weapon3", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon3"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 4}; Console()->Register("+weapon4", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon4"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 5}; Console()->Register("+weapon5", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon5"); }
	/*
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 6}; Console()->Register("+weapon6", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon6"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 7}; Console()->Register("+weapon7", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon7"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 8}; Console()->Register("+weapon8", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon8"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 9}; Console()->Register("+weapon9", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon9"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 10}; Console()->Register("+weapon10", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon10"); }
	*/
	
	{ static CInputSet s_Set = {this, &m_InputData.m_NextWeapon, 0}; Console()->Register("+nextweapon", "", CFGFLAG_CLIENT, ConKeyInputNextPrevWeapon, (void *)&s_Set, "Switch to next weapon"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_PrevWeapon, 0}; Console()->Register("+prevweapon", "", CFGFLAG_CLIENT, ConKeyInputNextPrevWeapon, (void *)&s_Set, "Switch to previous weapon"); }
}

void CControls::OnMessage(int Msg, void *pRawMsg)
{
	/*
	if(Msg == NETMSGTYPE_SV_WEAPONPICKUP)
	{
		CNetMsg_Sv_WeaponPickup *pMsg = (CNetMsg_Sv_WeaponPickup *)pRawMsg;
		if(g_Config.m_ClAutoswitchWeapons)
			m_InputData.m_WantedWeapon = pMsg->m_Weapon+1;
	}
	*/
	
	/*
	if(Msg == NETMSGTYPE_SV_WEAPONPICKUP)
	{
		CNetMsg_Sv_WeaponPickup *pMsg = (CNetMsg_Sv_WeaponPickup *)pRawMsg;
		
		CustomStuff()->m_WeaponpickTimer = 1.0f;
		CustomStuff()->m_WeaponpickWeapon = pMsg->m_Weapon;
		CustomStuff()->m_LastWeaponPicked = false;
			
		if(g_Config.m_ClAutoswitchWeapons)
		{
			// not working yet
			m_InputData.m_WantedWeapon = pMsg->m_Weapon+1;
		}
	}
	*/
}


//input count
struct CInputCount
{
	int m_Presses;
	int m_Releases;
};

CInputCount CountInput(int Prev, int Cur)
{
	CInputCount c = {0, 0};
	Prev &= INPUT_STATE_MASK;
	Cur &= INPUT_STATE_MASK;
	int i = Prev;

	while(i != Cur)
	{
		i = (i+1)&INPUT_STATE_MASK;
		if(i&1)
			c.m_Presses++;
		else
			c.m_Releases++;
	}

	return c;
}


int CControls::SnapInput(int *pData)
{
	static int64 LastSendTime = 0;
	static int PrevWeapon = 0;
	bool Send = false;

	// update player state
	if(m_pClient->m_pChat->IsActive())
		m_InputData.m_PlayerFlags = PLAYERFLAG_CHATTING;
	else if(m_pClient->m_pMenus->IsActive())
		m_InputData.m_PlayerFlags = PLAYERFLAG_IN_MENU;
	else
		m_InputData.m_PlayerFlags = PLAYERFLAG_PLAYING;

	if(m_pClient->m_pScoreboard->Active())
		m_InputData.m_PlayerFlags |= PLAYERFLAG_SCOREBOARD;

	if(m_LastData.m_PlayerFlags != m_InputData.m_PlayerFlags)
		Send = true;

	m_LastData.m_PlayerFlags = m_InputData.m_PlayerFlags;

	// we freeze the input if chat or menu is activated
	if(!(m_InputData.m_PlayerFlags&PLAYERFLAG_PLAYING))
	{
		OnReset();

		mem_copy(pData, &m_InputData, sizeof(m_InputData));

		// send once a second just to be sure
		if(time_get() > LastSendTime + time_freq())
			Send = true;
	}
	else
	{

		m_InputData.m_TargetX = (int)m_MousePos.x;
		m_InputData.m_TargetY = (int)m_MousePos.y;
		if(!m_InputData.m_TargetX && !m_InputData.m_TargetY)
		{
			m_InputData.m_TargetX = 1;
			m_MousePos.x = 1;
		}

		// set direction
		m_InputData.m_Direction = 0;
		if(m_InputDirectionLeft && !m_InputDirectionRight)
			m_InputData.m_Direction = -1;
		if(!m_InputDirectionLeft && m_InputDirectionRight)
			m_InputData.m_Direction = 1;

		// stress testing
		if(g_Config.m_DbgStress)
		{
			float t = Client()->LocalTime();
			mem_zero(&m_InputData, sizeof(m_InputData));

			m_InputData.m_Direction = ((int)t/2)&1;
			m_InputData.m_Jump = ((int)t);
			m_InputData.m_Fire = ((int)(t*10));
			m_InputData.m_Hook = ((int)(t*2))&1;
			m_InputData.m_Down = ((int)(t*3))&1;
			m_InputData.m_Charge = ((int)(t*4))&1;
			m_InputData.m_WantedWeapon = ((int)t)%4;
			m_InputData.m_TargetX = (int)(sinf(t*3)*100.0f);
			m_InputData.m_TargetY = (int)(cosf(t*3)*100.0f);
		}

		// get wanted weapon from picker
		/*
		if (m_PickedWeapon >= 0 && !m_BuildMode)
			m_InputData.m_WantedWeapon = m_PickedWeapon+1;
		*/
		
		m_PickedWeapon = -1;
		
		if (m_InputData.m_WantedWeapon != PrevWeapon)
		{
			PrevWeapon = m_InputData.m_WantedWeapon;
		}
		
		// check if we need to send input
		if(m_InputData.m_Direction != m_LastData.m_Direction) Send = true;
		else if(m_InputData.m_Jump != m_LastData.m_Jump) Send = true;
		else if(m_InputData.m_Fire != m_LastData.m_Fire) Send = true;
		else if(m_InputData.m_Hook != m_LastData.m_Hook) Send = true;
		else if(m_InputData.m_Down != m_LastData.m_Down) Send = true;
		else if(m_InputData.m_Charge != m_LastData.m_Charge) Send = true;
		else if(m_InputData.m_WantedWeapon != m_LastData.m_WantedWeapon) Send = true;
		else if(m_InputData.m_NextWeapon != m_LastData.m_NextWeapon) Send = true;
		else if(m_InputData.m_PrevWeapon != m_LastData.m_PrevWeapon) Send = true;

		// send at at least 10hz
		if(time_get() > LastSendTime + time_freq()/25)
			Send = true;
		
		m_Build = 0;
	}
	
	// copy and return size
	m_LastData = m_InputData;

	if(!Send)
		return 0;

	LastSendTime = time_get();
	mem_copy(pData, &m_InputData, sizeof(m_InputData));
	return sizeof(m_InputData);
}

void CControls::OnRender()
{
	// update target pos
	if(m_pClient->m_Snap.m_pGameInfoObj && !m_pClient->m_Snap.m_SpecInfo.m_Active)
		m_TargetPos = m_pClient->m_LocalCharacterPos + m_MousePos;
	else if(m_pClient->m_Snap.m_SpecInfo.m_Active && m_pClient->m_Snap.m_SpecInfo.m_UsePosition)
		m_TargetPos = m_pClient->m_Snap.m_SpecInfo.m_Position + m_MousePos;
	else
		m_TargetPos = m_MousePos;
}

bool CControls::OnMouseMove(float x, float y)
{
	if((m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED) ||
		(m_pClient->m_Snap.m_SpecInfo.m_Active && m_pClient->m_pChat->IsActive()))
		return false;

	Input()->SetMouseModes(IInput::MOUSE_MODE_WARP_CENTER);
	Input()->ShowCursor(false);

	Input()->GetRelativePosition(&x, &y);

	if (Input()->UsingGamepad())
	{
		if (m_pClient->m_Snap.m_SpecInfo.m_Active)
			m_MousePos += vec2(x, y) * ((200.0f + g_Config.m_InpMousesens) / 1500.0f);
		else
			m_MousePos += vec2(x, y) * ((200.0f + g_Config.m_InpMousesens) / 150.0f);
	}
	else
		m_MousePos += vec2(x, y) * ((200.0f + g_Config.m_InpMousesens) / 150.0f);
	ClampMousePos();

	return true;
}

void CControls::ClampMousePos()
{
	if(m_pClient->m_Snap.m_SpecInfo.m_Active && !m_pClient->m_Snap.m_SpecInfo.m_UsePosition)
	{
		m_MousePos.x = clamp(m_MousePos.x, 200.0f, Collision()->GetWidth()*32-200.0f);
		m_MousePos.y = clamp(m_MousePos.y, 200.0f, Collision()->GetHeight()*32-200.0f);

	}
	else
	{
		float CameraMaxDistance = 200.0f;
		float FollowFactor = g_Config.m_ClMouseFollowfactor/100.0f;
		float MouseMax = min(CameraMaxDistance/FollowFactor + g_Config.m_ClMouseDeadzone, (float)g_Config.m_ClMouseMaxDistance);

		if(length(m_MousePos) > MouseMax)
			m_MousePos = normalize(m_MousePos)*MouseMax;
	}
}
