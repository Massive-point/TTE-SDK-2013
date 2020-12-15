//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		dualpistol - hand gun
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	DUALPISTOL_FASTEST_REFIRE_TIME		0.1f
#define	DUALPISTOL_FASTEST_DRY_REFIRE_TIME	0.2f

#define	DUALPISTOL_ACCURACY_SHOT_PENALTY_TIME		0.2f	// Applied amount of time each shot adds to the time we must recover from
#define	DUALPISTOL_ACCURACY_MAXIMUM_PENALTY_TIME	1.5f	// Maximum penalty to deal out

ConVar	dualpistol_use_new_accuracy( "dualpistol_use_new_accuracy", "1" );

//-----------------------------------------------------------------------------
// CWeapondualpistol
//-----------------------------------------------------------------------------

class CWeapondualpistol : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeapondualpistol, CBaseHLCombatWeapon );

	CWeapondualpistol(void);

	DECLARE_SERVERCLASS();

	void	Precache( void );
	void	ItemPostFrame( void );
	void	ItemPreFrame( void );
	void	ItemBusyFrame( void );
	void	PrimaryAttack( void );
	//void	LeftGunAttack(void);
	void	AddViewKick( void );
	void	DryFire( void );
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	void	UpdatePenaltyTime( void );

	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	Activity	GetPrimaryAttackActivity( void );

	void	MakeTracer(const Vector &vecTracerSrc, const trace_t &tr, int iTracerType);

	virtual bool Reload( void );
	
	virtual int	GetMinBurst() 
	{ 
		return 1; 
	}

	virtual int	GetMaxBurst() 
	{ 
		return 3; 
	}

	virtual float GetFireRate( void ) 
	{
		return 0.5f; 
	}

	DECLARE_ACTTABLE();

private:
	CNetworkVar(float, m_flSoonestPrimaryAttack);
	CNetworkVar(float, m_flLastAttackTime);
	CNetworkVar(float, m_flAccuracyPenalty);
	CNetworkVar(int, m_nNumShotsFired);

	//bool bFlip;
};


IMPLEMENT_SERVERCLASS_ST(CWeapondualpistol, DT_Weapondualpistol)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_dualpistol, CWeapondualpistol );
PRECACHE_WEAPON_REGISTER( weapon_dualpistol );

BEGIN_DATADESC( CWeapondualpistol )

	DEFINE_FIELD( m_flSoonestPrimaryAttack, FIELD_TIME ),
	DEFINE_FIELD( m_flLastAttackTime,		FIELD_TIME ),
	DEFINE_FIELD( m_flAccuracyPenalty,		FIELD_FLOAT ), //NOTENOTE: This is NOT tracking game time
	DEFINE_FIELD( m_nNumShotsFired,			FIELD_INTEGER ),

END_DATADESC()

acttable_t	CWeapondualpistol::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_PISTOL,				true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_PISTOL,			true },
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_PISTOL,		true },
	{ ACT_RELOAD,					ACT_RELOAD_PISTOL,				true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_PISTOL,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_PISTOL,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_PISTOL,true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_PISTOL_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_PISTOL_LOW,	false },
	{ ACT_COVER_LOW,				ACT_COVER_PISTOL_LOW,			false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_PISTOL_LOW,		false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_PISTOL,		false },
	{ ACT_WALK,						ACT_WALK_PISTOL,				false },
	{ ACT_RUN,						ACT_RUN_PISTOL,					false },
};


IMPLEMENT_ACTTABLE( CWeapondualpistol );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapondualpistol::CWeapondualpistol( void )
{
	m_flSoonestPrimaryAttack = gpGlobals->curtime;
	m_flAccuracyPenalty = 0.0f;

	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;

	m_bFiresUnderwater	= true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapondualpistol::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapondualpistol::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
		case EVENT_WEAPON_PISTOL_FIRE:
		{
			Vector vecShootOrigin, vecShootDir;
			vecShootOrigin = pOperator->Weapon_ShootPosition();

			CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			ASSERT( npc != NULL );

			vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );

			CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );

			WeaponSound( SINGLE_NPC );
			pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2 );
			pOperator->DoMuzzleFlash();
			m_iClip1 = m_iClip1 - 1;
		}
		break;
		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeapondualpistol::DryFire( void )
{
	WeaponSound( EMPTY );
	SendWeaponAnim( ACT_VM_DRYFIRE );
	
	m_flSoonestPrimaryAttack	= gpGlobals->curtime + DUALPISTOL_FASTEST_DRY_REFIRE_TIME;
	m_flNextPrimaryAttack		= gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeapondualpistol::PrimaryAttack( void )
{
	if ((gpGlobals->curtime - m_flLastAttackTime) > 0.5f)
	{
		m_nNumShotsFired = 0;
	}
	else
	{
		m_nNumShotsFired++;
	}

	m_flLastAttackTime = gpGlobals->curtime;
	m_flSoonestPrimaryAttack = gpGlobals->curtime + DUALPISTOL_FASTEST_REFIRE_TIME;

	BaseClass::PrimaryAttack();
	//Flipping Code -Jman
	//if (!bFlip)
	//{
	//	BaseClass::PrimaryAttack();
	//	bFlip = true;
	//}
	//else
	//{
	//	LeftGunAttack();
	//	bFlip = false;
	//}
}

//-----------------------------------------------------------------------------
// Purpose: Dualies Firing
//-----------------------------------------------------------------------------
//void CWeapondualpistol::LeftGunAttack()
//{
//	// If my clip is empty (and I use clips) start reload
//	if (UsesClipsForAmmo1() && !m_iClip1)
//	{
//		Reload();
//		return;
//	}
//
//	// Only the player fires this way so we can cast
//	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
//
//	if (!pPlayer)
//	{
//		return;
//	}
//
//	// MUST call sound before removing a round from the clip of a CMachineGun
//	WeaponSound(SINGLE);
//
//	pPlayer->DoMuzzleFlash();
//
//	SendWeaponAnim(GetSecondaryAttackActivity());
//
//	// player "shoot" animation
//	pPlayer->SetAnimation(PLAYER_ATTACK1);
//
//	FireBulletsInfo_t info;
//	info.m_vecSrc = pPlayer->Weapon_ShootPosition();
//
//	info.m_vecDirShooting = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
//
//	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
//	// especially if the weapon we're firing has a really fast rate of fire.
//	info.m_iShots = 0;
//	float fireRate = GetFireRate();
//
//	while (m_flNextPrimaryAttack <= gpGlobals->curtime)
//	{
//		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
//		info.m_iShots++;
//		if (!fireRate)
//			break;
//	}
//
//	// Make sure we don't fire more than the amount in the clip
//	if (UsesClipsForAmmo1())
//	{
//		info.m_iShots = min(info.m_iShots, m_iClip1);
//		m_iClip1 -= info.m_iShots;
//	}
//	else
//	{
//		info.m_iShots = min(info.m_iShots, pPlayer->GetAmmoCount(m_iPrimaryAmmoType));
//		pPlayer->RemoveAmmo(info.m_iShots, m_iPrimaryAmmoType);
//	}
//
//	info.m_flDistance = MAX_TRACE_LENGTH;
//	info.m_iAmmoType = m_iPrimaryAmmoType;
//	info.m_iTracerFreq = 2;
//
//#if !defined( CLIENT_DLL )
//	// Fire the bullets
//	info.m_vecSpread = pPlayer->GetAttackSpread(this);
//#else
//	//!!!HACKHACK - what does the client want this function for? 
//	info.m_vecSpread = GetActiveWeapon()->GetBulletSpread();
//#endif // CLIENT_DLL
//
//	pPlayer->FireBullets(info);
//
//	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
//	{
//		// HEV suit - indicate out of ammo condition
//		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
//	}
//
//	//Add our view kick in
//	AddViewKick();
//}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapondualpistol::UpdatePenaltyTime( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	// Check our penalty time decay
	if ( ( ( pOwner->m_nButtons & IN_ATTACK ) == false ) && ( m_flSoonestPrimaryAttack < gpGlobals->curtime ) )
	{
		m_flAccuracyPenalty -= gpGlobals->frametime;
		m_flAccuracyPenalty = clamp( m_flAccuracyPenalty, 0.0f, DUALPISTOL_ACCURACY_MAXIMUM_PENALTY_TIME );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapondualpistol::ItemPreFrame( void )
{
	UpdatePenaltyTime();

	BaseClass::ItemPreFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapondualpistol::ItemBusyFrame( void )
{
	UpdatePenaltyTime();

	BaseClass::ItemBusyFrame();
}

//-----------------------------------------------------------------------------
// Purpose: Allows firing as fast as button is pressed
//-----------------------------------------------------------------------------
void CWeapondualpistol::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	if ( m_bInReload )
		return;
	
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	//Allow a refire as fast as the player can click
	if ( ( ( pOwner->m_nButtons & IN_ATTACK ) == false ) && ( m_flSoonestPrimaryAttack < gpGlobals->curtime ) )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime - 0.1f;
	}
	else if ( ( pOwner->m_nButtons & IN_ATTACK ) && ( m_flNextPrimaryAttack < gpGlobals->curtime ) && ( m_iClip1 <= 0 ) )
	{
		DryFire();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
Activity CWeapondualpistol::GetPrimaryAttackActivity( void )
{
	if (m_iClip1 % 2 == 0)
	{
		return ACT_VM_PRIMARYATTACK;
	}
	else
	{
		return ACT_VM_SECONDARYATTACK;
	}

	//if ( m_nNumShotsFired < 1 )
	//	return ACT_VM_PRIMARYATTACK;

	//if ( m_nNumShotsFired < 2 )
	//	return ACT_VM_RECOIL1;

	//if ( m_nNumShotsFired < 3 )
	//	return ACT_VM_RECOIL2;

	//return ACT_VM_RECOIL3;
}

void CWeapondualpistol::MakeTracer(const Vector &vecTracerSrc, const trace_t &tr, int iTracerType)
{
	//Custom tracer. Broken for now.
	return;
	//CBaseEntity *pOwner = GetOwner();

	//if (pOwner == NULL)
	//{
	//	BaseClass::MakeTracer(vecTracerSrc, tr, iTracerType);
	//	return;
	//}

	//Vector vNewSrc = vecTracerSrc;

	//int iEntIndex = pOwner->entindex();
	//if (g_pGameRules->IsMultiplayer())
	//{
	//	iEntIndex = entindex();
	//}

	//UTIL_ParticleTracer("weapon_tracer", vNewSrc, tr.endpos, entindex(), 1, 4);

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeapondualpistol::Reload( void )
{
	bool fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
	if ( fRet )
	{
		WeaponSound( RELOAD );
		m_flAccuracyPenalty = 0.0f;
	}
	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapondualpistol::AddViewKick( void )
{
	// Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;

	DoRecoil( pPlayer, GetWeaponRecoilViewKickVector(), m_fFireDuration, GetWeaponRecoilSlideLimit(), GetWeaponRecoilViewSlideVector() );
}
