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

#include "scite.h"
#include "scintilla_editor.h"

#include "term_context.h"
#include "term_window.h"

const GUI::gui_char appName[] = L"scintilla_editor";

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
ElapsedTime::ElapsedTime(){}
void Menu::Destroy() {}
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
