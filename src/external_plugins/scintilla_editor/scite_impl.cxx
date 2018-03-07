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
#include "color_theme.h"
#include "term_cell.h"

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

    if (!pEditor)
        return 0;
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
    , m_pTermWindow{nullptr}
    , m_PropsHomeDir{} {
}

FilePath SciTE::GetDefaultDirectory() {
    FilePath p{m_PropsHomeDir};

    return p.AbsolutePath().NormalizePath();
}

FilePath SciTE::GetSciteDefaultHome() {
    FilePath p{m_PropsHomeDir};

    return p.AbsolutePath().NormalizePath();
}

FilePath SciTE::GetSciteUserHome() {
    FilePath p{m_PropsHomeDir};

    return p.AbsolutePath().NormalizePath();
}

static
struct color_base_s {
    const char * key;
    uint32_t color_index;
} g_color_props[] = {
    {"colour.base02", 0},
    {"colour.red", 1},
    {"colour.green", 2},
    {"colour.yellow", 3},
    {"colour.blue", 4},
    {"colour.magenta", 5},
    {"colour.cyan", 6},
    {"colour.base2", 7},
    {"colour.base03", 8},
    {"colour.orange", 9},
    {"colour.base01", 10},
    {"colour.base00", 11},
    {"colour.base0", 12},
    {"colour.violet", 13},
    {"colour.base1", 14},
    {"colour.base3", 15},
    {"caret.fore", TermCell::DefaultCursorColorIndex},
};

static
const char * g_props[] = {
    "colour.code.comment.box","fore:$(colour.base01),italics",
    "colour.code.comment.line","fore:$(colour.base01),italics",
    "colour.code.comment.doc","fore:$(colour.base01),italics",
    "colour.code.comment.nested","fore:$(colour.base01),italics",
    "colour.text.comment","fore:$(colour.base01),italics",
    "colour.other.comment","fore:$(colour.base01),italics",
    "colour.embedded.comment","fore:$(colour.base01),italics",
    "colour.embedded.js","fore:$(colour.base01)",
    "colour.notused","fore:$(colour.base01)",
    "colour.number","fore:$(colour.cyan)",
    "colour.keyword","fore:$(colour.green)",
    "colour.string","fore:$(colour.cyan)",
    "colour.char","fore:$(colour.cyan)",
    "colour.operator","fore:$(colour.base01)",
    "colour.preproc","fore:$(colour.orange)",
    "colour.error","fore:$(colour.red)",
    "style.cpp.16","fore:$(colour.blue)",
};

void SciTE::Initialize(ScintillaEditor * pEditor,
                       TermWindow * pTermWindow,
                       const std::string & propsHomeDir,
                       const std::string & fileName) {
    m_pEditor = pEditor;
    m_pTermWindow = pTermWindow;

    m_PropsHomeDir = propsHomeDir;

    wEditor.SetID(pEditor);
    wOutput.SetID(new ScintillaEditor);

    buffers.Allocate(1);
    buffers.Add();

    SetFileName(fileName, true);

    char buf[255] = {0};

#define TC2SC(tc) (tc >> 24) & 0xFF, (tc >> 16) & 0xFF, (tc >> 8) & 0xFF

    sprintf(buf, "back:#%02x%02x%02x,fore:#%02x%02x%02x",
            TC2SC(pTermWindow->GetColorByIndex(TermCell::DefaultBackColorIndex)),
            TC2SC(pTermWindow->GetColorByIndex(TermCell::DefaultForeColorIndex)));

    props.Set("style.*.32", buf);

    for(size_t i=0;i < sizeof(g_color_props) / sizeof(color_base_s);i++) {
        sprintf(buf, "#%02x%02x%02x", TC2SC(pTermWindow->GetColorByIndex(g_color_props[i].color_index)));
        props.Set(g_color_props[i].key, buf);
    }

    for(size_t i=0;i < sizeof(g_props) / sizeof(const char *);i+=2) {
        props.Set(g_props[i], g_props[i+1]);
    }
#undef TC2SC

    ReloadProperties();
}
