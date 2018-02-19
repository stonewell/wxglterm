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

void ScintillaEditor::SetVerticalScrollPos() {
}

void ScintillaEditor::SetHorizontalScrollPos() {
}

bool ScintillaEditor::ModifyScrollBars(Sci::Line nMax, Sci::Line nPage) {
    (void)nMax;
    (void)nPage;
    return false;
}

void ScintillaEditor::Copy() {
}

void ScintillaEditor::Paste() {
}

void ScintillaEditor::ClaimSelection() {
}

void ScintillaEditor::NotifyChange() {
}

void ScintillaEditor::NotifyParent(SCNotification scn) {
    (void)scn;
}

void ScintillaEditor::CopyToClipboard(const SelectionText &selectedText) {
    (void)selectedText;
}

void ScintillaEditor::SetMouseCapture(bool on) {
    (void)on;
}

bool ScintillaEditor::HaveMouseCapture() {
    return false;
}

sptr_t ScintillaEditor::DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
    return ScintillaBase::WndProc(iMessage, wParam, lParam);
}

void ScintillaEditor::CreateCallTipWindow(PRectangle rc) {
    (void)rc;
}

void ScintillaEditor::AddToPopUp(const char *label, int cmd, bool enabled) {
    (void)label;
    (void)cmd;
    (void)enabled;
}
