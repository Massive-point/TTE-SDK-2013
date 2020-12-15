//========= Copyright © 0-2021, VOLVO, All rights reserved. ============//
//
// Purpose:	Binoculars
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

//-----------------------------------------------------------------------------
// CWeaponBinoculars
//-----------------------------------------------------------------------------

class CWeaponBinoculars : public CBaseHLCombatWeapon
{
public:

	DECLARE_CLASS(CWeaponBinoculars, CBaseHLCombatWeapon);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CWeaponBinoculars();

	void			Precache(void);
	bool			Deploy(void);
	virtual void	ItemPostFrame(void);
	void			PrimaryAttack(void);
	void			WeaponIdle(void);
	bool			WeaponShouldBeLowered(void);

};

IMPLEMENT_SERVERCLASS_ST(CWeaponBinoculars, DT_WeaponBinoculars)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_binoculars, CWeaponBinoculars);
PRECACHE_WEAPON_REGISTER(weapon_binoculars);

BEGIN_DATADESC(CWeaponBinoculars)

END_DATADESC()

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponBinoculars::CWeaponBinoculars()
{
	m_bFiresUnderwater = true;
}

//-----------------------------------------------------------------------------
// Purpose: Precache the weapon
//-----------------------------------------------------------------------------
void CWeaponBinoculars::Precache(void)
{
	BaseClass::Precache();
}

bool CWeaponBinoculars::Deploy(void)
{
	return BaseClass::Deploy();
}

//------------------------------------------------------------------------------
// Purpose : Update weapon
//------------------------------------------------------------------------------
void CWeaponBinoculars::ItemPostFrame(void)
{
	CBaseCombatWeapon::ItemPostFrame();

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

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

void CWeaponBinoculars::PrimaryAttack()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}


}

void CWeaponBinoculars::WeaponIdle(void)
{
	BaseClass::WeaponIdle();
}

bool CWeaponBinoculars::WeaponShouldBeLowered(void)
{
	return false;
}
