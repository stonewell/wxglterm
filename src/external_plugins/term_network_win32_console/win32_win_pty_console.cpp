#include "win32_win_pty_console.h"
#include <iostream>


Win32WinPtyConsole::Win32WinPtyConsole()
    : Win32Console()
    , m_hPipeIn { INVALID_HANDLE_VALUE }
    , m_hPipeOut { INVALID_HANDLE_VALUE }
{
}

Win32WinPtyConsole::~Win32WinPtyConsole() {
    Terminate();
}

bool Win32WinPtyConsole::Create(uint32_t rows, uint32_t cols,
                                std::shared_ptr<char> cmd_line,
                                std::vector<char *> env) {
    (void)rows;
    (void)cols;
    (void)cmd_line;
    (void)env;

    return false;
}

bool Win32WinPtyConsole::Resize(uint32_t rows, uint32_t cols) {
    (void)rows;
    (void)cols;

    return false;
}

bool Win32WinPtyConsole::Terminate() {
    return false;
}
