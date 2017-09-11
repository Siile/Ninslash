#ifndef GAME_CLIENT_COMPONENTS_INVENTORY_H
#define GAME_CLIENT_COMPONENTS_INVENTORY_H
#include <base/vmath.h>
#include <game/client/component.h>

class CInventory : public CComponent
{
	void DrawCircle(float x, float y, float r, int Segments);
	void DrawLayer(vec2 Pos, vec2 Size);
	void DrawInventory(vec2 Pos, vec2 Size);
	void RenderMouse();
	
	bool m_WasActive;
	bool m_Active;
	bool m_Render;

	vec2 m_SelectorMouse;

	static void ConKeyInventory(IConsole::IResult *pResult, void *pUserData);

	bool m_ResetMouse;
	bool m_Mouse1;
	bool m_MouseTrigger;
	
	int m_DragItem;
	
	void Swap(int Item1, int Item2);
	
public:
	CInventory();

	virtual void OnReset();
	virtual void OnConsoleInit();
	virtual void OnRender();
	virtual void OnRelease();
	virtual void OnMessage(int MsgType, void *pRawMsg);
	virtual bool OnMouseMove(float x, float y);
	virtual bool OnInput(IInput::CEvent Event);
	
	void Tick();
};

#endif
