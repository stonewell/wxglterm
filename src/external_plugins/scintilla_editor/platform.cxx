#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "Platform.h"

namespace Scintilla {
ElapsedTime::ElapsedTime() {
    bigBit = 0;
    littleBit = 0;
}

double ElapsedTime::Duration(bool reset) {
	return reset;
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
	return ColourDesired(0xe0, 0xe0, 0xe0);
}

ColourDesired Platform::ChromeHighlight() {
	return ColourDesired(0xff, 0xff, 0xff);
}

int Platform::DefaultFontSize() {
    return 14;
}

const char *Platform::DefaultFont() {
    return "!Sans";
}

unsigned int Platform::DoubleClickTime() {
	return 500; 	// Half a second
}

ListBox::ListBox() {
}

ListBox::~ListBox() {
}

ListBox *ListBox::Allocate() {
	return nullptr;
}

Window::~Window() {}

void Window::Destroy() {
}

PRectangle Window::GetClientPosition() {
	// On GTK+, the client position is the window position
	return GetPosition();
}

void Window::InvalidateAll() {
}

void Window::InvalidateRectangle(PRectangle rc) {
    (void)rc;
}

void Window::SetFont(Font &) {
	// Can not be done generically but only needed for ListBox
}

void Window::Show(bool show) {
    (void)show;
}

PRectangle Window::GetPosition() {
    PRectangle rc(0,0,0,0);

    return rc;
}

void Window::SetPosition(PRectangle rc) {
    (void)rc;
}

void Window::SetPositionRelative(PRectangle rc, Window relativeTo) {
    (void)rc;
    (void)relativeTo;
}

void Window::SetCursor(Cursor curs) {
    (void)curs;
}

PRectangle Window::GetMonitorRect(Point pt) {
    (void)pt;

    return GetPosition();
}

Font::Font() : fid(0) {}

Font::~Font() {}

void Font::Create(const FontParameters &fp) {
    (void)fp;
	Release();
}

void Font::Release() {
	fid = 0;
}

Menu::Menu() : mid(0) {}

void Menu::CreatePopUp() {
	Destroy();
}

void Menu::Destroy() {
	mid = 0;
}

void Menu::Show(Point pt, Window &w) {
    (void)pt;
    (void)w;
}

Surface *Surface::Allocate(int) {
    return nullptr;
}

}
