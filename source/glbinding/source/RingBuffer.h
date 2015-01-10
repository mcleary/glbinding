#pragma once
#include <atomic>
#include <array>
#include <map>

namespace glbinding
{

template <typename T, unsigned long n>
class RingBuffer 
{
    std::array<T, n+1> m_ringBuffer;
    const int m_size = n+1;
    std::atomic<unsigned long> m_head;
    std::atomic<unsigned long> m_tail;
    std::map<std::string,std::atomic<unsigned long>> m_tails;

    unsigned long next(unsigned long current) {
        return (current + 1) % m_size;
    }

    public:
    RingBuffer()
    {
        m_head = 0;
        m_tail = 0;
    };

    bool push(T);
    bool pull(T&);
    int size();

    bool isFull();
    bool isEmpty();

    void registerConsumer(std::string);
    void deregisterConsumer(std::string);
    bool pull(std::string, T&);
    int size(std::string);

    private:
    void updateTail();
};

} // namespace glbinding

#include "Ringbuffer.hpp"