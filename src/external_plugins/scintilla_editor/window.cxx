#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <memory>
#include <sstream>

#include "Platform.h"
#include "term_window.h"
extern TermWindow * TermWindowFromEditor(void * wid);

#if 0
#define DGB_FUNC_CALLED {printf("file:%s func:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__);}
#else
#define DGB_FUNC_CALLED
#endif

namespace Scintilla {
Window::~Window() {
    DGB_FUNC_CALLED;
}

void Window::Destroy() {
    DGB_FUNC_CALLED;
}

PRectangle Window::GetClientPosition() const {
    DGB_FUNC_CALLED;
	return GetPosition();
}

void Window::InvalidateAll() {
    DGB_FUNC_CALLED;
}

void Window::InvalidateRectangle(PRectangle rc) {
    DGB_FUNC_CALLED;
    (void)rc;
}

void Window::SetFont(Font &) {
	// Can not be done generically but only needed for ListBox
    DGB_FUNC_CALLED;
}

void Window::Show(bool show) {
    (void)show;
    DGB_FUNC_CALLED;
}

PRectangle Window::GetPosition() const {
    TermWindow * pWindow = TermWindowFromEditor(wid);

    PRectangle rc(0,0,
                  pWindow ? pWindow->GetWidth() : 0,
                  pWindow ? pWindow->GetHeight() : 0);
    return rc;
}

void Window::SetPosition(PRectangle rc) {
    DGB_FUNC_CALLED;
    (void)rc;
}

void Window::SetPositionRelative(PRectangle rc, const Window * relativeTo) {
    DGB_FUNC_CALLED;
    (void)rc;
    (void)relativeTo;
}

void Window::SetCursor(Cursor curs) {
    DGB_FUNC_CALLED;
    (void)curs;
}

PRectangle Window::GetMonitorRect(Point pt) {
    (void)pt;

    DGB_FUNC_CALLED;
    return GetPosition();
}
}
