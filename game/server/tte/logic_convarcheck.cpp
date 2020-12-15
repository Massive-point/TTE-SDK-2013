//========= Copyright Binka Corporation, LOLOLOLOLO KEK rights reserved. ============//
//
// Purpose: To check console variable and fire output.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Class Convarcheck
//-----------------------------------------------------------------------------

class CConVarCheck : public CPointEntity
{
public:
	DECLARE_CLASS(CConVarCheck, CPointEntity);

	void	Spawn(void);
	void	Precache(void);
	void	Activate(void);
	int		m_active;
	void TurnOn(void);
	void TurnOff(void);
	void	UpdateThink(void);

	float	OldConvarValue;
	bool	b_firstcvcpass = true;

	// Input handlers
	void InputCheckCvar(inputdata_t &inputdata);
	void InputGetCvValue(inputdata_t &inputdata);
	void InputTurnOn(inputdata_t &inputdata);
	void InputTurnOff(inputdata_t &inputdata);
	void InputToggle(inputdata_t &inputdata);

	COutputEvent m_OnCvarTrue;
	COutputEvent m_OnCvarFalse;
	COutputFloat m_OnGetCvValue;

	COutputEvent m_OnCvarChange;

	DECLARE_DATADESC();

	string_t m_iszCvarHkey;

};

LINK_ENTITY_TO_CLASS(logic_convarcheck, CConVarCheck);

BEGIN_DATADESC(CConVarCheck)

DEFINE_FIELD(m_active, FIELD_INTEGER),
DEFINE_FIELD(OldConvarValue, FIELD_FLOAT),
DEFINE_FIELD(b_firstcvcpass, FIELD_BOOLEAN),
DEFINE_KEYFIELD(m_iszCvarHkey, FIELD_STRING, "cvarhname"),
// Inputs
DEFINE_INPUTFUNC(FIELD_VOID, "CheckCvar", InputCheckCvar),
DEFINE_INPUTFUNC(FIELD_VOID, "GetCvValue", InputGetCvValue),
DEFINE_INPUTFUNC(FIELD_VOID, "TurnOn", InputTurnOn),
DEFINE_INPUTFUNC(FIELD_VOID, "TurnOff", InputTurnOff),
DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),

// Outputs
DEFINE_OUTPUT(m_OnCvarTrue, "OnCvarTrue"),
DEFINE_OUTPUT(m_OnCvarFalse, "OnCvarFalse"),
DEFINE_OUTPUT(m_OnGetCvValue, "OnGetCvValue"),
DEFINE_OUTPUT(m_OnCvarChange, "OnCvarChange"),

// Function Pointers
DEFINE_FUNCTION(UpdateThink),

END_DATADESC()

void CConVarCheck::Spawn(void)
{
	BaseClass::Spawn();
	Precache();
}

void CConVarCheck::Precache(void)
{
	BaseClass::Precache();
}

void CConVarCheck::Activate(void)
{
	BaseClass::Activate();

	if (m_active == 1)
	{
		SetThink(&CConVarCheck::UpdateThink);
		SetNextThink(gpGlobals->curtime);
	}
}

void CConVarCheck::InputTurnOn(inputdata_t &inputdata)
{
	if (!m_active)
	{
		TurnOn();
	}
}

void CConVarCheck::InputTurnOff(inputdata_t &inputdata)
{
	if (m_active)
	{
		TurnOff();
	}
}

void CConVarCheck::InputToggle(inputdata_t &inputdata)
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

void CConVarCheck::TurnOn(void)
{
	m_active = 1;

	SetThink(&CConVarCheck::UpdateThink);
	SetNextThink(gpGlobals->curtime);
}

void CConVarCheck::TurnOff(void)
{
	m_active = 0;

	SetNextThink(TICK_NEVER_THINK);
	SetThink(NULL);
}

//Check for Convar change
void CConVarCheck::UpdateThink(void)
{
	char *m_pszCvarhName = (char *)STRING(m_iszCvarHkey);
	ConVarRef cvarhname(m_pszCvarhName);
	if (!cvarhname.IsValid())
		return;

	float flNewConvarValue = cvarhname.GetFloat();

	if (b_firstcvcpass == true)
	{
		OldConvarValue = flNewConvarValue;
		b_firstcvcpass = false;
	}

	if (flNewConvarValue != OldConvarValue)
	{
		m_OnCvarChange.FireOutput(NULL, this);
		OldConvarValue = flNewConvarValue;
	}

	SetNextThink(gpGlobals->curtime);
}

void CConVarCheck::InputGetCvValue(inputdata_t &inputdata)
{
	char *m_pszCvarhName = (char *)STRING(m_iszCvarHkey);
	ConVarRef cvarhname(m_pszCvarhName);
	if (!cvarhname.IsValid())
		return;

	float flOutValue = cvarhname.GetFloat();
	m_OnGetCvValue.Set(flOutValue, inputdata.pActivator, inputdata.pCaller);
}

void CConVarCheck::InputCheckCvar(inputdata_t &inputdata)
{
	char *m_pszCvarhName = (char *)STRING(m_iszCvarHkey);
	ConVarRef cvarhname(m_pszCvarhName);
	if (!cvarhname.IsValid())
		return;

	//For expensive shadows
	//cl_disable_mapptexture
	if (!Q_stricmp(m_pszCvarhName, "cl_disable_mapptexture"))
	{
		ConVarRef ShittyShadowconVar("r_flashlightdepthtexture");
		if (ShittyShadowconVar.IsValid())
		{
			if (!ShittyShadowconVar.GetBool())
			{
				m_OnCvarTrue.FireOutput(inputdata.pActivator, this);
				return;
			}
		}

	}
	//

	if (cvarhname.GetBool())
	{
		m_OnCvarTrue.FireOutput(inputdata.pActivator, this);
	}
	else
	{
		m_OnCvarFalse.FireOutput(inputdata.pActivator, this);
	}
}
