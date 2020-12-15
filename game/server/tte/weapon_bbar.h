//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef WEAPON_ALYXGUN_H
#define WEAPON_ALYXGUN_H

#include "basehlcombatweapon.h"

#if defined( _WIN32 )
#pragma once
#endif

class CWeaponBootyBlasterAR : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CWeaponBootyBlasterAR, CHLSelectFireMachineGun );

	CWeaponBootyBlasterAR();
	~CWeaponBootyBlasterAR();

	DECLARE_SERVERCLASS();
	
	void	Precache( void );

	virtual void	PrimaryAttack();
	virtual void	SecondaryAttack();
	virtual void	AddViewKick();

	void ItemPostFrame();
	bool	Reload(void);

	virtual int		GetMinBurst( void ) { return 4; }
	virtual int		GetMaxBurst( void ) { return 7; }
	virtual float	GetMinRestTime( void );
	virtual float	GetMaxRestTime( void );

	virtual void Equip( CBaseCombatCharacter *pOwner );

	float	GetFireRate( void ) { return 0.1f; }
	float	GetBurstCycleRate( void ) { return 0.1f; }

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector vecSpread = Vector( 1.0f );

		float spread = GetWpnData().flHalfRadianBulletSpread;

		if( IsIronsighted() )
		{
			spread = GetWpnData().flHalfRadianBulletSpreadIronsighted;
		}

		vecSpread = Vector( spread, spread, spread );

		return vecSpread;
	}

	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	int		WeaponRangeAttack1Condition( float flDot, float flDist );
	int		WeaponRangeAttack2Condition( float flDot, float flDist );

	void FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles );

	void Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	float m_flTooCloseTimer;

	DECLARE_ACTTABLE();

private:
	bool m_bAllowBurstAttack;
};

#endif // WEAPON_ALYXGUN_H
