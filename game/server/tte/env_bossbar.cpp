//========= Copyright Binka Corporation, All rights reserved. ============//
//
// Purpose: Health bar for boss fights.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Class CBossBar
//-----------------------------------------------------------------------------

class CBossBar : public CPointEntity
{
public:
	DECLARE_CLASS(CBossBar, CPointEntity);

	void	Spawn(void);
	void	Precache(void);
	void	Activate(void);
	int		m_active;
	void TurnOn(void);
	void TurnOff(void);
	void	UpdateThink(void);

	string_t m_iszBossEntity;
	string_t m_iszBossHudName;

	// Input handlers
	void InputTurnOn(inputdata_t &inputdata);
	void InputTurnOff(inputdata_t &inputdata);
	void InputToggle(inputdata_t &inputdata);

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(env_bossbar, CBossBar);

BEGIN_DATADESC(CBossBar)

DEFINE_FIELD(m_active, FIELD_INTEGER),

DEFINE_KEYFIELD(m_iszBossEntity, FIELD_STRING, "BossEntity"),
DEFINE_KEYFIELD(m_iszBossHudName, FIELD_STRING, "bosshudname"),

// Input functions
DEFINE_INPUTFUNC(FIELD_VOID, "TurnOn", InputTurnOn),
DEFINE_INPUTFUNC(FIELD_VOID, "TurnOff", InputTurnOff),
DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),

// Function Pointers
DEFINE_FUNCTION(UpdateThink),

END_DATADESC()

void CBossBar::Spawn(void)
{
	BaseClass::Spawn();
	Precache();
}

void CBossBar::Precache(void)
{
	BaseClass::Precache();
}

void CBossBar::Activate(void)
{
	BaseClass::Activate();

	if (m_active == 1)
	{
		SetThink(&CBossBar::UpdateThink);
		SetNextThink(gpGlobals->curtime);
	}
}

void CBossBar::InputTurnOn(inputdata_t &inputdata)
{
	if (!m_active)
	{
		TurnOn();
	}
}

void CBossBar::InputTurnOff(inputdata_t &inputdata)
{
	if (m_active)
	{
		TurnOff();
	}
}

void CBossBar::InputToggle(inputdata_t &inputdata)
{
	if (m_active)
	{
		TurnOff();
	}
	else
	{
		TurnOn();
	}
}

void CBossBar::TurnOn(void)
{
	m_active = 1;

	SetThink(&CBossBar::UpdateThink);
	SetNextThink(gpGlobals->curtime);

}

void CBossBar::TurnOff(void)
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
	{
		return;
	}

	m_active = 0;
	pPlayer->m_iBossHealthBarActive = m_active;
	SetNextThink(TICK_NEVER_THINK);
	SetThink(NULL);
}

//Update variables
void CBossBar::UpdateThink(void)
{
	CBaseEntity *pBossEnt = gEntList.FindEntityByName(NULL, m_iszBossEntity);
	if (!pBossEnt)
	{
		return;
	}
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
	{
		return;
	}

	int CurrBossHealth;
	CurrBossHealth = pBossEnt->GetHealth();
	if (CurrBossHealth <= 0)
	{
		CurrBossHealth = 0;
	}
	pPlayer->m_iBossHealthBarMax = pBossEnt->GetMaxHealth();
	pPlayer->m_iBossHealthBarCur = CurrBossHealth;
	pPlayer->m_iBossHealthBarActive = m_active;
	pPlayer->m_iszBossNameHUD = m_iszBossHudName;

	//Warning("Entity Name: %s\n Max Health: %i\n Current Health: %i\n",
	//	m_iszBossHudName,
	//	pBossEnt->GetMaxHealth(),
	//	CurrBossHealth);

	SetNextThink(gpGlobals->curtime);
}