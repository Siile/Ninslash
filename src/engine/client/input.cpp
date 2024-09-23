

#include <SDL.h>

#include <base/system.h>
#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/gamepad.h>
#include <engine/storage.h>
#include <engine/input.h>
#include <engine/keys.h>

#include "input.h"

//print >>f, "int inp_key_code(const char *key_name) { int i; if (!strcmp(key_name, \"-?-\")) return -1; else for (i = 0; i < 512; i++) if (!strcmp(key_strings[i], key_name)) return i; return -1; }"

// this header is protected so you don't include it from anywere
#define KEYS_INCLUDE
#include "keynames.h"
#undef KEYS_INCLUDE

void CInput::AddEvent(int Unicode, int Key, int Flags)
{
	if(m_NumEvents != INPUT_BUFFER_SIZE)
	{
		m_aInputEvents[m_NumEvents].m_Unicode = Unicode;
		m_aInputEvents[m_NumEvents].m_Key = Key;
		m_aInputEvents[m_NumEvents].m_Flags = Flags;
		m_NumEvents++;
	}
}

CInput::CInput()
{
	mem_zero(m_aInputCount, sizeof(m_aInputCount));
	mem_zero(m_aInputState, sizeof(m_aInputState));

	m_MouseModes = 0;

	m_InputCurrent = 0;
	m_InputDispatched = false;

	m_FirstWarp = false;
	m_LastMousePosX = 0;
	m_LastMousePosY = 0;

	m_pCursorSurface = NULL;
	m_pCursor = NULL;

	m_LastRelease = 0;
	m_ReleaseDelta = -1;

	m_MouseLeft = false;
	m_MouseEntered = false;


	m_GamepadMove = 0;
	m_GamepadDown = false;
	m_GamepadJump = false;
	m_GamepadShoot = false;
	m_GamepadSelect = false;
	
	m_UsingGamepad = false;
	m_GamepadAimX = 0;
	m_GamepadAimY = 0;
	m_GamepadOldAimX = 0;
	m_GamepadOldAimY = 0;
	
	m_NumEvents = 0;

	m_pClipboardText = 0;
}

void CInput::Init()
{
	m_pGraphics = Kernel()->RequestInterface<IEngineGraphics>();
	m_pGamepad = Kernel()->RequestInterface<IEngineGamepad>();
	SDL_StartTextInput();
	ShowCursor(true);
	//m_pGraphics->GrabWindow(true);
}

void CInput::LoadHardwareCursor()
{
	if(m_pCursor != NULL)
		return;

	CImageInfo CursorImg;
	if(!m_pGraphics->LoadPNG(&CursorImg, "gui_cursor_small.png", IStorage::TYPE_ALL))
		return;

	m_pCursorSurface = SDL_CreateRGBSurfaceFrom(
		CursorImg.m_pData, CursorImg.m_Width, CursorImg.m_Height,
		32, 4*CursorImg.m_Width,
		0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	if(!m_pCursorSurface)
		return;

	m_pCursor = SDL_CreateColorCursor(m_pCursorSurface, 0, 0);
}

int CInput::ShowCursor(bool show)
{
	if(g_Config.m_InpHWCursor)
	{
		LoadHardwareCursor();
		SDL_SetCursor(m_pCursor);
	}
	return SDL_ShowCursor(show);
}

void CInput::SetMouseModes(int modes)
{
	if((m_MouseModes & MOUSE_MODE_WARP_CENTER) && !(modes & MOUSE_MODE_WARP_CENTER))
		m_pGraphics->WarpMouse(m_LastMousePosX, m_LastMousePosY);
	else if(!(m_MouseModes & MOUSE_MODE_WARP_CENTER) && (modes & MOUSE_MODE_WARP_CENTER))
		m_FirstWarp = true;

	m_MouseModes = modes;
}

int CInput::GetMouseModes()
{
	return m_MouseModes;
}

void CInput::GetMousePosition(float *x, float *y)
{
	/*
	if (m_UsingGamepad)
	{
		*x = m_GamepadAimX;
		*y = m_GamepadAimY;
		return;
	}
	*/
	
	if(GetMouseModes() & MOUSE_MODE_NO_MOUSE)
		return;

	float Sens = g_Config.m_InpMousesens/100.0f;
	int nx = 0, ny = 0;
	SDL_GetMouseState(&nx, &ny);

	if(m_FirstWarp)
	{
		m_LastMousePosX = nx;
		m_LastMousePosY = ny;
		m_FirstWarp = false;
	}
	
	*x = nx * Sens;
	*y = ny * Sens;

	if(GetMouseModes() & MOUSE_MODE_WARP_CENTER)
		m_pGraphics->WarpMouse( Graphics()->ScreenWidth()/2,Graphics()->ScreenHeight()/2);
}

void CInput::GetRelativePosition(float *x, float *y)
{
	if(m_FirstWarp)
	{
		*x = 0;
		*y = 0;
		return;
	}


	if (m_UsingGamepad)
	{
		*x = m_GamepadAimX / 400.0f;
		*y = m_GamepadAimY / 400.0f;
		return;
	}
	
	*x *= (float)100/g_Config.m_InpMousesens;
	*y *= (float)100/g_Config.m_InpMousesens;
	*x -= Graphics()->ScreenWidth()/2;
	*y -= Graphics()->ScreenHeight()/2;
}

bool CInput::MouseMoved()
{
	int x = 0, y = 0;
	SDL_GetRelativeMouseState(&x, &y);
	return x != 0 || y != 0;
}

bool CInput::GamepadMoved()
{
	if (!m_UsingGamepad)
		return false;

	if (m_GamepadOldAimX != m_GamepadAimX || m_GamepadOldAimY != m_GamepadAimY || abs(m_GamepadAimX) + abs(m_GamepadAimY) > 10000)
	{
		m_GamepadOldAimX = m_GamepadAimX;
		m_GamepadOldAimY = m_GamepadAimY;
		return true;
	}
	return false;
}

int CInput::MouseDoubleClick()
{
	if(m_ReleaseDelta >= 0 && m_ReleaseDelta < (time_freq() >> 2))
	{
		m_LastRelease = 0;
		m_ReleaseDelta = -1;
		return 1;
	}
	return 0;
}

const char *CInput::GetClipboardText()
{
	if(m_pClipboardText)
		SDL_free(m_pClipboardText);
	m_pClipboardText = SDL_GetClipboardText();
	if(m_pClipboardText)
		str_sanitize_cc(m_pClipboardText);
	return m_pClipboardText;
}

void CInput::SetClipboardText(const char *Text)
{
	SDL_SetClipboardText(Text);
}

bool CInput::MouseLeft()
{
	return m_MouseLeft;
}

bool CInput::MouseEntered()
{
	return m_MouseEntered;
}

void CInput::ClearKeyStates()
{
	mem_zero(m_aInputState, sizeof(m_aInputState));
	mem_zero(m_aInputCount, sizeof(m_aInputCount));
}

int CInput::KeyState(int Key)
{
	return m_aInputState[m_InputCurrent][Key];
}

void CInput::ResetGamepad()
{
	m_GamepadMove = 0;
	m_GamepadDown = false;
	m_GamepadJump = false;
	m_GamepadShoot = false;
	m_GamepadSelect = false;
	
	m_UsingGamepad = false;
	m_GamepadAimX = 0;
	m_GamepadAimY = 0;
	m_GamepadOldAimX = 0;
	m_GamepadOldAimY = 0;
}

int CInput::Update()
{
//	if(m_InputGrabbed && !Graphics()->WindowActive())
//		MouseModeAbsolute();

	/*if(!input_grabbed && Graphics()->WindowActive())
		Input()->MouseModeRelative();*/

	if(m_InputDispatched)
	{
		// clear and begin count on the other one
		m_InputCurrent^=1;
		mem_zero(&m_aInputCount[m_InputCurrent], sizeof(m_aInputCount[m_InputCurrent]));
		mem_zero(&m_aInputState[m_InputCurrent], sizeof(m_aInputState[m_InputCurrent]));
		m_InputDispatched = false;
	}

	{
		int i;
		const Uint8 *pState = SDL_GetKeyboardState(&i);
		if(i >= KEY_LAST)
			i = KEY_LAST-1;
		mem_copy(m_aInputState[m_InputCurrent], pState, i);
	}

	m_MouseLeft = false;
	m_MouseEntered = false;

	// these states must always be updated manually because they are not in the GetKeyState from SDL
	int i = SDL_GetMouseState(NULL, NULL);
	if(i&SDL_BUTTON(1)) m_aInputState[m_InputCurrent][KEY_MOUSE_1] = 1; // 1 is left
	if(i&SDL_BUTTON(3)) m_aInputState[m_InputCurrent][KEY_MOUSE_2] = 1; // 3 is right
	if(i&SDL_BUTTON(2)) m_aInputState[m_InputCurrent][KEY_MOUSE_3] = 1; // 2 is middle
	if(i&SDL_BUTTON(4)) m_aInputState[m_InputCurrent][KEY_MOUSE_4] = 1;
	if(i&SDL_BUTTON(5)) m_aInputState[m_InputCurrent][KEY_MOUSE_5] = 1;
	if(i&SDL_BUTTON(6)) m_aInputState[m_InputCurrent][KEY_MOUSE_6] = 1;
	if(i&SDL_BUTTON(7)) m_aInputState[m_InputCurrent][KEY_MOUSE_7] = 1;
	if(i&SDL_BUTTON(8)) m_aInputState[m_InputCurrent][KEY_MOUSE_8] = 1;

	{
		SDL_Event Event;

		while(SDL_PollEvent(&Event))
		{
			int Key = -1;
			int Action = IInput::FLAG_PRESS;
			
			int LastGamepadMove = m_GamepadMove;
			bool LastGamepadDown = m_GamepadDown;
			bool LastGamepadJump = m_GamepadJump;
			bool LastGamepadShoot = m_GamepadShoot;
			bool LastGamepadSelect = m_GamepadSelect;
			
			switch (Event.type)
			{
				case SDL_TEXTINPUT:
				{
					int TextLength, i;
					TextLength = strlen(Event.text.text);
					for(i = 0; i < TextLength; i++)
					{
						AddEvent(Event.text.text[i], 0, 0);
					}
					break;
				}
				// handle keys
				case SDL_KEYDOWN:
					Key = SDL_GetScancodeFromName(SDL_GetKeyName(Event.key.keysym.sym));
					break;
				case SDL_KEYUP:
					Action = IInput::FLAG_RELEASE;
					Key = SDL_GetScancodeFromName(SDL_GetKeyName(Event.key.keysym.sym));
					break;

					
				// handle gamepad
				case SDL_CONTROLLERDEVICEADDED:
					ResetGamepad();
					Gamepad()->ConnectGamepad();
					Gamepad()->Rumble(1.0f, 2000);
					break;
					
					
				case SDL_CONTROLLERDEVICEREMOVED:
					ResetGamepad();
					Gamepad()->DisconnectGamepad(0);
					break;
				
				
				case SDL_CONTROLLERBUTTONUP:
					Action = IInput::FLAG_RELEASE;
					
				// fall through
				case SDL_CONTROLLERBUTTONDOWN:
					m_UsingGamepad = true;
					if(Event.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) Key = KEY_GAMEPAD_SHOULDER_LEFT;
					if(Event.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) Key = KEY_GAMEPAD_SHOULDER_RIGHT;
					if(Event.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) Key = KEY_GAMEPAD_BUTTON_BACK;
					if(Event.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSTICK) Key = KEY_GAMEPAD_BUTTON_LEFTSTICK;
					if(Event.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSTICK) Key = KEY_GAMEPAD_BUTTON_RIGHTSTICK;
					if(Event.cbutton.button == SDL_CONTROLLER_BUTTON_A) Key = KEY_GAMEPAD_BUTTON_A;
					if(Event.cbutton.button == SDL_CONTROLLER_BUTTON_B) Key = KEY_GAMEPAD_BUTTON_B;
					if(Event.cbutton.button == SDL_CONTROLLER_BUTTON_X) Key = KEY_GAMEPAD_BUTTON_X;
					if(Event.cbutton.button == SDL_CONTROLLER_BUTTON_Y) Key = KEY_GAMEPAD_BUTTON_Y;
					break;
					
				case SDL_CONTROLLERAXISMOTION:
					if (!m_UsingGamepad)
						break;
					
					// attack
					if (Event.jaxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
					{
						if (Event.jaxis.value < 100)
							m_GamepadShoot = false;
						else
							m_GamepadShoot = true;
					}
					
					// select
					if (Event.jaxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
					{
						if (Event.jaxis.value < 100)
							m_GamepadSelect = false;
						else
							m_GamepadSelect = true;
					}
					
					// jump
					if (Event.jaxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
					{
						if (Event.jaxis.value > -20000)
							m_GamepadJump = false;
						else
							m_GamepadJump = true;
						
						if (Event.jaxis.value < 20000)
							m_GamepadDown = false;
						else
							m_GamepadDown = true;
					}
					
					// move
					if (Event.jaxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
					{
						if (Event.jaxis.value > 20000)
							m_GamepadMove = 1;
						else if (Event.jaxis.value < -20000)
							m_GamepadMove = -1;
						else
							m_GamepadMove = 0;
					}
					
					// aim
					if (Event.jaxis.axis == SDL_CONTROLLER_AXIS_RIGHTX)
						m_GamepadAimX = Event.jaxis.value;
					
					if (Event.jaxis.axis == SDL_CONTROLLER_AXIS_RIGHTY)
						m_GamepadAimY = Event.jaxis.value;
					break;
					
					
				// handle mouse buttons
				case SDL_MOUSEBUTTONUP:
					Action = IInput::FLAG_RELEASE;

					if(Event.button.button == 1) // ignore_convention
					{
						m_ReleaseDelta = time_get() - m_LastRelease;
						m_LastRelease = time_get();
					}
				
				// fall through
				case SDL_MOUSEBUTTONDOWN:
					m_UsingGamepad = false;
					
					if(Event.button.button == SDL_BUTTON_LEFT) Key = KEY_MOUSE_1; // ignore_convention
					if(Event.button.button == SDL_BUTTON_RIGHT) Key = KEY_MOUSE_2; // ignore_convention
					if(Event.button.button == SDL_BUTTON_MIDDLE) Key = KEY_MOUSE_3; // ignore_convention
					if(Event.button.button == 6) Key = KEY_MOUSE_6; // ignore_convention
					if(Event.button.button == 7) Key = KEY_MOUSE_7; // ignore_convention
					if(Event.button.button == 8) Key = KEY_MOUSE_8; // ignore_convention
					break;

				case SDL_MOUSEWHEEL:
					if(Event.wheel.y > 0) Key = KEY_MOUSE_WHEEL_UP; // ignore_convention
					if(Event.wheel.y < 0) Key = KEY_MOUSE_WHEEL_DOWN; // ignore_convention
					AddEvent(0, Key, Action);
					Action = IInput::FLAG_RELEASE;
					break;

				case SDL_WINDOWEVENT:
					if(Event.window.event == SDL_WINDOWEVENT_ENTER) m_MouseEntered = true;
					if(Event.window.event == SDL_WINDOWEVENT_LEAVE) m_MouseLeft = true;
					break;

				// other messages
				case SDL_QUIT:
					return 1;
			}

			//
			if(Key != -1)
			{
				m_aInputCount[m_InputCurrent][Key].m_Presses++;
				if(Action == IInput::FLAG_PRESS)
					m_aInputState[m_InputCurrent][Key] = 1;
				AddEvent(0, Key, Action);
			}

			
			// send gamepad shoot
			if (LastGamepadShoot != m_GamepadShoot)
			{
				Action = m_GamepadShoot ? IInput::FLAG_PRESS : IInput::FLAG_RELEASE;
				m_aInputCount[m_InputCurrent][KEY_GAMEPAD_TRIGGER_RIGHT].m_Presses++;
				if(Action == IInput::FLAG_PRESS)
					m_aInputState[m_InputCurrent][KEY_GAMEPAD_TRIGGER_RIGHT] = 1;
				AddEvent(0, KEY_GAMEPAD_TRIGGER_RIGHT, Action);
			}
			
			// send gamepad select
			if (LastGamepadSelect != m_GamepadSelect)
			{
				Action = m_GamepadSelect ? IInput::FLAG_PRESS : IInput::FLAG_RELEASE;
				m_aInputCount[m_InputCurrent][KEY_GAMEPAD_TRIGGER_LEFT].m_Presses++;
				if(Action == IInput::FLAG_PRESS)
					m_aInputState[m_InputCurrent][KEY_GAMEPAD_TRIGGER_LEFT] = 1;
				AddEvent(0, KEY_GAMEPAD_TRIGGER_LEFT, Action);
			}
			
			// send gamepad jump
			if (LastGamepadJump != m_GamepadJump)
			{
				Action = m_GamepadJump ? IInput::FLAG_PRESS : IInput::FLAG_RELEASE;
				m_aInputCount[m_InputCurrent][KEY_GAMEPAD_AXIS_UP].m_Presses++;
				if(Action == IInput::FLAG_PRESS)
					m_aInputState[m_InputCurrent][KEY_GAMEPAD_AXIS_UP] = 1;
				AddEvent(0, KEY_GAMEPAD_AXIS_UP, Action);
			}
			
			// send gamepad down
			if (LastGamepadDown != m_GamepadDown)
			{
				Action = m_GamepadDown ? IInput::FLAG_PRESS : IInput::FLAG_RELEASE;
				m_aInputCount[m_InputCurrent][KEY_GAMEPAD_AXIS_DOWN].m_Presses++;
				if(Action == IInput::FLAG_PRESS)
					m_aInputState[m_InputCurrent][KEY_GAMEPAD_AXIS_DOWN] = 1;
				AddEvent(0, KEY_GAMEPAD_AXIS_DOWN, Action);
			}
			
			// send gamepad axis x
			if (LastGamepadMove != m_GamepadMove)
			{
				if (m_GamepadMove == -1)
				{
					// press left
					m_aInputCount[m_InputCurrent][KEY_GAMEPAD_AXIS_LEFT].m_Presses++;
					m_aInputState[m_InputCurrent][KEY_GAMEPAD_AXIS_LEFT] = 1;
					AddEvent(0, KEY_GAMEPAD_AXIS_LEFT, IInput::FLAG_PRESS);
					
					// release right
					if (LastGamepadMove == 1)
					{
						m_aInputCount[m_InputCurrent][KEY_GAMEPAD_AXIS_RIGHT].m_Presses++;
						m_aInputState[m_InputCurrent][KEY_GAMEPAD_AXIS_RIGHT] = 1;
						AddEvent(0, KEY_GAMEPAD_AXIS_RIGHT, IInput::FLAG_RELEASE);
					}
				}
				else if (m_GamepadMove == 1)
				{
					// press right
					m_aInputCount[m_InputCurrent][KEY_GAMEPAD_AXIS_RIGHT].m_Presses++;
					m_aInputState[m_InputCurrent][KEY_GAMEPAD_AXIS_RIGHT] = 1;
					AddEvent(0, KEY_GAMEPAD_AXIS_RIGHT, IInput::FLAG_PRESS);
					
					// release left
					if (LastGamepadMove == -1)
					{
						m_aInputCount[m_InputCurrent][KEY_GAMEPAD_AXIS_LEFT].m_Presses++;
						m_aInputState[m_InputCurrent][KEY_GAMEPAD_AXIS_LEFT] = 1;
						AddEvent(0, KEY_GAMEPAD_AXIS_LEFT, IInput::FLAG_RELEASE);
					}
				}
				else if (m_GamepadMove == 0)
				{
					// release left
					if (LastGamepadMove == -1)
					{
						m_aInputCount[m_InputCurrent][KEY_GAMEPAD_AXIS_LEFT].m_Presses++;
						m_aInputState[m_InputCurrent][KEY_GAMEPAD_AXIS_LEFT] = 1;
						AddEvent(0, KEY_GAMEPAD_AXIS_LEFT, IInput::FLAG_RELEASE);
					}
					
					// release right
					if (LastGamepadMove == 1)
					{
						m_aInputCount[m_InputCurrent][KEY_GAMEPAD_AXIS_RIGHT].m_Presses++;
						m_aInputState[m_InputCurrent][KEY_GAMEPAD_AXIS_RIGHT] = 1;
						AddEvent(0, KEY_GAMEPAD_AXIS_RIGHT, IInput::FLAG_RELEASE);
					}
				}
			}
		}
	}

	return 0;
}


IEngineInput *CreateEngineInput() { return new CInput; }
