#pragma once
#include <atomic>
#include <array>
#include <map>
#include <vector>

namespace glbinding
{

template <typename T, uint64_t N>
class RingBuffer 
{

    uint64_t next(uint64_t current) {
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
    uint64_t size();

    bool isFull();
    bool isEmpty();

    using TailIdentifier = unsigned int;

    TailIdentifier addTail();
    void removeTail(TailIdentifier);
    T pull(TailIdentifier, bool& ok);
    T pull(TailIdentifier);
    std::vector<T> pullTail(TailIdentifier, uint64_t length);
    std::vector<T> pullTail(TailIdentifier);
    uint64_t sizeTail(TailIdentifier);

protected:
    void updateTail();
    uint64_t size(uint64_t, uint64_t);
    std::vector<T> pullBlock(uint64_t, uint64_t);

protected:
    std::array<T, N+1> m_buffer;
    const uint64_t m_size = N+1;
    std::atomic<uint64_t> m_head;
    std::atomic<uint64_t> m_tail;
    std::map<TailIdentifier, std::atomic<uint64_t>> m_tails;
    std::mutex m_tail_mutex;
};

} // namespace glbinding

#include "Ringbuffer.hpp"