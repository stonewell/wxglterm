#include <windows.h>
#include <process.h>
#include <iostream>

#include "pseudo_console_api.h"

HRESULT CreatePseudoConsoleAndPipes(COORD consoleSize, HPCON* phPC, HANDLE* phPipeIn, HANDLE* phPipeOut)
{
    HRESULT hr{ E_UNEXPECTED };
    HANDLE hPipePTYIn{ INVALID_HANDLE_VALUE };
    HANDLE hPipePTYOut{ INVALID_HANDLE_VALUE };

    // Create the pipes to which the ConPTY will connect
    if (CreatePipe(&hPipePTYIn, phPipeOut, NULL, 0) &&
        CreatePipe(phPipeIn, &hPipePTYOut, NULL, 0))
    {
        consoleSize.X = 80;
        consoleSize.Y = 25;
        // Create the Pseudo Console of the required size, attached to the PTY-end of the pipes
        hr = CreatePseudoConsole(consoleSize, hPipePTYIn, hPipePTYOut, 0, phPC);

        if (hr != S_OK) {
            std::cerr << "create pseudo console failed!:" << hr << std::endl;
        }
        // Note: We can close the handles to the PTY-end of the pipes here
        // because the handles are dup'ed into the ConHost and will be released
        // when the ConPTY is destroyed.
        if (INVALID_HANDLE_VALUE != hPipePTYOut) CloseHandle(hPipePTYOut);
        if (INVALID_HANDLE_VALUE != hPipePTYIn) CloseHandle(hPipePTYIn);
    } else {
        std::cerr << "create pipes failed!" << std::endl;
    }

    return hr;
}

// Initializes the specified startup info struct with the required properties and
// updates its thread attribute list with the specified ConPTY handle
HRESULT InitializeStartupInfoAttachedToPseudoConsole(STARTUPINFOEX* pStartupInfo, HPCON hPC)
{
    HRESULT hr{ E_UNEXPECTED };

    if (pStartupInfo)
    {
        ZeroMemory(pStartupInfo, sizeof(STARTUPINFOEX));

        size_t attrListSize{};

        pStartupInfo->StartupInfo.cb = sizeof(STARTUPINFOEX);

        // Get the size of the thread attribute list.
        InitializeProcThreadAttributeList(NULL, 1, 0, &attrListSize);

        // Allocate a thread attribute list of the correct size
        pStartupInfo->lpAttributeList =
            reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(malloc(attrListSize));

        // Initialize thread attribute list
        if (pStartupInfo->lpAttributeList
            && InitializeProcThreadAttributeList(pStartupInfo->lpAttributeList, 1, 0, &attrListSize))
        {
            // Set Pseudo Console attribute
            hr = UpdateProcThreadAttribute(
                pStartupInfo->lpAttributeList,
                0,
                PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                hPC,
                sizeof(HPCON),
                NULL,
                NULL)
                ? S_OK
                : HRESULT_FROM_WIN32(GetLastError());
        }
        else
        {
            std::cerr << "create attribute list failed" << std::endl;
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    return hr;
}

extern "C"
int test_main() {
    COORD c_size;
    c_size.X=80;
    c_size.Y=25;

    HPCON hPC = 0;
    HANDLE hPipeIn, hPipeOut;

    HRESULT hr = CreatePseudoConsoleAndPipes(c_size, &hPC, &hPipeIn, &hPipeOut);

    std::cout << "create result:" << hr << ", " << hPC << std::endl;
    return 0;
}
