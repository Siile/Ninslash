#include <base/math.h>
#include <base/system.h>

#include <engine/graphics.h>
#include <engine/storage.h>

#include <engine/shared/config.h>

#include <SDL3/SDL.h>

#include "gamepad.h"

#include <math.h>

int CGamepad::Init()
{
	m_GamepadEnabled = 0;
	m_RumbleEnabled = 0;
	m_pGraphics = Kernel()->RequestInterface<IEngineGraphics>();
	m_pStorage = Kernel()->RequestInterface<IStorage>();

	// if(!g_Config.m_SndEnable)
	//	return 0;

	if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD))
	{
		dbg_msg("gamepad", "unable to init SDL gamecontroller: %s", SDL_GetError());
		return -1;
	}

	if (!SDL_InitSubSystem(SDL_INIT_HAPTIC))
	{
		dbg_msg("gamepad", "unable to init SDL haptic: %s", SDL_GetError());
	}
	else
		m_RumbleEnabled = true;

	m_GamepadEnabled = 1;

	ScanGamepads();

	return 0;
}

void CGamepad::Rumble(float Strength, unsigned int Length)
{
	if (!IsRumbleEnabled())
		return;

	SDL_PlayHapticRumble(m_Haptic, Strength, Length);
}

void CGamepad::ConnectGamepad()
{
	dbg_msg("gamepad", "Gamepad connected!");
	ScanGamepads();
}

void CGamepad::DisconnectGamepad(int DeviceID)
{
	// SDL_GameControllerClose(gamepad);

	// if (DeviceID == g_Config.m_GamepadID)
	dbg_msg("gamepad", "Gamepad disconnected!");
	ScanGamepads();
}

void CGamepad::ScanGamepads()
{
	for (int i = 0; i < 9; i++)
	{
		SDL_Gamepad *Pad = SDL_OpenGamepad(i);

		if (Pad)
		{
			SDL_Joystick *Joy = SDL_GetGamepadJoystick(Pad);

			if (Joy)
			{
				int InstanceID = SDL_GetJoystickID(Joy);
				dbg_msg("gamepad", "Gamepad found, id: %u", InstanceID);

				if (m_RumbleEnabled)
				{
					if (SDL_IsJoystickHaptic(Joy))
					{
						m_Haptic = SDL_OpenHapticFromJoystick(Joy);

						if (SDL_HapticRumbleSupported(m_Haptic))
						{
							if (SDL_InitHapticRumble(m_Haptic) != 0)
							{
								dbg_msg("gamepad", "Haptic Rumble Init: %s", SDL_GetError());
								SDL_CloseHaptic(m_Haptic);
								m_Haptic = NULL;
							}
						}
					}
				}

				break;
			}
		}
	}
}

int CGamepad::Shutdown()
{
	SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
	return 0;
}

IEngineGamepad *CreateEngineGamepad() { return new CGamepad; }
