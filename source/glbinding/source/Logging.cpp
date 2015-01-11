#include <glbinding/Logging.h>
#include <glbinding/callbacks.h>
#include "RingBuffer.h"

namespace glbinding
{
bool Logging::s_stop = false;
bool Logging::s_persisted = false;
std::mutex Logging::s_lockfinish;
std::condition_variable Logging::s_finishcheck;
Logging::FunctionCallBuffer Logging::s_buffer;

void Logging::start()
{
        using milliseconds = std::chrono::milliseconds;
        auto timestamp = milliseconds(std::time(0)).count();

        std::string logname = "logs/log_";
        logname += std::to_string(timestamp);
        start(logname);
};

void Logging::start(std::string filepath)
{
    addCallbackMask(CallbackMask::Logging);
    s_stop = false;
    s_persisted = false;

    std::thread writer([filepath]()
    {
        unsigned int key = s_buffer.addTail();
        std::ofstream logfile;
        logfile.open (filepath, std::ios::out);

        while(!s_stop || !s_buffer.isEmpty())
        {
            std::vector<BufferType> entries = s_buffer.pullCompleteTail(key);
            if (entries.size() != 0)
            {
                for (BufferType entry : entries)
                {
                    logfile << entry;
                };
                logfile.flush();
            }
            else
            {
                std::chrono::milliseconds dura( 10 );
                std::this_thread::sleep_for( dura );
            }

        }
        logfile.close();
        s_buffer.removeTail(key);
        s_persisted = true;
        s_finishcheck.notify_all();
    });
    writer.detach();
};

void Logging::stop()
{
    s_stop = true;
    std::unique_lock<std::mutex> locker(s_lockfinish);

    // Spurious wake-ups: http://www.codeproject.com/Articles/598695/Cplusplus-threads-locks-and-condition-variables
    while(!s_persisted)
    {
        s_finishcheck.wait(locker);
    }
    removeCallbackMask(CallbackMask::Logging);
};

void Logging::log(const FunctionCall & call)
{
    while(!s_buffer.push(call.toString()));
    // s_buffer.push(call.toString());
};

} // namespace glbinding
