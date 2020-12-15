//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: TTE meme cannon
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "particle_parse.h"
#include "in_buttons.h"
#include "props.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar tte_memecannon_radius( "tte_memecannon_radius", "64.0" );

class CWeaponMeme : public CHLSelectFireMachineGun
{
public:
	DECLARE_CLASS( CWeaponMeme, CHLSelectFireMachineGun );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	
	CWeaponMeme();
	~CWeaponMeme();

	virtual void	Precache( void );
	virtual void	FireEffectStarted( void );
	virtual void	FireEffectStopped( void );
	virtual void	ItemPostFrame( void );

	virtual bool	StartFireEffect( void );

protected:
	void BurnObjectsInRadius( float distance, float radius );

private:
	float m_flIgnitionTime;

	bool m_bStartBurning;
	bool m_bHit;
	bool m_bNeedsReload;

	int	m_beamIndex;

private:
	CNetworkVar( bool, m_bActive );
};

BEGIN_DATADESC( CWeaponMeme )
	DEFINE_FIELD( m_beamIndex, FIELD_INTEGER ),
	DEFINE_FIELD( m_bActive, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bNeedsReload, FIELD_BOOLEAN ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeaponMeme, DT_WeaponMeme)
	SendPropInt( SENDINFO( m_bActive) ),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_meme, CWeaponMeme );
PRECACHE_WEAPON_REGISTER( weapon_meme );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponMeme::CWeaponMeme()
{
	m_bStartBurning = false;
	m_bHit = false;

	m_flIgnitionTime = 0.0f;

	m_bActive = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponMeme::~CWeaponMeme()
{
	StopParticleEffects( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMeme::Precache( void )
{
	m_beamIndex = PrecacheModel( "sprites/bluelaser1.vmt" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMeme::FireEffectStarted( void )
{
	m_bStartBurning = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMeme::FireEffectStopped( void )
{
	m_bStartBurning = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponMeme::StartFireEffect( void )
{
	return ( m_bStartBurning == false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMeme::BurnObjectsInRadius( float distance, float radius )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( !pOwner )
	{
		return;
	}

	// Ok.. find eligible entities in a cone in front of us.
	Vector vOrigin = pOwner->Weapon_ShootPosition( );
	Vector vForward, vRight, vUp;
	AngleVectors( pOwner->GetAbsAngles(), &vForward, &vRight, &vUp );

	trace_t	tr;
	UTIL_TraceLine( vOrigin, vOrigin + vForward * MAX_TRACE_LENGTH, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr );

	int brightness;
	brightness = 255 * 1.0f;
	UTIL_Beam(  vOrigin + vRight * 5.0f - vUp * 1.0f,
				tr.endpos,
				m_beamIndex,
				0,		//halo index
				0,		//frame start
				2.0f,	//framerate
				0.1f,	//life
				3,		// width
				10,		// endwidth
				10,		// fadelength,
				0,		// noise
				255,		// red
				255,	// green
				255,		// blue,
				brightness, // bright
				100  // speed
				);

	CBaseEntity	*pEntity = NULL;

	while( ( pEntity = gEntList.FindEntityInSphere( pEntity, tr.endpos, tte_memecannon_radius.GetFloat() )) != NULL)
	{
		if( pEntity )
		{
			if( pEntity == GetOwner() )
			{
				continue;
			}

			pEntity->TakeDamage( CTakeDamageInfo( this, this, 100, DMG_DISSOLVE ) );
		}
	}

	UTIL_ImpactTrace( &tr, DMG_DISSOLVE );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMeme::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if( pOwner == NULL )
	{
		return;
	}

	/*if( m_bNeedsReload && !m_bInReload )
	{
		Reload();

		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	}

	if( m_bNeedsReload && gpGlobals->curtime > m_flNextPrimaryAttack && m_bInReload )
	{
		m_bNeedsReload = false;
		m_bInReload = false;
	}*/

	if( ( pOwner->m_nButtons & IN_ATTACK ) /*&& !m_bNeedsReload*/ )
	{
		if( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		{
			m_bHit = false;

			FireEffectStopped();

			if( pOwner->GetViewModel() )
			{
				StopParticleEffects( pOwner->GetViewModel() );
			}
		
			StopParticleEffects( this );

			pOwner->SwitchToNextBestWeapon( this );

			return;
		}

		Vector vMuzzlePos, vForward, vRight, vUp = Vector();
		QAngle vMuzzleAng = QAngle();

		if( pOwner->GetViewModel() )	// TODO: properly transform viewmodel attachment to world
		{
			pOwner->EyeVectors( &vForward, &vRight, &vUp );

			vMuzzlePos = pOwner->Weapon_ShootPosition() + vForward * 4.0f + vRight - vUp;
		}
		else
		{
			GetAttachment( LookupAttachment( "muzzle" ), vMuzzlePos, vMuzzleAng );

			AngleVectors( vMuzzleAng, &vForward );
		}

		//Drain ammo
		if( m_flNextPrimaryAttack < gpGlobals->curtime )
		{
			pOwner->SetAnimation( PLAYER_ATTACK1 );
			pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );

			m_flNextPrimaryAttack = gpGlobals->curtime + 0.65f;// GetFireRate();

			BurnObjectsInRadius( 0.0f, 0.0f );
			SendWeaponAnim( GetPrimaryAttackActivity() );

			//m_bNeedsReload = true;
		}
	}

	if( pOwner->m_afButtonReleased & IN_ATTACK || !( pOwner->m_nButtons & IN_ATTACK ) /*|| m_bNeedsReload*/ )	// if attack button was released or we don't press it at all
	{
		m_bHit = false;

		FireEffectStopped();

		if( pOwner->GetViewModel() )
		{
			StopParticleEffects( pOwner->GetViewModel() );
		}
		
		StopParticleEffects( this );
	}

	BaseClass::ItemPostFrame();
}