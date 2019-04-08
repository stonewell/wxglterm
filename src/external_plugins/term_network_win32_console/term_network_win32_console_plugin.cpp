#include <windows.h>

#include <algorithm>
#include <iostream>
#include <fstream>
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

#include "win32_console.h"

#include "term_network_win32_console_export.h"

static
void delete_data(void * data);

class TermNetworkWin32Console
        : public TermNetwork
        , public PortableThread::IPortableRunnable
{
public:
    TermNetworkWin32Console() :
        PLUGIN_BASE_INIT_LIST("term_network_win32_console", "terminal network plugin using win32 console", 1)
        , m_Stopped{false}
        , m_Rows(0)
        , m_Cols(0)
        , m_ReadBuffer(8192)
        , m_PtyReaderThread(this)
        , m_Envs {}
        , m_Win32Console {}
        , m_hPipeIn { INVALID_HANDLE_VALUE }
        , m_hPipeOut { INVALID_HANDLE_VALUE }
        , m_CmdLine {}
        , m_Log {}
        , m_DoLog {false}
    {
    }

    virtual ~TermNetworkWin32Console() = default;

	PLUGIN_BASE_DEFINE();

    MultipleInstancePluginPtr NewInstance() override {
        return MultipleInstancePluginPtr{new TermNetworkWin32Console, delete_data};
    }

    void Disconnect() override {
        m_Stopped = true;

        if (m_Win32Console)
            m_Win32Console->Terminate();

        m_PtyReaderThread.Join();
    }

public:
    void Connect(const char * host, int port, const char * user_name, const char * password) override {
        (void)host;
        (void)port;
        (void)user_name;
        (void)password;

        std::string shell = GetPluginConfig()->GetEntry("shell", "NOT FOUND");
        bool force_winpty = GetPluginConfig()->GetEntryBool("force_winpty", false);
        bool force_conpty = GetPluginConfig()->GetEntryBool("force_conpty", false);
        std::string log_file = GetPluginConfig()->GetEntry("log_file", "NOT FOUND");

        if (log_file != "NOT FOUND") {
            m_Log.open(log_file);
            m_DoLog = true;
        }

        if (shell == "NOT FOUND")
        {
            char * sys_shell = getenv("ComSpec");

            if (sys_shell)
                shell = std::string(sys_shell);
            else
                shell = std::string("cmd.exe");
        }

        m_CmdLine = {
            _strdup(shell.c_str()),
            [](char * data) {
                free(data);
            }};

        m_Win32Console = CreateWin32Console(force_conpty, force_winpty);

        if (m_Win32Console) {
            if (m_Win32Console->Create(m_Rows, m_Cols, m_CmdLine, m_Envs)) {
                m_hPipeIn = m_Win32Console->GetConInPipe();
                m_hPipeOut = m_Win32Console->GetConOutPipe();

                m_PtyReaderThread.Start();
            } else {
                std::cerr << "win32 console create process failed." << std::endl;
                m_Win32Console = nullptr;
            }
        } else {
            std::cerr << "win32 console create failed." << std::endl;
        }
    }

    void Send(const std::vector<unsigned char> & data, size_t n) override {
        if (!m_Win32Console) return;

        size_t offset = 0;
        while(n > 0) {
            DWORD wc = 0;

            std::cerr << "send data:" << n << std::endl;
            DWORD wn = n > 4096 ? 4096 : (DWORD)(n & 0xFFFFFFFF);
            if (!WriteFile(m_hPipeOut, &data[offset], wn, &wc, NULL) || wc == 0) {
                std::cerr << "write failed:" << GetLastError() << std::endl;
                break;
            }

            n -= wc;
            offset += wc;
        }
    }

    void Resize(uint32_t row, uint32_t col) override {
        m_Rows = row;
        m_Cols = col;

        if (!m_Win32Console) return;

        if (!m_Win32Console->Resize(row, col)) {
            std::cerr << "win32 console resize failed, row:" << row << ", col:" << col << "." << std::endl;
        } else {
            std::cerr << "win32 console resize row:" << row << ", col:" << col << "." << std::endl;
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

            if (m_Stopped) {
                std::cerr << "really stopped!" << std::endl;
                break;
            }

            DWORD result = WaitForSingleObject(m_hPipeIn, 1000);

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
                if (!ReadFile(m_hPipeIn, &m_ReadBuffer[0], (DWORD)m_ReadBuffer.size(), &count, NULL)) {
                    std::cerr << "read failed\n" << std::endl;
                    term_window->Close();
                    break;
                }

                if (count > 0) {
					if (m_DoLog) {
                        std::string log_msg{};

                        for(size_t i=0;i < count; i++) {
                            if (m_ReadBuffer[i] == '\x1b')
                                log_msg.append("\\E");
                            else
                                log_msg.append((char *)&m_ReadBuffer[i], 1);
                        }

                        m_Log << log_msg;
					}
                    term_data_handler->OnData(m_ReadBuffer, count);
                }
            }
        }
        return 0;
    }

private:
    bool m_Stopped;

    uint32_t m_Rows;
    uint32_t m_Cols;

    std::vector<unsigned char> m_ReadBuffer;
    PortableThread::CPortableThread m_PtyReaderThread;
    std::vector<char *> m_Envs;

    Win32ConsolePtr m_Win32Console;
    HANDLE m_hPipeIn;
    HANDLE m_hPipeOut;

    std::shared_ptr<char> m_CmdLine;
    std::ofstream m_Log;
    bool m_DoLog;
};

void delete_data(void * data) {
    delete (Plugin*)data;
}

extern "C"
void TERM_NETWORK_WIN32_CONSOLE_EXPORT register_plugins(PluginManagerPtr plugin_manager) {
    plugin_manager->RegisterPlugin(PluginPtr {new TermNetworkWin32Console, delete_data});;
}
