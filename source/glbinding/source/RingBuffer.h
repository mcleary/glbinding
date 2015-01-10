#pragma once
#include <atomic>
#include <array>
#include <map>
#include <vector>

namespace glbinding
{

template <typename T, uint64_t n>
class RingBuffer 
{
    std::array<T, n+1> m_buffer;
    const uint64_t m_size = n+1;
    std::atomic<uint64_t> m_head;
    std::atomic<uint64_t> m_tail;
    std::map<unsigned int, std::atomic<uint64_t>> m_tails;
    std::mutex m_tail_mutex;

    uint64_t next(uint64_t current) {
        return (current + 1) % (2 * m_size);
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

    unsigned int addTail();
    void removeTail(unsigned int);
    bool pullTail(unsigned int, T&);
    std::vector<T> pullTail(unsigned int, uint64_t length);
    int sizeTail(unsigned int);

    private:
    void updateTail();
    std::vector<T> pullBlock(uint64_t, uint64_t);

};

} // namespace glbinding

#include "Ringbuffer.hpp"