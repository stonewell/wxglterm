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
#include "plugin_base.h"
#include "term_network.h"
#include "term_data_handler.h"
#include "term_context.h"

#include "PortableThread.h"

#include "app_config_impl.h"

static
void delete_data(void * data);

class TermNetworkPty
        : public virtual PluginBase
        , public virtual TermNetwork
        , public virtual PortableThread::IPortableRunnable
{
public:
    TermNetworkPty() :
        PluginBase("term_network_use_pty", "terminal network plugin using pty", 1)
        , m_Rows(0)
        , m_Cols(0)
        , m_MasterFD(-1)
        , m_ReadBuffer(8192)
        , m_PtyReaderThread(this)
    {
        std::cout << "$$$$$$$$$$$$$$$$$$$$$$$ term network pty created:" << this << std::endl;
    }

    virtual ~TermNetworkPty() = default;

    MultipleInstancePluginPtr NewInstance() override {
        return MultipleInstancePluginPtr{new TermNetworkPty, delete_data};
    }

    void Disconnect() override {
        if (m_MasterFD == -1)
            return;

        close(m_MasterFD);
        m_PtyReaderThread.Join();
    }

    std::shared_ptr<char> m_TermName;
    std::shared_ptr<char> m_CmdLine;
    std::vector<char *> m_Args;

    void BuildEnviron() {
        m_Envs.clear();

        extern char ** environ;

        char ** tmp = environ;

        std::string term_name = "TERM=" + GetPluginContext()->GetAppConfig()->GetEntry("/term/term_name", "xterm-256color");

        m_TermName = {strdup(term_name.c_str()), [](char * data) {
                free(data);
            }};

        while(*tmp) {
            if (term_name != "NOT FOUND" && !strncmp(*tmp, "TERM=", strlen("TERM=")))
            {
                m_Envs.push_back(m_TermName.get());
            }
            else
            {
                m_Envs.push_back(*tmp);
            }
            tmp++;
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
        std::cout << "cmd line:" << cmd_line << std::endl;
        while(token != NULL) {
            std::cout << "token:" << token << std::endl;
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

        std::cout << "????? shell:"
                  << shell
                  << std::endl;
        BuildEnviron();

        BuildCmdLine(shell);

        pid = forkpty(&m_MasterFD, NULL, NULL, &ws);

        // impossible to fork
        if (pid < 0) {
            std::cout << "pty fork failed" << std::endl;
            return;
        }
        // child
        else if (pid == 0) {
            execvpe(m_Args[0], &m_Args[0], &m_Envs[0]);
        }
        // parent
        else {
            int flags = fcntl(m_MasterFD, F_GETFL, 0);
            fcntl(m_MasterFD, F_SETFL, flags | O_NONBLOCK);

            m_PtyReaderThread.Start();
        }
    }

    void Send(const char * data, size_t n) override {
        (void)data;
        (void)n;
        write(m_MasterFD, data, n);
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

        std::cout << "resize pty:" << row << "," << col << std::endl;
        int result = ioctl(m_MasterFD, TIOCSWINSZ, &ws);
        std::cout << "resize result:" << result << "," << errno
                  << ", " << EINVAL
                  << ", " << ENOTTY
                  << ", " << EPERM
                  << ", " << strerror(errno)
                  << std::endl;
    }

    unsigned long Run(void * /*pArgument*/) override {
        struct timeval tv;

        TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

        if (!context)
            return 0;

        TermDataHandlerPtr term_data_handler =
                context->GetTermDataHandler();

        for (;;) {

            fd_set          read_fd;
            fd_set          except_fd;

            tv.tv_sec = 5;
            tv.tv_usec = 0;

            FD_ZERO(&read_fd);
            FD_ZERO(&except_fd);

            FD_SET(m_MasterFD, &read_fd);
            FD_SET(m_MasterFD, &except_fd);

            int result = select(m_MasterFD+1, &read_fd, NULL, &except_fd, &tv);

            if (result == -1) {
                if (errno == EINTR) {
                    continue;
                }

                std::cout << "select failed with err:" << errno << std::endl;
                break;
            } else if (result == 0) {
                //timeout
                std::cout << "select timed out:...." << std::endl;
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

                    std::cout << "read failed\n" << std::endl;
                    break;
                }

                if (count > 0)
                    term_data_handler->OnData(&m_ReadBuffer[0], count);
            }
        }
        return 0;
    }
private:
    uint32_t m_Rows;
    uint32_t m_Cols;
    int m_MasterFD;
    std::vector<char> m_ReadBuffer;
    PortableThread::CPortableThread m_PtyReaderThread;
    std::vector<char *> m_Envs;
};

void delete_data(void * data) {
    std::cout << "delete plugin:" << data << std::endl;
    delete (Plugin*)data;
}

extern "C"
void register_plugins(PluginManagerPtr plugin_manager) {
    std::cout << "register plugin term_network_pty"
              << std::endl;
    plugin_manager->RegisterPlugin(PluginPtr {new TermNetworkPty, delete_data});;
}
