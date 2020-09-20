#include "customstuff.h"

#include <game/client/components/tracer.h>
#include <game/client/components/inventory.h>



CCustomStuff::CCustomStuff(CGameClient *pClient)
{
	m_pClient = pClient;
	
	for (int i = 0; i < MAX_DROIDS; i++)
		m_apDroidAnim[i] = NULL;
	
	Reset();
	m_CameraTargetCenter = vec2(0, 0);
	m_CameraCenter = vec2(0, 0);
	m_SelectedGroup = 0;
}

CCustomStuff::~CCustomStuff()
{
	for (int i = 0; i < MAX_DROIDS; i++)
		if (m_apDroidAnim[i])
			delete m_apDroidAnim[i];
}


void CCustomStuff::Reset()
{
	m_ChargeAngle = 0;
	m_Inventory = false;
	
	for (int i = 0; i < 12; i++)
		m_aItem[i] = 0;
	
	m_Gold = 0;
	
	/*
	int w = 1<<0 | 1<<5;
	m_aItem[0] = w;
	w = 0; w |= 1<<1; w |= 1<<6;
	m_aItem[1] = w;
	w = 0; w |= 1<<2; w |= 1<<7;
	m_aItem[2] = w;
	w = 0; w |= 1<<6;
	m_aItem[3] = w;
	*/
	
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
	
	m_WeaponSlot = 0;
	for (int i = 0; i < 4; i++)
		m_aSnapWeapon[i] = -1;
	
	m_LocalKits = 0;
	m_Picker = 0;
	m_LastUpdate = time_get();
	m_Tick = 0;
	
	m_LocalTeam = TEAM_SPECTATORS;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
		m_aPlayerInfo[i].Reset();
	
	for (int i = 0; i < MAX_DROIDS; i++)
		if (m_apDroidAnim[i])
			m_apDroidAnim[i]->Reset();
	
	m_WantedWeapon = 1;
	m_SawbladeAngle = 0;
	m_MonsterAnim = 0;
	
	for (int i = 0; i < MAX_DROIDS; i++)
	{
		m_DroidDamageIntensity[i] = 0.0f;
		m_DroidDamageType[i] = 0;
	}
	
	for (int i = 0; i < 512; i++)
	{
		m_aTurretFlame[i] = 0;
		m_aJumppad[i] = 0;
	}
	
	for (int i = 0; i < 64; i++)
	{
		m_FlametrapState[i] = 0;
		m_FlametrapSoundTick[i] = 0;
		m_FlametrapLastSound[i] = 0;
	}

	ClearImpacts();
	
	for (int i = 0; i < MAX_IMPACTS; i++)
		m_aImpactTick[i] = 0;
	
	for (int i = 0; i < MAX_TURRETMUZZLES; i++)
		m_aTurretMuzzle[i].Reset();
	
	m_WeaponpickTimer = 0;
	m_WeaponpickWeapon = 0;
	
	m_WeaponSignalTimer = 0;
	m_WeaponSignal = -1;
	
	m_LastWeaponPicked = true;
	
	m_BuildPos = vec2(0, 0);
	m_BuildPosValid = false;
	m_FlipBuilding = false;
}

void CCustomStuff::SetTurretMuzzle(ivec2 Pos, int AttackTick, int Weapon)
{
	if (!AttackTick)
		return;
	
	// find existing one
	for (int i = 0; i < MAX_TURRETMUZZLES; i++)
	{
		if (m_aTurretMuzzle[i].m_Pos.x == Pos.x && m_aTurretMuzzle[i].m_Pos.y == Pos.y)
		{
			if (AttackTick && m_aTurretMuzzle[i].m_AttackTick != AttackTick)
			{
				m_aTurretMuzzle[i].m_Time = 0.0f;
				m_aTurretMuzzle[i].m_AttackTick = AttackTick;
				m_aTurretMuzzle[i].m_Weapon = Weapon;
				m_aTurretMuzzle[i].m_Muzzle = rand()%4;
			}
			
			return;
		}
	}

	// .. or a new one
	for (int i = 0; i < MAX_TURRETMUZZLES; i++)
	{
		if (m_aTurretMuzzle[i].m_Weapon == 0)
		{
			m_aTurretMuzzle[i].m_Pos = Pos;
			m_aTurretMuzzle[i].m_Time = 0.0f;
			m_aTurretMuzzle[i].m_AttackTick = AttackTick;
			m_aTurretMuzzle[i].m_Weapon = Weapon;
			m_aTurretMuzzle[i].m_Muzzle = rand()%4;
			return;
		}
	}
}


CTurretMuzzle CCustomStuff::GetTurretMuzzle(ivec2 Pos)
{
	for (int i = 0; i < MAX_TURRETMUZZLES; i++)
		if (m_aTurretMuzzle[i].m_Pos.x == Pos.x && m_aTurretMuzzle[i].m_Pos.y == Pos.y)
			return m_aTurretMuzzle[i];
	
	return CTurretMuzzle();
}



float CCustomStuff::ChargeIntensity(int Charge)
{
	return (0.8f+cos(m_ChargeAngle*4.0f)*0.2f)*min(1.0f, Charge*0.01f);
}


CDroidAnim *CCustomStuff::GetDroidAnim(int Index)
{
	int i = Index%MAX_DROIDS;
	if (!m_apDroidAnim[i])
		m_apDroidAnim[i] = new CDroidAnim(m_pClient);
	
	return m_apDroidAnim[i];
}


bool CCustomStuff::IsBot(int ClientID)
{
	if (ClientID < 0 && ClientID >= MAX_CLIENTS)
		return false;
	
	return m_aPlayerInfo[ClientID].m_RenderInfo.m_IsBot;
}

vec4 CCustomStuff::BloodColor(int ClientID)
{
	//if (ClientID >= 0 && ClientID < MAX_CLIENTS)
	//	return m_aPlayerInfo[ClientID].m_RenderInfo.m_ColorSkin;

	if (ClientID < 0 && ClientID >= MAX_CLIENTS)
		return vec4(1, 0, 0, 1);

	int c = m_aPlayerInfo[ClientID].m_RenderInfo.m_BloodColor;
	
	switch (c)
	{
		case 0: return vec4(1, 0, 0, 1);
		case 1: return vec4(0, 1, 0, 1);
		case 2: return vec4(0, 0, 1, 1);
	};

	//if (ClientID%2)
	//	return vec4(0, 1, 0, 1);
	
	return vec4(1, 0, 0, 1);
}


void CCustomStuff::Tick(bool Paused)
{
	if (Paused)
		return;
	
	m_Tick++;

	// Client side building animation
	m_SawbladeAngle += 0.07f;
	
	if (m_WeaponpickTimer > 0.0f)
	{
		m_WeaponpickTimer -= 0.0035f;
		
		if (m_LastWeaponPicked)
			m_WeaponpickTimer -= 0.0035f;
	}
	
	m_ChargeAngle += 0.1f;
	
	if (m_DoorTimer > 0.0f)
	{
		m_DoorTimer += 0.01f;
	
		if (m_DoorTimer > 1.0f)
			m_DoorTimer = 0.0f;
	}
	
	if (m_WeaponSignalTimer > 0.0f)
		m_WeaponSignalTimer -= 0.035f;
	
	m_MonsterAnim += 0.025f;
	
	//if (m_MonsterAnim >= 1.0f)
	//	m_MonsterAnim -= 1.0f;
	
	if (!Paused)
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
			m_aPlayerInfo[i].Tick();
		
		for (int i = 0; i < MAX_DROIDS; i++)
			if (m_apDroidAnim[i])
				m_apDroidAnim[i]->Tick();
		
		for (int i = 0; i < MAX_DROIDS; i++)
			if (m_DroidDamageIntensity[i] > 0.0f)
				m_DroidDamageIntensity[i] -= 0.05f;
			
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
		
		// turret muzzle
		for (int i = 0; i < MAX_TURRETMUZZLES; i++)
		{
			if (m_aTurretMuzzle[i].m_Weapon)
			{
				m_aTurretMuzzle[i].m_Time += 0.15f;
				
				if (m_aTurretMuzzle[i].m_Time > 1.0f)
					m_aTurretMuzzle[i].m_Weapon = 0;
			}
		}
	}
	
	m_pClient->m_pTracers->Tick();
	m_pClient->m_pInventory->Tick();
	
	// Camera
	
	m_CameraCenter.x += (m_CameraTargetCenter.x-m_CameraCenter.x) / 24.0f;
	m_CameraCenter.y += (m_CameraTargetCenter.y-m_CameraCenter.y) / 24.0f;
	
	if (m_CameraShake > 0.0f)
	{
		m_CameraShake *= 0.95f;
		m_CameraShake -= 0.22f;
	}
	
	// building
	if (m_LocalTeam == TEAM_SPECTATORS)
	{
		m_BuildMode = false;
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
		if (i++ > 3)
		{
			m_LastUpdate = currentTime;
			break;
		}
	}
	
}
