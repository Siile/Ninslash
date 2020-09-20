#ifndef GAME_CLIENT_COMPONENTS_GAMEVOTE_H
#define GAME_CLIENT_COMPONENTS_GAMEVOTE_H
#include <game/client/component.h>

struct CGameVoteDetails
{
	char m_aName[32];
	char m_aDescription[128];
	
	bool m_Valid;
	int m_Texture;
	int m_Votes;
	float m_NameWidth;
	float m_DescriptionWidth;
	float m_VotesWidth;
	
	CGameVoteDetails()
	{
		m_Valid = false;
		m_Texture = -1;
		m_Votes = 0;
	}
};

class CGameVoteDisplay : public CComponent
{
	int m_GameVoteCount;
	CGameVoteDetails m_aGameVoteDetails[6];

	int m_Selected;
	int m_TimeLeft;
	int m_TimeLeftTick;

	bool m_ResetMouse;
	bool m_Mouse1;
	bool m_MouseTrigger;
	bool m_Mouse1Loaded;
	
	vec2 m_MoveStartPos;
	bool m_Moved;
	bool m_MoveTrigger;
	vec2 m_SelectorMouse;
	
	void RenderMouse();
	void SendVote();

public:
	virtual void OnReset();
	virtual void OnRender();
	virtual void OnMessage(int MsgType, void *pRawMsg);
	virtual bool OnMouseMove(float x, float y);
	virtual bool OnInput(IInput::CEvent Event);
	
	bool IsActive() { return m_GameVoteCount > 0; }
};

#endif
