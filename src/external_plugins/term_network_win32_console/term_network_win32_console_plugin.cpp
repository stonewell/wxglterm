#include <windows.h>

#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <vector>

#include <string.h>

#include "plugin_manager.h"
#include "plugin_base.h"
#include "term_network.h"
#include "term_data_handler.h"
#include "term_context.h"
#include "term_window.h"

#include "PortableThread.h"

#include "app_config_impl.h"

#include "pseudo_console_api.h"

static
void delete_data(void * data);
extern
HRESULT CreatePseudoConsoleAndPipes(COORD, HPCON*, HANDLE*, HANDLE*);
extern
HRESULT InitializeStartupInfoAttachedToPseudoConsole(STARTUPINFOEX*, HPCON);

class TermNetworkWin32Console
        : public virtual PluginBase
        , public virtual TermNetwork
        , public virtual PortableThread::IPortableRunnable
{
public:
    TermNetworkWin32Console() :
        PluginBase("term_network_win32_console", "terminal network plugin using win32 console", 1)
        , hPC { INVALID_HANDLE_VALUE }
        , hPipeIn { INVALID_HANDLE_VALUE }
        , hPipeOut { INVALID_HANDLE_VALUE }
        , m_Rows(0)
        , m_Cols(0)
        , m_ReadBuffer(8192)
        , m_PtyReaderThread(this)
    {
    }

    virtual ~TermNetworkWin32Console() = default;

    MultipleInstancePluginPtr NewInstance() override {
        return MultipleInstancePluginPtr{new TermNetworkWin32Console, delete_data};
    }

    void Disconnect() override {
        m_Stopped = true;

        // Now safe to clean-up client app's process-info & thread
        CloseHandle(piClient.hThread);
        CloseHandle(piClient.hProcess);

        // Cleanup attribute list
        DeleteProcThreadAttributeList(startupInfo.lpAttributeList);
        free(startupInfo.lpAttributeList);

        // Close ConPTY - this will terminate client process if running
        ClosePseudoConsole(hPC);

        // Clean-up the pipes
        if (INVALID_HANDLE_VALUE != hPipeOut) CloseHandle(hPipeOut);
        if (INVALID_HANDLE_VALUE != hPipeIn) CloseHandle(hPipeIn);

        m_PtyReaderThread.Join();
    }

public:
    void Connect(const char * host, int port, const char * user_name, const char * password) override {
        (void)host;
        (void)port;
        (void)user_name;
        (void)password;

        AllocConsole();

        std::string shell = GetPluginConfig()->GetEntry("shell", "NOT FOUND");

        if (shell == "NOT FOUND")
        {
            char * sys_shell = getenv("ComSpec");

            if (sys_shell)
                shell = std::string(sys_shell);
            else
                shell = std::string("cmd.exe");
        }

        COORD c_size;
        c_size.X = (short)m_Cols ? (short)m_Cols : 80;
        c_size.Y = (short)m_Rows ? (short)m_Rows : 25;
        HRESULT hr = CreatePseudoConsoleAndPipes(c_size, &hPC, &hPipeIn, &hPipeOut);

        if (S_OK == hr)
        {
            // Initialize the necessary startup info struct
            if (S_OK == InitializeStartupInfoAttachedToPseudoConsole(&startupInfo, hPC))
            {
                m_CmdLine = {
                    _strdup(shell.c_str()),
                    [](char * data) {
                        free(data);
                    }};

                std::cerr << "+++++++network command line:" << shell << std::endl;

                hr = CreateProcess(
                    NULL,                           // No module name - use Command Line
                    m_CmdLine.get(),                      // Command Line
                    NULL,                           // Process handle not inheritable
                    NULL,                           // Thread handle not inheritable
                    FALSE,                          // Inherit handles
                    EXTENDED_STARTUPINFO_PRESENT,   // Creation flags
                    NULL,                           // Use parent's environment block
                    NULL,                           // Use parent's starting directory
                    &startupInfo.StartupInfo,       // Pointer to STARTUPINFO
                    &piClient)                      // Pointer to PROCESS_INFORMATION
                        ? S_OK
                        : GetLastError();

                if (S_OK == hr)
                {
                    m_PtyReaderThread.Start();
                std::cerr << "reader thread started" << std::endl;
                }
                else {
                    std::cerr << "unable to create process:"
                              << shell
                              << ", hpc:" << hPC
                              << ", error:" << GetLastError()
                              << std::endl;
                }
            }
            else {
                std::cerr << "initialize startup info and attach pseudo console"
                          << std::endl;
            }
        }
        else {
            std::cerr << "unable to create pseudo console and pipes"
                      << std::endl;
        }
    }

    void Send(const std::vector<unsigned char> & data, size_t n) override {
        while(n > 0) {
            DWORD wc = 0;

            std::cerr << "send data:" << n << std::endl;
            DWORD wn = n > 4096 ? 4096 : (DWORD)(n & 0xFFFFFFFF);
            if (!WriteFile(hPipeOut, &data[0], wn, &wc, NULL) || wc == 0) {
                std::cerr << "write failed:" << GetLastError() << std::endl;
                break;
            }

            n -= wc;
        }
    }

    void Resize(uint32_t row, uint32_t col) override {
        m_Rows = row;
        m_Cols = col;

        COORD c_size;
        c_size.X = (short)col;
        c_size.Y = (short)row;

        std::cerr << "resized, row:" << row << ", col:" << col << std::endl;
        if (hPC != INVALID_HANDLE_VALUE) {
            if (S_OK != ResizePseudoConsole(hPC, c_size)) {
                std::cerr << "failed resized, row:" << row << ", col:" << col << std::endl;
            }
        } else {
            std::cerr << "resized, row:" << row << ", col:" << col << ", hpc not ready" << std::endl;
        }
    }

    unsigned long Run(void * /*pArgument*/) override {
        TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

        if (!context)
            return 0;

        TermDataHandlerPtr term_data_handler =
                context->GetTermDataHandler();
        TermWindowPtr term_window =
                context->GetTermWindow();

        for (;;) {

            std::cerr << "stopped!" << std::endl;
            if (m_Stopped) {
                std::cerr << "stopped!" << std::endl;
                break;
            }

            DWORD result = WaitForSingleObject(hPipeIn, 1000);

            switch(result) {
            case WAIT_OBJECT_0:
                break;
            case WAIT_TIMEOUT:
                continue;
            default:
                break;
            }

            if (result != WAIT_OBJECT_0)
            {
                std::cerr << "wait object failed" << std::endl;
                term_window->Close();
                break;
            }

            if (result == WAIT_OBJECT_0)
            {
                DWORD count = 0;
                if (!ReadFile(hPipeIn, &m_ReadBuffer[0], (DWORD)m_ReadBuffer.size(), &count, NULL)) {
                    std::cerr << "read failed\n" << std::endl;
                    term_window->Close();
                    break;
                }

                if (count > 0) {
                    term_data_handler->OnData(m_ReadBuffer, count);
                }
            }
        }
        return 0;
    }

private:
    std::shared_ptr<char> m_CmdLine;
    bool m_Stopped;
    PROCESS_INFORMATION piClient;
    STARTUPINFOEX startupInfo;
    HPCON hPC;

    //  Create the Pseudo Console and pipes to it
    HANDLE hPipeIn;
    HANDLE hPipeOut;

    uint32_t m_Rows;
    uint32_t m_Cols;

    std::vector<unsigned char> m_ReadBuffer;
    PortableThread::CPortableThread m_PtyReaderThread;
    std::vector<char *> m_Envs;
    pid_t m_PtyPid;
};

void delete_data(void * data) {
    delete (Plugin*)data;
}

extern "C"
void register_plugins(PluginManagerPtr plugin_manager) {
    plugin_manager->RegisterPlugin(PluginPtr {new TermNetworkWin32Console, delete_data});;
}
