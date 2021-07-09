//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Crowbar - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "soundent.h"
#include "basebludgeonweapon.h"
#include "vstdlib/random.h"
#include "npcevent.h"
#include "ai_basenpc.h"
#include "weapon_crowbar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar    sk_plr_dmg_crowbar("sk_plr_dmg_crowbar", "0");
ConVar    tte_chainsaw_damage("tte_chainsaw_damage", "50");
ConVar    sk_npc_dmg_crowbar("sk_npc_dmg_crowbar", "0");

ConVar    tte_chainsaw_cooldown("tte_chainsaw_cooldown", "4");

#define	SECONDARY_RANGE		75.0f

#define BLUDGEON_HULL_DIM		16
static const Vector g_bludgeonMins(-BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM);
static const Vector g_bludgeonMaxs(BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM);

//-----------------------------------------------------------------------------
// CWeaponCrowbar
//-----------------------------------------------------------------------------

IMPLEMENT_SERVERCLASS_ST(CWeaponCrowbar, DT_WeaponCrowbar)
END_SEND_TABLE()

#ifndef HL2MP
LINK_ENTITY_TO_CLASS(weapon_crowbar, CWeaponCrowbar);
PRECACHE_WEAPON_REGISTER(weapon_crowbar);
#endif

acttable_t CWeaponCrowbar::m_acttable[] =
{
	{ ACT_MELEE_ATTACK1, ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE, ACT_IDLE_ANGRY_MELEE, false },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_MELEE, false },
};

BEGIN_DATADESC(CWeaponCrowbar)
DEFINE_FIELD(m_flNextSpinTime, FIELD_TIME),
DEFINE_FIELD(m_flNextChainsawTime, FIELD_TIME),
END_DATADESC()

IMPLEMENT_ACTTABLE(CWeaponCrowbar);

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponCrowbar::CWeaponCrowbar(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: Get the damage amount for the animation we're doing
// Input  : hitActivity - currently played activity
// Output : Damage amount
//-----------------------------------------------------------------------------
float CWeaponCrowbar::GetDamageForActivity(Activity hitActivity)
{
	if (hitActivity == ACT_VM_HITCENTER2)
	{
		//Sets the damage for secondary attack.
		if ((GetOwner() != NULL) && (GetOwner()->IsPlayer()))
			return (sk_plr_dmg_crowbar.GetFloat() * 0.8);
	}

	if ((GetOwner() != NULL) && (GetOwner()->IsPlayer()))
		return sk_plr_dmg_crowbar.GetFloat();

	return sk_npc_dmg_crowbar.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CWeaponCrowbar::AddViewKick(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	QAngle punchAng;

	punchAng.x = random->RandomFloat(1.0f, 2.0f);
	punchAng.y = random->RandomFloat(-2.0f, -1.0f);
	punchAng.z = 0.0f;

	pPlayer->ViewPunch(punchAng);
}


//-----------------------------------------------------------------------------
// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
//-----------------------------------------------------------------------------
ConVar sk_crowbar_lead_time("sk_crowbar_lead_time", "0.9");

int CWeaponCrowbar::WeaponMeleeAttack1Condition(float flDot, float flDist)
{
	// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
	CAI_BaseNPC *pNPC = GetOwner()->MyNPCPointer();
	CBaseEntity *pEnemy = pNPC->GetEnemy();
	if (!pEnemy)
		return COND_NONE;

	Vector vecVelocity;
	vecVelocity = pEnemy->GetSmoothedVelocity();

	// Project where the enemy will be in a little while
	float dt = sk_crowbar_lead_time.GetFloat();
	dt += random->RandomFloat(-0.3f, 0.2f);
	if (dt < 0.0f)
		dt = 0.0f;

	Vector vecExtrapolatedPos;
	VectorMA(pEnemy->WorldSpaceCenter(), dt, vecVelocity, vecExtrapolatedPos);

	Vector vecDelta;
	VectorSubtract(vecExtrapolatedPos, pNPC->WorldSpaceCenter(), vecDelta);

	if (fabs(vecDelta.z) > 70)
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	Vector vecForward = pNPC->BodyDirection2D();
	vecDelta.z = 0.0f;
	float flExtrapolatedDist = Vector2DNormalize(vecDelta.AsVector2D());
	if ((flDist > 64) && (flExtrapolatedDist > 64))
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	float flExtrapolatedDot = DotProduct2D(vecDelta.AsVector2D(), vecForward.AsVector2D());
	if ((flDot < 0.7) && (flExtrapolatedDot < 0.7))
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}


//-----------------------------------------------------------------------------
// Animation event handlers
//-----------------------------------------------------------------------------
void CWeaponCrowbar::HandleAnimEventMeleeHit(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	// Trace up or down based on where the enemy is...
	// But only if we're basically facing that direction
	Vector vecDirection;
	AngleVectors(GetAbsAngles(), &vecDirection);

	CBaseEntity *pEnemy = pOperator->MyNPCPointer() ? pOperator->MyNPCPointer()->GetEnemy() : NULL;
	if (pEnemy)
	{
		Vector vecDelta;
		VectorSubtract(pEnemy->WorldSpaceCenter(), pOperator->Weapon_ShootPosition(), vecDelta);
		VectorNormalize(vecDelta);

		Vector2D vecDelta2D = vecDelta.AsVector2D();
		Vector2DNormalize(vecDelta2D);
		if (DotProduct2D(vecDelta2D, vecDirection.AsVector2D()) > 0.8f)
		{
			vecDirection = vecDelta;
		}
	}

	Vector vecEnd;
	VectorMA(pOperator->Weapon_ShootPosition(), 50, vecDirection, vecEnd);
	CBaseEntity *pHurt = pOperator->CheckTraceHullAttack(pOperator->Weapon_ShootPosition(), vecEnd,
		Vector(-16, -16, -16), Vector(36, 36, 36), sk_npc_dmg_crowbar.GetFloat(), DMG_CLUB, 0.75);

	// did I hit someone?
	if (pHurt)
	{
		// play sound
		WeaponSound(MELEE_HIT);

		// Fake a trace impact, so the effects work out like a player's crowbaw
		trace_t traceHit;
		UTIL_TraceLine(pOperator->Weapon_ShootPosition(), pHurt->GetAbsOrigin(), MASK_SHOT_HULL, pOperator, COLLISION_GROUP_NONE, &traceHit);
		ImpactEffect(traceHit);
	}
	else
	{
		WeaponSound(MELEE_MISS);
	}
}


//-----------------------------------------------------------------------------
// Animation event
//-----------------------------------------------------------------------------
void CWeaponCrowbar::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_MELEE_HIT:
		if ((GetOwner() != NULL) && (GetOwner()->IsPlayer()))
		{
			ChainsawAttack();
		}
		else
		{
			HandleAnimEventMeleeHit(pEvent, pOperator);
		}
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}


void CWeaponCrowbar::SecondaryAttack(void)
{
	if (m_flNextChainsawTime > gpGlobals->curtime)
	{
		return;
	}

	//BaseClass::SecondaryAttack();
	SendWeaponAnim(ACT_VM_HITCENTER2);
	//Play swing sound
	WeaponSound(SINGLE);
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
	m_flNextChainsawTime = gpGlobals->curtime + tte_chainsaw_cooldown.GetFloat();
}

void CWeaponCrowbar::WeaponIdle(void)
{
	if (m_flNextSecondaryAttack <= gpGlobals->curtime)
	{
		BaseClass::WeaponIdle();
	}
}

void CWeaponCrowbar::ItemPostFrame(void)
{
	//We use CBaseCombatWeapon::ItemPostFrame() to allow holstering for this weapon.
	CBaseCombatWeapon::ItemPostFrame();
	//Allow to use ADS button for secondary attack.

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	//Don't attack if holstered.
	if (pOwner->m_bHolsteredAW)
	{
		return;
	}

	if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		PrimaryAttack();
		m_flNextSpinTime = gpGlobals->curtime + 0.01f;
	}
	else if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
		SecondaryAttack();
		m_flNextSpinTime = gpGlobals->curtime + 0.01f;
	}
	else if ((pOwner->m_nButtons & IN_IRONSIGHT) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
		SecondaryAttack();
		m_flNextSpinTime = gpGlobals->curtime + 0.01f;
	}
	else if ((pOwner->m_nButtons & IN_RELOAD) && (m_flNextSpinTime <= gpGlobals->curtime))
	{
		SpinSpinner();
	}
	else
	{
		WeaponIdle();
		return;
	}
}


void CWeaponCrowbar::SpinSpinner(void)
{
	SendWeaponAnim(ACT_VM_FIDGET);
	m_flNextSpinTime = gpGlobals->curtime + SequenceDuration();
}

//------------------------------------------------------------------------------
// Purpose : Starts the swing of the weapon and determines the animation
// Input   : bIsSecondary - is this a secondary attack?
//------------------------------------------------------------------------------
void CWeaponCrowbar::ChainsawAttack(void)
{
	trace_t m_traceHit;
	// Try a ray
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	pOwner->EyeVectors(&forward, NULL, NULL);

	Vector swingEnd = swingStart + forward * SECONDARY_RANGE;

	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &m_traceHit);

	if (m_traceHit.fraction == 1.0)
	{
		float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull(swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &m_traceHit);
	}

	if (m_traceHit.fraction != 1.0f)
	{
		//Make sound for the AI

		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

		CSoundEnt::InsertSound(SOUND_BULLET_IMPACT, m_traceHit.endpos, 400, 0.2f, pPlayer);

		CBaseEntity	*pHitEntity = m_traceHit.m_pEnt;

		//Apply damage to a hit target
		if (pHitEntity != NULL)
		{
			Vector hitDirection;
			pPlayer->EyeVectors(&hitDirection, NULL, NULL);
			VectorNormalize(hitDirection);

			ClearMultiDamage();
			CTakeDamageInfo info(GetOwner(), GetOwner(), (tte_chainsaw_damage.GetFloat()), DMG_CLUB);
			CalculateMeleeDamageForce(&info, hitDirection, m_traceHit.endpos);
			pHitEntity->DispatchTraceAttack(info, hitDirection, &m_traceHit);
			ApplyMultiDamage();

			// Now hit all triggers along the ray that... 
			TraceAttackToTriggers(CTakeDamageInfo(GetOwner(), GetOwner(), (tte_chainsaw_damage.GetFloat()), DMG_CLUB), m_traceHit.startpos, m_traceHit.endpos, hitDirection);

			//Play an impact sound	
			if (pHitEntity->Classify() != CLASS_NONE)
			{
				WeaponSound(MELEE_HIT);
			}
			else
			{
				//WeaponSound(MELEE_HIT_WORLD);
			}
		}

		//Apply an impact effect
		UTIL_ImpactTrace(&m_traceHit, DMG_CLUB);

	}


}

