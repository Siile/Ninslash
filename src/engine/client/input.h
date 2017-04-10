

#ifndef ENGINE_CLIENT_INPUT_H
#define ENGINE_CLIENT_INPUT_H

class CInput : public IEngineInput
{
	IEngineGraphics *m_pGraphics;
	IEngineGamepad *m_pGamepad;

	int m_MouseModes;

	int m_LastMousePosX;
	int m_LastMousePosY;
	bool m_FirstWarp;
	char *m_pClipboardText;
	bool m_MouseLeft;
	bool m_MouseEntered;
	
	int m_GamepadMove;
	bool m_GamepadDown;
	bool m_GamepadJump;
	bool m_GamepadShoot;
	bool m_GamepadSelect;
	
	bool m_UsingGamepad;
	int m_GamepadAimX;
	int m_GamepadAimY;
	
	int m_GamepadOldAimX;
	int m_GamepadOldAimY;

	void ResetGamepad();
	
	SDL_Surface *m_pCursorSurface;
	SDL_Cursor *m_pCursor;

	int64 m_LastRelease;
	int64 m_ReleaseDelta;

	void AddEvent(int Unicode, int Key, int Flags);

	IEngineGraphics *Graphics() { return m_pGraphics; }
	IEngineGamepad *Gamepad() { return m_pGamepad; }

public:
	CInput();

	virtual void Init();

	virtual void SetMouseModes(int modes);
	virtual int GetMouseModes();
	virtual void GetMousePosition(float *x, float *y);
	virtual void GetRelativePosition(float *x, float *y);
	virtual bool MouseMoved();
	virtual bool GamepadMoved();
	virtual bool UsingGamepad(){ return m_UsingGamepad; }
	virtual int MouseDoubleClick();
	virtual const char* GetClipboardText();
	virtual void SetClipboardText(const char *Text);
	virtual bool MouseLeft();
	virtual bool MouseEntered();

	virtual int ShowCursor(bool show);

	void LoadHardwareCursor();

	void ClearKeyStates();
	int KeyState(int Key);

	int ButtonPressed(int Button) { return m_aInputState[m_InputCurrent][Button]; }

	virtual int Update();
};

#endif
