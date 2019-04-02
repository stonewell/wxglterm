#pragma once

#include <windows.h>
#include <memory>
#include <vector>

class Win32Console {
public:
    Win32Console() = default;
    virtual ~Win32Console() = default;

public:
    virtual bool Create(uint32_t rows, uint32_t cols,
                        std::shared_ptr<char> cmd_line,
                        std::vector<char *> env) = 0;
    virtual bool Resize(uint32_t rows, uint32_t cols) = 0;

    virtual HANDLE GetConInPipe() = 0;
    virtual HANDLE GetConOutPipe() = 0;

    virtual bool Terminate() = 0;
};

using Win32ConsolePtr = std::shared_ptr<Win32Console>;

Win32ConsolePtr CreateWin32Console(bool forceConPty = false);
