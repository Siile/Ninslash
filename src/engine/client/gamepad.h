#ifndef ENGINE_CLIENT_GAMEPAD_H
#define ENGINE_CLIENT_GAMEPAD_H

#include <engine/gamepad.h>

class CGamepad : public IEngineGamepad
{
	int m_GamepadEnabled;
	int m_RumbleEnabled;
	
	SDL_Haptic* m_Haptic;
	
public:
	IEngineGraphics *m_pGraphics;
	IStorage *m_pStorage;

	virtual int Init();

	int Shutdown();
	void ScanGamepads();
	
	virtual void ConnectGamepad();
	virtual void DisconnectGamepad(int DeviceID);

	virtual bool IsGamepadEnabled() { return m_GamepadEnabled != 0; }
	virtual bool IsRumbleEnabled() { return m_Haptic != NULL; }
	
	virtual void Rumble(float Strength, unsigned int Length);

};

#endif
