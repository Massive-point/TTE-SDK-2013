//========= Copyright Pepsi Corporation, All rights reserved. ============//
//
// Purpose: Healthkit, but with custom model.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "gamerules.h"
#include "player.h"
#include "items.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Heals the player when picked up.
//-----------------------------------------------------------------------------
class CHealthKitCustom : public CItem
{
public:
	DECLARE_CLASS(CHealthKitCustom, CItem);
	DECLARE_DATADESC();

	void Spawn(void);
	void Precache(void);
	bool MyTouch(CBasePlayer *pPlayer);

	string_t m_schkPickupSound;
	int m_ichkHealthBonus;
};

LINK_ENTITY_TO_CLASS(item_custom_healthkit, CHealthKitCustom);
PRECACHE_REGISTER(item_custom_healthkit);

//-----------------------------------------------------------------------------
// Save/load: 
//-----------------------------------------------------------------------------
BEGIN_DATADESC(CHealthKitCustom)

DEFINE_KEYFIELD(m_schkPickupSound, FIELD_SOUNDNAME, "chkpickupsound"),
DEFINE_KEYFIELD(m_ichkHealthBonus, FIELD_INTEGER, "chkhealthbonus"),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHealthKitCustom::Spawn(void)
{
	Precache();
	SetModel(STRING(GetModelName()));

	BaseClass::Spawn();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHealthKitCustom::Precache(void)
{
	//Custom model
	if (!GetModelName())
	{
		SetModelName(MAKE_STRING("models/items/healthkit.mdl"));
	}
	PrecacheModel(STRING(GetModelName()));

	PrecacheScriptSound("HealthKit.Touch");
	//Custom sound
	if (m_schkPickupSound != NULL_STRING)
	{
		PrecacheScriptSound(STRING(m_schkPickupSound));
	}

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
// Output : 
//-----------------------------------------------------------------------------
bool CHealthKitCustom::MyTouch(CBasePlayer *pPlayer)
{
	if (pPlayer->TakeHealth(m_ichkHealthBonus, DMG_GENERIC))
	{
		CSingleUserRecipientFilter user(pPlayer);
		user.MakeReliable();

		UserMessageBegin(user, "ItemPickup");
		WRITE_STRING(GetClassname());
		MessageEnd();

		if (m_schkPickupSound != NULL_STRING)
		{
			CPASAttenuationFilter filter(this);

			EmitSound_t ep;
			ep.m_nChannel = CHAN_BODY;
			ep.m_pSoundName = (char*)STRING(m_schkPickupSound);
			ep.m_flVolume = 1.0;
			ep.m_SoundLevel = SNDLVL_NONE;

			EmitSound(filter, entindex(), ep);
		}
		else
		{
			CPASAttenuationFilter filter(pPlayer, "HealthKit.Touch");
			EmitSound(filter, pPlayer->entindex(), "HealthKit.Touch");
		}


		if (g_pGameRules->ItemShouldRespawn(this))
		{
			Respawn();
		}
		else
		{
			UTIL_Remove(this);
		}

		return true;
	}

	return false;
}

