//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Crossroads Devtest
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "basehlcombatweapon.h"
#include "particle_parse.h"
#include "basecombatweapon_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponKar98 : public CBaseCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponKar98, CBaseCombatWeapon );
	DECLARE_SERVERCLASS(); 
	//DECLARE_ACTTABLE();
	
	CWeaponKar98();

	virtual void PrimaryAttack();
	virtual void EnableIronsights();
	virtual void AddViewKick();
	void	WeaponIdle(void);
	virtual void Precache();
	bool	Reload(void);
	void ItemPostFrame();
	void Unscope();
	float	GetFireRate( void ) { return 1.25f; }

	virtual const Vector& GetBulletSpread( void )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

		float speed = pOwner->GetAbsVelocity().Length();
		float spread = GetWpnData().flHalfRadianBulletSpread;

		static Vector cone = Vector( spread, spread, spread );

		if( m_iIronsightLevel > 0 )
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
	unsigned int m_iIronsightLevel;
	bool		m_bCanScope = true;
};

IMPLEMENT_SERVERCLASS_ST( CWeaponKar98, DT_WeaponKar98 )
END_SEND_TABLE();

LINK_ENTITY_TO_CLASS( weapon_kar98, CWeaponKar98 );
PRECACHE_WEAPON_REGISTER( weapon_kar98 );

BEGIN_DATADESC( CWeaponKar98 )
DEFINE_FIELD(m_iIronsightLevel, FIELD_INTEGER),
DEFINE_FIELD(m_bCanScope, FIELD_BOOLEAN),
END_DATADESC()

CWeaponKar98::CWeaponKar98()
{
}

void CWeaponKar98::Precache( void )
{
	PrecacheParticleSystem( "weapon_muzzle_smoke" );
	PrecacheScriptSound("Default.Zoom");
	BaseClass::Precache();
}

void CWeaponKar98::PrimaryAttack()
{
	BaseClass::PrimaryAttack();

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration(); 

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	DispatchParticleEffect( "weapon_muzzle_smoke", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", true);
	Unscope();
}

void CWeaponKar98::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	if (!(pOwner->m_nButtons & IN_IRONSIGHT))
	{
		m_bCanScope = true;
	}

	if (m_iIronsightLevel > 0 && pOwner->m_bHolsteredAW)
	{
		Unscope();
	}

}

void CWeaponKar98::Unscope(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
	{
		return;
	}
	CBaseViewModel *vm = pPlayer->GetViewModel(m_nViewModelIndex);
	pPlayer->SetFOV(pPlayer, pPlayer->GetDefaultFOV(), 0.05f);

	m_iIronsightLevel = 0;

	if (vm)
	{
		vm->RemoveEffects(EF_NODRAW);
	}
}

bool CWeaponKar98::Reload(void)
{
	if (m_iClip1 == GetMaxClip1())
	{
		return false;
	}

	Unscope();
	return BaseClass::Reload();
}


void CWeaponKar98::EnableIronsights()
{
		CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
		if (pPlayer == NULL)
		{
			return;
		}
		//Can't scope if holstered or no ammo.
		if (!m_bCanScope || m_iClip1 == 0 || pPlayer->m_bHolsteredAW)
		{
			return;
		}
		m_bCanScope = false;

		CBaseViewModel *vm = pPlayer->GetViewModel( m_nViewModelIndex );

		if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV())
		{
			pPlayer->SetFOV( pPlayer, 40, 0.15f );

			m_iIronsightLevel = 1;

			if( vm )
			{
				vm->AddEffects( EF_NODRAW );
			}
		}
		else if (pPlayer->GetFOV() == 40)
		{
			pPlayer->SetFOV( pPlayer, 15, 0.05 );

			m_iIronsightLevel = 2;
		}
		else if (pPlayer->GetFOV() == 15)
		{
		Unscope();
		}

		m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;   

		EmitSound( "Default.Zoom" ); // zoom sound
}

void CWeaponKar98::AddViewKick( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if( pOwner )
	{
		DoRecoil( pOwner, GetWeaponRecoilViewKickVector(), m_fFireDuration, GetWeaponRecoilSlideLimit(), GetWeaponRecoilViewSlideVector() );
	}
}

void CWeaponKar98::WeaponIdle(void)
{
	if (m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		BaseClass::WeaponIdle();
	}
}