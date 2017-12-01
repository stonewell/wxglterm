#include <pybind11/embed.h>

#include <iostream>
#include <unistd.h>
#include <vector>

#ifdef __APPLE__
#include <util.h>
#include <sys/ioctl.h>
#else
#include <pty.h>
#endif

#include <termios.h>
#include <fcntl.h>

#include <string.h>

#include <sys/select.h>
#include <sys/wait.h>

#include "plugin_manager.h"
#include "plugin.h"
#include "term_network.h"
#include "term_data_handler.h"
#include "term_context.h"
#include "term_window.h"

#include "PortableThread.h"

#include "app_config_impl.h"

static
void delete_data(void * data);

class TermNetworkPty
        : public virtual Plugin
        , public virtual TermNetwork
        , public virtual PortableThread::IPortableRunnable
{
public:
    TermNetworkPty() :
        Plugin()
        , m_Name("term_network_use_pty")
        , m_Description("terminal network plugin using pty")
        , m_Version(1)
        , m_Rows(0)
        , m_Cols(0)
        , m_MasterFD(-1)
        , m_ReadBuffer(8192)
        , m_PtyReaderThread(this)
    {
    }

    virtual ~TermNetworkPty() = default;

    MultipleInstancePluginPtr NewInstance() override {
        return MultipleInstancePluginPtr{new TermNetworkPty, delete_data};
    }

    bool m_Stopped;
    void Disconnect() override {
        m_Stopped = true;

        if (m_MasterFD == -1)
            return;

        close(m_MasterFD);
        m_PtyReaderThread.Join();
    }

    const char * GetName() override {
        return m_Name.c_str();
    }
    const char * GetDescription() override {
        return m_Description.c_str();
    }

    uint32_t GetVersion() override {
        return m_Version;
    }

    ContextPtr GetPluginContext() const override {
        return m_Context;
    }

    AppConfigPtr GetPluginConfig() const override {
        return m_PluginConfig;
    }

    void InitPlugin(ContextPtr context,
                    AppConfigPtr plugin_config) override {
        m_Context = context;
        m_PluginConfig = plugin_config;
    }
private:
    std::string m_Name;
    std::string m_Description;
    uint32_t m_Version;

protected:
    ContextPtr m_Context;
    AppConfigPtr m_PluginConfig;

    std::shared_ptr<char> m_TermName;
    std::shared_ptr<char> m_CmdLine;
    std::vector<char *> m_Args;

public:
    void BuildEnviron() {
        m_Envs.clear();

        extern char ** environ;

        char ** tmp = environ;

        std::string term_name = "TERM=" + GetPluginContext()->GetAppConfig()->GetEntry("/term/term_name", "xterm-256color");

        m_TermName = {strdup(term_name.c_str()), [](char * data) {
                free(data);
            }};

        bool term_updated = false;
        while(*tmp) {
            if (term_name != "NOT FOUND" && !strncmp(*tmp, "TERM=", strlen("TERM=")))
            {
                m_Envs.push_back(m_TermName.get());
                term_updated = true;
            }
            else
            {
                m_Envs.push_back(*tmp);
            }
            tmp++;
        }

        if (!term_updated) {
            m_Envs.push_back(m_TermName.get());
        }

        m_Envs.push_back(NULL);
    }

    void BuildCmdLine(const std::string & cmd_line)
    {
        m_CmdLine = {
            strdup(cmd_line.c_str()), [](char * data) {
                free(data);
            }};

        char * token = strtok(m_CmdLine.get(), " ");

        m_Args.clear();

        while(token != NULL) {
            m_Args.push_back(token);
            token = strtok(NULL, " ");
        }

        m_Args.push_back(NULL);
    }

    void Connect(const char * host, int port, const char * user_name, const char * password) override {
        (void)host;
        (void)port;
        (void)user_name;
        (void)password;

        m_Stopped = false;

        pid_t pid;
        struct winsize ws {
            (unsigned short)m_Rows,
                    (unsigned short)m_Cols,
                    0,
                    0
                    };

        std::string shell = GetPluginConfig()->GetEntry("shell", "NOT FOUND");

        if (shell == "NOT FOUND")
        {
            char * sys_shell = getenv("SHELL");

            if (sys_shell)
                shell = std::string(sys_shell);
            else
                shell = std::string("/bin/bash -i -l");
        }

        BuildEnviron();

        BuildCmdLine(shell);

        pid = forkpty(&m_MasterFD, NULL, NULL, &ws);

        // impossible to fork
        if (pid < 0) {
            std::cerr << "pty fork failed" << std::endl;
            return;
        }
        // child
        else if (pid == 0) {
#ifdef __APPLE__
            execve(m_Args[0], &m_Args[0], &m_Envs[0]);
#else
            execvpe(m_Args[0], &m_Args[0], &m_Envs[0]);
#endif
        }
        // parent
        else {
            int flags = fcntl(m_MasterFD, F_GETFL, 0);
            fcntl(m_MasterFD, F_SETFL, flags | O_NONBLOCK);

            m_PtyReaderThread.Start();
        }
    }

    void Send(const std::vector<unsigned char> & data, size_t n) override {
        write(m_MasterFD, &data[0], n);
    }

    void Resize(uint32_t row, uint32_t col) override {
        m_Rows = row;
        m_Cols = col;
        /*
          struct winsize {
          unsigned short ws_row;
          unsigned short ws_col;
          unsigned short ws_xpixel;   / unused /
          unsigned short ws_ypixel;   / unused /
          };
        */

        if (m_MasterFD == -1)
            return;

        struct winsize ws {
            (unsigned short)row,
                    (unsigned short)col,
                    0,
                    0
                    };

        int result = ioctl(m_MasterFD, TIOCSWINSZ, &ws);
        if (result) {
            std::cerr << "resize result:" << result << "," << errno
                      << ", " << EINVAL
                      << ", " << ENOTTY
                      << ", " << EPERM
                      << ", " << strerror(errno)
                      << std::endl;
        }
    }

    unsigned long Run(void * /*pArgument*/) override {
        struct timeval tv;

        TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

        if (!context)
            return 0;

        TermDataHandlerPtr term_data_handler =
                context->GetTermDataHandler();
        TermWindowPtr term_window =
                context->GetTermWindow();

        for (;;) {

            fd_set          read_fd;
            fd_set          except_fd;

            tv.tv_sec = 1;
            tv.tv_usec = 0;

            FD_ZERO(&read_fd);
            FD_ZERO(&except_fd);

            FD_SET(m_MasterFD, &read_fd);
            FD_SET(m_MasterFD, &except_fd);

            int result = select(m_MasterFD+1, &read_fd, NULL, &except_fd, &tv);

            if (m_Stopped)
                break;

            if (result == -1) {
                if (errno == EINTR) {
                    continue;
                }

                std::cerr << "select failed with err:" << errno << std::endl;
                break;
            } else if (result == 0) {
                //timeout
                continue;
            }

            if (FD_ISSET(m_MasterFD, &except_fd))
            {
                break;
            }

            if (FD_ISSET(m_MasterFD, &read_fd))
            {
                int count = read(m_MasterFD, &m_ReadBuffer[0], m_ReadBuffer.size());
                if (count == -1) {
                    if (errno == EINTR)
                        continue;

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
    uint32_t m_Rows;
    uint32_t m_Cols;
    int m_MasterFD;
    std::vector<unsigned char> m_ReadBuffer;
    PortableThread::CPortableThread m_PtyReaderThread;
    std::vector<char *> m_Envs;
};

void delete_data(void * data) {
    delete (Plugin*)data;
}

extern "C"
void register_plugins(PluginManagerPtr plugin_manager) {
    plugin_manager->RegisterPlugin(PluginPtr {new TermNetworkPty, delete_data});;
}
