//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		TTE sticky grenade
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	GRENADESTICKY_H
#define	GRENADESTICKY_H

#include "basegrenade_shared.h"

#define	MAX_AR2_NO_COLLIDE_TIME 0.2

class SmokeTrail;
class CWeaponAR2;

class CGrenadeSticky : public CBaseGrenade
{
public:
	DECLARE_CLASS( CGrenadeSticky, CBaseGrenade );

	CHandle< SmokeTrail > m_hSmokeTrail;
	float				 m_fSpawnTime;
	float				m_fDangerRadius;


	void		Spawn( void );
	void		Precache( void );
	void 		GrenadeStickyTouch( CBaseEntity *pOther );
	void		GrenadeStickyThink( void );
	void		Event_Killed( const CTakeDamageInfo &info );

	bool CreateVPhysics( void );

public:
	void EXPORT				Detonate(void);
	CGrenadeSticky(void);

	DECLARE_DATADESC();

	float m_flGrenadeDetonateDelay;
};

#endif	//GRENADESTICKY_H
