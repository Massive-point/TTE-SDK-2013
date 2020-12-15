//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Crossroads devtest
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "basehlcombatweapon.h"
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponSTG44 : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponSTG44, CHLSelectFireMachineGun );
	DECLARE_SERVERCLASS(); 
//	DECLARE_ACTTABLE();
	
	CWeaponSTG44();

	void AddViewKick();
	void Spawn();
	void Precache();
	void PrimaryAttack();

	float GetFireRate( void ) { return 0.126f; } //476 RPM. Used to be 0.0923f
};


IMPLEMENT_SERVERCLASS_ST( CWeaponSTG44, DT_WeaponSTG44 )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_stg44, CWeaponSTG44 );
PRECACHE_WEAPON_REGISTER( weapon_stg44 );

BEGIN_DATADESC( CWeaponSTG44 )
END_DATADESC()

CWeaponSTG44::CWeaponSTG44()
{
}

void CWeaponSTG44::Spawn( )
{
	BaseClass::Spawn();
}


void CWeaponSTG44::Precache()
{
	PrecacheParticleSystem( "weapon_muzzle_smoke_long_b" );

	BaseClass::Precache();
}

void CWeaponSTG44::PrimaryAttack()
{
	//crossroads devtest
	if(m_nShotsFired >= 5)
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		//We shot >5, clean up and start the muzzle smoking effect (like l4d)
		DispatchParticleEffect( "weapon_muzzle_smoke_long", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", true);
	}
	
	BaseClass::PrimaryAttack();
}

void CWeaponSTG44::AddViewKick( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if( pPlayer )
	{
		DoRecoil( pPlayer, GetWeaponRecoilViewKickVector(), m_fFireDuration, GetWeaponRecoilSlideLimit(), GetWeaponRecoilViewSlideVector() );
	}
}