//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		mudafugin U S P babyyyyyyyyyyyyyyyyyyy
//
// 11.08.2020 Binka:
// I removed silencer activities since we use bodygroups now. Sorry about the mess, I ate spaghetti today.
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "grenade_yeet.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponUSP : public CBaseCombatWeapon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CWeaponUSP, CBaseCombatWeapon );
	DECLARE_SERVERCLASS(); 
	//No need this unless npc use this gun.
	//DECLARE_ACTTABLE();
	
	CWeaponUSP();

	void PrimaryAttack();
	void SecondaryAttack();
	void Yeet();
	void YeetEnd();
	void Precache();
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	void AddViewKick();
	void DelayPickupUSP();
	void DryFire();
	void HandleFireOnEmpty();
	void ItemPostFrame();
	void Spawn();
	void WeaponIdle();
	void EnableIronsights();
	void	CheckThrowPosition(CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc);
	//Silencer off by default.
	bool m_bSilencerOn = false;
	bool m_bAllowNextAttack = true;
	bool m_bSilCanADS = true;
	bool m_bYeeeted = false;

	bool Deploy();
	bool Holster( CBaseCombatWeapon *pSwitchingTo );
	bool Reload();

	//float GetFireRate( void ) { return 0.1704f; }


	//Activity GetDrawActivity( void );
	//Activity GetPrimaryAttackActivity( void );

	void	MakeTracer(const Vector &vecTracerSrc, const trace_t &tr, int iTracerType);

	virtual const Vector& GetBulletSpread( void )
	{
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

		cone = Vector( spread, spread, spread );

		return cone;
	}
};


IMPLEMENT_SERVERCLASS_ST(CWeaponUSP, DT_WeaponUSP)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_usp, CWeaponUSP);
PRECACHE_WEAPON_REGISTER(weapon_usp);

BEGIN_DATADESC(CWeaponUSP)
DEFINE_FIELD(m_bSilencerOn, FIELD_BOOLEAN),
DEFINE_FIELD(m_bYeeeted, FIELD_BOOLEAN),

// Function Pointers
DEFINE_THINKFUNC(DelayPickupUSP),

END_DATADESC()

CWeaponUSP::CWeaponUSP()
{

}

void CWeaponUSP::Spawn( )
{
	BaseClass::Spawn();

	if (m_bYeetedEmpty)
	{
		SetThink(&CWeaponUSP::DelayPickupUSP);
		SetNextThink(gpGlobals->curtime + 0.1);
		m_bYeetedEmpty = false;
	}

}

void CWeaponUSP::Precache()
{
	UTIL_PrecacheOther("npc_grenade_yeet");
	BaseClass::Precache();
}

void CWeaponUSP::DelayPickupUSP()
{
	RemoveSpawnFlags(SF_WEAPON_NO_PLAYER_PICKUP);
}

void CWeaponUSP::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	CBaseViewModel *ActiveVM = pOwner->GetViewModel(0);

	switch (pEvent->event)
	{
	case EVENT_WEAPON_THROW:
		if (ActiveVM)
		{
			ActiveVM->SetBodygroup(2, 0);
		}
		m_bSilencerOn = false;
		m_bHasSupressorOn = false;
		m_bSilCanADS = true;
		break;

	case EVENT_WEAPON_THROW2:
		m_bSilencerOn = true;
		m_bHasSupressorOn = true;
		m_bSilCanADS = true;
		break;

	case EVENT_WEAPON_THROW3:
		Yeet();
		break;

	//Yeet end.
	case EVENT_WEAPON_AR2_ALTFIRE:
		YeetEnd();
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}

}

bool CWeaponUSP::Deploy()
{
	//Don't deploy if holstered.
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return false;

	if (pOwner)
	{
		if (pOwner->m_bHolsteredAW)
		{
			return false;
		}
	}

	//Trollololo
	if (m_bYeeeted)
	{
		return DefaultDeploy((char*)GetViewModel(), (char*)GetWorldModel(), ACT_VM_HOLSTER_SPECIAL, (char*)GetAnimPrefix());
	}

	if (m_bHasSupressorOn)
	{
		m_bSilencerOn = true;
	}

	m_bSilCanADS = true;
	//DefaultDeploy has to be called before SetBodygroup.
	bool fRet;

	if (m_iClip1 <= 0)
	{
		fRet = DefaultDeploy((char*)GetViewModel(), (char*)GetWorldModel(), ACT_CROSSBOW_DRAW_UNLOADED, (char*)GetAnimPrefix());
	}
	else
	{
		fRet = DefaultDeploy((char*)GetViewModel(), (char*)GetWorldModel(), ACT_VM_DRAW, (char*)GetAnimPrefix());
	}


	CBaseViewModel *ActiveVM = pOwner->GetViewModel(0);
	if (!ActiveVM)
		return false;

	if( m_bSilencerOn )
	{
		ActiveVM->SetBodygroup(2, 1);
	}
	else
	{
		ActiveVM->SetBodygroup(2, 0);
	}

	return fRet;
	//return BaseClass::Deploy();
}


bool CWeaponUSP::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_bSilCanADS = true;
	//Set viewmodel to bodygroup to default when switching to other weapon.
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return false;

	CBaseViewModel *ActiveVM = pOwner->GetViewModel(0);
	//But only when switching to other weapon.
	if (pSwitchingTo && ActiveVM)
	{
		ActiveVM->SetBodygroup(2, 0);
	}

	if (m_bYeeeted)
	{
		SetThink(&CWeaponUSP::DestroyItem);
		SetNextThink(gpGlobals->curtime + 0.1);
		return true;
	}

	return BaseClass::Holster( pSwitchingTo );
}

void CWeaponUSP::EnableIronsights()
{
	//Don't allow ADS if no ammo in clip or during attaching silencer animation.
	if (!m_bSilCanADS || !m_iClip1)
	{
		return;
	}

	return BaseClass::EnableIronsights();
}

void CWeaponUSP::SecondaryAttack()
{
	DisableIronsights();
	m_bSilCanADS = false;

	//Oh Boi. Let's set the bodygroup in code. Maybe it will finally set right one on save/restore.
	//This value might apply to all weapons viewmodels, so we need to be careful.
	//The value of m_bSilencerOn will be changed be anim events.

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	CBaseViewModel *ActiveVM = pOwner->GetViewModel(0);
	if ( m_bSilencerOn )
	{
		//This is triggered by anim event.
		//ActiveVM->SetBodygroup(2, 0);
		if (m_iClip1 == 0)
			SendWeaponAnim(ACT_VM_DETACH_SILENCER_EMPTY);
		else
			SendWeaponAnim(ACT_VM_DETACH_SILENCER);
	}
	else
	{
		if (ActiveVM)
		{
			ActiveVM->SetBodygroup(2, 1);
		}
		
		if (m_iClip1 == 0)
			SendWeaponAnim(ACT_VM_ATTACH_SILENCER_EMPTY);
		else
			SendWeaponAnim(ACT_VM_ATTACH_SILENCER);
	}

	m_flNextSecondaryAttack = gpGlobals->curtime + 3;
	m_flNextPrimaryAttack = gpGlobals->curtime + 3;
}

void CWeaponUSP::Yeet()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors(&vForward, &vRight, NULL);
	Vector vecSrc = vecEye + vForward * 0.0f + vRight * 0.0f;
	CheckThrowPosition(pPlayer, vecEye, vecSrc);

	Vector vecThrow;
	pPlayer->GetVelocity(&vecThrow, NULL);
	vecThrow += vForward * 1200;
	Fragyeet_Create(vecSrc, vec3_angle, vecThrow, AngularImpulse(600, random->RandomInt(-1200, 1200), 0), pPlayer, m_bSilencerOn);

	m_bYeeeted = true;
}

void CWeaponUSP::YeetEnd()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}
	//Remove weapon.
	pPlayer->SwitchToNextBestWeapon(this);
	//SetWeaponVisible(false);
	SetThink(&CWeaponUSP::DestroyItem);
	SetNextThink(gpGlobals->curtime + 0.1);
}

void CWeaponUSP::CheckThrowPosition(CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc)
{
	trace_t tr;

	UTIL_TraceHull(vecEye, vecSrc, -Vector(4.0f + 2, 4.0f + 2, 4.0f + 2), Vector(4.0f + 2, 4.0f + 2, 4.0f + 2),
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr);

	if (tr.DidHit())
	{
		vecSrc = tr.endpos;
	}
}

void CWeaponUSP::HandleFireOnEmpty()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}
	//So far nothing.
	UTIL_HudHintText(pPlayer, "%+attack% + %+ironsight% TO YEEEEEEET");
	//CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	//if (!pPlayer)
	//{
	//	return;
	//}
	////YEEEEEEEEET !!!
	//if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	//{
	//	DisableIronsights();
	//	SendWeaponAnim(ACT_VM_THROW);
	//	m_flNextSecondaryAttack = gpGlobals->curtime + 3;
	//	m_flNextPrimaryAttack = gpGlobals->curtime + 3;
	//	m_bSilCanADS = false;

	//}


	// If we're already firing on empty, reload if we can
	//if (m_bFireOnEmpty)
	//{
	//	ReloadOrSwitchWeapons();
	//	m_fFireDuration = 0.0f;
	//}
	//else
	//{
	//	if (m_flNextEmptySoundTime < gpGlobals->curtime)
	//	{
	//		WeaponSound(EMPTY);
	//		m_flNextEmptySoundTime = gpGlobals->curtime + 0.5;
	//	}
	//	m_bFireOnEmpty = true;
	//}

}

void CWeaponUSP::PrimaryAttack()
{	
	if (!m_bAllowNextAttack)
	{
		return;
	}
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	if (m_iClip1 <= 0)
	{
		if (!m_bFireOnEmpty)
		{
			Reload();
		}
		else
		{
			DryFire();
		}

		return;
	}

	if (m_bSilencerOn)
	{
		WeaponSound(SPECIAL1);
	}
	else
	{
		WeaponSound(SINGLE);
		CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, GetOwner());
	}


	//pPlayer->DoMuzzleFlash();

	m_iClip1--;

	if (IsIronsighted())
	{
		if (m_iClip1 == 0)
			SendWeaponAnim(ACT_VM_PRIMARYATTACK_EMPTY_ADS);
		else
			SendWeaponAnim(ACT_VM_PRIMARYATTACK_ADS);
	}
	else
	{
		if (m_iClip1 == 0)
			SendWeaponAnim(ACT_VM_PRIMARYATTACK_EMPTY);
		else
			SendWeaponAnim(ACT_VM_PRIMARYATTACK);
	}


	pPlayer->SetAnimation(PLAYER_ATTACK1);

	//Max fire rate.
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.1f;
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.1f;

	m_bAllowNextAttack = false;

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming;
	Vector vecSpread;

	vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);

	vecSpread = pPlayer->GetAttackSpread(this);
	
	FireBulletsInfo_t info(1, vecSrc, vecAiming, vecSpread, MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
	info.m_pAttacker = pPlayer;
	//Disable tracers in ADS for now.
	if (IsIronsighted())
	{
		info.m_iTracerFreq = 0;
	}
	else
	{
		info.m_iTracerFreq = 1;
	}
	//info.m_flDamage = 10; //Damage override
	pPlayer->FireBullets(info);


	//pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	//Add our view kick in
	AddViewKick();
}


bool CWeaponUSP::Reload()
{
	m_bSilCanADS = true;
	bool fRet;
	if (m_iClip1 == 0)
	{
		fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD_EMPTY);
	}
	else
	{
		fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	}

	return fRet;
}

void CWeaponUSP::WeaponIdle()
{
	//Don't idle during silencer animation.
	if (m_flNextPrimaryAttack <= gpGlobals->curtime)
	{

		if (m_iClip1 <= 0)
		{
			if (!HasWeaponIdleTimeElapsed())
				return;

			SendWeaponAnim(ACT_VM_IDLE_EMPTY);
		}
		else
		{
			if (IsIronsighted())
			{
				SendWeaponAnim(ACT_VM_IDLE_ADS);
			}
			else
			{
				SendWeaponAnim(ACT_VM_IDLE);
			}
		}
	}
}

void CWeaponUSP::AddViewKick( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if( pPlayer )
	{
		DoRecoil( pPlayer, GetWeaponRecoilViewKickVector(), m_fFireDuration, GetWeaponRecoilSlideLimit(), GetWeaponRecoilViewSlideVector() );
	}
}

void CWeaponUSP::DryFire( void )
{
	//WeaponSound(EMPTY);
	//SendWeaponAnim(ACT_VM_DRYFIRE);

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
}

void CWeaponUSP::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	if (m_bInReload)
		return;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	if (!(pOwner->m_nButtons & IN_ATTACK))
	{
		m_bAllowNextAttack = true;
	}

	
	//The YEET.
	if (!m_iClip1 && (pOwner->m_nButtons & IN_ATTACK) && (pOwner->m_nButtons & IN_IRONSIGHT) && m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		DisableIronsights();
		SendWeaponAnim(ACT_VM_THROW);
		m_flNextSecondaryAttack = gpGlobals->curtime + 3;
		m_flNextPrimaryAttack = gpGlobals->curtime + 3;
		m_bSilCanADS = false;
	}
}


//BMS tracer looks bad in this weapon. We override it with default one until we find better tracer.
void CWeaponUSP::MakeTracer(const Vector &vecTracerSrc, const trace_t &tr, int iTracerType)
{
	CBaseEntity *pOwner = GetOwner();

	if (pOwner == NULL)
	{
		BaseClass::MakeTracer(vecTracerSrc, tr, iTracerType);
		return;
	}

	const char *pszTracerName = GetTracerType();

	Vector vNewSrc = vecTracerSrc;
	int iEntIndex = pOwner->entindex();

	if (g_pGameRules->IsMultiplayer())
	{
		iEntIndex = entindex();
	}

	int iAttachment = GetTracerAttachment();

	switch (iTracerType)
	{
	case 1:
		UTIL_Tracer(vNewSrc, tr.endpos, iEntIndex, iAttachment, 0.0f, true, pszTracerName);
		break;

	case 4:
		UTIL_Tracer(vNewSrc, tr.endpos, iEntIndex, iAttachment, 0.0f, true, pszTracerName);
		break;
	}
}




//Snippets


//pOwner->SwitchToNextBestWeapon(this);
//if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
//{
//
//	SetThink(&CWeaponUSP::DestroyItem);
//	SetNextThink(gpGlobals->curtime + 0.1);
//
//}








//CBaseEntity* gun = CBaseEntity::Create("weapon_usp", GetAbsOrigin(), GetAbsAngles(), this);
//CBaseCombatWeapon* usp = dynamic_cast<CBaseCombatWeapon*>(gun);
//if (!usp) return;
//
//usp->m_iClip1 = 0;