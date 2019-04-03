#include "win32_console.h"
#include "win32_con_pty_console.h"
#ifdef BUILD_WITH_WINPTY
#include "win32_win_pty_console.h"
#endif
#include <iostream>

Win32ConsolePtr CreateWin32Console(bool forceConPty, bool forceWinPty) {
    bool use_con_pty = HasConPtyApi();
    bool use_win_pty =
#ifdef BUILD_WITH_WINPTY
            HasWinPtyApi();
#else
    false;
#endif

    if (forceConPty && !use_con_pty) {
        std::cerr << "force con pty but con pty is not avaiable!" << std::endl;
        return Win32ConsolePtr{};
    }

    if (forceWinPty && !use_win_pty) {
        std::cerr << "force win pty but win pty is not avaiable!" << std::endl;
        return Win32ConsolePtr{};
    }

    if (use_con_pty) {
        return std::make_shared<Win32ConPtyConsole>();
    }

#ifdef BUILD_WITH_WINPTY
    if (use_win_pty) {
        return std::make_shared<Win32WinPtyConsole>();
    }
#endif
    return Win32ConsolePtr{};
}
