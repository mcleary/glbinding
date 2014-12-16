#pragma once

#include <sstream>
#include <thread>
#include <fstream>

#include <glbinding/AbstractFunction.h> 
#include <glbinding/AbstractValue.h> 
#include <glbinding/callbacks.h>

namespace glbinding 
{

template <typename T, unsigned long n>
class RingBuffer;

class GLBINDING_API Logging
{
    friend class AbstractFunction;

    private:
    static bool s_active;
    static bool s_stop;
    static bool s_finished;
    static glbinding::RingBuffer<std::string, 100> s_buffer;
    static std::mutex s_lockfinish;
    static std::condition_variable s_finishcheck;

    Logging() = delete;
    ~Logging() = delete;

    public:
    static void start();
    static void stop();

    protected:
    static bool isActive();
    static void log(const FunctionCall & call);
};
} // namespace glbinding
