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

void Window::SetFont(Font &) {
	// Can not be done generically but only needed for ListBox
}

void Window::Show(bool show) {
    (void)show;
}
}
