#ifndef GAME_CLIENT_CUSTOMSTUFF_H
#define GAME_CLIENT_CUSTOMSTUFF_H

#include <base/system.h>

#include <game/generated/protocol.h>
#include <game/generated/client_data.h>
#include <game/client/gameclient.h>
#include <game/weapons.h>
#include "customstuff/playerinfo.h"

enum Pickers
{
	PICKER_WEAPON,
	PICKER_ITEM,
	PICKER_TOOL,
	PICKER_EMOTICON,
};

class CCustomStuff
{
private:
	// local tick for helping rendering physics & stuff
	int m_Tick;
	
	int64 m_LastUpdate;
	
	vec2 m_CameraTargetCenter;
	vec2 m_CameraCenter;
	
public:
	int GetSpriteFrame(int Speed, int Range)
	{
		return (m_Tick / Speed)%Range;
	}

	int m_Picker;
	
	float m_MonsterDamageIntensity[MAX_MONSTERS];
	float m_MonsterDamageType[MAX_MONSTERS];
	
	float m_SawbladeAngle;
	float m_MonsterAnim;
	
	// for weapon picker
	int m_WantedWeapon;
	int m_SelectedGroup;
	
	int m_LocalWeapons;
	int m_aLocalItems[NUM_PLAYERITEMS];
	
	// for weapon pick effect
	float m_WeaponpickTimer;
	int m_WeaponpickWeapon;
	bool m_LastWeaponPicked;
	
	int m_SelectedWeapon;

	int LocalTick(){ return m_Tick; }
	
	void SetCameraTarget(vec2 Center){ m_CameraTargetCenter = Center; }
	vec2 GetCameraCenter(){ return m_CameraCenter; }
	
	CCustomStuff();
	
	void Reset();
	
	void Tick(bool Paused);
	void Update(bool Paused = false);
	
	CPlayerInfo m_aPlayerInfo[MAX_CLIENTS];
};



#endif