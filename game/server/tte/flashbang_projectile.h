//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef HEGRENADE_PROJECTILE_H
#define HEGRENADE_PROJECTILE_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "basegrenade_shared.h"

class CFlashbangProjectile : public CBaseGrenade
{
public:
	DECLARE_CLASS( CFlashbangProjectile, CBaseGrenade );

	float				 m_fSpawnTime;
	float				m_fDangerRadius;

	void		Spawn( void );
	void		Precache( void );
	void 		FlashbangProjectileTouch( CBaseEntity *pOther );
	void		FlashbangProjectileThink( void );
	void		Event_Killed( const CTakeDamageInfo &info );

	bool		CreateVPhysics();

	void		SetNPCBlindTime( float flTime ) { m_flNPCBlindTimeMultiplier = flTime; }

	// Grenade stuff.
	static CFlashbangProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner, float flNPCBlindDelay = 1.0f );	

public:
	void EXPORT				Detonate(void);
	CFlashbangProjectile(void);

	DECLARE_DATADESC();

	float m_flGrenadeDetonateDelay;

private:
	float m_flNPCBlindTimeMultiplier;
};


#endif // HEGRENADE_PROJECTILE_H
