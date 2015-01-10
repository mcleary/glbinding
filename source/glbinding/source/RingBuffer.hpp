#pragma once

#include <iostream>
#include <iomanip>
#include <thread>

#include "RingBuffer.h"

namespace glbinding
{


template <typename T, uint64_t n>
bool RingBuffer<T, n>::push(const T object)
{
    uint64_t head = m_head.load(std::memory_order_relaxed);
    uint64_t nextHead = next(head);
    if (nextHead == m_tail.load(std::memory_order_acquire)) {
        return false;
    }
    m_buffer.at(head % m_size) = object;
    m_head.store(nextHead, std::memory_order_release);
    return true;
}

template <typename T, uint64_t n>
bool RingBuffer<T, n>::pull(T & object)
{
    uint64_t tail = m_tail.load(std::memory_order_relaxed);
    if (tail == m_head.load(std::memory_order_acquire)) {
        return false;
    }
    object = m_buffer.at(tail % m_size);
    m_tail.store(next(tail), std::memory_order_release);
    return true;
}

template <typename T, uint64_t n>
int RingBuffer<T, n>::size()
{
    return m_head.load(std::memory_order_acquire) - m_tail.load(std::memory_order_acquire);
}

template <typename T, uint64_t n>
bool RingBuffer<T, n>::isFull()
{
    uint64_t head = m_head.load(std::memory_order_relaxed);
    uint64_t nextHead = next(head);
    if (nextHead == m_tail.load(std::memory_order_acquire)) {
        return true;
    }
    return false;
}

template <typename T, uint64_t n>
bool RingBuffer<T, n>::isEmpty()
{
    uint64_t tail = m_tail.load(std::memory_order_relaxed);
    if (tail == m_head.load(std::memory_order_acquire)) {
        return true;
    }
    return false;
}

template <typename T, uint64_t n>
unsigned int RingBuffer<T, n>::addTail()
{
    unsigned int i = 0;
    while(true)
    {
        if ( m_tails.find(i) != m_tails.end() ) {
          i++;
        } else {
            break;
        }
    }
    m_tails[i] = m_tail.load(std::memory_order_acquire);
    return i;
}

template <typename T, uint64_t n>
void RingBuffer<T, n>::removeTail(unsigned int key)
{
    m_tails.erase(key);
    updateTail();
}

template <typename T, uint64_t n>
bool RingBuffer<T, n>::pullTail(unsigned int key, T & object)
{
    uint64_t tail = m_tails[key].load(std::memory_order_relaxed);
    if (tail == m_head.load(std::memory_order_acquire)) {
        return false;
    }
    object = m_buffer.at(tail % m_size);
    m_tails[key].store(next(tail), std::memory_order_release);
    updateTail();
    return true;
}

template <typename T, uint64_t n>
int RingBuffer<T, n>::sizeTail(unsigned int key)
{
    return m_head.load(std::memory_order_acquire) - m_tails[key].load(std::memory_order_acquire);
}

template <typename T, uint64_t n>
void RingBuffer<T, n>::updateTail()
{
    m_tail_mutex.lock();
    uint64_t tail = m_tail.load(std::memory_order_acquire);
    uint64_t currentMin = 2 * n;

    for(auto it = m_tails.cbegin(); it != m_tails.cend(); ++it ) {
        uint64_t tailPos = it->second.load(std::memory_order_acquire);
        if (tailPos == tail)
        {
            m_tail_mutex.unlock();
            return;
        }

        if (tailPos < currentMin)
        {
            currentMin = tailPos;
        }
    }

    if (currentMin == 2 * n)
    {
        m_tail_mutex.unlock();
        return;
    }

    m_tail.store(currentMin, std::memory_order_release);
    m_tail_mutex.unlock();
}

template <typename T, uint64_t n>
std::vector<T> RingBuffer<T, n>::pullBlock(uint64_t begin, uint64_t end) {
    std::vector<T> result;

    for (uint64_t i = begin; i <= end; i++)
    {
        result.push_back(m_buffer.at(i % m_size));
    }

    return result;
}

} // namespace glbinding