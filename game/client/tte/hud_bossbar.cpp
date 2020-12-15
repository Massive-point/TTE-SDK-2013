//========= Copyright © 2020-2052, Binka Corporation, All rights reserved. ============//
//
// Purpose: Draws Boss Health Bar on HUD.
//
//=============================================================================//


#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "c_baseplayer.h"

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Panel.h>

//extern ConVar cl_disable_bossbar;

class CHudBossBar : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudBossBar, vgui::Panel);

public:
	CHudBossBar(const char *pElementName);
	bool ShouldDraw();
private:
	void	Paint(void);
	void	ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	CHudTexture		*icon_flash_empty;
	CHudTexture		*icon_flash_full;

	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Default");
	CPanelAnimationVar(Color, m_BossTextColor, "BossTextColor", "255 16 0 255");
};

DECLARE_HUDELEMENT(CHudBossBar);


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudBossBar::CHudBossBar(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBossBar")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetProportional(false);
	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

bool CHudBossBar::ShouldDraw()
{
	//if (cl_disable_bossbar.GetBool())
	//{
	//	return false;
	//}

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
	{
		return false;
	}

	if (!pPlayer->m_iBossHealthBarActive)
	{
		return false;
	}

	bool bret = CHudElement::ShouldDraw();
	if (!bret)
		return false;

	return true;
}

void CHudBossBar::Paint(void)
{
	if (!ShouldDraw())
		return;

	int		x, y;
	bool	bIsOn;
	Color	clrFlash;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	if (!icon_flash_empty)
	{
		icon_flash_empty = gHUD.GetIcon("bossbar_empty");
	}

	if (!icon_flash_full)
	{
		icon_flash_full = gHUD.GetIcon("bossbar_full");
	}

	if (!icon_flash_empty || !icon_flash_full)
	{
		return;
	}

	bIsOn = pPlayer->m_iBossHealthBarActive;


	clrFlash.SetColor(255, 255, 255, 255);

	x = 0;
	y = 0;

	// Draw the flashlight casing
	icon_flash_empty->DrawSelf(x, y, clrFlash);

	// draw the flashlight energy level
	int nOffset = icon_flash_empty->Width() * (1.0 - ((float)pPlayer->m_iBossHealthBarCur / pPlayer->m_iBossHealthBarMax));
	if (nOffset < icon_flash_empty->Width())
	{
		icon_flash_full->DrawSelfCropped(x + nOffset, y, nOffset, 0, icon_flash_full->Width() - nOffset, icon_flash_full->Height(), clrFlash);
	}

	//Display Boss Name
	wchar_t text[128];
	g_pVGuiLocalize->ConvertANSIToUnicode(pPlayer->m_iszBossNameHUD, text, sizeof(text));
	if (text)
	{
		int wide, tall;
		vgui::surface()->GetTextSize(m_hTextFont, text, wide, tall);
		vgui::surface()->DrawSetTextFont(m_hTextFont);
		vgui::surface()->DrawSetTextColor(m_BossTextColor);
		vgui::surface()->DrawSetTextPos((256 - wide * 0.5), 40);
		vgui::surface()->DrawPrintText(text, wcslen(text));
	}
}

void CHudBossBar::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetProportional(false);
	SetPaintBackgroundEnabled(false);
}
