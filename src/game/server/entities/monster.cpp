#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "monster.h"


CMonster::CMonster(CGameWorld *pGameWorld, vec2 Pos)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_MONSTER)
{
	m_ProximityRadius = MonsterPhysSize;

	m_StartPos = Pos;
	
	Reset();
	GameWorld()->InsertEntity(this);

}

void CMonster::Reset()
{
	m_Health = 30;
	m_Pos = m_StartPos;
	m_Status = MONSTERSTATUS_IDLE;
	m_Dir = -1;
	m_DeathTick = 0;
}



void CMonster::TakeDamage(vec2 Force, int Dmg, int From, vec2 Pos, int Type)
{
	// skip everything while spawning
	//if (m_aStatus[STATUS_SPAWNING] > 0.0f)
	//	return false;
	

	
	// create healthmod indicator
	if (Type == DAMAGETYPE_NORMAL)
	{
		vec2 DmgPos = m_Pos + vec2(0, -12);
		if (Pos.x != 0 && Pos.y != 0)
			DmgPos = Pos;
		//	DmgPos = (DmgPos + Pos) / 2.0f;
		
		if(Server()->Tick() < m_DamageTakenTick+25)
		{
			GameServer()->CreateDamageInd(DmgPos, GetAngle(-Force), Dmg);
		}
		else
		{
			GameServer()->CreateDamageInd(DmgPos, GetAngle(-Force), Dmg);
		}
		m_Status = MONSTERSTATUS_HURT;
	}
	else
	if (Type == DAMAGETYPE_ELECTRIC)
	{
		//GameServer()->SendEffect(m_pPlayer->GetCID(), EFFECT_ELECTRODAMAGE);
		m_Status = MONSTERSTATUS_ELECTRIC;
	}
	
	m_Health -= Dmg;
	

	// do damage Hit sound
	/*
	if(From >= 0 && From != m_pPlayer->GetCID() && GameServer()->m_apPlayers[From])
	{
		GameServer()->m_apPlayers[From]->m_InterestPoints += Dmg * 5;
		
		int Mask = CmaskOne(From);
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS && GameServer()->m_apPlayers[i]->m_SpectatorID == From)
				Mask |= CmaskOne(i);
		}
		GameServer()->CreateSound(GameServer()->m_apPlayers[From]->m_ViewPos, SOUND_HIT, Mask);
	}
	*/

	
	// check for death
	if(m_Health <= 0)
	{
		// set attacker's face to happy (taunt!)
		if (From >= 0 && GameServer()->m_apPlayers[From])
		{
			CCharacter *pChr = GameServer()->m_apPlayers[From]->GetCharacter();
			if (pChr)
				pChr->SetEmote(EMOTE_HAPPY, Server()->Tick() + Server()->TickSpeed());
		}

		GameServer()->CreateEffect(FX_MONSTERDEATH, m_Pos);
		m_DeathTick = Server()->Tick();
		
		// random pickup drop
		if (frandom()*10 < 4)
			GameServer()->m_pController->DropPickup(m_Pos + vec2(0, -42), POWERUP_ARMOR, Force+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);
		else
			GameServer()->m_pController->DropPickup(m_Pos + vec2(0, -42), POWERUP_HEALTH, Force+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);
		
		return;
	}

	GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_SHORT);
	m_DamageTakenTick = Server()->Tick();
}



void CMonster::Tick()
{
	if (m_Health <= 0)
	{
		if (m_DeathTick < Server()->Tick() - Server()->TickSpeed() * 30)
		{
			Reset();
			GameServer()->CreateEffect(FX_MONSTERSPAWN, m_Pos);
		}
		return;
	}
	
	if (m_Dir == -1)
	{
		if (GameServer()->Collision()->IsTileSolid(m_Pos.x-32, m_Pos.y-8) || !GameServer()->Collision()->IsTileSolid(m_Pos.x-32, m_Pos.y+18))
			m_Dir = 1;
		
		m_Pos.x -= 0.5f;
	}
	else if (m_Dir == 1)
	{
		if (GameServer()->Collision()->IsTileSolid(m_Pos.x+32, m_Pos.y-8) || !GameServer()->Collision()->IsTileSolid(m_Pos.x+32, m_Pos.y+18))
			m_Dir = -1;
		
		m_Pos.x += 0.5f;
	}
	
	if(Server()->Tick() > m_DamageTakenTick+15)
		m_Status = MONSTERSTATUS_IDLE;
	
	GameServer()->m_World.m_Core.AddMonster(m_Pos);
}



void CMonster::TickPaused()
{
	//if(m_SpawnTick != -1)
	//	++m_SpawnTick;
}

void CMonster::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient) || m_Health <= 0)
		return;

	CNetObj_Monster *pP = static_cast<CNetObj_Monster *>(Server()->SnapNewItem(NETOBJTYPE_MONSTER, m_ID, sizeof(CNetObj_Monster)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Status = m_Status;
	pP->m_Dir = m_Dir;
}
