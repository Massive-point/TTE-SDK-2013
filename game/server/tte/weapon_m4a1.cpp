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

class CWeaponM4A1 : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponM4A1, CHLSelectFireMachineGun );
	DECLARE_SERVERCLASS(); 
//	DECLARE_ACTTABLE();
	
	CWeaponM4A1();

	void AddViewKick();
	void Spawn();
	void Precache();
	void SecondaryAttack();
	void PrimaryAttack();
	void WeaponIdle();

	float GetFireRate( void ) { return 0.09009f; }
	float m_flDoneSwitchingSilencer;

	int m_silencedModelIndex;
	int GetWorldModelIndex(void);

	bool m_bSilencerOn;
	bool m_inPrecache;
	bool m_bDelayFire;
	bool Deploy();
	bool Holster( CBaseCombatWeapon *pSwitchingTo );
	bool Reload();

	const char * GetWorldModel( void ) const;

	Activity GetDrawActivity( void );
	Activity GetPrimaryAttackActivity( void );

	WeaponSound_t GetAttackSound( void );

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

		if( m_bSilencerOn )
		{
			spread *= GetWpnData().flBulletSpreadSilencedModifier;
		}

		if( speed > 10 )
		{
			spread *= 1.5f;
		}

		cone = Vector( spread, spread, spread );

		return cone;
	}
};


IMPLEMENT_SERVERCLASS_ST( CWeaponM4A1, DT_WeaponM4A1 )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_m4a1, CWeaponM4A1 );
PRECACHE_WEAPON_REGISTER( weapon_m4a1 );

BEGIN_DATADESC( CWeaponM4A1 )
END_DATADESC()

CWeaponM4A1::CWeaponM4A1()
{
	m_inPrecache = false;
}


void CWeaponM4A1::Spawn( )
{
	BaseClass::Spawn();

	m_bSilencerOn = false;
	m_flDoneSwitchingSilencer = 0.0f;
	m_bDelayFire = true;
}


void CWeaponM4A1::Precache()
{
	PrecacheParticleSystem( "weapon_muzzle_smoke_long_b" );

	m_inPrecache = true;
	BaseClass::Precache();

	m_silencedModelIndex = CBaseEntity::PrecacheModel( GetWpnData().m_szSilencerModel );
	m_inPrecache = false;
}


int CWeaponM4A1::GetWorldModelIndex( void )
{
	if ( !m_bSilencerOn || m_inPrecache )
	{
		return m_iWorldModelIndex;
	}
	else
	{
		return m_silencedModelIndex;
	}
}

const char * CWeaponM4A1::GetWorldModel( void ) const
{
	if ( !m_bSilencerOn || m_inPrecache )
	{
		return BaseClass::GetWorldModel();
	}
	else
	{
		return GetWpnData().m_szSilencerModel;
	}
}

bool CWeaponM4A1::Deploy()
{
	m_flDoneSwitchingSilencer = 0.0f;
	m_bDelayFire = true;

	return BaseClass::Deploy();
}

WeaponSound_t CWeaponM4A1::GetAttackSound( void )
{
	if( m_bSilencerOn )
	{
		return SPECIAL1;
	}
	else
	{
		return SINGLE;
	}
}

Activity CWeaponM4A1::GetDrawActivity( void )
{
	if( m_bSilencerOn )
	{
		return ACT_VM_DRAW_SILENCED;
	}
	else
	{
		return ACT_VM_DRAW;
	}
}

Activity CWeaponM4A1::GetPrimaryAttackActivity( void )
{
	if( m_bSilencerOn )
	{
		return ACT_VM_PRIMARYATTACK_SILENCED;
	}
	else
	{
		return ACT_VM_PRIMARYATTACK;
	}
}

bool CWeaponM4A1::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if ( m_flDoneSwitchingSilencer > 0.0f && m_flDoneSwitchingSilencer > gpGlobals->curtime )
	{
		// still switching the silencer.  Cancel the switch.
		m_bSilencerOn = !m_bSilencerOn;
		//SetModel( GetWorldModel() );
	}

	return BaseClass::Holster( pSwitchingTo );
}

void CWeaponM4A1::SecondaryAttack()
{
	if ( m_bSilencerOn )
	{
		m_bSilencerOn = false;
		SendWeaponAnim( ACT_VM_DETACH_SILENCER );
	}
	else
	{
		m_bSilencerOn = true;
		SendWeaponAnim( ACT_VM_ATTACH_SILENCER );
	}
	m_flDoneSwitchingSilencer = gpGlobals->curtime + 2;

	m_flNextSecondaryAttack = gpGlobals->curtime + 2;
	m_flNextPrimaryAttack = gpGlobals->curtime + 2;
	SetWeaponIdleTime( gpGlobals->curtime + 2 );

	//SetModel( GetWorldModel() );
}

void CWeaponM4A1::PrimaryAttack()
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

bool CWeaponM4A1::Reload()
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return false;

	if (pPlayer->GetAmmoCount( GetPrimaryAmmoType() ) <= 0)
		return false;

	int iResult = 0;
	
	if ( m_bSilencerOn )
		 iResult = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD_SILENCED );
	else
		 iResult = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );

	if ( !iResult )
		return false;

	pPlayer->SetAnimation( PLAYER_RELOAD );

	m_bDelayFire = false;
	return true;
}

void CWeaponM4A1::WeaponIdle()
{
	if (m_flTimeWeaponIdle > gpGlobals->curtime)
		return;

	// only idle if the slid isn't back
	if ( m_iClip1 != 0 )
	{
		SetWeaponIdleTime( gpGlobals->curtime + 20 );
		if ( m_bSilencerOn )
			SendWeaponAnim( ACT_VM_IDLE_SILENCED );
		else
			SendWeaponAnim( ACT_VM_IDLE );
	}
}

void CWeaponM4A1::AddViewKick( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if( pPlayer )
	{
		DoRecoil( pPlayer, GetWeaponRecoilViewKickVector(), m_fFireDuration, GetWeaponRecoilSlideLimit(), GetWeaponRecoilViewSlideVector() );
	}
}