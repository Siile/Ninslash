#include <base/system.h>
#include <base/math.h>

#include <engine/shared/config.h>
#include <engine/storage.h>
#include <engine/shared/linereader.h>
#include <engine/shared/linewriter.h>

#include <stdio.h>

#include "playerdata.h"

CPlayerData::CPlayerData(const char *pName, int ColorID, IStorage *pStorage)
{
	m_pChild1 = 0;
	m_pChild2 = 0;
	
	str_copy(m_aName, pName, 16);
	m_ColorID = ColorID;
	
	m_pStorage = pStorage;
	LoadDataFromFile();
	Reset();
}

void CPlayerData::Die()
{
}

void CPlayerData::Add(CPlayerData *pPlayerData)
{
	int Color = pPlayerData->m_ColorID;
	
	if (Color < m_ColorID)
	{
		if (m_pChild1)
			m_pChild1->Add(pPlayerData);
		else
			m_pChild1 = pPlayerData;
	}
	else
	{
		if (m_pChild2)
			m_pChild2->Add(pPlayerData);
		else
			m_pChild2 = pPlayerData;
	}
}

CPlayerData *CPlayerData::Get(const char *pName, int ColorID)
{
	if (ColorID == m_ColorID)
	{
		if(str_comp(pName, m_aName) == 0)
			return this;
	}
	
	if (ColorID < m_ColorID)
	{
		if (m_pChild1)
			return m_pChild1->Get(pName, ColorID);
		else
			return 0;
	}
	else
	{
		if (m_pChild2)
			return m_pChild2->Get(pName, ColorID);
		else
			return 0;
	}
	
	return 0;
}


int CPlayerData::GetHighScore(int Score)
{
	int s = max(m_HighestLevel, Score);

	if (m_pChild1)
		s = max(s, m_pChild1->GetHighScore(s));

	if (m_pChild2)
		s = max(s, m_pChild2->GetHighScore(s));

	return s;
}

int CPlayerData::GetPlayerCount(int Score)
{

	if (m_pChild1)
		Score += m_pChild1->GetPlayerCount(0);

	if (m_pChild2)
		Score += m_pChild2->GetPlayerCount(0);

	return Score+1;
}


void CPlayerData::Reset()
{
	for (int i = 0; i < 99; i++)
	{
		m_aWeaponType[i] = 0;
		m_aWeaponAmmo[i] = 0;
		
		m_aAmmo[i] = -1;
	}
	
	m_Armor = 0;
	m_Weapon = 0;
	m_Kits = 0;
	m_Score = 0;
	m_Gold = 0;
	m_HighestLevel = 0;
	m_HighestLevelSeed = 0;
}

void CPlayerData::LoadDataFromFile()
{
	if (!g_Config.m_SvSavePlayerdata)
		return;

	char aFilename[32];
	str_format(aFilename, sizeof(aFilename), "playerdatas/%s_%d.acc", m_aName, m_ColorID);

	IOHANDLE File = m_pStorage->OpenFile(aFilename, IOFLAG_READ, IStorage::TYPE_ALL);
	if(!File)
	{
		SaveToFile(); // Create file
		return;
	}
	CLineReader LineReader;
	LineReader.Init(File);
 
	// read each line
	while(char *pLine = LineReader.Get())
	{
		// skip blank/empty lines as well as comments
		if(str_length(pLine) > 0 && pLine[0] != '#' && pLine[0] != '\n' && pLine[0] != '\r'
			&& pLine[0] != '\t' && pLine[0] != '\v' && pLine[0] != ' ')
		{
			if(!str_comp_num(pLine, "Armor: ", 7)) sscanf(pLine, "Armor: %d", &m_Armor);
			if(!str_comp_num(pLine, "Kits: ", 6)) sscanf(pLine, "Kits: %d", &m_Kits);
			if(!str_comp_num(pLine, "Score: ", 7)) sscanf(pLine, "Score: %d", &m_Score);
			if(!str_comp_num(pLine, "Gold: ", 6)) sscanf(pLine, "Gold: %d", &m_Gold);
			if(!str_comp_num(pLine, "HighestLevelSeed: ", 18)) sscanf(pLine, "HighestLevelSeed: %d", &m_HighestLevelSeed);
			else if(!str_comp_num(pLine, "HighestLevel: ", 14)) sscanf(pLine, "HighestLevel: %d", &m_HighestLevel);
		}
	}


	io_close(File);
}

void CPlayerData::SaveToFile()
{
	if (!g_Config.m_SvSavePlayerdata)
		return;

	m_pStorage->CreateFolder("playerdatas", IStorage::TYPE_SAVE);

	char aFilename[32];
	str_format(aFilename, sizeof(aFilename), "playerdatas/%s_%d.acc", m_aName, m_ColorID);

	IOHANDLE File = m_pStorage->OpenFile(aFilename, IOFLAG_WRITE, IStorage::TYPE_SAVE);
	if(!File)
		return;

	CLineWriter Writer(File);

	char aBuf[256];
	
#define WRITE_LINE_INT(w, x) \
	do \
	{ \
		str_format(aBuf, sizeof(aBuf), "%d", x); \
		(w).Write(aBuf); \
	} while(0)

	// Warning! Weapon Type change 
	Writer.Write("Weapon:");
	for (int i = 0; i < 12; i++)
	{
		Writer.Write(" ");
		WRITE_LINE_INT(Writer, m_aWeaponType[i]);
	}
	Writer.Write("\nAmmo:");
	for (int i = 0; i < 12; i++)
	{
		Writer.Write(" ");
		WRITE_LINE_INT(Writer, m_aWeaponAmmo[i]);
	}

	Writer.Write("\nArmor: "); WRITE_LINE_INT(Writer, m_Armor);

	Writer.Write("\nKits: "); WRITE_LINE_INT(Writer, m_Kits);

	Writer.Write("\nScore: "); WRITE_LINE_INT(Writer, m_Score);

	Writer.Write("\nGold: "); WRITE_LINE_INT(Writer, m_Gold);

	Writer.Write("\nHighestLevel: "); WRITE_LINE_INT(Writer, m_HighestLevel);

	Writer.Write("\nHighestLevelSeed: "); WRITE_LINE_INT(Writer, m_HighestLevelSeed);
}