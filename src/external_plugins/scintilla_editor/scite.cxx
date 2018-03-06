#include "scite.h"

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
};

FilePath SciTE::GetDefaultDirectory() {
    return FilePath {};
}

FilePath SciTE::GetSciteDefaultHome() {
    return FilePath {};
}

FilePath SciTE::GetSciteUserHome() {
    return FilePath {};
}
