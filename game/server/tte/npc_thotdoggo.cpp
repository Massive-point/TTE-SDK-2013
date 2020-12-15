//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: A slow-moving, once-human headcrab victim with only melee attacks.
//
// UNDONE: Make head take 100% damage, body take 30% damage.
// UNDONE: Don't flinch every time you get hit.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "game.h"
#include "AI_Default.h"
#include "AI_Schedule.h"
#include "AI_Hull.h"
#include "AI_Route.h"
#include	"AI_Task.h"
#include	"AI_Node.h"
#include	"AI_Hint.h"
#include	"soundent.h"
#include	"EntityList.h"
#include	"activitylist.h"
#include	"animation.h"
#include	"ai_navigator.h"
#include	"AI_Criteria.h"
#include "ai_baseactor.h"
#include "NPCEvent.h"
#include "npc_thotdoggo.h"
#include "gib.h"
#include "ai_baseactor.h"
#include "sceneentity.h"
#include	"props.h"
#include	"particle_parse.h"
//#include "AI_Interactions.h"
#include "ndebugoverlay.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"

ConVar	sk_thotdoggo_health("sk_thotdoggo_health", "30");
ConVar  sk_thotdoggo_dmg_one_slash("sk_thotdoggo_dmg_one_slash", "10");
ConVar  sk_thotdoggo_dmg_both_slash("sk_thotdoggo_dmg_both_slash", "10");

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CNPC_Thotdoggo)
DEFINE_FIELD(m_flNextPainSoundTime, FIELD_TIME),
END_DATADESC()

LINK_ENTITY_TO_CLASS(npc_thotdoggo, CNPC_Thotdoggo);


//=========================================================
// Spawn
//=========================================================
void CNPC_Thotdoggo::Spawn()
{
	Precache();

	SetModel("models/thot_doggo.mdl");

	SetRenderColor(255, 255, 255, 255);

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	m_bloodColor = BLOOD_COLOR_RED;
	m_iHealth = sk_thotdoggo_health.GetFloat();
	//pev->view_ofs		= VEC_VIEW;// position of the eyes relative to monster's origin.
	m_flFieldOfView = 0.5;
	m_NPCState = NPC_STATE_NONE;
	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_DOORS_GROUP);

	NPCInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CNPC_Thotdoggo::Precache()
{
	PrecacheModel("models/thot_doggo.mdl");
	PrecacheScriptSound("Thotdoggo.AttackHit");
	PrecacheScriptSound("Thotdoggo.AttackMiss");
	PrecacheScriptSound("Thotdoggo.Pain");
	PrecacheScriptSound("Thotdoggo.Alert");
	PrecacheScriptSound("Thotdoggo.Attack");

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: Returns this monster's place in the relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_Thotdoggo::Classify(void)
{
	return CLASS_COMBINE;
}

int CNPC_Thotdoggo::TranslateSchedule(int scheduleType)
{
	switch (scheduleType)
	{
	case SCHED_CHASE_ENEMY:
		return SCHED_THOTDOGGO_CHASE_ENEMY;
		break;
	}

	return BaseClass::TranslateSchedule(scheduleType);
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CNPC_Thotdoggo::HandleAnimEvent(animevent_t *pEvent)
{
	Vector v_forward, v_right;
	switch (pEvent->event)
	{
	case THOTDOGGO_AE_ATTACK_RIGHT:
	{
		// do stuff for this event.
		//		ALERT( at_console, "Slash right!\n" );

		Vector vecMins = GetHullMins();
		Vector vecMaxs = GetHullMaxs();
		vecMins.z = vecMins.x;
		vecMaxs.z = vecMaxs.x;

		CBaseEntity *pHurt = CheckTraceHullAttack(70, vecMins, vecMaxs, sk_thotdoggo_dmg_one_slash.GetFloat(), DMG_CLUB);
		CPASAttenuationFilter filter(this);
		if (pHurt)
		{
			if (pHurt->GetFlags() & (FL_NPC | FL_CLIENT))
			{
				pHurt->ViewPunch(QAngle(5, 0, 18));

				GetVectors(&v_forward, &v_right, NULL);

				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() - v_right * 100 - v_forward * 100);
			}
			// Play a random attack hit sound
			EmitSound(filter, entindex(), "Thotdoggo.AttackHit");
		}
		else // Play a random attack miss sound
			EmitSound(filter, entindex(), "Thotdoggo.AttackMiss");

		if (random->RandomInt(0, 1))
			AttackSound();
	}
	break;

	case THOTDOGGO_AE_ATTACK_LEFT:
	{
		// do stuff for this event.
		//		ALERT( at_console, "Slash left!\n" );
		Vector vecMins = GetHullMins();
		Vector vecMaxs = GetHullMaxs();
		vecMins.z = vecMins.x;
		vecMaxs.z = vecMaxs.x;

		CBaseEntity *pHurt = CheckTraceHullAttack(70, vecMins, vecMaxs, sk_thotdoggo_dmg_one_slash.GetFloat(), DMG_CLUB);

		CPASAttenuationFilter filter2(this);
		if (pHurt)
		{
			if (pHurt->GetFlags() & (FL_NPC | FL_CLIENT))
			{
				pHurt->ViewPunch(QAngle(5, 0, -18));

				GetVectors(&v_forward, &v_right, NULL);

				//pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() - v_right * 100);
			}
			EmitSound(filter2, entindex(), "Thotdoggo.AttackHit");
		}
		else
		{
			EmitSound(filter2, entindex(), "Thotdoggo.AttackMiss");
		}

		if (random->RandomInt(0, 1))
			AttackSound();
	}
	break;

	case THOTDOGGO_AE_ATTACK_BOTH:
	{
		// do stuff for this event.
		Vector vecMins = GetHullMins();
		Vector vecMaxs = GetHullMaxs();
		vecMins.z = vecMins.x;
		vecMaxs.z = vecMaxs.x;

		CBaseEntity *pHurt = CheckTraceHullAttack(70, vecMins, vecMaxs, sk_thotdoggo_dmg_both_slash.GetFloat(), DMG_CLUB);


		CPASAttenuationFilter filter3(this);
		if (pHurt)
		{
			if (pHurt->GetFlags() & (FL_NPC | FL_CLIENT))
			{
				pHurt->ViewPunch(QAngle(5, 0, 0));

				GetVectors(&v_forward, &v_right, NULL);
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() - v_right * 100);
			}
			EmitSound(filter3, entindex(), "Thotdoggo.AttackHit");
		}
		else
			EmitSound(filter3, entindex(), "Thotdoggo.AttackMiss");

		if (random->RandomInt(0, 1))
			AttackSound();
	}
	break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}


static float DamageForce(const Vector &size, float damage)
{
	float force = damage * ((32 * 32 * 72.0) / (size.x * size.y * size.z)) * 5;

	if (force > 1000.0)
	{
		force = 1000.0;
	}

	return force;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pInflictor - 
//			pAttacker - 
//			flDamage - 
//			bitsDamageType - 
// Output : int
//-----------------------------------------------------------------------------
int CNPC_Thotdoggo::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	CTakeDamageInfo info = inputInfo;

	// Take 30% damage from bullets
	if (info.GetDamageType() == DMG_BULLET)
	{
		// This pushes Superchav
		//Vector vecDir = GetAbsOrigin() - info.GetInflictor()->WorldSpaceCenter();
		//VectorNormalize(vecDir);
		//float flForce = DamageForce(WorldAlignSize(), info.GetDamage());
		//SetAbsVelocity(GetAbsVelocity() + vecDir * flForce);
		info.ScaleDamage(0.3f);
	}

	//// HACK HACK -- until we fix this.
	//if (IsAlive())
	//	PainSound(info);

	return BaseClass::OnTakeDamage_Alive(info);
}

float CNPC_Thotdoggo::MaxYawSpeed(void)
{
	return BaseClass::MaxYawSpeed();
}

bool CNPC_Thotdoggo::ShouldGib(const CTakeDamageInfo &info)
{
	return false;
}

void CNPC_Thotdoggo::PainSound(const CTakeDamageInfo &info)
{
	if (gpGlobals->curtime < m_flNextPainSoundTime)
		return;

	m_flNextPainSoundTime = gpGlobals->curtime + random->RandomFloat(1.8, 2.2);

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Thotdoggo.Pain");
}

void CNPC_Thotdoggo::AlertSound(void)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Thotdoggo.Alert");
}

//void CNPC_Thotdoggo::IdleSound(void)
//{
//	// Play a random idle sound
//	CPASAttenuationFilter filter(this);
//	EmitSound(filter, entindex(), "Thotdoggo.Idle");
//}

void CNPC_Thotdoggo::AttackSound(void)
{
	// Play a random attack sound
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Thotdoggo.Attack");
}

//=========================================================
// DeathSound
//=========================================================
void CNPC_Thotdoggo::DeathSound(const CTakeDamageInfo &info)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Thotdoggo.Pain");
}

int CNPC_Thotdoggo::MeleeAttack1Conditions(float flDot, float flDist)
{
	if (flDist > 64)
	{
		return COND_TOO_FAR_TO_ATTACK;
	}
	else if (flDot < 0.7)
	{
		return 0;
	}
	else if (GetEnemy() == NULL)
	{
		return 0;
	}

	return COND_CAN_MELEE_ATTACK1;
}

void CNPC_Thotdoggo::RemoveIgnoredConditions(void)
{
	if (GetActivity() == ACT_MELEE_ATTACK1)
	{
		// Nothing stops an attacking thotdoggo.
		ClearCondition(COND_LIGHT_DAMAGE);
		ClearCondition(COND_HEAVY_DAMAGE);
	}

	if ((GetActivity() == ACT_SMALL_FLINCH) || (GetActivity() == ACT_BIG_FLINCH))
	{
		if (m_flNextFlinch < gpGlobals->curtime)
			m_flNextFlinch = gpGlobals->curtime + THOTDOGGO_FLINCH_DELAY;
	}

	BaseClass::RemoveIgnoredConditions();
}

//------------------------------------------------------------------------------
//
// Schedules
//
//------------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC(npc_thotdoggo, CNPC_Thotdoggo)

//=========================================================
// > ChaseEnemy
//=========================================================
DEFINE_SCHEDULE
(
SCHED_THOTDOGGO_CHASE_ENEMY,

"	Tasks"
"		 TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CHASE_ENEMY_FAILED"
"		 TASK_SET_TOLERANCE_DISTANCE	24"
"		 TASK_GET_CHASE_PATH_TO_ENEMY	600"
"		 TASK_RUN_PATH					0"
"		 TASK_WAIT_FOR_MOVEMENT			0"
"		 TASK_FACE_ENEMY				0"
""
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_ENEMY_DEAD"
"		COND_ENEMY_UNREACHABLE"
"		COND_CAN_RANGE_ATTACK1"
"		COND_CAN_MELEE_ATTACK1"
"		COND_CAN_RANGE_ATTACK2"
"		COND_CAN_MELEE_ATTACK2"
"		COND_TOO_CLOSE_TO_ATTACK"
"		COND_TASK_FAILED"
)

AI_END_CUSTOM_NPC()