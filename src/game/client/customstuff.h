#ifndef GAME_CLIENT_CUSTOMSTUFF_H
#define GAME_CLIENT_CUSTOMSTUFF_H

#include <base/system.h>

#include <game/generated/protocol.h>
#include <game/generated/client_data.h>
#include <game/client/gameclient.h>
#include <game/weapons.h>
#include "customstuff/playerinfo.h"
#include "customstuff/droidanim.h"

#define MAX_BG_SOUNDS 64

enum Pickers
{
	PICKER_WEAPON,
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
	
	CDroidAnim *m_apDroidAnim[MAX_DROIDS];
	
	float m_ChargeAngle;
	
	//friend class CGameClient;
	CGameClient *m_pClient;
	class CCollision *Collision() const { return m_pClient->Collision(); }
	
public:
	// client prediction for jump pads
	enum Impact
	{
		IMPACT_OFF,
		IMPACT_READY,
		IMPACT_SCYTHE,
		IMPACT_GRENADE,
		IMPACT_HIT,
		MAX_IMPACTSTATES
	};
	
	int64 m_aBGSound[MAX_BG_SOUNDS];
	int64 m_aBGEffect[MAX_BG_SOUNDS];
	
	int m_Impact;
	int m_aImpactState[MAX_IMPACTS];
	int64 m_aImpactTick[MAX_IMPACTS];
	vec4 m_aImpactPos[MAX_IMPACTS];
	vec2 m_aImpactVel[MAX_IMPACTS];

	vec4 BloodColor(int ClientID);
	bool IsBot(int ClientID);
	
	float ChargeIntensity(int Charge);
	
	CDroidAnim *GetDroidAnim(int Index);
	
	void ClearImpacts()
	{
		m_Impact = 0;
		
		for (int i = 0; i < MAX_IMPACTS; i++)
		{
			m_aImpactState[i] = IMPACT_OFF;
			m_aImpactPos[i] = vec4(0, 0, 0, 0);
			m_aImpactVel[i] = vec2(0, 0);
		}
	}
	
	void AddImpact(vec4 Pos, int State, vec2 Vel = vec2(0, -1))
	{
		if (m_Impact < MAX_IMPACTS)
		{
			m_aImpactState[m_Impact] = State;
			m_aImpactPos[m_Impact] = Pos;
			m_aImpactVel[m_Impact] = Vel;
			m_Impact++;
		}
	}
	
	bool Impact(vec2 Pos, vec2 *pVel)
	{
		if (!m_Impact)
			return false;
		
		for (int i = 0; i < m_Impact; i++)
		{
			if ((m_aImpactState[i] == IMPACT_HIT || m_aImpactState[i] == IMPACT_GRENADE || (m_aImpactState[i] == IMPACT_SCYTHE && pVel->y >= 0.0f)) &&
				m_aImpactPos[i].x < Pos.x && m_aImpactPos[i].z > Pos.x &&
				m_aImpactPos[i].y < Pos.y && m_aImpactPos[i].w > Pos.y)
			{
				if (pVel)
					*pVel = m_aImpactVel[i];
				
				if (m_aImpactState[i] == IMPACT_GRENADE)
					pVel->x += (frandom()-frandom())*0.7f;
				
				if (m_aImpactState[i] == IMPACT_SCYTHE)
					pVel->x += (frandom()-frandom())*0.3f;
				
				return true;
			}
		}		
		
		return false;
	}
	
	int GetSpriteFrame(int Speed, int Range)
	{
		return (m_Tick / Speed)%Range;
	}
	
	int m_aTurretFlame[512];
	float m_aJumppad[512];
	
	float m_DoorTimer;
	
	int m_Picker;
	int m_LocalTeam;
	
	int m_LatestWeapon;
	
	vec2 m_BuildPos;
	bool m_BuildPosValid;
	bool m_FlipBuilding;
	
	float m_CameraShake;
	
	void SetScreenshake(float Amount)
	{
		if (m_CameraShake < Amount)
			m_CameraShake = Amount;
	}
	
	struct CLocal
	{
		int m_Buff;
		int m_BuffStartTick;
		
		CLocal()
		{
			m_Buff = -1;
			m_BuffStartTick = 0;
		}
	};
	
	CLocal m_Local;
	
	bool m_LocalAlive;
	vec2 m_LocalPos;
	int m_LocalWeapon;
	vec4 m_LocalColor;
	
	bool m_BuildMode;
	
	// Droids
	float m_DroidDamageIntensity[MAX_DROIDS];
	float m_DroidDamageType[MAX_DROIDS];
	
	int m_FlametrapState[64];
	int m_FlametrapSoundTick[64];
	int m_FlametrapLastSound[64];
	
	float m_SawbladeAngle;
	float m_MonsterAnim;
	
	// for weapon picker
	int m_WantedWeapon;
	int m_SelectedGroup;
	
	int m_WeaponSlot;
	int m_aSnapWeapon[4];
	int m_LocalWeapons;
	int m_LocalUpgrades;
	int m_LocalUpgrades2;
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
	
	CCustomStuff(CGameClient *pClient);
	~CCustomStuff();
	
	void Reset();
	
	void Tick(bool Paused);
	void Update(bool Paused = false);
	
	CPlayerInfo m_aPlayerInfo[MAX_CLIENTS];
};



#endif
