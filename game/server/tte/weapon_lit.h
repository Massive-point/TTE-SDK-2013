//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: TTE flare gun
//
//=============================================================================//

#include "basehlcombatweapon.h"
#include "soundenvelope.h"
#include "weapon_flaregun.h"

#ifndef WEAPON_TTE_H
#define WEAPON_TTE_H
#ifdef _WIN32
#pragma once
#endif

//---------------------
// Flaregun
//---------------------
class CLitGun:public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS( CLitGun, CBaseHLCombatWeapon );

	DECLARE_SERVERCLASS();

	void Precache( void );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
};

#endif // WEAPON_TTE_H
