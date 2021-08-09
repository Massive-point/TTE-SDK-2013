using namespace vgui;

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "c_baseplayer.h"

#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls\Panel.h>
#include <vgui\ISurface.h> 
#include <vgui/ILocalize.h>
#include "usermessages.h"
#include "hud_macros.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "engine/IEngineSound.h"

namespace vgui
{
	class IScheme;
};

class CHudMoney : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudMoney, vgui::Panel);

public:
	CHudMoney(const char* pElementName);
	bool ShouldDraw();
	virtual void Init();
	virtual void	ApplySchemeSettings(vgui::IScheme* scheme);
protected:
	virtual void OnThink(void);
	void Paint();
private:
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Default");
	CPanelAnimationVar(Color, m_hMoneyTextColor, "Text Color", "214 116 188 213");
	Label* m_pLabel;
};