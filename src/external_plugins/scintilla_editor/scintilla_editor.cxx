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

#include "scintilla_editor.h"

using namespace Scintilla;

#if 0
#define DGB_FUNC_CALLED {printf("file:%s func:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__);}
#else
#define DGB_FUNC_CALLED
#endif

void ScintillaEditor::SetVerticalScrollPos() {
    DGB_FUNC_CALLED;
}

void ScintillaEditor::SetHorizontalScrollPos() {
    DGB_FUNC_CALLED;
}

bool ScintillaEditor::ModifyScrollBars(Sci::Line nMax, Sci::Line nPage) {
    (void)nMax;
    (void)nPage;
    DGB_FUNC_CALLED;
    return false;
}

void ScintillaEditor::Copy() {
    DGB_FUNC_CALLED;
}

void ScintillaEditor::Paste() {
    DGB_FUNC_CALLED;
}

void ScintillaEditor::ClaimSelection() {
    DGB_FUNC_CALLED;
}

void ScintillaEditor::NotifyChange() {
    DGB_FUNC_CALLED;
}

void ScintillaEditor::NotifyParent(SCNotification scn) {
    DGB_FUNC_CALLED;
    (void)scn;
}

void ScintillaEditor::CopyToClipboard(const SelectionText &selectedText) {
    (void)selectedText;
    DGB_FUNC_CALLED;
}

void ScintillaEditor::SetMouseCapture(bool on) {
    (void)on;
    DGB_FUNC_CALLED;
}

bool ScintillaEditor::HaveMouseCapture() {
    DGB_FUNC_CALLED;
    return false;
}

sptr_t ScintillaEditor::DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
    (void)iMessage;
    (void)wParam;
    (void)lParam;
    DGB_FUNC_CALLED;
    return 0;
}

void ScintillaEditor::CreateCallTipWindow(PRectangle rc) {
    DGB_FUNC_CALLED;
    (void)rc;
}

void ScintillaEditor::AddToPopUp(const char *label, int cmd, bool enabled) {
    (void)label;
    (void)cmd;
    (void)enabled;
    DGB_FUNC_CALLED;
}
