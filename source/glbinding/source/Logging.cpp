#include <glbinding/Logging.h>
#include "RingBuffer.hpp"

namespace glbinding
{
bool Logging::s_active = false;
bool Logging::s_stop = false;
bool Logging::s_finished = false;
std::mutex Logging::s_lockfinish;
std::condition_variable Logging::s_finishcheck;
Logging::FunctionCallBuffer Logging::s_buffer;

void Logging::start()
{
    s_active = true;
    s_stop = false;
    s_finished = false;
    std::thread writer([&]()
    {
        using milliseconds = std::chrono::milliseconds;
        auto unix_timestamp = milliseconds(std::time(NULL));
        int unix_timestamp_x_1000 = milliseconds(unix_timestamp).count();

        std::string logname = "logs/log_";
        logname += std::to_string(unix_timestamp_x_1000);
        std::ofstream logfile;
        logfile.open (logname, std::ios::out);

        std::string entry;
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
        s_finished = true;
        s_finishcheck.notify_all();
    });
    writer.detach();
};

void Logging::stop()
{
    s_stop = true;
    std::unique_lock<std::mutex> locker(s_lockfinish);

    // Spurious wake-ups: http://www.codeproject.com/Articles/598695/Cplusplus-threads-locks-and-condition-variables
    while(!s_finished)
    {
        s_finishcheck.wait(locker);
    }
    s_active = false;
};

bool Logging::isActive()
{
    return s_active;
};

void Logging::log(const FunctionCall & call)
{
    while(!s_buffer.push(call.toString()));
};

} // namespace glbinding
