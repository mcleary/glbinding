#pragma once

#include <thread>
#include <fstream>

#include <glbinding/AbstractValue.h> 
#include <glbinding/callbacks.h>


namespace glbinding 
{

//ToDo: Comment why array and not vector
//ToDo: Reason why 1000
static const int LOG_BUFFER_SIZE = 1000000;

template <typename T, uint64_t n>
class RingBuffer;


class GLBINDING_API Logging
{

public:
    static void start();
    static void start(std::string filepath);
    static void stop();
    static void pause();
    static void resume();

    static void log(const FunctionCall & call);

    using BufferType = std::string;
    using FunctionCallBuffer = glbinding::RingBuffer<BufferType, LOG_BUFFER_SIZE>;
    static FunctionCallBuffer& getBuffer();

private:
    Logging() = delete;
    ~Logging() = delete;

private:
    static bool s_stop;
    static bool s_persisted;
    static std::mutex s_lockfinish;
    static std::condition_variable s_finishcheck;

    static FunctionCallBuffer s_buffer;


};


} // namespace glbinding
