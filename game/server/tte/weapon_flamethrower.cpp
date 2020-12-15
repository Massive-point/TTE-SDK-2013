//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: TTE flamethrower
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "particle_parse.h"
#include "in_buttons.h"
#include "props.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	tte_flamethrower_radius( "weapon_lpo50_radius", "16", FCVAR_REPLICATED );
ConVar	tte_flamethrower_distance( "weapon_lpo50_distance", "512", FCVAR_REPLICATED );
ConVar	tte_flamethrower_debug( "weapon_flamethrower_debug", "0" );

class CWeaponFlamethower : public CHLSelectFireMachineGun
{
public:
	DECLARE_CLASS( CWeaponFlamethower, CHLSelectFireMachineGun );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	
	CWeaponFlamethower();
	~CWeaponFlamethower();

	virtual void	FireEffectStarted( void );
	virtual void	FireEffectStopped( void );
	virtual void	ItemPostFrame( void );

	virtual bool	StartFireEffect( void );

protected:
	void BurnObjectsInRadius( float distance, float radius );

	float m_flRadius;
	float m_flDistance;

private:
	float m_flIgnitionTime;

	bool m_bStartBurning;
	bool m_bHit;
};

BEGIN_DATADESC( CWeaponFlamethower )
	DEFINE_FIELD( m_flDistance, FIELD_FLOAT ),
	DEFINE_FIELD( m_flRadius, FIELD_FLOAT ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeaponFlamethower, DT_WeaponFlamethower)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_flamethrower, CWeaponFlamethower );
PRECACHE_WEAPON_REGISTER( weapon_flamethrower );

static inline void GenerateRandomFlameThrowerVelocity( float radius, Vector &vOut, const Vector &vForward, const Vector &vRight, const Vector &vUp )
{
	static float radians = DEG2RAD( 90 - radius );
	static float v = cos( radians ) / sin( radians );
	
	vOut = vForward + vRight * RandomFloat( -v, v ) + vUp * RandomFloat( -v, v );
	VectorNormalize( vOut );
}

template< class T >
int FindInArray( const T pTest, const T *pArray, int arrayLen )
{
	for ( int i=0; i < arrayLen; i++ )
	{
		if ( pTest == pArray[i] )
			return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponFlamethower::CWeaponFlamethower()
{
	m_flDistance = tte_flamethrower_distance.GetFloat();
	m_flRadius = tte_flamethrower_radius.GetFloat();

	m_bStartBurning = false;
	m_bHit = false;

	m_flIgnitionTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponFlamethower::~CWeaponFlamethower()
{
	StopParticleEffects( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFlamethower::FireEffectStarted( void )
{
	m_bStartBurning = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFlamethower::FireEffectStopped( void )
{
	m_bStartBurning = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponFlamethower::StartFireEffect( void )
{
	return ( m_bStartBurning == false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFlamethower::BurnObjectsInRadius( float distance, float radius )
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

	// Find some entities to burn.
	CBaseEntity *pHitEnts[64];
	int nHitEnts = 0;

	#define NUM_TEST_VECTORS	30
	for( int iTest = 0; iTest < NUM_TEST_VECTORS; iTest++ )
	{	
		Vector vVel;
		GenerateRandomFlameThrowerVelocity( radius, vVel, vForward, vRight, vUp );

		trace_t tr;
		UTIL_TraceLine( vOrigin, vOrigin + vVel * distance, MASK_SHOT & (~CONTENTS_HITBOX), this, COLLISION_GROUP_NONE, &tr );

		if ( tr.m_pEnt )
		{
			CBaseEntity *pTestEnt = tr.m_pEnt;

			if( pTestEnt )
			{
				if ( FindInArray( pTestEnt, pHitEnts, nHitEnts ) == -1 )
				{
					pHitEnts[nHitEnts++] = pTestEnt;

					if ( nHitEnts >= ARRAYSIZE( pHitEnts ) )
					{
						break;
					}
				}
			}
		}
	}

	if( nHitEnts < 1 )
	{
		m_bHit = false;
	}

	for( int iHitEnt = 0; iHitEnt < nHitEnts; iHitEnt++ )
	{
		if( !m_bHit )
		{
			m_flIgnitionTime = gpGlobals->curtime;
		}

		m_bHit = true;

		CBaseEntity *pEnt = pHitEnts[iHitEnt];

		float flDist = ( pEnt->GetAbsOrigin() - vOrigin ).Length();
		float flPercent = MAX( 0.1f, 1.0f - flDist / distance );
		float flDamage = flPercent * 15.0f;//GetWeaponGenericDamage();

		// We've got a target, and damage. Now hurt them.
		CTakeDamageInfo info;
		info.SetDamage( flDamage );
		info.SetAttacker( GetOwner() );
		info.SetInflictor( GetOwner() );
		info.SetDamageType( DMG_BURN );
		
		pEnt->TakeDamage( info );

		CBaseAnimating *pAnim = dynamic_cast< CBaseAnimating *>( pEnt );

		if( pAnim && ( gpGlobals->curtime > ( m_flIgnitionTime + RandomFloat( 0.01f, 0.25f ) ) ) )
		{
			pAnim->Ignite( 30.0f );
		}

		/*
		if( pEnt->IsPlayer() || pEnt->IsNPC() )
		{
			CBaseCombatCharacter *pCharacter = dynamic_cast< CBaseCombatCharacter *>( pEnt );

			if( pCharacter && ( gpGlobals->curtime > ( m_flIgnitionTime + RandomFloat( 0.2f, 0.5f ) ) ) )
			{
				pCharacter->Ignite( 3.0f, false, 16.0f, false );
			}
		}
		else
		{
			CDynamicProp *pProp = dynamic_cast< CDynamicProp *>( pEnt );

			if( pProp && ( gpGlobals->curtime > ( m_flIgnitionTime + RandomFloat( 0.2f, 0.5f ) ) ) )
			{
				pProp->Ignite( 3.0f, false, 16.0f, false );
			}
		}*/
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFlamethower::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if( pOwner == NULL )
	{
		return;
	}

	if( pOwner->m_nButtons & IN_ATTACK )
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

			m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();

			BurnObjectsInRadius( m_flDistance, m_flRadius );
			SendWeaponAnim( GetPrimaryAttackActivity() );
		}

		if( tte_flamethrower_debug.GetBool() )
		{
			int	radius = (int)m_flRadius;

			Vector vTestPos	= vMuzzlePos + ( vForward * m_flDistance );

			trace_t	tr;
			UTIL_TraceHull( vMuzzlePos, vTestPos, Vector( -radius, -radius, -radius ), Vector( radius, radius, radius ), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr );

			NDebugOverlay::Line( vMuzzlePos, tr.endpos, 0, 0, 128, false, 0.0f );
			
			NDebugOverlay::Box( vMuzzlePos, Vector(-1, -1, -1), Vector(1, 1, 1), 0, 0, 128, false, 0.0f );
			NDebugOverlay::Box( tr.endpos, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 0, 128, false, 0.0f );
			NDebugOverlay::Box( tr.endpos, Vector(-radius, -radius, -radius), Vector(radius, radius, radius), 0, 0, 255, false, 0.0f );
		}
	}

	if( pOwner->m_afButtonReleased & IN_ATTACK || !( pOwner->m_nButtons & IN_ATTACK ) )	// if attack button was released or we don't press it at all
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