#ifndef GAME_CLIENT_COMPONENTS_SKINS_H
#define GAME_CLIENT_COMPONENTS_SKINS_H
#include <base/vmath.h>
#include <base/tl/sorted_array.h>
#include <game/client/component.h>

class CSkins : public CComponent
{
public:
	struct CSkin
	{
		int m_OrgTexture;
		int m_ColorTexture;
		char m_aName[24];
		vec3 m_BloodColor;

		bool operator<(const CSkin &Other) { return str_comp(m_aName, Other.m_aName) < 0; }
	};
	
	struct CTopper
	{
		int m_Texture;
		char m_aName[24];

		bool operator<(const CTopper &Other) { return str_comp(m_aName, Other.m_aName) < 0; }
	};
	
	struct CEye
	{
		int m_Texture;
		char m_aName[24];

		bool operator<(const CEye &Other) { return str_comp(m_aName, Other.m_aName) < 0; }
	};

	void OnInit();

	vec3 GetColorV3(int v);
	vec4 GetColorV4(int v);
	int Num();
	int NumToppers();
	int NumEyes();
	const CSkin *Get(int Index);
	const CTopper *GetTopper(int Index);
	const CEye *GetEye(int Index);
	int Find(const char *pName);
	int FindTopper(const char *pName);
	int FindEye(const char *pName);

private:
	sorted_array<CSkin> m_aSkins;
	sorted_array<CTopper> m_aToppers;
	sorted_array<CEye> m_aEyes;

	static int SkinScan(const char *pName, int IsDir, int DirType, void *pUser);
	static int TopperScan(const char *pName, int IsDir, int DirType, void *pUser);
	static int EyeScan(const char *pName, int IsDir, int DirType, void *pUser);
};
#endif
