#pragma once

#include <thread>
#include <fstream>

#include <glbinding/AbstractFunction.h> 
#include <glbinding/AbstractValue.h> 
#include <glbinding/callbacks.h>

namespace glbinding 
{
static const int LOG_BUFFER_SIZE = 1000;

template <typename T, unsigned long n>
class RingBuffer;

class GLBINDING_API Logging
{
    friend class AbstractFunction;

public:
    static void start();
    static void stop();

protected:
    static bool isActive();
    static void log(const FunctionCall & call);

private:
    Logging() = delete;
    ~Logging() = delete;

private:
    static bool s_active;

    static bool s_stop;
    static bool s_finished;
    static std::mutex s_lockfinish;
    static std::condition_variable s_finishcheck;

    using FunctionCallBuffer = glbinding::RingBuffer<std::string, LOG_BUFFER_SIZE>;
    static FunctionCallBuffer s_buffer;

};
} // namespace glbinding
