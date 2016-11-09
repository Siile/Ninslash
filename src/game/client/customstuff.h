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
	int m_LocalTeam;
	
	vec2 m_BuildPos;
	bool m_BuildPosValid;
	bool m_FlipBuilding;
	
	float m_CameraShake;
	
	void SetScreenshake(float Amount)
	{
		if (m_CameraShake < Amount)
			m_CameraShake = Amount;
	}
	
	bool m_LocalAlive;
	vec2 m_LocalPos;
	int m_LocalWeapon;
	vec4 m_LocalColor;
	
	bool m_BuildMode;
	
	float m_MonsterDamageIntensity[MAX_MONSTERS];
	float m_MonsterDamageType[MAX_MONSTERS];
	
	int m_FlametrapState[64];
	int m_FlametrapSoundTick[64];
	int m_FlametrapLastSound[64];
	
	float m_SawbladeAngle;
	float m_MonsterAnim;
	
	// for weapon picker
	int m_WantedWeapon;
	int m_SelectedGroup;
	
	int m_LocalWeapons;
	int m_LocalKits;
	int m_aLocalItems[NUM_PLAYERITEMS];
	
	// for weapon pick effect
	float m_WeaponpickTimer;
	int m_WeaponpickWeapon;
	bool m_LastWeaponPicked;
	
	float m_WeaponSignalTimer;
	int m_WeaponSignal;
	
	int m_SelectedWeapon;

	int LocalTick(){ return m_Tick; }
	int m_WeaponDropTick;
	int m_SwitchTick;
	
	void SetCameraTarget(vec2 Center){ m_CameraTargetCenter = Center; }
	vec2 GetCameraCenter(){ return m_CameraCenter; }
	
	CCustomStuff();
	
	void Reset();
	
	void Tick(bool Paused);
	void Update(bool Paused = false);
	
	CPlayerInfo m_aPlayerInfo[MAX_CLIENTS];
};



#endif
