#include "win32_console.h"
#include "win32_con_pty_console.h"
#ifdef BUILD_WITH_WINPTY
#include "win32_win_pty_console.h"
#endif

Win32ConsolePtr CreateWin32Console(bool forceConPty) {
    if (forceConPty || HasConPtyApi())
        return std::make_shared<Win32ConPtyConsole>();

#ifdef BUILD_WITH_WINPTY
    if (HasWinPtyApi()) {
        return std::make_shared<Win32WinPtyConsole>();
    }
#endif
    return Win32ConsolePtr{};
}
