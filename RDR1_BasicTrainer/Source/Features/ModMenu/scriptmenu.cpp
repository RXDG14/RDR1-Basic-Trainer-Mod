/*
	THIS FILE IS A PART OF RDR 2 SCRIPT HOOK SDK
				http://dev-c.com
			(C) Alexander Blade 2019
*/

#include "scriptmenu.h"
#include <format>
#include "pch.h"

inline int s_CustomFontId{};
inline int s_CustomFontId2{};

void DrawRect(float lineLeft, float lineTop, float lineWidth, float lineHeight, int r, int g, int b, int a)
{
	//GRAPHICS::DRAW_RECT((lineLeft + (lineWidth * 0.5f)), (lineTop + (lineHeight * 0.5f)), lineWidth, lineHeight, r, g, b, a, 0, 0);
	//drawRect((lineLeft + (lineWidth * 0.5f)), (lineTop + (lineHeight * 0.5f)), lineWidth, lineHeight, r, g, b, a, 0);
	RH_DrawRect((lineLeft + (lineWidth * 0.5f)), (lineTop + (lineHeight * 0.5f)), lineWidth, lineHeight, r, g, b, a);
}

void DDrawText(float x, float y, const std::string& _Text, int r, int g, int b, int a, int fontID, float fontSize, TextAlignment textAlignment)
{
	//UI::DRAW_TEXT(GAMEPLAY::CREATE_STRING(10, "LITERAL_STRING", str), x, y);
	//const char* str
	//const std::string& _Text;
	const char* safeCStr = _strdup(_Text.c_str());
	RH_DrawText(safeCStr, x, y, fontID, fontSize, r, g, b, a, textAlignment);
}

void MenuItemBase::WaitAndDraw(int ms)
{
	DWORD time = GetTickCount64() + ms;
	bool waited = false;
	while (GetTickCount64() < time || !waited)
	{
		ScriptWait(0);
		waited = true;
		if (auto menu = GetMenu())
			menu->OnDraw();
	}
}

void MenuItemBase::SetStatusText(string text, int ms)
{
	MenuController *controller;
	if (m_menu && (controller = m_menu->GetController()))
		controller->SetStatusText(text, ms);
}

void MenuItemBase::OnDraw(float lineTop, float lineLeft, bool active)
{
	ColorRgba color = active ? GetColorTextActive() : GetColorText();
	color = active ? m_colorRectActive : m_colorRect;
	// rect
	DrawRect(lineLeft, lineTop, m_lineWidth, m_lineHeight, color.r, color.g, color.b, color.a);
	// text
	DDrawText(lineLeft + m_textLeft, lineTop + m_lineHeight / 4.5f, const_cast<char*>(GetCaption().c_str()), color.r, color.g, color.b, color.a, s_CustomFontId, 0.05f, TextAlignment_Left);
	
}

void MenuItemDefault::OnDraw(float lineTop, float lineLeft, bool active)
{
	ColorRgba rectColor = active ? m_colorRectActive : m_colorRect;//active ? GetColorRectActive() : GetColorRect();
	ColorRgba textColor = active ? GetColorTextActive() : GetColorText();
	DrawRect(lineLeft, lineTop, m_lineWidth, m_lineHeight, rectColor.r, rectColor.g, rectColor.b, rectColor.a);
	DDrawText(lineLeft + m_textLeft, 0.02f + lineTop + m_lineHeight / 4.5f, GetCaption().c_str(), textColor.r, textColor.g, textColor.b, textColor.a, s_CustomFontId, 0.05f, TextAlignment_Left);
}

void MenuItemTitle::OnDraw(float lineTop, float lineLeft, bool active)
{
	ColorRgba rectColor = active ? m_colorRectActive : m_colorRect;//active ? GetColorRectActive() : GetColorRect();
	ColorRgba textColor = active ? GetColorTextActive() : GetColorText();
	DrawRect(lineLeft, lineTop, m_lineWidth, m_lineHeight, rectColor.r, rectColor.g, rectColor.b, rectColor.a);
	DDrawText(lineLeft + m_textLeft, 0.02f+lineTop + m_lineHeight / 5.0f, const_cast<char*>(GetCaption().c_str()), textColor.r, textColor.g, textColor.b, textColor.a, s_CustomFontId, 0.05f, TextAlignment_Left);
}

void MenuItemSwitchable::OnDraw(float lineTop, float lineLeft, bool active)
{	
	MenuItemDefault::OnDraw(lineTop, lineLeft, active);
	float lineWidth = GetLineWidth();
	float lineHeight = GetLineHeight();
	ColorRgba color = active ? GetColorTextActive() : GetColorText();
	DDrawText(0.03f + lineLeft + lineWidth - lineWidth / 6.35f, 0.02f+lineTop + lineHeight / 4.8f, const_cast<char*>(GetState() ? "[Y]" : "[N]"), color.r, color.g, color.b, color.a, s_CustomFontId, 0.05f, TextAlignment_Right);
}

void MenuItemMenu::OnDraw(float lineTop, float lineLeft, bool active)
{
	MenuItemDefault::OnDraw(lineTop, lineLeft, active);
	float lineWidth = GetLineWidth();
	float lineHeight = GetLineHeight();
	ColorRgba color = active ? GetColorTextActive() : GetColorText();
	DDrawText(lineLeft + lineWidth - lineWidth / 8, 0.02f+lineTop + lineHeight / 3.5f, const_cast < char*>("*"), color.r, color.g, color.b, color.a, s_CustomFontId, 0.05f, TextAlignment_Right);
}



void MenuItemMenu::OnSelect()
{
	if (auto parentMenu = GetMenu())
		if (auto controller = parentMenu->GetController())
			controller->PushMenu(m_menu);
}

void MenuBase::OnDraw()
{
	float lineTop = MenuBase_menuTop;
	float lineLeft = MenuBase_menuLeft;
	if (m_itemTitle->GetClass() == eMenuItemClass::ListTitle)
		reinterpret_cast<MenuItemListTitle *>(m_itemTitle)->
			SetCurrentItemInfo(GetActiveItemIndex() + 1, static_cast<int>(m_items.size()));
	m_itemTitle->OnDraw(lineTop, lineLeft, false);
	lineTop += m_itemTitle->GetLineHeight();
	for (int i = 0; i < MenuBase_linesPerScreen; i++)
	{
		int itemIndex = m_activeScreenIndex * MenuBase_linesPerScreen + i;
		if (itemIndex == m_items.size())
			break;
		MenuItemBase *item = m_items[itemIndex];
		item->OnDraw(lineTop, lineLeft, m_activeLineIndex == i);
		lineTop += item->GetLineHeight() - item->GetLineHeight() * MenuBase_lineOverlap;
	}
}

int MenuBase::OnInput()
{
	const int itemCount = static_cast<int>(m_items.size());
	const int itemsLeft = itemCount % MenuBase_linesPerScreen;
	const int screenCount = itemCount / MenuBase_linesPerScreen + (itemsLeft ? 1 : 0);
	const int lineCountLastScreen = itemsLeft ? itemsLeft : MenuBase_linesPerScreen;

	auto buttons = MenuInput::GetButtonState();	

	int waitTime = 0;

	if (buttons.a || buttons.b || buttons.up || buttons.down)
	{
		MenuInput::MenuInputBeep();
		waitTime = buttons.b ? 200 : 150;
	}

	if (buttons.a)
	{
		int activeItemIndex = GetActiveItemIndex();
		m_items[activeItemIndex]->OnSelect();
	} else
	if (buttons.b)
	{ 
		if (auto controller = GetController())
			controller->PopMenu();
	} else		
	if (buttons.up)
	{
		if (m_activeLineIndex-- == 0)
		{
			if (m_activeScreenIndex == 0)
			{
				m_activeScreenIndex = screenCount - 1;
				m_activeLineIndex = lineCountLastScreen - 1;
			} else
			{
				m_activeScreenIndex--;
				m_activeLineIndex = MenuBase_linesPerScreen - 1;
			}
		}
	} else
	if (buttons.down)
	{
		m_activeLineIndex++;
		if (m_activeLineIndex == ((m_activeScreenIndex == (screenCount - 1)) ? lineCountLastScreen : MenuBase_linesPerScreen))
		{
			if (m_activeScreenIndex == screenCount - 1)
				m_activeScreenIndex = 0;
			else
				m_activeScreenIndex++;
			m_activeLineIndex = 0;
		}
	}

	return waitTime;
}

void MenuController::DrawStatusText()
{
	if (GetTickCount64() < m_statusTextMaxTicks)
	{
		DDrawText(0.5, 0.5, const_cast<char*>(m_statusText.c_str()), 255, 255, 255, 255, s_CustomFontId, 0.05f, TextAlignment_Right);
	}
}