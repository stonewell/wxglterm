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

#if 0
#define DGB_FUNC_CALLED {printf("file:%s func:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__);}
#else
#define DGB_FUNC_CALLED
#endif

namespace Scintilla {
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

ListBox::ListBox() noexcept{
    DGB_FUNC_CALLED;
}

ListBox::~ListBox() {
    DGB_FUNC_CALLED;
}

ListBox *ListBox::Allocate() {
    DGB_FUNC_CALLED;
	return nullptr;
}

Font::Font() noexcept : fid(0) {
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

Menu::Menu() noexcept : mid(0) {
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

}
