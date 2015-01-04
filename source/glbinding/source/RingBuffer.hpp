#pragma once

#include <iostream>
#include <atomic>
#include <array>
#include <iomanip>
#include <thread>

namespace glbinding
{

template <typename T, unsigned long n>
class RingBuffer 
{
    std::array<T, n+1> m_ringBuffer;
    const int m_size = n+1;
    std::atomic<unsigned long> m_head;
    std::atomic<unsigned long> m_tail;

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
    bool pull(T*);
    int size();

    bool isFull();
    bool isEmpty();
};

template <typename T, unsigned long n>
bool RingBuffer<T, n>::push(const T object) {
    unsigned long head = m_head.load(std::memory_order_relaxed);
    unsigned long nextHead = next(head);
    if (nextHead == m_tail.load(std::memory_order_acquire)) {
        return false;
    }
    m_ringBuffer.at(head) = object;
    m_head.store(nextHead, std::memory_order_release);
    return true;
}

template <typename T, unsigned long n>
bool RingBuffer<T, n>::pull(T * object) {
    unsigned long tail = m_tail.load(std::memory_order_relaxed);
    if (tail == m_head.load(std::memory_order_acquire)) {
        return false;
    }
    object = &m_ringBuffer.at(tail);
    m_tail.store(next(tail), std::memory_order_release);
    return true;
  }

template <typename T, unsigned long n> int RingBuffer<T, n>::size() {
    if (m_head >= m_tail)
        return m_head - m_tail;
    else
        return (m_size - m_tail) + m_head;
}

template <typename T, unsigned long n> bool RingBuffer<T, n>::isFull() {
    unsigned long head = m_head.load(std::memory_order_relaxed);
    unsigned long nextHead = next(head);
    if (nextHead == m_tail.load(std::memory_order_acquire)) {
        return true;
    }
    return false;
}

template <typename T, unsigned long n> bool RingBuffer<T, n>::isEmpty() {
    unsigned long tail = m_tail.load(std::memory_order_relaxed);
    if (tail == m_head.load(std::memory_order_acquire)) {
        return true;
    }
    return false;
}

} // namespace glbinding