//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basegrenade_shared.h"
#include "grenade_yeet.h"
#include "soundent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#define YEET_MODEL "models/weapons/w_pist_usp_yeeted.mdl"
#define YEET_MODEL2 "models/weapons/w_pist_usp_silencer_yeeted.mdl"


class CYeetFrag : public CBaseGrenade
{
	DECLARE_CLASS(CYeetFrag, CBaseGrenade);

	DECLARE_DATADESC();

	~CYeetFrag(void);

public:
	void	Spawn(void);
	void	Precache(void);
	void	OnRestore(void);
	bool	CreateVPhysics(void);
	void	GiveGunAndRemove(CBaseEntity *pOther);
	void	SetVelocity(const Vector &velocity, const AngularImpulse &angVelocity);
	int		OnTakeDamage(const CTakeDamageInfo &inputInfo);
	void	VPhysicsUpdate(IPhysicsObject *pPhysics);
	void	DelayPickup();
	bool	m_bSilencerType = false;
};

LINK_ENTITY_TO_CLASS(npc_grenade_yeet, CYeetFrag);

BEGIN_DATADESC(CYeetFrag)
DEFINE_FIELD(m_bSilencerType, FIELD_BOOLEAN),

// Function Pointers
DEFINE_THINKFUNC(DelayPickup),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CYeetFrag::~CYeetFrag(void)
{
}

void CYeetFrag::Spawn(void)
{
	Precache();

	SetModel(YEET_MODEL);

	m_takedamage = DAMAGE_EVENTS_ONLY;
	m_iHealth = 1;

	SetSize(-Vector(16, 16, 16), Vector(16, 16, 16));
	SetCollisionGroup(COLLISION_GROUP_INTERACTIVE_DEBRIS);
	CreateVPhysics();

	BaseClass::Spawn();

	SetThink(&CYeetFrag::DelayPickup);
	SetNextThink(gpGlobals->curtime + 1);
}

void CYeetFrag::OnRestore(void)
{
	BaseClass::OnRestore();

	SetThink(&CYeetFrag::DelayPickup);
	SetNextThink(gpGlobals->curtime + 0.4);
}

bool CYeetFrag::CreateVPhysics()
{
	VPhysicsInitNormal(SOLID_BBOX, 0, false);
	return true;
}

// this will hit only things that are in newCollisionGroup, but NOT in collisionGroupAlreadyChecked
class CTraceFilterCollisionGroupDelta : public CTraceFilterEntitiesOnly
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE(CTraceFilterCollisionGroupDelta);

	CTraceFilterCollisionGroupDelta(const IHandleEntity *passentity, int collisionGroupAlreadyChecked, int newCollisionGroup)
		: m_pPassEnt(passentity), m_collisionGroupAlreadyChecked(collisionGroupAlreadyChecked), m_newCollisionGroup(newCollisionGroup)
	{
	}

	virtual bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask)
	{
		if (!PassServerEntityFilter(pHandleEntity, m_pPassEnt))
			return false;
		CBaseEntity *pEntity = EntityFromEntityHandle(pHandleEntity);

		if (pEntity)
		{
			if (g_pGameRules->ShouldCollide(m_collisionGroupAlreadyChecked, pEntity->GetCollisionGroup()))
				return false;
			if (g_pGameRules->ShouldCollide(m_newCollisionGroup, pEntity->GetCollisionGroup()))
				return true;
		}

		return false;
	}

protected:
	const IHandleEntity *m_pPassEnt;
	int		m_collisionGroupAlreadyChecked;
	int		m_newCollisionGroup;
};

void CYeetFrag::VPhysicsUpdate(IPhysicsObject *pPhysics)
{
	BaseClass::VPhysicsUpdate(pPhysics);
}


void CYeetFrag::Precache(void)
{
	PrecacheModel(YEET_MODEL);
	PrecacheModel(YEET_MODEL2);

	BaseClass::Precache();
}

void CYeetFrag::GiveGunAndRemove(CBaseEntity *pOther)
{
	if (!pOther)
		return;
	if (pOther->IsPlayer())
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		if (!pPlayer)
			return;
		//If player has the gun don't create it.
		CBaseEntity *pHasWeaponUsp = pPlayer->HasNamedPlayerItem("weapon_usp");
		if (pHasWeaponUsp)
		{
			return;
		}

		CBaseEntity *gun = CreateEntityByName("weapon_usp");
		CBaseCombatWeapon *usp = dynamic_cast<CBaseCombatWeapon*>(gun);
		if (usp)
		{
			usp->SetAbsOrigin(pOther->EyePosition());
			usp->m_bHasSupressorOn = m_bSilencerType;	
			usp->AddSpawnFlags(SF_WEAPON_NO_PLAYER_PICKUP);
			usp->m_bYeetedEmpty = true;
			usp->Spawn();
			usp->m_iClip1 = 0;
			//usp->AddEffects(EF_NODRAW);
		}

		UTIL_Remove(this);
	}

}
void CYeetFrag::DelayPickup()
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if (pPhysicsObject)
	{
		pPhysicsObject->SetMass(2);
	}
	//Pick it up
	AddSolidFlags(FSOLID_TRIGGER);
	//Can pick it one second after spawn.
	SetTouch(&CYeetFrag::GiveGunAndRemove);
}


void CYeetFrag::SetVelocity(const Vector &velocity, const AngularImpulse &angVelocity)
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if (pPhysicsObject)
	{
		pPhysicsObject->AddVelocity(&velocity, &angVelocity);
		//Mass 
		pPhysicsObject->SetMass(360);
	}
}

int CYeetFrag::OnTakeDamage(const CTakeDamageInfo &inputInfo)
{
	VPhysicsTakeDamage(inputInfo);

	return BaseClass::OnTakeDamage(inputInfo);
}


CBaseGrenade *Fragyeet_Create(const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, bool SpawnSiltype)
{
	CYeetFrag *pYeet = (CYeetFrag *)CBaseEntity::Create("npc_grenade_yeet", position, angles, pOwner);

	pYeet->SetVelocity(velocity, angVelocity);
	pYeet->SetThrower(ToBaseCombatCharacter(pOwner));
	pYeet->SetOwnerEntity(NULL);
	pYeet->m_takedamage = DAMAGE_EVENTS_ONLY;
	pYeet->m_bSilencerType = SpawnSiltype;

	if (!SpawnSiltype)
	{
		pYeet->SetModel(YEET_MODEL);
	}
	else
	{
		pYeet->SetModel(YEET_MODEL2);
	}

	return pYeet;
}
