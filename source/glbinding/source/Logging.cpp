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

        while(!s_stop || (s_buffer.size(key) != 0))
        {
            auto i = s_buffer.cbegin(key);
            while(s_buffer.valid(key, i))
            {
                logfile << (*i)->toString();
                i = s_buffer.next(key, i);
            }
            std::chrono::milliseconds dura( 10 );
            std::this_thread::sleep_for( dura );
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

void Logging::log(FunctionCall * call)
{
    delete s_buffer.hat();
    while(!s_buffer.push(call));
    // s_buffer.push(call.toString());
}

Logging::TailIdentifier Logging::addTail()
{
    return s_buffer.addTail();
}

void Logging::removeTail(TailIdentifier key)
{
    s_buffer.removeTail(key);
}

const std::vector<Logging::BufferType>::const_iterator Logging::cbegin(TailIdentifier key)
{
    return s_buffer.cbegin(key);
}

bool Logging::valid(TailIdentifier key, const std::vector<Logging::BufferType>::const_iterator & it)
{
    return s_buffer.valid(key, it);
}

const std::vector<Logging::BufferType>::const_iterator Logging::next(TailIdentifier key, const std::vector<Logging::BufferType>::const_iterator & it)
{
    return s_buffer.next(key, it);
}

void Logging::release(TailIdentifier key, const std::vector<Logging::BufferType>::const_iterator & it)
{
    release(key, it);
}

unsigned int Logging::size(TailIdentifier key)
{
    return s_buffer.size(key);
}

} // namespace glbinding
