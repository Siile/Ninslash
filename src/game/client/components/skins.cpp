

#include <math.h>

#include <base/system.h>
#include <base/math.h>

#include <engine/graphics.h>
#include <engine/storage.h>
#include <engine/shared/config.h>

#include "skins.h"


int CSkins::TopperScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	CSkins *pSelf = (CSkins *)pUser;
	int l = str_length(pName);
	if(l < 4 || IsDir || str_comp(pName+l-4, ".png") != 0)
		return 0;

	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "toppers/%s", pName);
	CImageInfo Info;
	if(!pSelf->Graphics()->LoadPNG(&Info, aBuf, DirType))
	{
		str_format(aBuf, sizeof(aBuf), "failed to load topper from %s", pName);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
		return 0;
	}

	CTopper Topper;
	Topper.m_Texture = pSelf->Graphics()->LoadTextureRaw(Info.m_Width, Info.m_Height, Info.m_Format, Info.m_pData, Info.m_Format, 0);
	mem_free(Info.m_pData);

	// set skin data
	str_copy(Topper.m_aName, pName, min((int)sizeof(Topper.m_aName),l-3));
	if(g_Config.m_Debug)
	{
		str_format(aBuf, sizeof(aBuf), "load topper %s", Topper.m_aName);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
	}
	pSelf->m_aToppers.add(Topper);

	return 0;
}

int CSkins::EyeScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	CSkins *pSelf = (CSkins *)pUser;
	int l = str_length(pName);
	if(l < 4 || IsDir || str_comp(pName+l-4, ".png") != 0)
		return 0;

	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "eyes/%s", pName);
	CImageInfo Info;
	if(!pSelf->Graphics()->LoadPNG(&Info, aBuf, DirType))
	{
		str_format(aBuf, sizeof(aBuf), "failed to load eye from %s", pName);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
		return 0;
	}

	CEye Eye;
	Eye.m_Texture = pSelf->Graphics()->LoadTextureRaw(Info.m_Width, Info.m_Height, Info.m_Format, Info.m_pData, Info.m_Format, 0);
	mem_free(Info.m_pData);

	// set skin data
	str_copy(Eye.m_aName, pName, min((int)sizeof(Eye.m_aName),l-3));
	if(g_Config.m_Debug)
	{
		str_format(aBuf, sizeof(aBuf), "load eye %s", Eye.m_aName);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
	}
	pSelf->m_aEyes.add(Eye);

	return 0;
}


void CSkins::OnInit()
{
	// load toppers
	m_aToppers.clear();
	Storage()->ListDirectory(IStorage::TYPE_ALL, "toppers", TopperScan, this);
	if(!m_aToppers.size())
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "gameclient", "failed to load toppers. folder='toppers/'");
		CTopper DummySkin;
		DummySkin.m_Texture = -1;
		str_copy(DummySkin.m_aName, "dummy", sizeof(DummySkin.m_aName));
		m_aToppers.add(DummySkin);
	}
	
	// load eyes
	m_aEyes.clear();
	Storage()->ListDirectory(IStorage::TYPE_ALL, "eyes", EyeScan, this);
	if(!m_aEyes.size())
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "gameclient", "failed to load eyes. folder='eyes/'");
		CEye DummySkin;
		DummySkin.m_Texture = -1;
		str_copy(DummySkin.m_aName, "dummy", sizeof(DummySkin.m_aName));
		m_aEyes.add(DummySkin);
	}
}


int CSkins::NumToppers()
{
	return m_aToppers.size();
}

int CSkins::NumEyes()
{
	return m_aEyes.size();
}


const CSkins::CTopper *CSkins::GetTopper(int Index)
{
	return &m_aToppers[max(0, Index%m_aToppers.size())];
}

const CSkins::CEye *CSkins::GetEye(int Index)
{
	return &m_aEyes[max(0, Index%m_aEyes.size())];
}


int CSkins::FindTopper(const char *pName)
{
	for(int i = 0; i < m_aToppers.size(); i++)
	{
		if(str_comp(m_aToppers[i].m_aName, pName) == 0)
			return i;
	}
	return -1;
}

int CSkins::FindEye(const char *pName)
{
	for(int i = 0; i < m_aEyes.size(); i++)
	{
		if(str_comp(m_aEyes[i].m_aName, pName) == 0)
			return i;
	}
	return -1;
}

vec3 CSkins::GetColorV3(int v)
{
	return HslToRgb(vec3(((v>>16)&0xff)/255.0f, ((v>>8)&0xff)/255.0f, 0.5f+(v&0xff)/255.0f*0.5f));
}

vec4 CSkins::GetColorV4(int v)
{
	vec3 r = GetColorV3(v);
	return vec4(r.r, r.g, r.b, 1.0f);
}
