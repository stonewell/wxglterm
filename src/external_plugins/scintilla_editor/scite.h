#pragma once

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>

#include <string>
#include <vector>
#include <deque>
#include <set>
#include <map>
#include <algorithm>
#include <memory>
#include <sstream>
#include <iomanip>

#include "Scintilla.h"
#include "ILoader.h"

#include "GUI.h"

#include "StringList.h"
#include "StringHelpers.h"
#include "FilePath.h"
#include "StyleDefinition.h"
#include "PropSetFile.h"
#include "StyleWriter.h"
#include "Extender.h"
#include "SciTE.h"
#include "Mutex.h"
#include "JobQueue.h"
#include "Cookie.h"
#include "Worker.h"
#include "FileWorker.h"
#include "MatchMarker.h"
#include "SciTEBase.h"
#include "SciTEKeys.h"
#include "StripDefinition.h"

class SciTE : public SciTEBase {
public:
	virtual FilePath GetDefaultDirectory();
	virtual FilePath GetSciteDefaultHome();
	virtual FilePath GetSciteUserHome();
	virtual void Find() {}
	virtual MessageBoxChoice WindowMessageBox(GUI::Window &, const GUI::gui_string &, MessageBoxStyle style = mbsIconWarning) {(void)style; return mbOK;}
	virtual void FindMessageBox(const std::string &, const std::string *findItem = 0) {(void)findItem;}
	virtual void FindIncrement() {}
	virtual void FindInFiles() {}
	virtual void Replace() {}
	virtual void DestroyFindReplace() {}
	virtual void GoLineDialog() {}
	virtual bool AbbrevDialog() {return false;}
	virtual void TabSizeDialog() {}
	virtual bool ParametersOpen() {return false;}
	virtual void ParamGrab() {}
	virtual bool ParametersDialog(bool) {return false;}
	virtual void FindReplace(bool) {}
	virtual void StopExecute() {}
	virtual void SetFileProperties(PropSetFile &) {}
	virtual void AboutDialog() {}
	virtual void QuitProgram() {}
	virtual void SetStatusBarText(const char *) {}
	virtual void ShowToolBar() {}
	virtual void ShowTabBar() {}
	virtual void ShowStatusBar() {}
	virtual void ActivateWindow(const char *) {}
	virtual void SizeContentWindows() {}
	virtual void SizeSubWindows() {}

	virtual void SetMenuItem(int, int , int ,
                             const GUI::gui_char *, const GUI::gui_char *mnemonic = 0) {(void)mnemonic;}
	virtual void DestroyMenuItem(int, int) {}
	virtual void CheckAMenuItem(int, bool) {}
	virtual void EnableAMenuItem(int, bool) {}
	virtual void AddToPopUp(const char *, int cmd = 0, bool enabled = true) {(void)cmd;(void)enabled;}
	virtual void TabInsert(int, const GUI::gui_char *) {}
	virtual void TabSelect(int) {}
	virtual void RemoveAllTabs() {}
	virtual void WarnUser(int) {}
	virtual bool OpenDialog(const FilePath &, const GUI::gui_char *) {return false;}
	virtual bool SaveAsDialog() {return false;}
	virtual void SaveACopy() {}
	virtual void SaveAsHTML() {}
	virtual void SaveAsRTF() {}
	virtual void SaveAsPDF() {}
	virtual void SaveAsTEX() {}
	virtual void SaveAsXML() {}
	virtual bool PerformOnNewThread(Worker *) {return false;}
	// WorkerListener
	virtual void PostOnMainThread(int, Worker *) {}
	virtual void GetWindowPosition(int *, int *, int *, int *, int *) {};
    virtual bool PreOpenCheck(const GUI::gui_char *) {return false;};
};
