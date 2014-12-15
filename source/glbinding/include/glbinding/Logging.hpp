#pragma once

#include <sstream>
#include <thread>
#include <fstream>

#include <glbinding/AbstractFunction.h> 
#include <glbinding/AbstractValue.h> 
#include <glbinding/callbacks.h>
#include <glbinding/RingBuffer.hpp>

namespace glbinding 
{

class Logging
{
    friend class AbstractFunction;

    private:
    static bool s_active;
    static bool s_stop;
    static bool s_finished;
    static glbinding::RingBuffer<std::string, 100> s_buffer;
    static std::mutex s_lockfinish;
    static std::condition_variable s_finishcheck;

    public:
    static void start()
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

    static void stop()
    {
    s_stop = true;
    std::unique_lock<std::mutex> locker(s_lockfinish);

    while(!s_finished){
        s_finishcheck.wait(locker);
    }
    s_active = false;
    };

    static bool isActive()
    {
        return s_active;
    };

protected:


    static void log(const FunctionCall & call)
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

};
} // namespace glbinding
