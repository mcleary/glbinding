#pragma once

#include <thread>
#include <fstream>

#include <glbinding/AbstractValue.h> 
#include <glbinding/callbacks.h>


namespace glbinding 
{

template <typename T>
class RingBuffer;


class GLBINDING_API Logging
{

public:
    static void start();
    static void start(const std::string & filepath);
    static void stop();
    static void pause();
    static void resume();

    static void log(FunctionCall && call);

    using BufferType = FunctionCall;
    using TailIdentifier = unsigned int;
    static TailIdentifier addTail();
    static void removeTail(TailIdentifier);
    static BufferType* pull(TailIdentifier key, bool & ok);
    static BufferType* pull(TailIdentifier key);
    static std::vector<BufferType*> pullTail(TailIdentifier key, uint64_t length);
    static std::vector<BufferType*> pullTail(TailIdentifier key);
    static uint64_t sizeTail(TailIdentifier key);

private:
    Logging() = delete;
    ~Logging() = delete;

private:
    static bool s_stop;
    static bool s_persisted;
    static std::mutex s_lockfinish;
    static std::condition_variable s_finishcheck;


    using FunctionCallBuffer = glbinding::RingBuffer<BufferType>;
    static FunctionCallBuffer s_buffer;


};


} // namespace glbinding
