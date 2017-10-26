#include <pybind11/embed.h>

#include <iostream>
#include <unistd.h>
#include <vector>

#include <string.h>

#include <sys/select.h>
#include <sys/wait.h>

#include "plugin_base.h"
#include "term_network.h"
#include "term_data_handler.h"
#include "term_context.h"

#include "PortableThread.h"

#include "term_data_handler_impl.h"
#include "read_write_queue.h"

using TermDataQueue = moodycamel::BlockingReaderWriterQueue<char, 4096>;

class TermDataHandlerImpl
        : public virtual PluginBase
        , public virtual TermDataHandler
        , public virtual PortableThread::IPortableRunnable
{
public:
    TermDataHandlerImpl() :
        PluginBase("term_data_handler", "terminal data handler", 1)
        , m_DataHandlerThread(this)
        , m_TermDataQueue {4096}
        , m_Stopped{true}
    {
    }

    virtual ~TermDataHandlerImpl() = default;

    MultipleInstancePluginPtr NewInstance() override {
        return MultipleInstancePluginPtr{new TermDataHandlerImpl};
    }

    unsigned long Run(void * /*pArgument*/) override;
    void OnData(const char * data, size_t data_len) override;
    void Start() override;
    void Stop() override;

private:
    PortableThread::CPortableThread m_DataHandlerThread;
    TermDataQueue m_TermDataQueue;
    bool m_Stopped;
};

TermDataHandlerPtr CreateTermDataHandler()
{
    return TermDataHandlerPtr { new TermDataHandlerImpl };
}

unsigned long TermDataHandlerImpl::Run(void * /*pArgument*/) {
    while(true) {
        char c = '\0';

        if (!m_TermDataQueue.wait_dequeue_timed(c, std::chrono::microseconds(1000))) {
            std::cout << "queue wait timed out ..." << std::endl;

            if (m_Stopped)
                break;

            continue;
        }

        //process char
    };

    return 0;
}

void TermDataHandlerImpl::OnData(const char * data, size_t data_len) {
    for (size_t i = 0; i < data_len; i++) {
        m_TermDataQueue.enqueue(data[i]);
    }
}

void TermDataHandlerImpl::Start() {
    if (!m_Stopped)
        return;

    m_DataHandlerThread.Start();
}

void TermDataHandlerImpl::Stop() {
    if (m_Stopped)
        return;

    m_Stopped = true;

    OnData("A", 1);

    m_DataHandlerThread.Join();
}
