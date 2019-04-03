#include "win32_con_pty_console.h"
#include <iostream>

extern
HRESULT CreatePseudoConsoleAndPipes(COORD, HPCON*, HANDLE*, HANDLE*);
extern
HRESULT InitializeStartupInfoAttachedToPseudoConsole(STARTUPINFOEX*, HPCON);

Win32ConPtyConsole::Win32ConPtyConsole()
    : m_PiClient {}
    , m_StartupInfo {}
    , m_hPC { INVALID_HANDLE_VALUE }
    , m_hPipeIn { INVALID_HANDLE_VALUE }
    , m_hPipeOut { INVALID_HANDLE_VALUE }
{
}

Win32ConPtyConsole::~Win32ConPtyConsole() {
    Terminate();
}

bool Win32ConPtyConsole::Create(uint32_t rows, uint32_t cols,
                                std::shared_ptr<char> cmd_line,
                                std::vector<char *> env) {
    (void)env;

    COORD c_size;
    c_size.X = (short)cols ? (short)cols : 1;
    c_size.Y = (short)rows ? (short)rows : 1;

    HRESULT hr = CreatePseudoConsoleAndPipes(c_size, &m_hPC, &m_hPipeIn, &m_hPipeOut);

    if (S_OK == hr)
    {
        // Initialize the necessary startup info struct
        hr = InitializeStartupInfoAttachedToPseudoConsole(&m_StartupInfo, m_hPC);
        if (S_OK == hr)
        {
            hr = CreateProcess(
                NULL,                           // No module name - use Command Line
                cmd_line.get(),                      // Command Line
                NULL,                           // Process handle not inheritable
                NULL,                           // Thread handle not inheritable
                FALSE,                          // Inherit handles
                EXTENDED_STARTUPINFO_PRESENT,   // Creation flags
                NULL,                           // Use parent's environment block
                NULL,                           // Use parent's starting directory
                &m_StartupInfo.StartupInfo,       // Pointer to STARTUPINFO
                &m_PiClient)                      // Pointer to PROCESS_INFORMATION
                    ? S_OK
                    : GetLastError();

            if (S_OK != hr) {
                std::cerr << "unable to create process:"
                          << cmd_line.get()
                          << ", hpc:" << m_hPC
                          << ", hr:" << hr
                          << ", error:" << GetLastError()
                          << std::endl;
            } else {
                std::cerr << "created process:"
                          << cmd_line.get()
                          << ", rows:"
                          << rows << "," << c_size.Y
                          << ", cols:"
                          << cols
                          << ", " << c_size.X
                          << std::endl;
            }
        }
        else {
            std::cerr << "initialize startup info and attach pseudo console:"
                      << hr
                      << std::endl;
        }
    }
    else {
        std::cerr << "unable to create pseudo console and pipes:"
                  << hr
                  << std::endl;
    }

    return hr == S_OK;
}

bool Win32ConPtyConsole::Resize(uint32_t rows, uint32_t cols) {
    COORD c_size;
    c_size.X = (short)cols;
    c_size.Y = (short)rows;

    if (m_hPC != INVALID_HANDLE_VALUE) {
        if (S_OK != ResizePseudoConsole(m_hPC, c_size)) {
            std::cerr << "failed resized, row:" << rows << ", col:" << cols << std::endl;
            return false;
        }
        std::cerr << "resized, row:" << rows << ", col:" << cols << std::endl;
        return true;
    } else {
        std::cerr << "resized, row:" << rows << ", col:" << cols << ", hpc not ready" << std::endl;
        return false;
    }
}

bool Win32ConPtyConsole::Terminate() {
    // Now safe to clean-up client app's process-info & thread
    if (m_PiClient.hThread)
        CloseHandle(m_PiClient.hThread);

    if (m_PiClient.hProcess)
        CloseHandle(m_PiClient.hProcess);

    // Cleanup attribute list
    if (m_StartupInfo.lpAttributeList) {
        DeleteProcThreadAttributeList(m_StartupInfo.lpAttributeList);
        free(m_StartupInfo.lpAttributeList);
    }

    // Close ConPTY - this will terminate client process if running
    if (m_hPC != INVALID_HANDLE_VALUE)
        ClosePseudoConsole(m_hPC);

    // Clean-up the pipes
    if (INVALID_HANDLE_VALUE != m_hPipeOut) CloseHandle(m_hPipeOut);
    if (INVALID_HANDLE_VALUE != m_hPipeIn) CloseHandle(m_hPipeIn);

    ZeroMemory(&m_PiClient, sizeof(m_PiClient));
    ZeroMemory(&m_StartupInfo, sizeof(m_StartupInfo));
    m_hPC = INVALID_HANDLE_VALUE;
    m_hPipeIn = INVALID_HANDLE_VALUE;
    m_hPipeOut = INVALID_HANDLE_VALUE;

    return true;
}
