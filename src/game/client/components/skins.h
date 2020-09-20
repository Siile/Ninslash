#ifndef GAME_CLIENT_COMPONENTS_SKINS_H
#define GAME_CLIENT_COMPONENTS_SKINS_H
#include <base/vmath.h>
#include <base/tl/sorted_array.h>
#include <game/client/component.h>

class CSkins : public CComponent
{
public:
	struct CSkinPart
	{
		int m_Texture;
		char m_aName[24];

		bool operator<(const CSkinPart &Other) { return str_comp(m_aName, Other.m_aName) < 0; }
	};

	void OnInit();

	vec3 GetColorV3(int v);
	vec4 GetColorV4(int v);
	
	int NumToppers();
	int NumEyes();
	int NumHeads();
	int NumBodies();
	int NumHands();
	int NumFeet();
	
	int NumGameVotes();
	
	const CSkinPart *GetTopper(int Index);
	const CSkinPart *GetEye(int Index);
	const CSkinPart *GetHead(int Index);
	const CSkinPart *GetBody(int Index);
	const CSkinPart *GetHand(int Index);
	const CSkinPart *GetFoot(int Index);
	
	const CSkinPart *GetGameVote(int Index);
	
	int FindTopper(const char *pName);
	int FindEye(const char *pName);
	int FindHead(const char *pName);
	int FindBody(const char *pName);
	int FindHand(const char *pName);
	int FindFoot(const char *pName);
	
	int FindGameVote(const char *pName);

private:
	sorted_array<CSkinPart> m_aToppers;
	sorted_array<CSkinPart> m_aEyes;
	sorted_array<CSkinPart> m_aHeads;
	sorted_array<CSkinPart> m_aBodies;
	sorted_array<CSkinPart> m_aHands;
	sorted_array<CSkinPart> m_aFeet;
	sorted_array<CSkinPart> m_aGameVote;

	static int TopperScan(const char *pName, int IsDir, int DirType, void *pUser);
	static int EyeScan(const char *pName, int IsDir, int DirType, void *pUser);
	static int HeadScan(const char *pName, int IsDir, int DirType, void *pUser);
	static int BodyScan(const char *pName, int IsDir, int DirType, void *pUser);
	static int HandScan(const char *pName, int IsDir, int DirType, void *pUser);
	static int FootScan(const char *pName, int IsDir, int DirType, void *pUser);
	
	static int GameVoteScan(const char *pName, int IsDir, int DirType, void *pUser);
};
#endif
