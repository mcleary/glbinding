#include <glbinding/Logging.hpp>
#include "RingBuffer.hpp"

namespace glbinding
{
    bool Logging::s_active = false;
    bool Logging::s_stop = false;
    bool Logging::s_finished = false; 
    glbinding::RingBuffer<std::string, 100> Logging::s_buffer;
    std::mutex Logging::s_lockfinish;
    std::condition_variable Logging::s_finishcheck;

    void Logging::start()
    {
        s_active = true;
        s_stop = false;
        s_finished = false;
        std::thread writer([&]()
        {
            auto unix_timestamp = std::chrono::seconds(std::time(NULL));
            int unix_timestamp_x_1000 = std::chrono::milliseconds(unix_timestamp).count();

            std::string logname = "logs/logger_";
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
            s_finished = true;
            s_finishcheck.notify_all();
        });
        writer.detach();
    };

    void Logging::stop()
    {
    s_stop = true;
    std::unique_lock<std::mutex> locker(s_lockfinish);

    while(!s_finished){
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
    std::ostringstream os;
    os << call.function.name() << "(";

    for (unsigned i = 0; i < call.parameters.size(); ++i)
    {
        os << call.parameters[i]->asString();
        if (i < call.parameters.size() - 1)
            os << ", ";
    }

    os << ")";

    if (call.returnValue)
    {
        os << " -> " << call.returnValue->asString();
    }

    os << std::endl;
    std::string input = os.str();
    while(!s_buffer.push(input)){}
    };
}
