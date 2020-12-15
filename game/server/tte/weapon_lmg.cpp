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

class CWeaponLMG : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponLMG, CHLSelectFireMachineGun );
	DECLARE_SERVERCLASS(); 
//	DECLARE_ACTTABLE();
	
	CWeaponLMG();

	void AddViewKick();
	void Spawn();
	void Precache();
	void PrimaryAttack();

	float GetFireRate( void ) { return 0.1f; }
};

IMPLEMENT_SERVERCLASS_ST( CWeaponLMG, DT_WeaponLMG )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_lmg, CWeaponLMG );
PRECACHE_WEAPON_REGISTER( weapon_lmg );

BEGIN_DATADESC( CWeaponLMG )
END_DATADESC()

CWeaponLMG::CWeaponLMG()
{
}

void CWeaponLMG::Spawn( )
{
	BaseClass::Spawn();
}


void CWeaponLMG::Precache()
{
	PrecacheParticleSystem( "weapon_muzzle_smoke_long_b" );

	BaseClass::Precache();
}

void CWeaponLMG::PrimaryAttack()
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

void CWeaponLMG::AddViewKick( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if( pPlayer )
	{
		DoRecoil( pPlayer, GetWeaponRecoilViewKickVector(), m_fFireDuration, GetWeaponRecoilSlideLimit(), GetWeaponRecoilViewSlideVector() );
	}
}