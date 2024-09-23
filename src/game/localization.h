

#ifndef GAME_LOCALIZATION_H
#define GAME_LOCALIZATION_H
#include <base/tl/string.h>
#include <base/tl/sorted_array.h>

// Don't edit it if you don't know what this do.
/*
Localize("Block"),Localize("Hard block"),Localize("Barrel"),Localize("Power barrel")
Localize("Turret stand"),Localize("Flamer"),Localize("Electric wall"),Localize("Teslacoil")
Localize("Shield generator"),Localize("Repair tool"),Localize("Grenade"),Localize("Electric grenade")
Localize("Supply grenade"),Localize("Weapon upgrade"),Localize("Energy shield"),Localize("Respawn device")
Localize("Mask of regeneration"),Localize("Mask of speed"),Localize("Mask of protection"),Localize("Mask of plenty")
Localize("Mask of melee"),Localize("Invisibility device"),Localize("Electrowall")
Localize("Area Shield"),Localize("The Cure"),Localize("Zombie claw")
Localize("Bomb (for destroying reactors)"),Localize("Terminate the enemies"),Localize("Reach the door"),
Localize("Survive the wave of enemies"),
*/

class CLocalizationDatabase
{
	class CString
	{
	public:
		unsigned m_Hash;

		// TODO: do this as an const char * and put everything on a incremental heap
		string m_Replacement;

		bool operator <(const CString &Other) const { return m_Hash < Other.m_Hash; }
		bool operator <=(const CString &Other) const { return m_Hash <= Other.m_Hash; }
		bool operator ==(const CString &Other) const { return m_Hash == Other.m_Hash; }
	};

	sorted_array<CString> m_Strings;
	int m_VersionCounter;
	int m_CurrentVersion;

public:
	CLocalizationDatabase();

	bool Load(const char *pFilename, class IStorage *pStorage, class IConsole *pConsole);

	int Version() { return m_CurrentVersion; }

	void AddString(const char *pOrgStr, const char *pNewStr);
	const char *FindString(unsigned Hash);
};

extern CLocalizationDatabase g_Localization;

class CLocConstString
{
	const char *m_pDefaultStr;
	const char *m_pCurrentStr;
	unsigned m_Hash;
	int m_Version;
public:
	CLocConstString(const char *pStr);
	void Reload();

	inline operator const char *()
	{
		if(m_Version != g_Localization.Version())
			Reload();
		return m_pCurrentStr;
	}
};


extern const char *Localize(const char *pStr);
#endif
