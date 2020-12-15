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

class CWeaponDMR : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponDMR, CBaseHLCombatWeapon );
	DECLARE_SERVERCLASS(); 
	//DECLARE_ACTTABLE();
	
	CWeaponDMR();

	void PrimaryAttack();
	void Precache();
	void AddViewKick();
	void ItemPostFrame( void );

	float	GetFireRate( void ) { return 0.09009f; } //0.075 burst fire mode

	virtual const Vector& GetBulletSpread( void )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

		float speed = pOwner->GetAbsVelocity().Length();
		float spread = GetWpnData().flHalfRadianBulletSpread;

		static Vector cone = Vector( spread, spread, spread );

		if( IsIronsighted() )
		{
			spread = GetWpnData().flHalfRadianBulletSpreadIronsighted;
		}

		if( speed > 10 )
		{
			spread *= 1.5f;
		}

		cone = Vector( spread, spread, spread );

		return cone;
	}

private:
	int	m_iBurstSize;
};

IMPLEMENT_SERVERCLASS_ST( CWeaponDMR, DT_WeaponDMR )
END_SEND_TABLE();

LINK_ENTITY_TO_CLASS( weapon_dmr, CWeaponDMR );
PRECACHE_WEAPON_REGISTER( weapon_dmr );

BEGIN_DATADESC( CWeaponDMR )
END_DATADESC()

CWeaponDMR::CWeaponDMR()
{
	m_iBurstSize = 1;
}

void CWeaponDMR::Precache()
{
	PrecacheParticleSystem( "weapon_muzzle_smoke_long_b" );

	BaseClass::Precache();
}

void CWeaponDMR::PrimaryAttack()
{
	if( m_iBurstSize != 0 )
	{
		//CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

		//We shot >5, clean up and start the muzzle smoking effect (like l4d)
	/*	if( m_nShotsFired > 5 )
		{
			DispatchParticleEffect( "weapon_muzzle_smoke_long_b", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", false );
		}*/

		BaseClass::PrimaryAttack();

		m_iBurstSize--;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDMR::AddViewKick( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if( pPlayer )
	{
		DoRecoil( pPlayer, GetWeaponRecoilViewKickVector(), m_fFireDuration, GetWeaponRecoilSlideLimit(), GetWeaponRecoilViewSlideVector() );
	}
}

void CWeaponDMR::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	if ( m_bInReload )
		return;
	
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	if ( !( pOwner->m_nButtons & IN_ATTACK ))
		m_iBurstSize = 1;
}