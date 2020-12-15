//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: TTE flashbang Grenade
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "flashbang_projectile.h"
#include "weapon_ar2.h"
#include "soundent.h"
#include "decals.h"
#include "shake.h"
#include "smoke_trail.h"
#include "ar2_explosion.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "world.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define AR2_GRENADE_MAX_DANGER_RADIUS	300

extern short	g_sModelIndexFireball;			// (in combatweapon.cpp) holds the index for the smoke cloud

BEGIN_DATADESC( CFlashbangProjectile )
	DEFINE_FIELD( m_fSpawnTime, FIELD_TIME ),
	DEFINE_FIELD( m_fDangerRadius, FIELD_FLOAT ),
	DEFINE_FIELD( m_flGrenadeDetonateDelay, FIELD_FLOAT ),
	DEFINE_FIELD( m_flNPCBlindTimeMultiplier, FIELD_FLOAT ),

	// Function pointers
	DEFINE_ENTITYFUNC( FlashbangProjectileTouch ),
	DEFINE_THINKFUNC( FlashbangProjectileThink ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( flashbang_projectile, CFlashbangProjectile );

float PercentageOfFlashForPlayer(CBaseEntity *player, Vector flashPos, CBaseEntity *pevInflictor)
{
	float retval = 0.0f;

	trace_t tr;

	Vector pos = player->EyePosition();
	Vector vecRight, vecUp;

	QAngle tempAngle;
	VectorAngles(player->EyePosition() - flashPos, tempAngle);
	AngleVectors(tempAngle, NULL, &vecRight, &vecUp);

	vecRight.NormalizeInPlace();
	vecUp.NormalizeInPlace();

	UTIL_TraceLine( flashPos, pos,
		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_DEBRIS|CONTENTS_MONSTER),
		pevInflictor, COLLISION_GROUP_NONE, &tr );

	if ((tr.fraction == 1.0) || (tr.m_pEnt == player))
	{
		return 1.0;
	}

	if (!(player->IsPlayer()))
	{
		// if this entity isn't a player, it's a hostage or some other entity, then don't bother with the expensive checks
		// that come below.
		return 0.0;
	}

	// check the point straight up.
	pos = flashPos + vecUp*50;

	UTIL_TraceLine(flashPos, pos,
		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_DEBRIS|CONTENTS_MONSTER),
		pevInflictor, COLLISION_GROUP_NONE, &tr );

	pos = player->EyePosition();

	UTIL_TraceLine( tr.endpos, pos,
		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_DEBRIS|CONTENTS_MONSTER),
		pevInflictor, COLLISION_GROUP_NONE, &tr );

	if ((tr.fraction == 1.0) || (tr.m_pEnt == player))
	{
		retval += 0.167;
	}

	// check the point up and right.
	pos = flashPos + vecRight*75 + vecUp*10;

	UTIL_TraceLine( flashPos, pos,
		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_DEBRIS|CONTENTS_MONSTER),
		pevInflictor, COLLISION_GROUP_NONE, &tr );

	pos = player->EyePosition();

	UTIL_TraceLine( tr.endpos, pos,
		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_DEBRIS|CONTENTS_MONSTER),
		pevInflictor, COLLISION_GROUP_NONE, &tr );

	if ((tr.fraction == 1.0) || (tr.m_pEnt == player))
	{
		retval += 0.167;
	}

	pos = flashPos - vecRight*75 + vecUp*10;

	UTIL_TraceLine( flashPos, pos,
		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_DEBRIS|CONTENTS_MONSTER),
		pevInflictor, COLLISION_GROUP_NONE, &tr );

	pos = player->EyePosition();

	UTIL_TraceLine( tr.endpos, pos,
		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_DEBRIS|CONTENTS_MONSTER),
		pevInflictor, COLLISION_GROUP_NONE, &tr );

	if ((tr.fraction == 1.0) || (tr.m_pEnt == player))
	{
		retval += 0.167;
	}

	return retval;
}

// --------------------------------------------------------------------------------------------------- //
//
// RadiusDamage - this entity is exploding, or otherwise needs to inflict damage upon entities within a certain range.
// 
// only damage ents that can clearly be seen by the explosion!
// --------------------------------------------------------------------------------------------------- //

void RadiusFlash( 
	Vector vecSrc, 
	CBaseEntity *pevInflictor, 
	CBaseEntity *pevAttacker, 
	float flDamage, 
	int iClassIgnore, 
	int bitsDamageType )
{
	vecSrc.z += 1;// in case grenade is lying on the ground

	if ( !pevAttacker )
		pevAttacker = pevInflictor;
	
	trace_t		tr;
	float		flAdjustedDamage;
	variant_t	var;
	Vector		vecEyePos;
	float		fadeTime, fadeHold;
	Vector		vForward;
	Vector		vecLOS;
	float		flDot;
	
	CBaseEntity		*pEntity = NULL;
	static float	flRadius = 1500;
	float			falloff = flDamage / flRadius;

	bool bInWater = (UTIL_PointContents( vecSrc ) == CONTENTS_WATER);

	// iterate on all entities in the vicinity.
	while ((pEntity = gEntList.FindEntityInSphere( pEntity, vecSrc, flRadius )) != NULL)
	{	
		bool bPlayer = pEntity->IsPlayer();
		bool bIsNpc = pEntity->IsNPC();
		
		if( !bPlayer && !bIsNpc )
			continue;

		vecEyePos = pEntity->EyePosition();

		// blasts don't travel into or out of water
		if ( bInWater && pEntity->GetWaterLevel() == 0)
			continue;
		if (!bInWater && pEntity->GetWaterLevel() == 3)
			continue;

		float percentageOfFlash = PercentageOfFlashForPlayer(pEntity, vecSrc, pevInflictor);

		if ( percentageOfFlash > 0.0 )
		{
			// decrease damage for an ent that's farther from the grenade
			flAdjustedDamage = flDamage - ( vecSrc - pEntity->EyePosition() ).Length() * falloff;
		
			if ( flAdjustedDamage > 0 )
			{
				// See if we were facing the flash
				AngleVectors( pEntity->EyeAngles(), &vForward );

				vecLOS = ( vecSrc - vecEyePos );

				float flDistance = vecLOS.Length();

				// Normalize both vectors so the dotproduct is in the range -1.0 <= x <= 1.0 
				vecLOS.NormalizeInPlace();
					
				flDot = DotProduct (vecLOS, vForward);

				float startingAlpha = 255;
	
				// if target is facing the bomb, the effect lasts longer
				if( flDot >= 0.5 )
				{
					// looking at the flashbang
					fadeTime = flAdjustedDamage * 2.5f;
					fadeHold = flAdjustedDamage * 1.25f;
				}
				else if( flDot >= -0.5 )
				{
					// looking to the side
					fadeTime = flAdjustedDamage * 1.75f;
					fadeHold = flAdjustedDamage * 0.8f;
				}
				else
				{
					// facing away
					fadeTime = flAdjustedDamage * 1.0f;
					fadeHold = flAdjustedDamage * 0.75f;
					startingAlpha = 200;
				}

				fadeTime *= percentageOfFlash;
				fadeHold *= percentageOfFlash;

				if ( bPlayer )
				{
    				// blind players and bots
					CBasePlayer *player = ToBasePlayer( pEntity );
					player->Blind( fadeHold, fadeTime, startingAlpha );

					// deafen players and bots
					player->Deafen( flDistance );
				}
				else if ( bIsNpc )
				{
					variant_t val;					
					val.SetFloat( fadeTime );
					pEntity->AcceptInput( "flashbang", pevInflictor, pevAttacker, val, 0 );
				}
			}	
		}
	}

	CPVSFilter filter(vecSrc);
	te->DynamicLight( filter, 0.0, &vecSrc, 255, 255, 255, 2, 400, 0.1, 768 );
}

// --------------------------------------------------------------------------------------------------- //
// CFlashbangProjectile implementation.
// --------------------------------------------------------------------------------------------------- //

CFlashbangProjectile* CFlashbangProjectile::Create( 
	const Vector &position, 
	const QAngle &angles, 
	const Vector &velocity, 
	const AngularImpulse &angVelocity, 
	CBaseCombatCharacter *pOwner, float flNPCBlindDelay )
{
	CFlashbangProjectile *pGrenade = (CFlashbangProjectile*)CBaseEntity::Create( "flashbang_projectile", position, angles, pOwner );

	if( !pGrenade )
	{
		return NULL;
	}

	// Hits everything but debris
	pGrenade->SetCollisionGroup( COLLISION_GROUP_WEAPON );
	pGrenade->SetAbsVelocity( velocity );
//	pGrenade->SetupInitialTransmittedGrenadeVelocity( velocity );
	pGrenade->SetThrower( pOwner );
	pGrenade->m_flDamage = 100;
	pGrenade->ChangeTeam( pOwner->GetTeamNumber() );
	pGrenade->CreateVPhysics();

	pGrenade->SetTouch( &CFlashbangProjectile::FlashbangProjectileTouch );
	pGrenade->SetThink( &CFlashbangProjectile::FlashbangProjectileThink );
	pGrenade->SetNextThink( gpGlobals->curtime );

	pGrenade->SetNPCBlindTime( flNPCBlindDelay );

//	pGrenade->SetDetonateTimerLength( 1.5 );

	pGrenade->ApplyLocalAngularVelocityImpulse( angVelocity );

	if( pGrenade->VPhysicsGetObject() )
	{
		pGrenade->VPhysicsGetObject()->AddVelocity( &velocity, &angVelocity );
	}

/*	pGrenade->SetGravity( BaseClass::GetGrenadeGravity() );
	pGrenade->SetFriction( BaseClass::GetGrenadeFriction() );
	pGrenade->SetElasticity( BaseClass::GetGrenadeElasticity() );
*/
	return pGrenade;
}

void CFlashbangProjectile::Spawn( void )
{
	Precache( );

	// Hits everything but debris
	SetCollisionGroup( COLLISION_GROUP_WEAPON );

	SetModel( "models/Weapons/w_grenade.mdl");
	UTIL_SetSize(this, Vector(-3, -3, -3), Vector(3, 3, 3));
//	UTIL_SetSize(this, Vector(0, 0, 0), Vector(0, 0, 0));

	SetUse( &CFlashbangProjectile::DetonateUse );
	SetTouch( &CFlashbangProjectile::FlashbangProjectileTouch );
	SetThink( &CFlashbangProjectile::FlashbangProjectileThink );
	SetNextThink( gpGlobals->curtime + 0.1f );

	m_flDamage = 1.0f;
	m_DmgRadius		= 350.0f;
	m_flGrenadeDetonateDelay = gpGlobals->curtime;
	m_takedamage	= DAMAGE_YES;
	m_iHealth		= 1;

	SetFriction( 1.3 );
	SetSequence( 0 );

	m_fDangerRadius = 100;

	m_fSpawnTime = gpGlobals->curtime;

	m_flGrenadeDetonateDelay = gpGlobals->curtime + 1.5f;
	m_bIsLive = true;
}

//-----------------------------------------------------------------------------
// Purpose:  The grenade has a slight delay before it goes live.  That way the
//			 person firing it can bounce it off a nearby wall.  However if it
//			 hits another character it blows up immediately
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CFlashbangProjectile::FlashbangProjectileThink( void )
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

void CFlashbangProjectile::Event_Killed( const CTakeDamageInfo &info )
{
	Detonate( );
}

void CFlashbangProjectile::FlashbangProjectileTouch( CBaseEntity *pOther )
{
	Assert( pOther );
	if ( !pOther->IsSolid() || m_flGrenadeDetonateDelay > gpGlobals->curtime )
		return;
}

void CFlashbangProjectile::Detonate(void)
{
	RadiusFlash ( GetAbsOrigin(), this, GetThrower(), 4, CLASS_NONE, DMG_BLAST );
	EmitSound( "Flashbang.Explode" );	

	// tell the bots a flashbang grenade has exploded
	CBasePlayer *player = ToBasePlayer(GetThrower());
	if ( player )
	{
		IGameEvent * event = gameeventmanager->CreateEvent( "flashbang_detonate" );
		if ( event )
		{
			event->SetInt( "userid", player->GetUserID() );
			event->SetFloat( "x", GetAbsOrigin().x );
			event->SetFloat( "y", GetAbsOrigin().y );
			event->SetFloat( "z", GetAbsOrigin().z );
			gameeventmanager->FireEvent( event );
		}
	}

	UTIL_Remove( this );
}

void CFlashbangProjectile::Precache( void )
{
	PrecacheModel("models/Weapons/w_grenade.mdl"); 
}


CFlashbangProjectile::CFlashbangProjectile(void)
{
}

bool CFlashbangProjectile::CreateVPhysics()
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, 0, false );
	return true;
}