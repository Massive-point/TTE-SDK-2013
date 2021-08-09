//=========== Copyright 2021, Bank of Tel, All rights reserved. ================
//
// Purpose: To interface with the money system.
//		
//=============================================================================
#include "cbase.h"
#include <convar.h>
#include "game.h"
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTTEMoneyManager : public CLogicalEntity
{
public:

	DECLARE_CLASS(CTTEMoneyManager, CLogicalEntity);

	void Spawn();
	void Precache(void);

	DECLARE_DATADESC();

	//Input Functions
	void InputCheckMoney(inputdata_t& inputData); //Check value of player money vs current check value
	void InputCheckMoneyWithInputNumber(inputdata_t& inputData); //Check value of player money vs integer gained from IO value.
	void SetMoneyCheckValue(inputdata_t& inputData); //Set the value which player money is tested against in CheckMoney function.
	void InputAddMoney(inputdata_t& inputData); //Give the player money.
	void InputSubMoney(inputdata_t& inputData); //Take money from the player.
	void InputSetMoney(inputdata_t& inputData); //Set player money.

private:

	int m_intMoneyCheckValue; //Stores money check value.

	COutputEvent m_OnPassCheck; //The player has enough money.
	COutputEvent m_OnFailCheck; //The player does not have enough money.
};

LINK_ENTITY_TO_CLASS(env_money_manager, CTTEMoneyManager);

BEGIN_DATADESC(CTTEMoneyManager)

DEFINE_KEYFIELD(m_intMoneyCheckValue, FIELD_INTEGER, "MoneyCheckValue"), //Initial value to compare against player money
DEFINE_INPUTFUNC(FIELD_VOID, "CheckMoney", InputCheckMoney), //Check value of player money vs current check value
DEFINE_INPUTFUNC(FIELD_INTEGER, "CheckMoneyViaNumber", InputCheckMoneyWithInputNumber), //Check value of player money vs integer gained from IO value.
DEFINE_INPUTFUNC(FIELD_INTEGER, "SetMoneyCheckValue", SetMoneyCheckValue), //Set the value which player money is tested against in CheckMoney function.
DEFINE_INPUTFUNC(FIELD_INTEGER, "AddMoney", InputAddMoney), //Give the player money
DEFINE_INPUTFUNC(FIELD_INTEGER, "SubMoney", InputSubMoney), //Take player money
DEFINE_INPUTFUNC(FIELD_INTEGER, "SetMoney", InputSetMoney), //Set player money
DEFINE_OUTPUT(m_OnPassCheck, "OnPassMoneyCheck"),
DEFINE_OUTPUT(m_OnFailCheck, "OnFailMoneyCheck"),
END_DATADESC();

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTTEMoneyManager::Spawn(void)
{
	Precache();
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTTEMoneyManager::Precache(void)
{
	PrecacheSound("ui/money_shot.wav");
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTTEMoneyManager::InputCheckMoney(inputdata_t& inputData)
{
	ConVar* money = cvar->FindVar("money"); //Grab the money value from our convar.
	if (money->GetInt() >= m_intMoneyCheckValue) //Compare against money check value.
	{
		m_OnPassCheck.FireOutput(this, this); //We passed the vibe check.
	}
	else
	{
		m_OnFailCheck.FireOutput(this, this); //We failed the vibe check.
	}
}

//-----------------------------------------------------------------------------
// Purpose: Same as above, but this time we accept values from IO.
//-----------------------------------------------------------------------------
void CTTEMoneyManager::InputCheckMoneyWithInputNumber(inputdata_t& inputData)
{
	ConVar* money = cvar->FindVar("money"); //Grab the money value from our convar.
	if (money->GetInt() >= inputData.value.Int()) //Compare against money check value.
	{
		m_OnPassCheck.FireOutput(this, this); //We passed the vibe check.
	}
	else
	{
		m_OnFailCheck.FireOutput(this, this); //We failed the vibe check.
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set money check value.
//-----------------------------------------------------------------------------
void CTTEMoneyManager::SetMoneyCheckValue(inputdata_t& inputData)
{
	if (inputData.value.Int())
	{
		m_intMoneyCheckValue = inputData.value.Int();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Add money
//-----------------------------------------------------------------------------
void CTTEMoneyManager::InputAddMoney(inputdata_t& inputData)
{
	if (inputData.value.Int())
	{
		ConVar* money = cvar->FindVar("money"); //Grab the money value from our convar.
		int tempvalue = money->GetInt() + inputData.value.Int();
		money->SetValue(tempvalue);
	}
	EmitSound("ui/money_shot.wav");
}

//-----------------------------------------------------------------------------
// Purpose: Sub money
//-----------------------------------------------------------------------------
void CTTEMoneyManager::InputSubMoney(inputdata_t& inputData)
{
	if (inputData.value.Int())
	{
		ConVar* money = cvar->FindVar("money"); //Grab the money value from our convar.
		int tempvalue = money->GetInt() - inputData.value.Int();
		money->SetValue(tempvalue);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set money
//-----------------------------------------------------------------------------
void CTTEMoneyManager::InputSetMoney(inputdata_t& inputData)
{
	if (inputData.value.Int())
	{
		ConVar* money = cvar->FindVar("money"); //Grab the money value from our convar.
		money->SetValue(inputData.value.Int());
	}
}