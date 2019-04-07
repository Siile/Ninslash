

#include "gamecore.h"

const char *CTuningParams::m_apNames[] =
{
	#define MACRO_TUNING_PARAM(Name,ScriptName,Value) #ScriptName,
	#include "tuning.h"
	#undef MACRO_TUNING_PARAM
};


bool CTuningParams::Set(int Index, float Value)
{
	if(Index < 0 || Index >= Num())
		return false;
	((CTuneParam *)this)[Index] = Value;
	return true;
}

bool CTuningParams::Get(int Index, float *pValue)
{
	if(Index < 0 || Index >= Num())
		return false;
	*pValue = (float)((CTuneParam *)this)[Index];
	return true;
}

bool CTuningParams::Set(const char *pName, float Value)
{
	for(int i = 0; i < Num(); i++)
		if(str_comp_nocase(pName, m_apNames[i]) == 0)
			return Set(i, Value);
	return false;
}

bool CTuningParams::Get(const char *pName, float *pValue)
{
	for(int i = 0; i < Num(); i++)
		if(str_comp_nocase(pName, m_apNames[i]) == 0)
			return Get(i, pValue);

	return false;
}

float VelocityRamp(float Value, float Start, float Range, float Curvature)
{
	if(Value < Start)
		return 1.0f;
	return 1.0f/powf(Curvature, (Value-Start)/Range);
}

CCharacterCore::CCharacterCore()
{
	Init(NULL, NULL);
}

void CCharacterCore::Init(CWorldCore *pWorld, CCollision *pCollision)
{
	m_pWorld = pWorld;
	m_pCollision = pCollision;
}

void CCharacterCore::Reset()
{
	m_DamageTick = 0;
	m_Pos = vec2(0,0);
	m_Vel = vec2(0,0);
	m_JumpTimer = 0;
	m_Jumped = 0;
	m_Sliding = 0;
	m_Jetpack = 0;
	m_JetpackPower = 200;
	m_Wallrun = 0;
	m_Roll = 0;
	m_Slide = 0;
	m_Status = 0;
	m_Status |= 1 << STATUS_SPAWNING;
	m_TriggeredEvents = 0;
	m_Health = 100;
	
	m_Action = 0;
	m_ActionState = 0;
	m_PlayerCollision = false;
	m_MonsterDamage = false;
	m_FluidDamage = false;
	
	m_DashTimer = 0;
	m_DashAngle = 0;
	
	m_ClientID = -1;
	m_KickDamage = -1;
	
	m_HandJetpack = false;
	m_OnWall = false;
	
	m_Direction = 0;
	m_Down = 0;
	m_Charge = 0;
	m_ChargeLevel = 0;
	m_Angle = 0;
	m_Anim = 0;
	m_LockDirection = 0;
}


bool CCharacterCore::IsGrounded()
{
	float PhysSize = 28.0f;
	
	if (m_Sliding)
		return true;
	
	for(int i = -PhysSize/2; i <= PhysSize/2; i++) {
		if(m_pCollision->CheckPoint(m_Pos.x+i, m_Pos.y+PhysSize/2+5, false, PlatformState())) {
			return true;
		}
	}
	
	return false;
}


bool CCharacterCore::PlatformState()
{
	float PhysSize = 28.0f;
	
	if (m_pCollision->IsPlatform(m_Pos.x-PhysSize/2, m_Pos.y+PhysSize/2+32) || m_pCollision->IsPlatform(m_Pos.x+PhysSize/2, m_Pos.y+PhysSize/2+32))
		return m_Slide == 0 && m_Roll == 0 && abs(m_Vel.x) < 5.0f && m_Down;
	
	return true;
}

int CCharacterCore::IsOnForceTile()
{
	float PhysSize = 28.0f;
	
	if (m_Sliding)
		return true;
	
	for(int i = -PhysSize/2; i <= PhysSize/2; i++) {
		if(m_pCollision->IsForceTile(m_Pos.x+i, m_Pos.y+PhysSize/2+5) != 0) {
			return m_pCollision->IsForceTile(m_Pos.x+i, m_Pos.y+PhysSize/2+5);
		}
	}
	
	return false;
}

bool CCharacterCore::IsInFluid()
{
	return m_pCollision->IsInFluid(m_Pos.x, m_Pos.y);
}



int CCharacterCore::SlopeState()
{
	float PhysSize = 28.0f;
	
	int tmp = 0;
	int height_left = 0;
	for(int x = -1; x <= -1; x++)
		for(int y = 0; y <= 4; y++)
			if ( (tmp = m_pCollision->CheckPoint(m_Pos.x-PhysSize/2+x, m_Pos.y+PhysSize/2+y)) > height_left) {
				height_left = tmp;
			}
		
	
	int height_right = 0;
	for(int x = 1; x <= 1; x++)
		for(int y = 0; y <= 4; y++)
			if ( (tmp = m_pCollision->CheckPoint(m_Pos.x+PhysSize/2+x, m_Pos.y+PhysSize/2+y)) > height_right) {
				height_right = tmp;
			}
	

// 	if ( (tmp = m_pCollision->CheckPoint(m_Pos.x-PhysSize/2-1, m_Pos.y+PhysSize/2+1)) > height_left)
// 		height_left = tmp;
// 	if ( (tmp = m_pCollision->CheckPoint(m_Pos.x-PhysSize/2, m_Pos.y+PhysSize/2+1)) > height_left)
// 		height_left = tmp;
// 	
// 	int height_right = max(m_pCollision->CheckPoint(m_Pos.x+PhysSize/2+1, m_Pos.y+PhysSize/2), m_pCollision->CheckPoint(m_Pos.x+PhysSize/2, m_Pos.y+PhysSize/2));
// 	if ( (tmp = m_pCollision->CheckPoint(m_Pos.x+PhysSize/2, m_Pos.y+PhysSize/2)) > height_right)
// 		height_right = tmp;
// 	if ( (tmp = m_pCollision->CheckPoint(m_Pos.x+PhysSize/2+1, m_Pos.y+PhysSize/2+1)) > height_right)
// 		height_right = tmp;
// 	if ( (tmp = m_pCollision->CheckPoint(m_Pos.x+PhysSize/2, m_Pos.y+PhysSize/2+1)) > height_right)
// 		height_right = tmp;
	//left hand side
	/*for(int j = 0; j <= 28; j++) {
		if(m_pCollision->CheckPoint(m_Pos.x-PhysSize/2-1, m_Pos.y+PhysSize/2+j)) {
			height_left = j;
			break;
		}
	}
	
	for(int j = 0; j <=28; j++) {
		if(m_pCollision->CheckPoint(m_Pos.x+PhysSize/2+1, m_Pos.y+PhysSize/2+j)) {
			height_right = j;
			break;
		}
	}*/
	//return 1;
	//std::cerr << "HEIGHTS: " << height_left << " - " << height_right << std::endl;
	if(height_left == CCollision::SS_COL_RL) {
		//std::cerr << "RET 1" << std::endl;
		return 1;
	}
	else if (height_right == CCollision::SS_COL_RR) {
		//std::cerr << "RET -1" << std::endl;
		return -1;
	}
	//std::cerr << "RET 0" << std::endl;
	return 0;
}



void CCharacterCore::Tick(bool UseInput)
{
	m_PlayerCollision = false;
	m_MonsterDamage = false;
	m_KickDamage = -1;
	m_FluidDamage = false;
	m_HandJetpack = false;
	
	int s = m_Status;
	if (s & (1<<STATUS_DEATHRAY))
	{
		m_Vel = vec2(0, 0);
		return;
	}
	
	if (Status(STATUS_SPAWNING))
		return;
	
	float PhysSize = 28.0f;
	m_TriggeredEvents = 0;

	// get ground state

	bool Grounded = IsGrounded();
	bool InFluid = IsInFluid();
	int slope = SlopeState();
	
	/*
	bool Grounded = false;
	if(m_pCollision->CheckPoint(m_Pos.x+PhysSize/2, m_Pos.y+PhysSize/2+5))
		Grounded = true;
	if(m_pCollision->CheckPoint(m_Pos.x-PhysSize/2, m_Pos.y+PhysSize/2+5))
		Grounded = true;
	*/

	vec2 TargetDirection = normalize(vec2(m_Input.m_TargetX, m_Input.m_TargetY));

	float ControlSpeed = m_pWorld->m_Tuning.m_ControlSpeed;
	
	float MaxSpeed = (Grounded ? m_pWorld->m_Tuning.m_GroundControlSpeed : m_pWorld->m_Tuning.m_AirControlSpeed) * ControlSpeed;
	float Accel = (Grounded ? m_pWorld->m_Tuning.m_GroundControlAccel : m_pWorld->m_Tuning.m_AirControlAccel);
	float Friction = Grounded ? m_pWorld->m_Tuning.m_GroundFriction : m_pWorld->m_Tuning.m_AirFriction;
	float SlideFriction = m_pWorld->m_Tuning.m_SlideFriction;

	float SlideSlopeAcceleration = m_pWorld->m_Tuning.m_SlideSlopeAcceleration;
	float SlopeDeceleration = m_pWorld->m_Tuning.m_SlopeDeceleration;
	float SlopeAscendingControlSpeed = m_pWorld->m_Tuning.m_SlopeAscendingControlSpeed*invsqrt2;
	float SlopeDescendingControlSpeed = m_pWorld->m_Tuning.m_SlopeDescendingControlSpeed*invsqrt2;
	float SlideControlSpeed = m_pWorld->m_Tuning.m_SlideControlSpeed*invsqrt2;
	
	float SlideActivationSpeed = m_pWorld->m_Tuning.m_SlideActivationSpeed;

	float JumpPower = m_pWorld->m_Tuning.m_JumpPower;
	float WallrunPower = m_pWorld->m_Tuning.m_WallrunImpulse;
	
	float HandJetpackControlSpeed = 11.5f * ControlSpeed;
	float HandJetpackImpulse = 1.10f;
	float JetpackControlSpeed = 9.5f * ControlSpeed;
	float JetpackControlAccel = 2.2f;
	
	float DashPower = float(m_pWorld->m_Tuning.m_DashPower);
	
	m_OnWall = false;
	
	// rage
	if (m_Status & (1<<STATUS_DASH))
	{
		Friction /= 1.4f;
		MaxSpeed *= 1.4f;
		Accel *= 1.4f;
		JumpPower *= 1.1f;
		WallrunPower *= 1.3f;
		HandJetpackControlSpeed *= 1.3f;
	}
	
	if (m_Status & (1<<STATUS_SLOWMOVING))
	{
		MaxSpeed *= 0.8f;
		Accel *= 0.8f;
		JumpPower *= 0.8f;
		WallrunPower *= 0.8f;
		HandJetpackControlSpeed *= 0.8f;
		JetpackControlSpeed *= 0.8f;
		SlideControlSpeed *= 0.8f;
		JetpackControlAccel *= 0.8f;
		HandJetpackImpulse *= 0.8f;
	}
	
	int Mask = m_Status>>STATUS_MASK1;
	
	if (Mask == 2)
	{
		Friction /= 1.1f;
		MaxSpeed *= 1.15f;
		Accel *= 1.1f;
		JumpPower *= 1.15f;
		WallrunPower *= 1.15f;
		HandJetpackControlSpeed *= 1.15f;
		HandJetpackImpulse *= 1.1f;
		JetpackControlSpeed *= 1.15f;
		SlideControlSpeed *= 1.15f;
		JetpackControlAccel *= 1.1f;
		DashPower *= 1.15f;
	}
	
	
	if (m_Slide > 0)
		HandJetpackControlSpeed *= 1.2f;
	
	
	// gravity & jump physics
	if (m_Action == COREACTION_JUMP)
	{
		// sharper jump upwards
		if (m_ActionState++ > 2)
			m_Vel.y += m_pWorld->m_Tuning.m_Gravity;
	}
	else if (m_Action == COREACTION_SLIDEKICK)
	{
		if (m_ActionState < 8)
			m_Vel.y += m_pWorld->m_Tuning.m_Gravity*2;
	}
	else if (m_Action == COREACTION_JUMPPAD)
	{
		// sharper jump upwards
		if (m_ActionState++ > 8)
			m_Vel.y += m_pWorld->m_Tuning.m_Gravity;
		else if (m_ActionState < 6)
			m_Vel.y -= 2.0f;
	}
	else if (m_Action == COREACTION_WALLJUMP)
	{
		int Dir = m_ActionState > 0 ? 1 : -1;
		int State = abs(m_ActionState);
		
		if (State < 4)
		{
			m_Vel.y = 0.0f;
		}
		else if (State == 4)
		{
			m_Vel.y = -JumpPower;
			m_Vel.x = 9.0f * Dir;
			m_LockDirection = 2 * Dir;
		}
		else if (State < 8)
		{
			// no gravity
		}
		else
		{
			m_Vel.y += m_pWorld->m_Tuning.m_Gravity;
		}
		
		m_ActionState += Dir;
	}
	else
	{
		m_Vel.y += m_pWorld->m_Tuning.m_Gravity;
	}
	
	
	
	
	
	m_Anim = 0;
	
	bool LoadJetpack = false;
	
	// fill jetpack
	if (Grounded)
	{
		LoadJetpack = true;
		
		if (m_Wallrun < -10 || m_Wallrun > 10)
			m_Wallrun = 0;
	}

	int ForceTileStatus = IsOnForceTile() * 4;
	
	// handle input
	if(UseInput)
	{
		m_Direction = m_Input.m_Direction;
		m_Down = m_Input.m_Down;
		
		m_Charge = m_Input.m_Charge;
		
		if (m_ChargeLevel < 0)
			m_ChargeLevel++;
		
		/*
		if (m_ChargeLevel < 0)
			m_ChargeLevel++;
		else
		{
			if (m_Charge)
				m_ChargeLevel = min(m_ChargeLevel+2, 100);
			else if (m_ChargeLevel > 80)
				m_ChargeLevel = -m_ChargeLevel/2;
			else
				m_ChargeLevel = max(m_ChargeLevel-2, 0);
				
		}
		*/
		
		// sliding
		Slide();
		
		// go down faster while holding down (default key: s)
		//if (!Grounded && m_Down && m_Vel.y < MaxSpeed*1.0f)
		//	m_Vel.y += 0.4f;
		
		
		if (m_LockDirection > 0)
		{
			m_LockDirection--;
			m_Direction = 1;
		}
		if (m_LockDirection < 0)
		{
			m_LockDirection++;
			m_Direction = -1;
		}

		// wall climbing
		if (!Grounded && m_Jetpack == 0)
		{
			// falling down
			if (m_Vel.y > -2.5f || m_Direction == 0)
			{
				if (m_Wallrun > 10 && m_Wallrun < 25)
					m_Wallrun++;
				else if (m_Wallrun < -10 && m_Wallrun > -25)
					m_Wallrun--;
				else
					m_Wallrun = 0;
				
				if (m_Direction < 0 && m_pCollision->CheckPoint(m_Pos.x-PhysSize, m_Pos.y+PhysSize/2) &&
										m_pCollision->CheckPoint(m_Pos.x-PhysSize, m_Pos.y-PhysSize/2))
				{
					if ((m_Input.m_Hook && m_JetpackPower > 0 && TargetDirection.y > 0) || m_Input.m_Down)
						m_Vel.y *= 0.97f;
					else if (m_Vel.y > 0.0f)
						m_Vel.y *= 0.8f;
					else
						m_Vel.y *= 0.9f;
					
					m_Anim = 1;
					
					LoadJetpack = true;
					m_OnWall = true;
				}
					
				if (m_Direction > 0 && m_pCollision->CheckPoint(m_Pos.x+PhysSize, m_Pos.y+PhysSize/2) &&
										m_pCollision->CheckPoint(m_Pos.x+PhysSize, m_Pos.y-PhysSize/2))
				{
					if ((m_Input.m_Hook && m_JetpackPower > 0 && TargetDirection.y > 0) || m_Input.m_Down)
						m_Vel.y *= 0.97f;
					else if (m_Vel.y > 0.0f)
						m_Vel.y *= 0.8f;
					else
						m_Vel.y *= 0.9f;
					
					m_Anim = -1;
					
					LoadJetpack = true;
					m_OnWall = true;
				}
				
				// wall jump
				if(m_Input.m_Jump && !(m_Jumped&1))
				{
					if (m_pCollision->CheckPoint(m_Pos.x-(PhysSize+6), m_Pos.y+PhysSize/2) &&
						m_pCollision->CheckPoint(m_Pos.x-(PhysSize+6), m_Pos.y-PhysSize/2))
					{
						m_Jumped |= 1;
						m_Wallrun = 11;
						SetAction(COREACTION_WALLJUMP, 1);
					}
						
					if (m_pCollision->CheckPoint(m_Pos.x+(PhysSize+6), m_Pos.y+PhysSize/2) &&
						m_pCollision->CheckPoint(m_Pos.x+(PhysSize+6), m_Pos.y-PhysSize/2))
					{
						m_Jumped |= 1;
						m_Wallrun = -11;
						SetAction(COREACTION_WALLJUMP, -1);
					}
				}
			}
			
			// going up
			else
			{
				int WalljumpEndFrame = 17;
				// wallrun
				if (m_pCollision->CheckPoint(m_Pos.x-(PhysSize+6), m_Pos.y+PhysSize/2) &&
					m_pCollision->CheckPoint(m_Pos.x-(PhysSize+6), m_Pos.y-PhysSize/2))
				{
					// run up
					if (++m_Wallrun > 5 && m_Wallrun < 11)
					{
						m_Wallrun = 1;
						m_Vel.y = -WallrunPower;
					}
					else if (m_Wallrun > WalljumpEndFrame || m_Wallrun < 0)
						m_Wallrun = 3;
					
					LoadJetpack = true;
					m_OnWall = true;
					
					// wall jump
					if(m_Input.m_Jump && !(m_Jumped&1))
					{
						//m_TriggeredEvents |= COREEVENT_AIR_JUMP;
						m_Vel.y = -(JumpPower+3.0f);
						m_Vel.x = 9.0f;
						m_Jumped |= 1;
						m_LockDirection = 2;
						m_Wallrun = 11;
					}
				}
				else
				if (m_pCollision->CheckPoint(m_Pos.x+(PhysSize+6), m_Pos.y+PhysSize/2) &&
					m_pCollision->CheckPoint(m_Pos.x+(PhysSize+6), m_Pos.y-PhysSize/2))
				{
					// run up
					if (--m_Wallrun < -5 && m_Wallrun > -11)
					{
						m_Wallrun = -1;
						m_Vel.y = -WallrunPower;
					}
					else if (m_Wallrun < -WalljumpEndFrame || m_Wallrun > 0)
						m_Wallrun = -3;

					LoadJetpack = true;
					m_OnWall = true;
						
					// wall jump
					if(m_Input.m_Jump && !(m_Jumped&1))
					{
						//m_TriggeredEvents |= COREEVENT_AIR_JUMP;
						m_Vel.y = -(JumpPower+3.0f);
						m_Vel.x = -9.0f;
						m_Jumped |= 1;
						m_LockDirection = -2;
						m_Wallrun = -11;
					}
				}
				else
				{
					if (m_Wallrun > 10 && m_Wallrun < 25)
						m_Wallrun++;
					else if (m_Wallrun < -10 && m_Wallrun > -25)
						m_Wallrun--;
					else
						m_Wallrun = 0;
				}
			}
		}
		
		
		// setup angle
		float a = 0;
		if(m_Input.m_TargetX == 0)
			a = atanf((float)m_Input.m_TargetY);
		else
			a = atanf((float)m_Input.m_TargetY/(float)m_Input.m_TargetX);

		if(m_Input.m_TargetX < 0)
			a = a+pi;

		m_Angle = (int)(a*256.0f);

		// handle jump
		if(m_Input.m_Jump)
		{
			// jetpack physics
			if (m_Jetpack == 1 && m_JetpackPower > 0 && (m_Wallrun == 0 || abs(m_Wallrun) > 15) && !(m_Action == COREACTION_SLIDEKICK && m_ActionState < 6))
			{
				/*
				if (m_Input.m_Down)
				{
					if (m_Vel.y > 2.0f)
						m_Vel.y -= m_pWorld->m_Tuning.m_Gravity*1.5f;
					
					m_JetpackPower -= 1;
				}
				else
					*/
				{
					m_Wallrun = 0;
					
					if (m_Vel.y > -JetpackControlSpeed)
						m_Vel.y -= m_pWorld->m_Tuning.m_Gravity*JetpackControlAccel;
					
					if (m_Direction == 1 && m_Vel.x < JetpackControlSpeed)
						m_Vel.x += 0.5f;
					
					if (m_Direction == -1 && m_Vel.x > -JetpackControlSpeed)
						m_Vel.x -= 0.5f;
					
					/*if (m_Vel.y < 0.0f)
						m_JetpackPower -= 1;
					else*/
						m_JetpackPower -= 1;
				}
			}
			
			bool Down = PlatformState();
			
			if (Grounded || (InFluid && m_Vel.y >= 0.0f))
			{
				if (!(m_Jumped&1) && !m_Roll)
				{
					if (!m_pCollision->CheckPoint(m_Pos.x, m_Pos.y-64, false, true))
					{
						// lazy timer for jump animations
						if (m_Slide > 0)
							m_JumpTimer = -6;
						else
							m_JumpTimer = 6;
						
						m_TriggeredEvents |= COREEVENT_GROUND_JUMP;
						if(slope == 0)
						{
							m_Action = COREACTION_JUMP;
							m_ActionState = 0;
							
							m_Vel.y = -JumpPower;
						}
						else
						{
							m_Action = COREACTION_JUMP;
							m_ActionState = 0;
							m_Vel.y = -JumpPower*invsqrt2;
						}
						m_Jumped |= 1;
						
						if (m_Slide > 0 && m_Slide < 12)
						{
							m_Action = COREACTION_SLIDEKICK;
							m_ActionState = 0;
							m_TriggeredEvents |= COREEVENT_SLIDEKICK;
						}
					}
				}
			}
			else
			{
				if(!(m_Jumped&1) && m_JetpackPower > 0)
				{
					m_Jetpack = 1;
				}
					
				// slidekick on air
				if (!(m_Jumped&2) && !m_Roll)
				{
					if (m_Slide > 0 && m_Slide < 12)
					{
						m_Jumped |= 3;
						m_Action = COREACTION_SLIDEKICK;
						m_ActionState = 0;
						m_TriggeredEvents |= COREEVENT_SLIDEKICK;
						m_Vel.y = -JumpPower * 0.7f;
					}
				}
			}
		}
		else
		{
			if (Grounded || m_OnWall)
				m_Jumped = 0;
			
			m_Jumped &= ~1;
			m_Jetpack = 0;
		}

		// press down on wall
		if (!m_Input.m_Hook && m_Input.m_Down && m_Vel.y < 0.0f && m_Wallrun != 0 && m_Wallrun <= 10 && m_Wallrun >= -10)
		{
			m_Wallrun = 0;
			m_Vel.y = 0.0f;
		}
		
		// hand turbo boost
		if(m_Input.m_Hook && m_JetpackPower > 0 && !InFluid)
		{
			if ((TargetDirection.x > 0 && m_Vel.x < HandJetpackControlSpeed + ForceTileStatus) || (TargetDirection.x < 0 && m_Vel.x > -HandJetpackControlSpeed + ForceTileStatus))
				m_Vel.x += TargetDirection.x*HandJetpackImpulse;
				
			if ((TargetDirection.y > 0 && m_Vel.y < HandJetpackControlSpeed) || (TargetDirection.y < 0 && m_Vel.y > -HandJetpackControlSpeed))
				m_Vel.y += TargetDirection.y*HandJetpackImpulse;
				
			if (TargetDirection.y > 0 && m_Vel.y < 0.0f && m_Wallrun != 0 && m_Wallrun <= 10 && m_Wallrun >= -10)
			{
				m_Wallrun = 0;
				m_Vel.y = 0.0f;
			}
				
			m_JetpackPower -= 1;
			m_HandJetpack = true;
		}
	}
	
	m_Sliding = false;
	
	if(slope != 0)
		m_Sliding = true;
	
	if( (m_Vel.x > SlideActivationSpeed && slope == 1) || (m_Vel.x < -SlideActivationSpeed && slope == -1)) {
		m_Sliding = true;
	}

	if(slope != 0 && m_Direction == slope && !m_Sliding)
		MaxSpeed = SlopeDescendingControlSpeed;
	else if(slope != 0 && m_Direction == -slope && !m_Sliding) {
		MaxSpeed = SlopeAscendingControlSpeed;
		float diff = SlopeDeceleration*fabs(m_Vel.x - MaxSpeed);
		
		if(m_Vel.x > MaxSpeed)
			m_Vel.x -= diff;
		if(m_Vel.x < -MaxSpeed)
			m_Vel.x += diff;
	}
	
	if (m_Action == COREACTION_HANG)
	{
		Accel *= 0.3f;
		MaxSpeed *= 0.3f;
	}
	
	// add the speed modification according to players wanted direction
	if (m_Slide == 0)
	{
		if(m_Direction < 0) // && (!m_Sliding || !Grounded))
		{
			if (slope > 0)
				m_Vel.x = SaturatedAdd(-MaxSpeed, MaxSpeed, m_Vel.x, -Accel*0.7f);
			else
				m_Vel.x = SaturatedAdd(-MaxSpeed+ForceTileStatus, MaxSpeed+ForceTileStatus, m_Vel.x, -Accel);
		}
		if(m_Direction > 0) // && (!m_Sliding || !Grounded))
		{
			if (slope < 0)
				m_Vel.x = SaturatedAdd(-MaxSpeed, MaxSpeed, m_Vel.x, Accel*0.7f);
			else
				m_Vel.x = SaturatedAdd(-MaxSpeed+ForceTileStatus, MaxSpeed+ForceTileStatus, m_Vel.x, Accel);
		}
	}
	
	if(m_Sliding && slope != 0) {
		m_Vel.x = SaturatedAdd(-SlideControlSpeed, SlideControlSpeed, m_Vel.x, slope*SlideSlopeAcceleration);
	}
	else if(m_Sliding && Grounded) {
		m_Vel.x *= SlideFriction;
	}
	else if(m_Direction == 0 && m_Slide == 0) {
		if(slope != 0 && !m_Jumped) {
			m_Vel.x *= Friction;// /invsqrt2;
			m_Vel.y *= Friction;// /invsqrt2;
		} else {
			m_Vel.x = (m_Vel.x + ForceTileStatus) * Friction;
		}
	}
	
	
	// handle jumping
	// 1 bit = to keep track if a jump has been made on this input
	// 2 bit = to keep track if a air-jump has been made
	//if(Grounded)
	//	m_Jumped &= ~2;
	
	
	if (LoadJetpack)
	{
		m_JetpackPower += 3;
		
		if (m_JetpackPower > 200)
			m_JetpackPower = 200;
	}

	
	// limit falling speed
	if (m_Vel.y > MaxSpeed*2.5f)
		m_Vel.y = MaxSpeed*2.5f;
	
	
	if (m_Roll > 0)
		m_Roll++;
	
	
	if (m_Roll > 0)
	{
		if (m_Vel.x < -2.0f)
		{
			if (m_Roll < 15)
				m_Anim = -2;
			else
			{
				m_Roll = 0;
				m_Slide = 5;
				m_Anim = -3;
			}
		}
		else if (m_Vel.x > 2.0f)
		{
			if (m_Roll < 15)
				m_Anim = 2;
			else
			{
				m_Roll = 0;
				m_Slide = 5;
				m_Anim = 3;
			}
		}
		else
		{
			m_Roll = 0;
		}
	}
	
	vec2 Dir = GetDirection(m_Angle);
	
	// roll dash
	if (m_Roll > 4 && !m_DashTimer && m_Input.m_Jump && ((m_Vel.x < 0) == (Dir.x < 0)) && Dir.y < 0.25f)
	{
		m_DashTimer = 4;
		m_DashAngle = m_Angle;
		
		m_JetpackPower = min(m_JetpackPower + 25, 200);
	}

	if (m_DashTimer > 0)
	{
		m_DashTimer--;
		
		vec2 d = GetDirection(m_DashAngle)*max(length(m_Vel)*1.05f, DashPower);
		m_Vel += (d - m_Vel) / 3.0f;
	}
	

	if (m_JumpTimer > 0)
	{
		m_JumpTimer--;
		m_Anim = 5;
	}
	else if (m_JumpTimer < 0)
	{
		m_JumpTimer++;
		m_Anim = -5;
	}
	
	if (m_Action == COREACTION_JUMPPAD && m_ActionState < 28)
		m_Anim = 6;
	
	m_BallHitVel = vec2(0, 0);

	if(m_pWorld)
	{
		// ball collision
		CBallCore *pBallCore = m_pWorld->m_pBall;
			
		if (pBallCore)
		{
			float BallSize = m_pWorld->m_Tuning.m_BallSize;
			vec2 BPos = pBallCore->m_Pos;
			float OffsetY = -26;
		
			if (m_Action == COREACTION_SLIDEKICK && m_ActionState > 2 &&  m_ActionState < 10)
				OffsetY = -14;
			
			if (m_Slide != 0 || m_Roll != 0)
				OffsetY = -6;
			
			if (m_DashTimer > 0)
				OffsetY = -12;
		
			vec2 Pos = m_Pos + vec2(0, OffsetY);
		
			float Distance = distance(Pos, BPos);
			vec2 Dir = normalize(Pos - BPos);
		
			if (Distance <= BallSize*0.80f+16)
			{
				float a = ((BallSize*0.80f+16) - Distance);
				float Velocity = 0.5f;
				
				m_BallHitVel = m_Vel;

				// make sure that we don't add excess force by checking the
				// direction against the current velocity. if not zero.
				if (length(m_Vel) > 0.0001)
					Velocity = 1-(dot(normalize(m_Vel), Dir)+1)/2;

				m_Vel += Dir*a*(Velocity*0.75f);
				m_Vel *= 0.85f;
			}
		}
		
		
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			CCharacterCore *pCharCore = m_pWorld->m_apCharacters[i];
			if(!pCharCore)
				continue;

			//player *p = (player*)ent;
			if(pCharCore == this  || pCharCore->Status(STATUS_SPAWNING)) // || !(p->flags&FLAG_ALIVE)
				continue; // make sure that we don't nudge our self

			// handle player <-> player collision
			float Distance = distance(m_Pos, pCharCore->m_Pos);
			vec2 Dir = normalize(m_Pos - pCharCore->m_Pos);
			
			if (m_pWorld->m_Tuning.m_PlayerCollision && m_Roll == 0)
			{
				if(m_Slide == 0 && 
					pCharCore->m_Slide == 0 && pCharCore->m_Roll == 0 && 
					Distance < PhysSize*1.35f && Distance > 12.0f)
				{
					m_PlayerCollision = true;
						
					float a = (PhysSize*1.45f - Distance);
					float Velocity = 0.5f;

					// make sure that we don't add excess force by checking the
					// direction against the current velocity. if not zero.
					if (length(m_Vel) > 0.0001)
						Velocity = 1-(dot(normalize(m_Vel), Dir)+1)/2;

					m_Vel += Dir*a*(Velocity*0.75f);
					m_Vel *= 0.85f;
				}
				
				if (abs(m_Vel.x) < 1.0f && abs(m_Vel.y) < 1.0f && Distance < PhysSize && m_Pos.y <= pCharCore->m_Pos.y)
				{
					if (!m_pCollision->CheckPoint(m_Pos.x-28.0f*0.5f, m_Pos.y-64.0f*0.5f) && !m_pCollision->CheckPoint(m_Pos.x+28.0f*0.5f, m_Pos.y-64.0f*0.5f))
					{
						m_Vel.y -= 1.0f;
						m_Pos.y -= 1.0f;
					}
				}
			}
		}
		
		
		// jumppads
		if (m_Action != COREACTION_JUMPPAD || (m_Action == COREACTION_JUMPPAD && m_ActionState > 12))
		{
			for(int i = 0; i < MAX_DROIDS; i++)
			{
				vec4 ImpactPos = m_pWorld->m_aImpactPos[i];
						
				if(ImpactPos.x == 0)
					continue;

				//if (abs(m_Pos.x - ImpactPos.x) < 64 && abs(m_Pos.y - (ImpactPos.y - 16)) < 16)
				if (m_Pos.x > ImpactPos.x && m_Pos.x < ImpactPos.z && m_Pos.y > ImpactPos.y && m_Pos.y < ImpactPos.w)
					Jumppad();
			}
		}
	}

	// fix to slope bug (standing near wall)
	if (IsGrounded() && !m_Sliding && abs(m_Vel.y) < 1.5f && abs(m_Vel.x) < 0.2f)
	{
		if (!m_pCollision->IsTileSolid(m_Pos.x-PhysSize, m_Pos.y+PhysSize*0.7) || !m_pCollision->IsTileSolid(m_Pos.x+PhysSize, m_Pos.y+PhysSize*0.7))
		{
			if (!m_pCollision->IsTileSolid(m_Pos.x-PhysSize*0.2f, m_Pos.y+25) || !m_pCollision->IsTileSolid(m_Pos.x+PhysSize*0.2f, m_Pos.y+25))
			{
				if (m_pCollision->IsTileSolid(m_Pos.x-PhysSize*1.2f, m_Pos.y))
					m_Pos.x += 2.0f;
				
				if (m_pCollision->IsTileSolid(m_Pos.x+PhysSize*1.2f, m_Pos.y))
					m_Pos.x -= 2.0f;
			}
		}
	}
	
	

	if (m_Action == COREACTION_SLIDEKICK)
	{
		if (m_ActionState == 1 && abs(m_Vel.x) < 22.0f)
			m_Vel.x *= 1.05f;
		
		if (m_ActionState < 0)
		{
			if (m_ActionState-- < -11)
				m_Action = COREACTION_IDLE;
		}
		else
		{
			if (m_ActionState++ > 11)
				m_Action = COREACTION_IDLE;
		}
		
		if (m_Vel.x > 0)
			m_Anim = 7;
		else
			m_Anim = -7;
	}
	
	
	// hang on to somethings
	if (m_Down || m_HandJetpack || m_Jetpack)
	{
		if (m_Action == COREACTION_HANG)
			m_Action = COREACTION_IDLE;
	}
	else
	{
		if (m_pCollision->IsHangTile(m_Pos + vec2(0, -32))) // -32
		{
			if (m_Vel.y >= 0.0f)
			{
				m_Vel.x *= 0.97f;
				m_Vel.y = 0.0f;
				m_Action = COREACTION_HANG;
				m_ActionState = 0;
				m_Pos.y -= (int(m_Pos.y)%32-26)/2.0f; // 26
				
				if (!LoadJetpack)
				{
					m_JetpackPower += 3;
					
					if (m_JetpackPower > 200)
						m_JetpackPower = 200;
				}
			}
		}
		else if (m_Action == COREACTION_HANG)
			m_Action = COREACTION_IDLE;
	}
	
	
	// clamp the velocity to something sane
	if(length(m_Vel) > 6000)
		m_Vel = normalize(m_Vel) * 6000;
	
	// electric damage effect
	s = m_Status;
	if (s & (1<<STATUS_ELECTRIC))
		m_Vel.x *= 0.85f;
	
	// fluid collision
	if (InFluid)
	{
		m_Vel *= 0.85f;
		m_Jetpack = false;;
		m_HandJetpack = 0;
		m_FluidDamage = true;
	}
}


void CCharacterCore::Jumppad()
{
	m_Vel.y = -9.0f;
	m_Action = COREACTION_JUMPPAD;
	m_ActionState = 0;
}


void CCharacterCore::Roll()
{
	float PhysSize = 28.0f;
	
	if (m_Roll > 0)
		return;
	
	vec2 TargetDirection = normalize(vec2(m_Input.m_TargetX, m_Input.m_TargetY));
	
	if (m_Vel.x < -2.5f && !m_pCollision->CheckPoint(m_Pos.x-(PhysSize+32), m_Pos.y+PhysSize/2) && TargetDirection.x < 0.0f)
	{
		m_Roll++;
		m_LockDirection = -8;
	}
	else if (m_Vel.x > 2.5f && !m_pCollision->CheckPoint(m_Pos.x+(PhysSize+32), m_Pos.y+PhysSize/2) && TargetDirection.x > 0.0f)
	{
		m_Roll++;
		m_LockDirection = 8;
	}
}


void CCharacterCore::Slide()
{
	float PhysSize = 28.0f;
	
	// start sliding
	if (m_Input.m_Down && m_Slide == 0 && m_Roll == 0)
	{
		vec2 TargetDirection = normalize(vec2(m_Input.m_TargetX, m_Input.m_TargetY));
		if ((m_Vel.x < -7.0f && TargetDirection.x < 0) ||
			(m_Vel.x > 7.0f && TargetDirection.x > 0))
			m_Slide++;
		else
		if ((TargetDirection.x > 0 && !m_pCollision->CheckPoint(m_Pos.x+(PhysSize+32), m_Pos.y+PhysSize/2) && m_pCollision->CheckPoint(m_Pos.x+(PhysSize+32), m_Pos.y-64)) ||
			(TargetDirection.x < 0 && !m_pCollision->CheckPoint(m_Pos.x-(PhysSize+32), m_Pos.y+PhysSize/2) && m_pCollision->CheckPoint(m_Pos.x-(PhysSize+32), m_Pos.y-64)))
		{
			m_Slide++;
			if (TargetDirection.x > 0 && abs(m_Vel.x) < 4.0f)
				m_Vel.x = 4;
			else if (TargetDirection.x < 0 && abs(m_Vel.x) < 4.0f)
				m_Vel.x = -4;
		}
	}

	if (!m_Input.m_Down && m_Slide > 0)
		m_Slide = -4;
	
	if (m_Wallrun != 0)
		m_Slide = 0;

	if (m_Slide != 0)
	{
		m_Slide++;
		
		if (m_Slide > 0)
		{
			//if (IsGrounded())
			{
				if (IsGrounded() && (!m_Input.m_Hook || ((IsOnForceTile() < 0 && m_Vel.x > 0) || (IsOnForceTile() > 0 && m_Vel.x < 0))))
					m_Vel.x *= 0.98f;
			
				if (m_Vel.x < -3.5f)// && !m_pCollision->CheckPoint(m_Pos.x-(PhysSize+32), m_Pos.y+PhysSize/2))
				{
					m_LockDirection = -2;
					m_Anim = -3;
				}
				else if (m_Vel.x > 3.5f)// && !m_pCollision->CheckPoint(m_Pos.x+(PhysSize+32), m_Pos.y+PhysSize/2))
				{
					m_LockDirection = 2;
					m_Anim = 3;
				}
				else
				{
					m_Slide = -4;
				}
			}
			//else
			//	m_Slide = -4;
		}
		
		// stand up animation after slide
		if (m_Slide < 0)
		{
			if (m_Vel.x < 0.0f)
			{
				m_LockDirection = -2;
				m_Anim = -4;
			}
			if (m_Vel.x > 0.0f)
			{
				m_LockDirection = 2;
				m_Anim = 4;
			}
		}
	}
	
	// force slide when in tunnel
	if ((m_Slide != 0 || m_Roll != 0) && 
		((m_pCollision->CheckPoint(m_Pos.x+PhysSize, m_Pos.y-64) && !m_pCollision->CheckPoint(m_Pos.x+PhysSize, m_Pos.y+PhysSize/2)) || 
		(m_pCollision->CheckPoint(m_Pos.x-PhysSize, m_Pos.y-64) && !m_pCollision->CheckPoint(m_Pos.x-PhysSize, m_Pos.y+PhysSize/2))))
	{
		if (abs(m_Vel.x) < 5.0f)
			m_Vel.x /= 0.98f;
			
		if (m_Vel.x < 0)
		{
			m_LockDirection = -2;
			m_Anim = -3;
		}
		else
		{
			m_LockDirection = 2;
			m_Anim = 3;
		}
		m_Slide = 5;
	}
}
	
	
void CCharacterCore::Move()
{
	int s = m_Status;
	if (s & (1<<STATUS_DEATHRAY))
		return;
	
	if (Status(STATUS_SPAWNING))
		return;
	
	float RampValue = VelocityRamp(length(m_Vel)*50, m_pWorld->m_Tuning.m_VelrampStart, m_pWorld->m_Tuning.m_VelrampRange, m_pWorld->m_Tuning.m_VelrampCurvature);

	m_Vel.x = m_Vel.x*RampValue;

	float VelY = m_Vel.y;
	
	bool Down = PlatformState();
	
	if (VelY < 0.0f)
		Down = true;
	
	vec2 NewPos = m_Pos;
	
	if ((m_Slide == 0 && m_Roll == 0) || m_Wallrun != 0)
	{
		NewPos.y -= 18;
		m_pCollision->MoveBox(&NewPos, &m_Vel, vec2(28.0f, 64.0f), 0, !m_Sliding, Down);
		NewPos.y += 18;
		
		int TopLeft = m_pCollision->CheckPoint(m_Pos.x-28.0f*0.5f, m_Pos.y-(64.0f)*0.5f-18, false, true);
		int TopRight = m_pCollision->CheckPoint(m_Pos.x+28.0f*0.5f, m_Pos.y-(64.0f)*0.5f-18, false, true);
		
		// unstuck jumpkick
		if (TopLeft && !TopRight)
			NewPos.x += 1;
		else if (TopRight && !TopLeft)
			NewPos.x -= 1;
	}
	else
	{
		NewPos.y -= 10;
		m_pCollision->MoveBox(&NewPos, &m_Vel, vec2(28.0f, 48.0f), 0, !m_Sliding, Down);
		NewPos.y += 10;
	}
	
	if (VelY > 13.0f && abs(m_Vel.y) < 2.0f && m_Input.m_Down)
		Roll();
	
	m_Vel.x = m_Vel.x*(1.0f/RampValue);

	/*
	if (m_Action == COREACTION_SLIDEKICK)
	{
		
		float Distance = distance(m_Pos, NewPos);
		int End = Distance+1;
		vec2 LastPos = m_Pos;
		for(int i = 0; i < End; i++)
		{
			float a = i/Distance;
			vec2 Pos = mix(m_Pos, NewPos, a);
			for(int p = 0; p < MAX_CLIENTS; p++)
			{
				CCharacterCore *pCharCore = m_pWorld->m_apCharacters[p];
				if(!pCharCore || pCharCore == this || pCharCore->m_Roll != 0 || pCharCore->Status(STATUS_SPAWNING))
					continue;
				float D = distance(Pos, pCharCore->m_Pos+vec2(0, -32));
				if(D < 40.0f)
				{
					m_PlayerCollision = true;
					if(a > 0.0f)
						m_Pos = LastPos;
					else if(distance(NewPos, pCharCore->m_Pos) > D)
						m_Pos = NewPos;
					
					pCharCore->m_Vel += m_Vel;
					//m_Vel.x *= 0.9f;
					
					return;
				}
			}
			LastPos = Pos;
		}
	}
	*/
	
	// ball collision
	if (true)
	{
		// ball
		CBallCore *pBallCore = m_pWorld->m_pBall;
			
		if (pBallCore)
		{
			float BallSize = m_pWorld->m_Tuning.m_BallSize;
	
			vec2 BPos = pBallCore->m_Pos;
			float OffsetY = -26;
			float Force = 1.0f;
		
			if (m_Action == COREACTION_SLIDEKICK && m_ActionState > 2 &&  m_ActionState < 10)
			{
				OffsetY = -14;
				Force = 2.0f;
			}
			
			if (m_Slide != 0 || m_Roll != 0)
				OffsetY = -6;
			
			if (m_DashTimer > 0)
				OffsetY = -12;
		
			float Distance = distance(m_Pos, NewPos);
			int End = Distance+1;
			vec2 LastPos = m_Pos;
		
			for(int i = 0; i < End; i++)
			{
				float a = i/Distance;
				vec2 Pos = mix(m_Pos, NewPos, a)+vec2(0, OffsetY);
				
				float D = distance(Pos, BPos);
				
				if (D <= BallSize*0.80f+16)
				{
					if(a > 0.0f)
						m_Pos = LastPos;
					else if(distance(NewPos, BPos) > D)
						m_Pos = NewPos;
					
					float theta = atan2((Pos.y - BPos.y), (Pos.x - BPos.x));
					float overlap = BallSize*0.80f+16 - D;
					
					vec2 BVel = -vec2(cos(theta), sin(theta)) * overlap;
					m_pCollision->MoveBox(&pBallCore->m_Pos, &BVel, vec2(BallSize, BallSize), 0.0f);
					
					pBallCore->PlayerHit();
					
					BPos = pBallCore->m_Pos;
					D = distance(Pos, BPos);
					
					float theta1 = GetAngle(m_BallHitVel);
					float theta2 = GetAngle(pBallCore->m_Vel);
					float phi = atan2(BPos.y - Pos.y, BPos.x - Pos.x);
					float m1 = 1.0f;
					float m2 = 0.5f;
					
					float v1 = length(m_BallHitVel);
					float v2 = length(pBallCore->m_Vel);
					
					float dx1F = (v1 * cos(theta1 - phi) * (m1-m2) + 2*m2*v2*cos(theta2 - phi)) / (m1+m2) * cos(phi) + v1*sin(theta1-phi) * cos(phi+pi/2);
					float dy1F = (v1 * cos(theta1 - phi) * (m1-m2) + 2*m2*v2*cos(theta2 - phi)) / (m1+m2) * sin(phi) + v1*sin(theta1-phi) * sin(phi+pi/2);
					float dx2F = (v2 * cos(theta2 - phi) * (m2-m1) + 2*m1*v1*cos(theta1 - phi)) / (m1+m2) * cos(phi) + v2*sin(theta2-phi) * cos(phi+pi/2);
					float dy2F = (v2 * cos(theta2 - phi) * (m2-m1) + 2*m1*v1*cos(theta1 - phi)) / (m1+m2) * sin(phi) + v2*sin(theta2-phi) * sin(phi+pi/2);

					//if (pBallCore->m_Status & (1<<BALLSTATUS_SUPER))
					//	m_Vel -= vec2(dx1F, dy1F)*0.6f;
					
					if (m_DashTimer > 0)
					{
						pBallCore->m_Vel = vec2(dx2F, dy2F)*1.3f;
						
						if (length(pBallCore->m_Vel) > 15.0f)
							pBallCore->m_Status |= 1 << BALLSTATUS_SUPER;
					}
					else
						//pBallCore->m_Vel = vec2(dx2F, dy2F)*0.8f + m_pCollision->Reflect(pBallCore->m_Vel, normalize(Pos - BPos))*0.25f + m_pCollision->Reflect(pBallCore->m_Vel, normalize(vec2(m_Input.m_TargetX, m_Input.m_TargetY)))*0.25f;
						pBallCore->m_Vel = vec2(dx2F, dy2F)*0.8f + m_pCollision->Reflect(pBallCore->m_Vel, normalize(Pos - BPos))*0.25f;
					
					//pBallCore->m_Vel = ;
					
					//m_Vel -= vec2(dx1F, dy1F)*0.1f;
					break;
				}
				
				
				LastPos = Pos;
			}
		}
	}
	
	
	// check player collision
	if ((m_Action == COREACTION_SLIDEKICK && m_ActionState > 2 &&  m_ActionState < 10) || (m_pWorld && m_pWorld->m_Tuning.m_PlayerCollision && m_Roll == 0 && m_Slide == 0))
	{
		float Distance = distance(m_Pos, NewPos);
		int End = Distance+1;
		vec2 LastPos = m_Pos;
		for(int i = 0; i < End; i++)
		{
			float a = i/Distance;
			vec2 Pos = mix(m_Pos, NewPos, a);
			
			for(int p = 0; p < MAX_CLIENTS; p++)
			{
				CCharacterCore *pCharCore = m_pWorld->m_apCharacters[p];
				if(!pCharCore || pCharCore == this || pCharCore->m_Roll != 0 || pCharCore->Status(STATUS_SPAWNING))
					continue;
				float D = distance(Pos, pCharCore->m_Pos);
				float D2 = 9000.0f;
				
				vec2 Off = vec2(20.0, 0.0f);
				if (m_Vel.x < 0)
					Off.x *= -1;
				
				if (m_Action == COREACTION_SLIDEKICK)
					D2 = distance(Pos+Off, pCharCore->m_Pos+vec2(0, -32));
				
				if(D2 < 40.0f || (D < 32.0f && D > 12.0f))
				{
					m_PlayerCollision = true;
					if(a > 0.0f)
						m_Pos = LastPos;
					else if(distance(NewPos, pCharCore->m_Pos) > D)
						m_Pos = NewPos;

					if (m_Action == COREACTION_SLIDEKICK)
					{
						if (m_ActionState > 0)
						{
							m_ActionState *= -1;
							pCharCore->m_KickDamage = m_ClientID;
								
							// both players hitting
							if (pCharCore->m_Action == COREACTION_SLIDEKICK && m_ActionState > 2 &&  m_ActionState < 10)
							{
								m_KickDamage = pCharCore->m_ClientID;
								pCharCore->m_ActionState *= -1;
								
								vec2 CCv = vec2(m_Vel.x*1.5f, -abs(m_Vel.x*0.5f));
								
								m_Vel = vec2(pCharCore->m_Vel.x*1.5f, -abs(pCharCore->m_Vel.x*0.5f));
								pCharCore->m_Vel = CCv;
							}
							// just this hitting
							else
							{
								pCharCore->m_Vel = vec2(m_Vel.x*1.5f, -abs(m_Vel.x*0.5f));
								m_Vel *= 0.3f;
							}
						}
						else
							m_Vel *= 0.3f;
					}
					
					return;
				}
			}
			LastPos = Pos;
		}
	}


	m_Pos = NewPos;
}

void CCharacterCore::Write(CNetObj_CharacterCore *pObjCore)
{
	if (!m_pWorld || !m_pCollision || !m_pCollision->m_pTiles || !m_pCollision->m_pLayers)
	{
		dbg_msg("Error", "CCharacterCore::Write(): m_pWorld or m_pCollision are NULL!");
		return;
	}

	pObjCore->m_X = round_to_int(m_Pos.x);
	pObjCore->m_Y = round_to_int(m_Pos.y);

	pObjCore->m_Health = m_Health;
	pObjCore->m_VelX = round_to_int(m_Vel.x*256.0f);
	pObjCore->m_VelY = round_to_int(m_Vel.y*256.0f);
	pObjCore->m_DamageTick = m_DamageTick;
	pObjCore->m_Jumped = m_Jumped;
	pObjCore->m_JumpTimer = m_JumpTimer;
	pObjCore->m_Direction = m_Direction;
	pObjCore->m_Down = m_Down;
	pObjCore->m_Charge = m_Charge;
	pObjCore->m_ChargeLevel = m_ChargeLevel;
	pObjCore->m_Sliding = m_Sliding;
	pObjCore->m_Grounded = IsGrounded();
	pObjCore->m_Angle = m_Angle;
	pObjCore->m_Anim = m_Anim;
	pObjCore->m_Jetpack = m_Jetpack;
	pObjCore->m_HandJetpack = m_HandJetpack;
	pObjCore->m_JetpackPower = m_JetpackPower;
	pObjCore->m_Wallrun = m_Wallrun;
	pObjCore->m_Roll = m_Roll;
	pObjCore->m_Slide = m_Slide;
	pObjCore->m_Status = m_Status;
	pObjCore->m_LockDirection = m_LockDirection;
	pObjCore->m_Slope = SlopeState();
	pObjCore->m_Action = m_Action;
	pObjCore->m_ActionState = m_ActionState;
	
	
	pObjCore->m_Movement1 = m_DashTimer | m_DashAngle<<6;
}

void CCharacterCore::Read(const CNetObj_CharacterCore *pObjCore)
{
	m_Pos.x = pObjCore->m_X;
	m_Pos.y = pObjCore->m_Y;
	
	m_Health = pObjCore->m_Health;
	m_Vel.x = pObjCore->m_VelX/256.0f;
	m_Vel.y = pObjCore->m_VelY/256.0f;
	m_DamageTick = pObjCore->m_DamageTick;
	m_Jumped = pObjCore->m_Jumped;
	m_JumpTimer = pObjCore->m_JumpTimer;
	m_Direction = pObjCore->m_Direction;
	m_Down = pObjCore->m_Down;
	m_Charge = pObjCore->m_Charge;
	m_ChargeLevel = pObjCore->m_ChargeLevel;
	m_Sliding = pObjCore->m_Sliding;
	m_Angle = pObjCore->m_Angle;
	m_Anim = pObjCore->m_Anim;
	m_Jetpack = pObjCore->m_Jetpack;
	m_HandJetpack = pObjCore->m_HandJetpack;
	m_JetpackPower = pObjCore->m_JetpackPower;
	m_Wallrun = pObjCore->m_Wallrun;
	m_Roll = pObjCore->m_Roll;
	m_Slide = pObjCore->m_Slide;
	m_Status = pObjCore->m_Status;
	m_LockDirection = pObjCore->m_LockDirection;
	m_Action = pObjCore->m_Action;
	m_ActionState = pObjCore->m_ActionState;
	
	m_DashTimer = pObjCore->m_Movement1&(63<<0);
	//m_DashAngle = (pObjCore->m_Movement1&(255<<6))>>6;
	m_DashAngle = pObjCore->m_Movement1>>6;
}

void CCharacterCore::Quantize()
{
	CNetObj_CharacterCore Core;
	Write(&Core);
	Read(&Core);
}




/*
	- - - - - BALL - - - - -
*/

CBallCore::CBallCore()
{
	Init(NULL, NULL);
}

void CBallCore::Init(CWorldCore *pWorld, CCollision *pCollision)
{
	m_pWorld = pWorld;
	m_pCollision = pCollision;
}

void CBallCore::Reset()
{
	m_Pos = vec2(0,0);
	m_Vel = vec2(0,0);
	m_Angle = 0.0f;
	m_AngleForce = 0.0f;
	m_Status = 0;
	m_Status |= 1 << BALLSTATUS_STATIONARY;
	m_ForceCoreSend = false;
	m_TriggeredEvents = 0;
}


void CBallCore::PlayerHit()
{
	m_ForceCoreSend = true;
	if (m_Status & (1<<BALLSTATUS_STATIONARY))
		m_Status ^= 1 << BALLSTATUS_STATIONARY;
}


const float CBallCore::BallSize()
{
	return m_pWorld->m_Tuning.m_BallSize;
}

bool CBallCore::PlatformState()
{
	float PhysSize = BallSize();
	
	if (m_pCollision->IsPlatform(m_Pos.x-PhysSize/2, m_Pos.y+PhysSize/2+24) || m_pCollision->IsPlatform(m_Pos.x+PhysSize/2, m_Pos.y+PhysSize/2+24))
		return m_Vel.y < 0.0f;
	
	return true;
}


void CBallCore::Tick()
{
	m_TriggeredEvents = 0;
	
	if (m_Status & (1<<BALLSTATUS_STATIONARY))
		return;
	
	if (m_Status & (1<<BALLSTATUS_SUPER) && length(m_Vel) < 15.0f)
		m_Status ^= 1 << BALLSTATUS_SUPER;
	
	if (m_Status & (1<<BALLSTATUS_SUPER))
		m_Vel.y += 0.45f;
	else
		m_Vel.y += 0.5f;
}

void CBallCore::Move()
{
	if (m_Status & (1<<BALLSTATUS_STATIONARY))
		return;
	
	// limit speed
	float Limit = 30.0f;
	float Elastic = 0.7f;
	
	bool Down = PlatformState();
	
	if (m_Status & (1<<BALLSTATUS_SUPER))
	{
		Limit = 40.0f;
		Elastic = 0.85f;
	}
	
	if (length(m_Vel) > Limit)
		m_Vel = normalize(m_Vel)*Limit;
	
	float BallSize = m_pWorld->m_Tuning.m_BallSize;
	
	bool Grounded = false;
	if (m_pCollision->CheckPoint(m_Pos.x+BallSize/2, m_Pos.y+BallSize/2+5, false, Down))
		Grounded = true;
	else if (m_pCollision->CheckPoint(m_Pos.x-BallSize/2, m_Pos.y+BallSize/2+5, false, Down))
		Grounded = true;
	
	int OnForceTile = m_pCollision->IsForceTile(m_Pos.x-BallSize/2, m_Pos.y+BallSize/2+5);
	if (OnForceTile == 0)
		OnForceTile = m_pCollision->IsForceTile(m_Pos.x+BallSize/2, m_Pos.y+BallSize/2+5);
	
	if (Grounded)
	{
		if (OnForceTile)
			m_Vel.x = (m_Vel.x + OnForceTile*0.4f) * 0.925f;
		
		//m_Vel.x *= 0.8f;
		m_AngleForce += (m_Vel.x- OnForceTile*0.55f*8.0f - m_AngleForce) / 2.0f;
		m_Vel.x *= 0.99f;
		m_Vel.y *= 0.99f;
	}
	else
	{
		//m_Vel.x *= 0.99f;
		//m_Vel.y *= 0.99f;
		m_AngleForce *= 0.99f;
	}
	
	vec2 NewPos = m_Pos;
	vec2 OldVel = m_Vel;
	
	
	
	m_pCollision->MoveBox(&NewPos, &m_Vel, vec2(BallSize, BallSize), Elastic, false, Down);
	

	if ((((OldVel.x < 0 && m_Vel.x > 0) || (OldVel.x > 0 && m_Vel.x < 0)) && abs(m_Vel.x) > 3.0f) ||
		(((OldVel.y < 0 && m_Vel.y > 0) || (OldVel.y > 0 && m_Vel.y < 0)) && abs(m_Vel.y) > 3.0f))
		m_TriggeredEvents |= COREEVENT_BALL_BOUNCE;
	
	m_Angle += clamp(m_AngleForce*0.04f, -0.3f, 0.3f);
	
	m_Pos = NewPos;
}

void CBallCore::Write(CNetObj_BallCore *pObjCore)
{
	if (!m_pWorld || !m_pCollision || !m_pCollision->m_pTiles || !m_pCollision->m_pLayers)
	{
		dbg_msg("Error", "CCharacterCore::Write(): m_pWorld or m_pCollision are NULL!");
		return;
	}

	pObjCore->m_X = round_to_int(m_Pos.x);
	pObjCore->m_Y = round_to_int(m_Pos.y);
	
	pObjCore->m_VelX = round_to_int(m_Vel.x*256.0f);
	pObjCore->m_VelY = round_to_int(m_Vel.y*256.0f);
	
	pObjCore->m_Angle = round_to_int(m_Angle*256.0f);
	pObjCore->m_AngleForce = round_to_int(m_AngleForce*256.0f);
	
	pObjCore->m_Status = m_Status;
}

void CBallCore::Read(const CNetObj_BallCore *pObjCore)
{
	m_Pos.x = pObjCore->m_X;
	m_Pos.y = pObjCore->m_Y;
	
	m_Vel.x = pObjCore->m_VelX/256.0f;
	m_Vel.y = pObjCore->m_VelY/256.0f;
	
	m_Angle = pObjCore->m_Angle/256.0f;
	m_AngleForce = pObjCore->m_AngleForce/256.0f;
	
	m_Status = pObjCore->m_Status;
}

void CBallCore::Quantize()
{
	CNetObj_BallCore Core;
	Write(&Core);
	Read(&Core);
}

