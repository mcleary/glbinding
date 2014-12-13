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

public:
    Logging()
    {
        m_active = false;
        m_stop = false;
        m_finished = false; 
    };
    
    // virtual ~Logging();

    static void start()
    {
        m_active = true;
        m_stop = false;
        m_finished = false;
        std::thread writer([&]()
        {
            auto unix_timestamp = std::chrono::seconds(std::time(NULL));
            int unix_timestamp_x_1000 = std::chrono::milliseconds(unix_timestamp).count();

            std::string logname = "logs/test_";
            logname += std::to_string(unix_timestamp_x_1000);
            std::ofstream logfile;
            logfile.open (logname, std::ios::out);

            std::string entry;
            while(!m_stop || !buffer.isEmpty())
            {
                if(buffer.pull(&entry))
                {
                    logfile << entry;
                    logfile.flush();
                };
            }
            logfile.close();
        });
        writer.detach();
    };

    static void stop()
    {
    m_stop = true;
    while(!m_finished){}
    m_active = false;
    };

protected:
    static bool m_active;
    static bool m_stop;
    static bool m_finished; 
    static glbinding::RingBuffer<std::string, 100> buffer;

    static bool isActive()
    {
    return m_active;
    };

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
    while(!buffer.push(input)){}
    };

};
} // namespace glbinding
