#include <vector>
#include <iostream>

#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cmath>

#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <cassert>
#include <functional>
#include <locale>
#include <codecvt>

#include "Platform.h"

#include "ILoader.h"
#include "ILexer.h"
#include "Scintilla.h"

#include "StringCopy.h"
#include "Position.h"
#include "UniqueString.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "ContractionState.h"
#include "CellBuffer.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "XPM.h"
#include "LineMarker.h"
#include "Style.h"
#include "ViewStyle.h"
#include "CharClassify.h"
#include "Decoration.h"
#include "CaseFolder.h"
#include "Document.h"
#include "UniConversion.h"
#include "Selection.h"
#include "PositionCache.h"
#include "EditModel.h"
#include "MarginView.h"
#include "EditView.h"
#include "Editor.h"
#include "AutoComplete.h"
#include "CallTip.h"
#include "ScintillaBase.h"

#include "scite_impl.h"
#include "scintilla_editor.h"

#include "term_context.h"
#include "term_window.h"

const GUI::gui_char appName[] = GUI_TEXT("scintilla_editor");

class MyMutex : public Mutex{
public:
	virtual void Lock() {}
	virtual void Unlock() {}
	virtual ~MyMutex() {}
};

Mutex * Mutex::Create() {
    return new MyMutex();
}

namespace GUI {
ElapsedTime::ElapsedTime(){
    (void)bigBit;
    (void)littleBit;
}
double ElapsedTime::Duration(bool reset) {
    (void)reset;
    return 0;
}

void Menu::Destroy() {}
void Menu::CreatePopUp() {}
void Menu::Show(Point, Window &){}

void Window::Destroy() {}
bool Window::HasFocus() { return false; }
Rectangle Window::GetPosition() { return Rectangle {}; }
void Window::SetPosition(Rectangle) {};
Rectangle Window::GetClientPosition() {return Rectangle {};}
void Window::Show(bool) {}
void Window::InvalidateAll() {}
void Window::SetTitle(const gui_char *) {}

gui_string StringFromUTF8(const char *s) {
    return gui_string {s};
}

gui_string StringFromUTF8(const std::string &s) {
    return s;
}
std::string UTF8FromString(const gui_string &s) {
    return s;
}
gui_string StringFromInteger(long i) {
	char number[32];
	sprintf(number, "%0ld", i);
	gui_char gnumber[32];
	size_t n=0;
	while (number[n]) {
		gnumber[n] = static_cast<gui_char>(number[n]);
		n++;
	}
	gnumber[n] = 0;
	return gui_string(gnumber);
}

gui_string StringFromLongLong(long long i) {
	try {
		std::ostringstream strstrm;
		strstrm << i;
		return StringFromUTF8(strstrm.str());
	} catch (std::exception &) {
		// Exceptions not enabled on stream but still causes diagnostic in Coverity.
		// Simply swallow the failure and return the default value.
	}
	return gui_string();
}

gui_string HexStringFromInteger(long i) {
	char number[32];
	sprintf(number, "%0lx", i);
	gui_char gnumber[32];
	size_t n = 0;
	while (number[n]) {
		gnumber[n] = static_cast<gui_char>(number[n]);
		n++;
	}
	gnumber[n] = 0;
	return gui_string(gnumber);
}

sptr_t ScintillaWindow::Send(unsigned int msg, uptr_t wParam, sptr_t lParam) {
    ScintillaEditor * pEditor = (ScintillaEditor*)GetID();
    return pEditor->WndProc(msg, wParam, lParam);
}

bool IsDBCSLeadByte(int codePage, char ch) {
	// Byte ranges found in Wikipedia articles with relevant search strings in each case
	unsigned char uch = static_cast<unsigned char>(ch);
	switch (codePage) {
		case 932:
			// Shift_jis
			return ((uch >= 0x81) && (uch <= 0x9F)) ||
				((uch >= 0xE0) && (uch <= 0xEF));
		case 936:
			// GBK
			return (uch >= 0x81) && (uch <= 0xFE);
		case 950:
			// Big5
			return (uch >= 0x81) && (uch <= 0xFE);
		// Korean EUC-KR may be code page 949.
	}
	return false;
}
};

SciTE::SciTE()
    : m_pEditor{nullptr}
    , m_pTermWindow{nullptr} {
}

FilePath SciTE::GetDefaultDirectory() {
    return FilePath {};
}

FilePath SciTE::GetSciteDefaultHome() {
    return FilePath {};
}

FilePath SciTE::GetSciteUserHome() {
    return FilePath {};
}

void SciTE::Initialize(ScintillaEditor * pEditor, TermWindow * pTermWindow) {
    m_pEditor = pEditor;
    m_pTermWindow = pTermWindow;
}
