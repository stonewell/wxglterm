#pragma once

#include "win32_console.h"
#include "pseudo_console_api.h"

class Win32ConPtyConsole : public Win32Console {
public:
    Win32ConPtyConsole();
    virtual ~Win32ConPtyConsole();

public:
    virtual bool Create(uint32_t rows, uint32_t cols,
                        std::shared_ptr<char> cmd_line,
                        std::vector<char *> env);
    virtual bool Resize(uint32_t rows, uint32_t cols);

    virtual HANDLE GetConInPipe() {return m_hPipeIn; }
    virtual HANDLE GetConOutPipe() {return m_hPipeOut; }
    virtual bool Terminate();

private:
    PROCESS_INFORMATION m_PiClient;
    STARTUPINFOEX m_StartupInfo;
    HPCON m_hPC;

    //  Create the Pseudo Console and pipes to it
    HANDLE m_hPipeIn;
    HANDLE m_hPipeOut;
};

bool HasConPtyApi();
