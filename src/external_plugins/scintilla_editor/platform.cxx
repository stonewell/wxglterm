#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "Platform.h"

#if 0
#define DGB_FUNC_CALLED {printf("file:%s func:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__);}
#else
#define DGB_FUNC_CALLED
#endif

namespace Scintilla {
ElapsedTime::ElapsedTime() {
    bigBit = 0;
    littleBit = 0;
    DGB_FUNC_CALLED;
}

double ElapsedTime::Duration(bool reset) {
	return reset;
    DGB_FUNC_CALLED;
}

void Platform::DebugPrintf(const char * format, ...) {
	va_list pArguments;
	va_start(pArguments, format);
	vprintf(format, pArguments);
	va_end(pArguments);
}

void Platform::Assert(const char *c, const char *file, int line) {
	printf("Assertion [%s] failed at %s %d\r\n", c, file, line);
	abort();
}

ColourDesired Platform::Chrome() {
    DGB_FUNC_CALLED;
	return ColourDesired(0xe0, 0xe0, 0xe0);
}

ColourDesired Platform::ChromeHighlight() {
    DGB_FUNC_CALLED;
	return ColourDesired(0xff, 0xff, 0xff);
}

int Platform::DefaultFontSize() {
    DGB_FUNC_CALLED;
    return 14;
}

const char *Platform::DefaultFont() {
    DGB_FUNC_CALLED;
    return "!Sans";
}

unsigned int Platform::DoubleClickTime() {
    DGB_FUNC_CALLED;
	return 500; 	// Half a second
}

ListBox::ListBox() {
    DGB_FUNC_CALLED;
}

ListBox::~ListBox() {
    DGB_FUNC_CALLED;
}

ListBox *ListBox::Allocate() {
    DGB_FUNC_CALLED;
	return nullptr;
}

Window::~Window() {
    DGB_FUNC_CALLED;
}

void Window::Destroy() {
    DGB_FUNC_CALLED;
}

PRectangle Window::GetClientPosition() {
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

PRectangle Window::GetPosition() {
    PRectangle rc(0,0,0,0);

    DGB_FUNC_CALLED;
    return rc;
}

void Window::SetPosition(PRectangle rc) {
    DGB_FUNC_CALLED;
    (void)rc;
}

void Window::SetPositionRelative(PRectangle rc, Window relativeTo) {
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

Font::Font() : fid(0) {
    DGB_FUNC_CALLED;
}

Font::~Font() {
    DGB_FUNC_CALLED;
}

void Font::Create(const FontParameters &fp) {
    (void)fp;
	Release();
    DGB_FUNC_CALLED;
}

void Font::Release() {
	fid = 0;
    DGB_FUNC_CALLED;
}

Menu::Menu() : mid(0) {
    DGB_FUNC_CALLED;
}

void Menu::CreatePopUp() {
	Destroy();
    DGB_FUNC_CALLED;
}

void Menu::Destroy() {
	mid = 0;
    DGB_FUNC_CALLED;
}

void Menu::Show(Point pt, Window &w) {
    (void)pt;
    (void)w;
    DGB_FUNC_CALLED;
}

Surface *Surface::Allocate(int) {
    DGB_FUNC_CALLED;
    return nullptr;
}

}
