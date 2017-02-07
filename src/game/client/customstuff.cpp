#include "customstuff.h"



CCustomStuff::CCustomStuff()
{
	Reset();
	m_CameraTargetCenter = vec2(0, 0);
	m_CameraCenter = vec2(0, 0);
	m_SelectedGroup = 0;
}


void CCustomStuff::Reset()
{
	m_Local.m_Buff = -1;
	
	m_DoorTimer = 0.0f;
	m_CameraShake = 0.0f;
	
	m_BuildMode = false;
	m_LocalPos = vec2(0, 0);
	m_LocalWeapon = 1;
	m_LocalColor = vec4(0, 0, 0, 0);
	m_LocalAlive = false;
	m_LatestWeapon = 1;
	
	m_WeaponDropTick = 0;
	m_SwitchTick = 0;
	m_SelectedWeapon = 0;
	m_LocalWeapons = 0;
	m_LocalKits = 0;
	m_Picker = 0;
	m_LastUpdate = time_get();
	m_Tick = 0;
	
	m_LocalTeam = TEAM_SPECTATORS;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
		m_aPlayerInfo[i].Reset();
	
	m_WantedWeapon = 1;
	m_SawbladeAngle = 0;
	m_MonsterAnim = 0;
	
	for (int i = 0; i < MAX_MONSTERS; i++)
	{
		m_MonsterDamageIntensity[i] = 0.0f;
		m_MonsterDamageType[i] = 0;
	}
	
	for (int i = 0; i < 512; i++)
	{
		m_aTurretFlame[i] = 0;
		m_aJumppad[i] = 0;
	}
	
	for (int i = 0; i < NUM_PLAYERITEMS; i++)
		m_aLocalItems[i] = 0;
	
	for (int i = 0; i < 64; i++)
	{
		m_FlametrapState[i] = 0;
		m_FlametrapSoundTick[i] = 0;
		m_FlametrapLastSound[i] = 0;
	}

	ClearImpacts();
	
	for (int i = 0; i < MAX_IMPACTS; i++)
		m_aImpactTick[i] = 0;
	
	m_WeaponpickTimer = 0;
	m_WeaponpickWeapon = 0;
	
	m_WeaponSignalTimer = 0;
	m_WeaponSignal = -1;
	
	m_LastWeaponPicked = true;
	
	m_BuildPos = vec2(0, 0);
	m_BuildPosValid = false;
	m_FlipBuilding = false;
}





void CCustomStuff::Tick(bool Paused)
{
	m_Tick++;
	
	if (!m_LocalAlive)
		m_Local.m_Buff = -1;

	// Client side building animation
	m_SawbladeAngle += 0.07f;
	
	if (m_WeaponpickTimer > 0.0f)
	{
		m_WeaponpickTimer -= 0.0035f;
		
		if (m_LastWeaponPicked)
			m_WeaponpickTimer -= 0.0035f;
	}
	
	
	if (m_DoorTimer > 0.0f)
		m_DoorTimer += 0.01f;
	
	if (m_DoorTimer > 1.0f)
		m_DoorTimer = 0.0f;
	
	if (m_WeaponSignalTimer > 0.0f)
		m_WeaponSignalTimer -= 0.035f;
	
	m_MonsterAnim += 0.006f;
	m_MonsterAnim += 0.006f;
	//if (m_MonsterAnim >= 1.0f)
	//	m_MonsterAnim -= 1.0f;
	
	if (!Paused)
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
			m_aPlayerInfo[i].Tick();
		
		for (int i = 0; i < MAX_MONSTERS; i++)
			if (m_MonsterDamageIntensity[i] > 0.0f)
				m_MonsterDamageIntensity[i] -= 0.05f;
			
		for (int i = 0; i < 64; i++)
		{
			if (m_FlametrapState[i] > 0)
			{
				m_FlametrapState[i]++;
				
				if (m_FlametrapState[i] > 13*6)
					m_FlametrapState[i] = 0;
			}
		}
		
		// turret flame frame animation
		for (int i = 0; i < 512; i++)
		{
			if (m_aTurretFlame[i] > 0)
			{
				if (++m_aTurretFlame[i] > 13*5)
					m_aTurretFlame[i] = 0;
			}
			
			if (m_aJumppad[i] > 0)
				m_aJumppad[i] += 0.1f;
		}
	}
	
	// Camera
	
	m_CameraCenter.x += (m_CameraTargetCenter.x-m_CameraCenter.x) / 24.0f;
	m_CameraCenter.y += (m_CameraTargetCenter.y-m_CameraCenter.y) / 24.0f;
	
	if (m_CameraShake > 0.0f)
		m_CameraShake -= 0.2f;
	
	// building
	if (m_LocalTeam == TEAM_SPECTATORS)
	{
		m_BuildMode = false;
		m_Local.m_Buff = -1;
	}
}


void CCustomStuff::Update(bool Paused)
{
	int64 currentTime = time_get();
	if ((currentTime-m_LastUpdate > time_freq()) || (m_LastUpdate == 0))
		m_LastUpdate = currentTime;
		
	int step = time_freq()/120;
	
	if (step <= 0)
		step = 1;
	
	int i = 0;
	for (;m_LastUpdate < currentTime; m_LastUpdate += step)
	{
		Tick(Paused);
		if (i++ > 20)
			break;
	}
	
}
