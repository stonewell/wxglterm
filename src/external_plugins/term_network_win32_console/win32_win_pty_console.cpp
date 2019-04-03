#include "win32_win_pty_console.h"
#include <iostream>

#include <locale>
#include <codecvt>

static
std::wstring_convert<std::codecvt_utf8<wchar_t
                                       #ifdef __GNUC__
                                       , 0x10ffff, std::little_endian
                                       #endif
                                       >, wchar_t> wcharconv;

static HANDLE ConnectToNamedPipe(LPCWSTR name, DWORD access);

Win32WinPtyConsole::Win32WinPtyConsole()
    : Win32Console()
    , m_hPipeIn { INVALID_HANDLE_VALUE }
    , m_hPipeOut { INVALID_HANDLE_VALUE }
    , m_pWinPtyConfig {nullptr}
    , m_pWinPty {nullptr}
    , m_pWinPtySpawnConfig {nullptr}
    , m_hProcess {INVALID_HANDLE_VALUE}
    , m_hThread {INVALID_HANDLE_VALUE}
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

    winpty_error_ptr_t err {nullptr};
    bool rt = false;

    do {
        m_pWinPtyConfig = winpty_config_new(0, &err);

        if (!m_pWinPtyConfig || err) {
            std::wcerr << "create winpty config fail:"
                       << winpty_error_code(err)
                       << ", "
                       << winpty_error_msg(err)
                       << std::endl;
            rt = false;
            break;
        }

        winpty_config_set_initial_size(m_pWinPtyConfig,
                                       (int)cols,
                                       (int)rows);

        m_pWinPty = winpty_open(m_pWinPtyConfig, &err);

        if (!m_pWinPty || err) {
            std::wcerr << "create winpty fail:"
                       << winpty_error_code(err)
                       << ", "
                       << winpty_error_msg(err)
                       << std::endl;
            rt = false;
            break;
        }

        m_hPipeIn = ConnectToNamedPipe(winpty_conout_name(m_pWinPty), GENERIC_READ);
        m_hPipeOut = ConnectToNamedPipe(winpty_conin_name(m_pWinPty), GENERIC_WRITE);

        std::wstring w_cmd_line = wcharconv.from_bytes(cmd_line.get());

        m_pWinPtySpawnConfig = winpty_spawn_config_new(WINPTY_SPAWN_FLAG_AUTO_SHUTDOWN | WINPTY_SPAWN_FLAG_EXIT_AFTER_SHUTDOWN,
                                                       nullptr,
                                                       w_cmd_line.c_str(),
                                                       nullptr,
                                                       nullptr,
                                                       &err);

        if (!m_pWinPtySpawnConfig || err) {
            std::wcerr << "create winpty spawn config fail:"
                       << w_cmd_line.c_str()
                       << ", err code:"
                       << winpty_error_code(err)
                       << ", "
                       << winpty_error_msg(err)
                       << std::endl;
            rt = false;
            break;
        }

        DWORD create_process_error = 0;

        rt = (TRUE == winpty_spawn(m_pWinPty,
                                   m_pWinPtySpawnConfig,
                                   &m_hProcess,
                                   &m_hThread,
                                   &create_process_error,
                                   &err));

        if (!rt || err) {
            std::wcerr << "create winpty spawn fail:"
                       << ", err code:"
                       << winpty_error_code(err)
                       << ", "
                       << winpty_error_msg(err)
                       << ", create error:"
                       << create_process_error
                       << std::endl;
            rt = false;
            break;
        }
    }
    while(false);

    if (err) {
        winpty_error_free(err);
    }

    if (!rt) {
        Terminate();
    }

    return rt;
}

bool Win32WinPtyConsole::Resize(uint32_t rows, uint32_t cols) {
    (void)rows;
    (void)cols;

    if (!m_pWinPty)
        return false;

    winpty_error_ptr_t err {nullptr};
    if (!winpty_set_size(m_pWinPty, (int)cols, (int)rows, &err) || err) {
        std::wcerr << "winpty set size fail:"
                   << winpty_error_code(err)
                   << ", "
                   << winpty_error_msg(err)
                   << std::endl;
        winpty_error_free(err);

        return false;
    }

    return true;
}

bool Win32WinPtyConsole::Terminate() {
    if (m_hThread != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hThread);
        m_hThread = INVALID_HANDLE_VALUE;
    }

    if (m_hProcess != INVALID_HANDLE_VALUE) {
        TerminateProcess(m_hProcess, 0);
        WaitForSingleObject(m_hProcess, INFINITE);
        CloseHandle(m_hProcess);
        m_hProcess = INVALID_HANDLE_VALUE;
    }

    if (m_hPipeIn != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hPipeIn);
        m_hPipeIn = INVALID_HANDLE_VALUE;
    }

    if (m_hPipeOut != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hPipeOut);
        m_hPipeOut = INVALID_HANDLE_VALUE;
    }

    if (m_pWinPty) {
        winpty_free(m_pWinPty);
        m_pWinPty = nullptr;
    }

    if (m_pWinPtySpawnConfig) {
        winpty_spawn_config_free(m_pWinPtySpawnConfig);
        m_pWinPtySpawnConfig = nullptr;
    }

    if (m_pWinPtyConfig) {
        winpty_config_free(m_pWinPtyConfig);
        m_pWinPtyConfig = nullptr;
    }

    return true;
}

HANDLE ConnectToNamedPipe(LPCWSTR name, DWORD access) {
    while (1)
    {
        HANDLE hPipe = CreateFileW(
            name,   // pipe name
            access,
            0,              // no sharing
            NULL,           // default security attributes
            OPEN_EXISTING,  // opens existing pipe
            0,              // default attributes
            NULL);          // no template file

        // Break if the pipe handle is valid.

        if (hPipe != INVALID_HANDLE_VALUE)
            return hPipe;

        // Exit if an error other than ERROR_PIPE_BUSY occurs.

        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            std::wcerr << L"Could not open pipe:" << name <<". GLE=" <<  GetLastError() << std::endl;;
            return INVALID_HANDLE_VALUE;
        }

        // All pipe instances are busy, so wait for 20 seconds.

        if ( ! WaitNamedPipeW(name, 20000))
        {
            std::cerr << "Could not open pipe:" << name << ", 20 second wait timed out." << std::endl;
            return INVALID_HANDLE_VALUE;
        }
    }

    return INVALID_HANDLE_VALUE;
}
