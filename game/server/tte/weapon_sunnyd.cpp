//========= Copyright © 1963-2020, Sunny Delight Beverages Co, All rights reserved. ============//
//
// Purpose:	SunnyD
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
#include "gib.h"

extern ConVar sk_plr_sunnyd_heal;
extern ConVar sk_plr_sunnyd_burptime;
extern ConVar sk_plr_sunnyd_burptimelimit;
extern ConVar sk_plr_sunnyd_burptimeclamp;

//-----------------------------------------------------------------------------
// CWeaponSunnyd
//-----------------------------------------------------------------------------

class CWeaponSunnyd : public CBaseHLCombatWeapon
{
public:

	DECLARE_CLASS(CWeaponSunnyd, CBaseHLCombatWeapon);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CWeaponSunnyd();

	void			Precache(void);
	bool			Deploy(void);
	void			Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	virtual void	ItemPostFrame(void);
	void			PrimaryAttack(void);
	void			DecrementAmmo(CBaseCombatCharacter *pOwner);
	void			Needleheal(void);
	void			TheBurp(void);
	void			WeaponIdle(void);
	void			DropEmpty(void);
	bool			WeaponShouldBeLowered(void);

	float			m_flBurpTime = 0;

};

IMPLEMENT_SERVERCLASS_ST(CWeaponSunnyd, DT_WeaponSunnyd)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_sunnyd, CWeaponSunnyd);
PRECACHE_WEAPON_REGISTER(weapon_sunnyd);

BEGIN_DATADESC(CWeaponSunnyd)
DEFINE_FIELD(m_flBurpTime, FIELD_TIME),
END_DATADESC()

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponSunnyd::CWeaponSunnyd()
{
	m_bFiresUnderwater = true;
}

//-----------------------------------------------------------------------------
// Purpose: Precache the weapon
//-----------------------------------------------------------------------------
void CWeaponSunnyd::Precache(void)
{
	BaseClass::Precache();
	PrecacheModel("models/weapons/w_sunnyd_empty.mdl");
}

bool CWeaponSunnyd::Deploy(void)
{
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.01f;

	return BaseClass::Deploy();
}

//------------------------------------------------------------------------------
// Purpose : Update weapon
//------------------------------------------------------------------------------
void CWeaponSunnyd::ItemPostFrame(void)
{
	CBaseCombatWeapon::ItemPostFrame();

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	//Don't attack if holstered.
	if (pOwner->m_bHolsteredAW)
	{
		return;
	}

	if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
		SecondaryAttack();
	}

	if ((pOwner->m_nButtons & IN_IRONSIGHT) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
		SecondaryAttack();
	}

	if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		PrimaryAttack();
	}
	else
	{
		WeaponIdle();
		return;
	}
}

void CWeaponSunnyd::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	CBaseViewModel *ActiveVM = pOwner->GetViewModel(0);

	switch (pEvent->event)
	{
	case EVENT_WEAPON_THROW:
		Needleheal();
		break;

	case EVENT_WEAPON_THROW2:
		TheBurp();
		break;

	case EVENT_WEAPON_THROW3:
		DropEmpty();
		break;

	case EVENT_WEAPON_AR1:
		if (ActiveVM)
		{
			ActiveVM->SetBodygroup(1, 0);
		}
		break;

	case EVENT_WEAPON_AR2:
		if (ActiveVM)
		{
			ActiveVM->SetBodygroup(1, 1);
		}
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}

}

void CWeaponSunnyd::PrimaryAttack()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}
	//Must have ammo
	if ( pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) == 1)
		SendWeaponAnim(ACT_VM_SECONDARYATTACK);
	else
		SendWeaponAnim(ACT_VM_PRIMARYATTACK);

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

void CWeaponSunnyd::Needleheal()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}
	
	pPlayer->TakeHealth(sk_plr_sunnyd_heal.GetFloat(), DMG_GENERIC);
	color32 sunny = { 240, 120, 0, 64 };
	UTIL_ScreenFade(pPlayer, sunny, 2.0f, 0.1f, FFADE_IN);

	if (m_flBurpTime <= gpGlobals->curtime)
	{
		m_flBurpTime = gpGlobals->curtime + sk_plr_sunnyd_burptime.GetFloat();
	}
	else
	{
		if (m_flBurpTime + sk_plr_sunnyd_burptime.GetFloat() > gpGlobals->curtime + sk_plr_sunnyd_burptimeclamp.GetFloat())
		{
			m_flBurpTime = gpGlobals->curtime + sk_plr_sunnyd_burptimeclamp.GetFloat();
		}
		else
		{
			m_flBurpTime = m_flBurpTime + sk_plr_sunnyd_burptime.GetFloat();
		}	
	}
	
	DecrementAmmo(pPlayer);
	//Warning("Burp Time: %5.2f\n",
	//	m_flBurpTime - gpGlobals->curtime);
}

void CWeaponSunnyd::TheBurp()
{
	if (m_flBurpTime > gpGlobals->curtime + sk_plr_sunnyd_burptimelimit.GetFloat())
	{
		WeaponSound(EMPTY);
	}
	//TheBurp() is called every time the drinking animation is finished, so we can use it to switch the weapon when we're out of ammo.
	if (!HasPrimaryAmmo())
	{
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		if (!pPlayer)
		{
			return;
		}

		pPlayer->SwitchToNextBestWeapon(this);
	}
}

void CWeaponSunnyd::DecrementAmmo(CBaseCombatCharacter *pOwner)
{
	pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
}

void CWeaponSunnyd::WeaponIdle(void)
{
	if (m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		if (!HasPrimaryAmmo())
		{
			if (HasWeaponIdleTimeElapsed())
			{
				SendWeaponAnim(ACT_CROSSBOW_IDLE_UNLOADED);
			}	
		}
		else
		{
			BaseClass::WeaponIdle();
		}	
	}
}

bool CWeaponSunnyd::WeaponShouldBeLowered(void)
{
	return false;
}

//Drops the bottle
void CWeaponSunnyd::DropEmpty(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;
	Vector	vForward, vRight, vUp;
	pOwner->EyeVectors(&vForward, &vRight, &vUp);
	Vector	muzzlePoint = pOwner->Weapon_ShootPosition() + vForward * 18.1768f + vRight * -0.197968f + vUp * -4.44936f;

	Vector vecThrow;
	AngularImpulse angImpulse;
	angImpulse = AngularImpulse(0, random->RandomInt(-0, 0), 0);
	pOwner->GetVelocity(&vecThrow, NULL);
	//vecThrow += vForward * 350 + Vector(0, 0, 50);
	vecThrow += vForward * 100;

	Vector vecAiming = pOwner->GetAutoaimVector(0);
	QAngle angAiming;
	VectorAngles(vecAiming, angAiming);
	angAiming.x = angAiming.x - 58.4611;
	angAiming.y = angAiming.y - 22.3922;
	angAiming.z = angAiming.z - 3.569;

	//Warning("Origin: %5.2f %5.2f %5.2f \n Angle: %5.2f %5.2f %5.2f \n",
	//	muzzlePoint.x,
	//	muzzlePoint.y,
	//	muzzlePoint.z,
	//	angAiming.x,
	//	angAiming.y,
	//	angAiming.z);

	CGib *pChunk = CREATE_ENTITY(CGib, "gib");
	pChunk->Spawn("models/weapons/w_sunnyd_empty.mdl");
	pChunk->SetBloodColor(DONT_BLEED);
	pChunk->SetAbsOrigin(muzzlePoint);
	//pChunk->SetAbsAngles(QAngle(-58.4611, -22.3922, 3.569));
	pChunk->SetAbsAngles(angAiming);
	pChunk->SetOwnerEntity(NULL);
	pChunk->SetCollisionGroup(COLLISION_GROUP_DEBRIS);
	//pChunk->m_lifeTime = gpGlobals->curtime + 6.0f;
	pChunk->m_bForceRemove = true;
	pChunk->SetNextThink(gpGlobals->curtime + 6.0f);
	pChunk->SetThink(&CGib::DieThink);
	IPhysicsObject *pPhysicsObject = pChunk->VPhysicsInitNormal(SOLID_VPHYSICS, pChunk->GetSolidFlags(), false);
	if (pPhysicsObject)
	{
		pPhysicsObject->EnableMotion(true);
		pChunk->SetAbsVelocity(vecThrow);
		pPhysicsObject->SetVelocity(&vecThrow, &angImpulse);
	}
}
