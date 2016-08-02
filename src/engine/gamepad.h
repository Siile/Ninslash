#ifndef ENGINE_GAMEPAD_H
#define ENGINE_GAMEPAD_H

#include "kernel.h"

class IGamepad : public IInterface
{
	MACRO_INTERFACE("gamepad", 0)
public:
	virtual bool IsGamepadEnabled() = 0;
	virtual bool IsRumbleEnabled() = 0;
	virtual void ConnectGamepad() = 0;
	virtual void DisconnectGamepad(int DeviceID) = 0;
	
	virtual void Rumble(float Strength, unsigned int Length) = 0;
};


class IEngineGamepad : public IGamepad
{
	MACRO_INTERFACE("enginegamepad", 0)
public:
	virtual int Init() = 0;
	virtual int Shutdown() = 0;
};

extern IEngineGamepad *CreateEngineGamepad();

#endif
