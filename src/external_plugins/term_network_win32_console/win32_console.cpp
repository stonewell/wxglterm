#include "win32_console.h"
#include "win32_con_pty_console.h"

Win32ConsolePtr CreateWin32Console(bool forceConPty) {
    if (forceConPty || HasConPtyApi())
        return std::make_shared<Win32ConPtyConsole>();

    return Win32ConsolePtr{};
}
