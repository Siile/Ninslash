#include <base/math.h>
#include <base/system.h>

#include <engine/graphics.h>
#include <engine/storage.h>

#include <engine/shared/config.h>

#include <SDL.h>

#include "gamepad.h"

#include <math.h>



int CGamepad::Init()
{
	m_GamepadEnabled = 0;
	m_RumbleEnabled = 0;
	m_pGraphics = Kernel()->RequestInterface<IEngineGraphics>();
	m_pStorage = Kernel()->RequestInterface<IStorage>();

	
	//if(!g_Config.m_SndEnable)
	//	return 0;

	if(SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0)
	{
		dbg_msg("gamepad", "unable to init SDL gamecontroller: %s", SDL_GetError());
		return -1;
	}
	
	if(SDL_InitSubSystem(SDL_INIT_HAPTIC) < 0)
	{
		dbg_msg("gamepad", "unable to init SDL haptic: %s", SDL_GetError());
	}
	else m_RumbleEnabled = true;

	m_GamepadEnabled = 1;

	ScanGamepads();
	
	return 0;
}


	
void CGamepad::Rumble(float Strength, unsigned int Length)
{
	if (!IsRumbleEnabled())
		return;
	
	SDL_HapticRumblePlay(m_Haptic, Strength, Length);
}
	
void CGamepad::ConnectGamepad()
{
	dbg_msg("gamepad", "Gamepad connected!");
	ScanGamepads();
}


void CGamepad::DisconnectGamepad(int DeviceID)
{
	//SDL_GameControllerClose(gamepad);
	
	//if (DeviceID == g_Config.m_GamepadID)
	dbg_msg("gamepad", "Gamepad disconnected!");
	ScanGamepads();
}
	
	
void CGamepad::ScanGamepads()
{
	for (int i = 0; i < 9; i++)
	{
		SDL_GameController *Pad = SDL_GameControllerOpen(i);
		
		if (Pad)
		{
			SDL_Joystick *Joy = SDL_GameControllerGetJoystick(Pad);
			
			if (Joy)
			{
				int InstanceID = SDL_JoystickInstanceID(Joy);
				dbg_msg("gamepad", "Gamepad found, id: %u", InstanceID);

				if (m_RumbleEnabled)
				{
					if (SDL_JoystickIsHaptic(Joy)) {
						m_Haptic = SDL_HapticOpenFromJoystick(Joy);
						
						if (SDL_HapticRumbleSupported(m_Haptic)) {
							if (SDL_HapticRumbleInit(m_Haptic) != 0) {
								dbg_msg("gamepad", "Haptic Rumble Init: %s", SDL_GetError());
								SDL_HapticClose(m_Haptic);
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
	SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
	return 0;
}



IEngineGamepad *CreateEngineGamepad() { return new CGamepad; }

