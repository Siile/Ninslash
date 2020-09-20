#ifndef GAME_SERVER_AISKIN_H
#define GAME_SERVER_AISKIN_H

struct CAISkin
{
	char m_aName[24];
	char m_aHead[24];
	char m_aBody[24];
	char m_aHand[24];
	char m_aFoot[24];
	char m_aTopper[24];
	char m_aEye[24];
	
	bool m_Valid;
	
	int m_ColorSkin;
	int m_ColorTopper;
	int m_ColorBody;
	int m_ColorFoot;
	int m_ColorBlood;
	int m_Level;
	
	CAISkin()
	{
		m_Valid = false;
		m_ColorSkin = 65535;
		m_ColorTopper = 1820416;
		m_ColorBody = 65535;
		m_ColorFoot = 255;
		m_ColorBlood = 0;
		m_Level = 0;
	}
};

#endif