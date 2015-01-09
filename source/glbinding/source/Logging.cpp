#include <glbinding/Logging.h>
#include <glbinding/callbacks.h>
#include "RingBuffer.hpp"

namespace glbinding
{
bool Logging::s_stop = false;
bool Logging::s_persisted = false;
std::mutex Logging::s_lockfinish;
std::condition_variable Logging::s_finishcheck;
Logging::FunctionCallBuffer Logging::s_buffer;

void Logging::start()
{

    addCallbackMask(CallbackMask::Logging);
    s_stop = false;
    s_persisted = false;

    std::thread writer([&]()
    {
        using milliseconds = std::chrono::milliseconds;
        auto timestamp = milliseconds(std::time(NULL)).count();

        std::string logname = "logs/log_";
        logname += std::to_string(timestamp);
        std::ofstream logfile;
        logfile.open (logname, std::ios::out);

        BufferType entry;
        while(!s_stop || !s_buffer.isEmpty())
        {
            if(s_buffer.pull(&entry))
            {
                logfile << entry;
                logfile.flush();
            };
        }
        logfile.close();
        //
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
    // while(!s_buffer.push(call.toString()));
    s_buffer.push(call.toString());
};

} // namespace glbinding
