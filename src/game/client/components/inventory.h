#ifndef GAME_CLIENT_COMPONENTS_INVENTORY_H
#define GAME_CLIENT_COMPONENTS_INVENTORY_H
#include <base/math.h>
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

	int m_Tab;
	int m_SelectedBuilding;
	
	static void ConKeyInventory(IConsole::IResult *pResult, void *pUserData);

	bool m_ResetMouse;
	bool m_Mouse1;
	bool m_MouseTrigger;
	bool m_Mouse1Loaded;
	
	vec2 m_MoveStartPos;
	bool m_Moved;
	bool m_MoveTrigger;
	
	int m_DragItem;
	int m_DragPart;
	int m_DragSlot;
	
	void Drop(int Slot);
	void Swap(int Item1, int Item2);
	void Combine(int Item1, int Item2);
	void TakePart(int Item1, int Slot, int Item2);

	void DrawCrafting(int Type, vec2 Pos, float Size);
	void DrawBuildMode();
	
	void MapscreenToGroup(float CenterX, float CenterY, struct CMapItemGroup *PGroup);
	
	bool m_Minimized;
	float m_Scale;
	
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
