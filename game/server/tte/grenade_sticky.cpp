//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: TTE Sticky Grenade
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "grenade_sticky.h"
#include "weapon_ar2.h"
#include "soundent.h"
#include "decals.h"
#include "shake.h"
#include "smoke_trail.h"
#include "ar2_explosion.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "world.h"

#ifdef PORTAL
	#include "portal_util_shared.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define AR2_GRENADE_MAX_DANGER_RADIUS	300

extern short	g_sModelIndexFireball;			// (in combatweapon.cpp) holds the index for the smoke cloud

// Moved to HL2_SharedGameRules because these are referenced by shared AmmoDef functions
extern ConVar    sk_plr_dmg_smg1_grenade;
extern ConVar    sk_npc_dmg_smg1_grenade;
extern ConVar    sk_max_smg1_grenade;
extern ConVar g_CV_SmokeTrail;

ConVar tte_sticky_grenade_radius( "tte_sticky_grenade_radious", "350" );
ConVar tte_sticky_grenade_expl_delay( "tte_sticky_grenade_expl_delay", "3" );

BEGIN_DATADESC( CGrenadeSticky )

	DEFINE_FIELD( m_hSmokeTrail, FIELD_EHANDLE ),
	DEFINE_FIELD( m_fSpawnTime, FIELD_TIME ),
	DEFINE_FIELD( m_fDangerRadius, FIELD_FLOAT ),
	DEFINE_FIELD( m_flGrenadeDetonateDelay, FIELD_FLOAT ),

	// Function pointers
	DEFINE_ENTITYFUNC( GrenadeStickyTouch ),
	DEFINE_THINKFUNC( GrenadeStickyThink ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( grenade_sticky, CGrenadeSticky );

void CGrenadeSticky::Spawn( void )
{
	Precache( );

	SetModel( "models/Weapons/w_grenade.mdl");
	SetSize( -Vector(4,4,4), Vector(4,4,4) );

	SetUse( &CGrenadeSticky::DetonateUse );
	SetTouch( &CGrenadeSticky::GrenadeStickyTouch );
	SetThink( &CGrenadeSticky::GrenadeStickyThink );
	SetNextThink( gpGlobals->curtime + 0.1f );

	SetCollisionGroup( COLLISION_GROUP_WEAPON );
	CreateVPhysics();

	if( GetOwnerEntity() && GetOwnerEntity()->IsPlayer() )
	{
		m_flDamage = sk_plr_dmg_smg1_grenade.GetFloat();
	}
	else
	{
		m_flDamage = sk_npc_dmg_smg1_grenade.GetFloat();
	}

	m_DmgRadius		= tte_sticky_grenade_radius.GetFloat();
	m_flGrenadeDetonateDelay = gpGlobals->curtime;
	m_takedamage	= DAMAGE_YES;
	m_iHealth		= 1;

	SetFriction( 0.8 );
	SetSequence( 0 );

	m_fDangerRadius = 100;

	m_fSpawnTime = gpGlobals->curtime;

	// -------------
	// Smoke trail.
	// -------------
	if( g_CV_SmokeTrail.GetInt() && !IsXbox() )
	{
		m_hSmokeTrail = SmokeTrail::CreateSmokeTrail();
		
		if( m_hSmokeTrail )
		{
			m_hSmokeTrail->m_SpawnRate = 48;
			m_hSmokeTrail->m_ParticleLifetime = 1;
			m_hSmokeTrail->m_StartColor.Init(0.1f, 0.1f, 0.1f);
			m_hSmokeTrail->m_EndColor.Init(0,0,0);
			m_hSmokeTrail->m_StartSize = 12;
			m_hSmokeTrail->m_EndSize = m_hSmokeTrail->m_StartSize * 4;
			m_hSmokeTrail->m_SpawnRadius = 4;
			m_hSmokeTrail->m_MinSpeed = 4;
			m_hSmokeTrail->m_MaxSpeed = 24;
			m_hSmokeTrail->m_Opacity = 0.2f;

			m_hSmokeTrail->SetLifetime(10.0f);
			m_hSmokeTrail->FollowEntity(this);
		}
	}

	m_flGrenadeDetonateDelay = gpGlobals->curtime + tte_sticky_grenade_expl_delay.GetFloat();
	m_bIsLive = true;
}

//-----------------------------------------------------------------------------
// Purpose:  The grenade has a slight delay before it goes live.  That way the
//			 person firing it can bounce it off a nearby wall.  However if it
//			 hits another character it blows up immediately
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CGrenadeSticky::GrenadeStickyThink( void )
{
	SetNextThink( gpGlobals->curtime + 0.05f );
	
	// If I just went solid and my velocity is zero, it means I'm resting on
	// the floor already when I went solid so blow up
	if (m_bIsLive)
	{
		if( gpGlobals->curtime > m_flGrenadeDetonateDelay )
		{
			Detonate();
		}
	}

	// The old way of making danger sounds would scare the crap out of EVERYONE between you and where the grenade
	// was going to hit. The radius of the danger sound now 'blossoms' over the grenade's lifetime, making it seem
	// dangerous to a larger area downrange than it does from where it was fired.
	if( m_fDangerRadius <= AR2_GRENADE_MAX_DANGER_RADIUS )
	{
		m_fDangerRadius += ( AR2_GRENADE_MAX_DANGER_RADIUS * 0.05 );
	}

	CSoundEnt::InsertSound( SOUND_DANGER, GetAbsOrigin() + GetAbsVelocity() * 0.5, m_fDangerRadius, 0.2, this, SOUNDENT_CHANNEL_REPEATED_DANGER );
}

void CGrenadeSticky::Event_Killed( const CTakeDamageInfo &info )
{
	Detonate( );
}

void CGrenadeSticky::GrenadeStickyTouch( CBaseEntity *pOther )
{
	Assert( pOther );
	if ( !pOther->IsSolid() || m_flGrenadeDetonateDelay > gpGlobals->curtime )
		return;

	m_flGrenadeDetonateDelay = gpGlobals->curtime + tte_sticky_grenade_expl_delay.GetFloat();
	m_bIsLive = true;
}

void CGrenadeSticky::Detonate(void)
{
	if (!m_bIsLive)
	{
		return;
	}
	m_bIsLive		= false;
	m_takedamage	= DAMAGE_NO;	

	if(m_hSmokeTrail)
	{
		UTIL_Remove(m_hSmokeTrail);
		m_hSmokeTrail = NULL;
	}

	CPASFilter filter( GetAbsOrigin() );

	te->Explosion( filter, 0.0,
		&GetAbsOrigin(), 
		g_sModelIndexFireball,
		2.0, 
		15,
		TE_EXPLFLAG_NONE,
		m_DmgRadius,
		m_flDamage );

	Vector vecForward = GetAbsVelocity();
	VectorNormalize(vecForward);
	trace_t		tr;
	UTIL_TraceLine ( GetAbsOrigin(), GetAbsOrigin() + 60*vecForward, MASK_SHOT, 
		this, COLLISION_GROUP_NONE, &tr);


	if ((tr.m_pEnt != GetWorldEntity()) || (tr.hitbox != 0))
	{
		// non-world needs smaller decals
		if( tr.m_pEnt && !tr.m_pEnt->IsNPC() )
		{
			UTIL_DecalTrace( &tr, "SmallScorch" );
		}
	}
	else
	{
		UTIL_DecalTrace( &tr, "Scorch" );
	}

	UTIL_ScreenShake( GetAbsOrigin(), 25.0, 150.0, 1.0, 750, SHAKE_START );

	RadiusDamage ( CTakeDamageInfo( this, GetThrower(), m_flDamage, DMG_BLAST ), GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );

	UTIL_Remove( this );
}

void CGrenadeSticky::Precache( void )
{
	PrecacheModel("models/Weapons/w_grenade.mdl"); 
}


CGrenadeSticky::CGrenadeSticky(void)
{
	m_hSmokeTrail  = NULL;
}

bool CGrenadeSticky::CreateVPhysics()
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, 0, false );
	return true;
}