#include <glbinding/Logging.h>

#include <cassert>

#include <glbinding/callbacks.h>
#include "RingBuffer.h"

namespace glbinding
{
//ToDo: Comment why array and not vector
//ToDo: Reason why 1000
static const unsigned int LOG_BUFFER_SIZE = 1000000;

bool Logging::s_stop = false;
bool Logging::s_persisted = false;
std::mutex Logging::s_lockfinish;
std::condition_variable Logging::s_finishcheck;
Logging::FunctionCallBuffer Logging::s_buffer(LOG_BUFFER_SIZE);

void Logging::start()
{
        using milliseconds = std::chrono::milliseconds;
        auto timestamp = milliseconds(std::time(0)).count();

        std::string logname = "logs/log_";
        logname += std::to_string(timestamp);
        start(logname);
};

void Logging::start(const std::string & filepath)
{
    addCallbackMask(CallbackMask::Logging);
    s_stop = false;
    s_persisted = false;

    std::thread writer([filepath]()
    {
        unsigned int key = s_buffer.addTail();
        std::ofstream logfile;
        logfile.open (filepath, std::ios::out);

        while(!s_stop || (s_buffer.sizeTail(key) != 0))
        {
            std::vector<BufferType*> entries = s_buffer.pullTail(key);
            if (entries.size() != 0)
            {
                for (BufferType* entry : entries)
                {
                    logfile << entry->toString();
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

void Logging::pause()
{
    removeCallbackMask(CallbackMask::Logging);
};

void Logging::resume()
{
    addCallbackMask(CallbackMask::Logging);
};

void Logging::log(FunctionCall && call)
{
    while(!s_buffer.push(std::forward<FunctionCall>(call)));
    // s_buffer.push(call.toString());
}

Logging::TailIdentifier Logging::addTail()
{
    return addTail();
}

void Logging::removeTail(TailIdentifier key)
{
    removeTail(key);
}

Logging::BufferType* Logging::pull(TailIdentifier key, bool & ok)
{
    return pull(key, ok);
}

Logging::BufferType* Logging::pull(TailIdentifier key)
{
    return pull(key);
}

std::vector<Logging::BufferType*> Logging::pullTail(TailIdentifier key, uint64_t length)
{
    return pullTail(key, length);
}

std::vector<Logging::BufferType*> Logging::pullTail(TailIdentifier key)
{
    return pullTail(key);
}

uint64_t Logging::sizeTail(TailIdentifier key)
{
    return sizeTail(key);
}

} // namespace glbinding
