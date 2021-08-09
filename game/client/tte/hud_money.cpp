//========= Copyright © 2021, Bank of Tel, All rights reserved. ============//
//
// Purpose: Draw wallet size on screen.
//
//=============================================================================//


#include "cbase.h"
#include "c_baseplayer.h"
#include "hud_money.h"
#include "tier0/memdbgon.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudMoney);

ConVar cl_showmoney("cl_showmoney", "1", FCVAR_NONE, "Toggles money hud.");


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudMoney::CHudMoney(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudMoney")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetHiddenBits(HIDEHUD_PLAYERDEAD);
	SetVisible(true);
	SetProportional(true);

	SetScheme(scheme()->LoadSchemeFromFile("resource/hud_money.res", "Money"));

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (pPlayer)
	{
		m_pLabel = FindControl<vgui::Label>("HudMoneyLabel", true);
		m_pLabel->SetVisible(false);
		m_pLabel->SetText("");
		IScheme* pScheme = scheme()->GetIScheme(GetScheme());
		vgui::HFont font = pScheme->GetFont("CreditsText");
		m_pLabel->SetFont(font);
		m_pLabel->SetWide(512);
	}

	ivgui()->AddTickSignal(GetVPanel(), 100);
}

void CHudMoney::Init()
{
	SetVisible(true);
}

bool CHudMoney::ShouldDraw()
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
	{
		return false;
	}
	if (!pPlayer->IsAlive())
	{
		return false;
	}
	return true;
}

void CHudMoney::Paint(void)
{
	DevMsg("Paint step for hud_money\n");
	if (!ShouldDraw())
	{
		return;
	}

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
	{
		return;
	}

	wchar_t text[128];

	ConVarRef pMoney("money");

	//Gross
	char outputText[MAX_PATH];
	outputText[0] = '$';
	outputText[1] = NULL;
	V_strcat(outputText, pMoney.GetString(), sizeof(pMoney.GetString()) + 1);
	g_pVGuiLocalize->ConvertANSIToUnicode(outputText, text, sizeof(text));

	//Working fonts:
	// Default
	// CreditsText

	vgui::HScheme scheme = vgui::scheme()->GetScheme("ClientScheme");
	vgui::HFont hFont = vgui::scheme()->GetIScheme(scheme)->GetFont("HudMoneyFont");
	if (!hFont)
	{
		hFont = vgui::scheme()->GetIScheme(scheme)->GetFont("HudMoneyFont");
	}

	//Text
	int wide, tall;
	vgui::surface()->DrawSetTextFont(hFont);
	vgui::surface()->GetTextSize(hFont, text, wide, tall);
	vgui::surface()->DrawSetTextColor(m_hMoneyTextColor);
	vgui::surface()->DrawSetTextPos(ScreenWidth() - wide, ScreenHeight()/2);
	
	//Background
	vgui::surface()->DrawSetColor(60, 30, 80, 100);
	vgui::surface()->DrawFilledRect(ScreenWidth() - wide, ScreenHeight()/2, ScreenWidth(), ScreenHeight()/2 + tall);

	//Draw Text
	vgui::surface()->DrawPrintText(text, wcslen(text));
}

void CHudMoney::OnThink(void)
{
	if (cl_showmoney.GetBool())
	{
		SetVisible(true);
	}
	else
	{
		SetVisible(false);
	}

	BaseClass::OnThink();
}

void CHudMoney::ApplySchemeSettings(vgui::IScheme* scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
	SetSize(ScreenWidth(), ScreenHeight());
}