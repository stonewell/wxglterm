#pragma once

#include "win32_console.h"
#include "winpty_api.h"

class Win32WinPtyConsole : public Win32Console {
public:
    Win32WinPtyConsole();
    virtual ~Win32WinPtyConsole();

public:
    virtual bool Create(uint32_t rows, uint32_t cols,
                        std::shared_ptr<char> cmd_line,
                        std::vector<char *> env);
    virtual bool Resize(uint32_t rows, uint32_t cols);

    virtual HANDLE GetConInPipe() {return m_hPipeIn; }
    virtual HANDLE GetConOutPipe() {return m_hPipeOut; }
    virtual bool Terminate();
private:
    HANDLE m_hPipeIn;
    HANDLE m_hPipeOut;

    winpty_config_t * m_pWinPtyConfig;
    winpty_t * m_pWinPty;
    winpty_spawn_config_t * m_pWinPtySpawnConfig;

    HANDLE m_hProcess;
    HANDLE m_hThread;
};

bool HasWinPtyApi();
