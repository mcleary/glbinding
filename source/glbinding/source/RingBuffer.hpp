#pragma once

#include <iostream>
#include <iomanip>
#include <thread>

#include "RingBuffer.h"

namespace glbinding
{


template <typename T, unsigned long n>
bool RingBuffer<T, n>::push(const T object)
{
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
bool RingBuffer<T, n>::pull(T & object)
{
    unsigned long tail = m_tail.load(std::memory_order_relaxed);
    if (tail == m_head.load(std::memory_order_acquire)) {
        return false;
    }
    object = m_ringBuffer.at(tail);
    m_tail.store(next(tail), std::memory_order_release);
    return true;
}

template <typename T, unsigned long n>
int RingBuffer<T, n>::size()
{
    if (m_head >= m_tail)
        return m_head - m_tail;
    else
        return (m_size - m_tail) + m_head;
}

template <typename T, unsigned long n>
bool RingBuffer<T, n>::isFull()
{
    unsigned long head = m_head.load(std::memory_order_relaxed);
    unsigned long nextHead = next(head);
    if (nextHead == m_tail.load(std::memory_order_acquire)) {
        return true;
    }
    return false;
}

template <typename T, unsigned long n>
bool RingBuffer<T, n>::isEmpty()
{
    unsigned long tail = m_tail.load(std::memory_order_relaxed);
    if (tail == m_head.load(std::memory_order_acquire)) {
        return true;
    }
    return false;
}

template <typename T, unsigned long n>
void RingBuffer<T, n>::registerConsumer(std::string key)
{
    m_tails[key] = m_tail.load(std::memory_order_acquire);
}

template <typename T, unsigned long n>
void RingBuffer<T, n>::deregisterConsumer(std::string key)
{
    m_tails.erase(key);
    updateTail();
}

template <typename T, unsigned long n>
bool RingBuffer<T, n>::pull(std::string key, T & object)
{
    unsigned long tail = m_tails[key].load(std::memory_order_relaxed);
    if (tail == m_head.load(std::memory_order_acquire)) {
        return false;
    }
    object = m_ringBuffer.at(tail);
    m_tails[key].store(next(tail), std::memory_order_release);
    updateTail();
    return true;
}

template <typename T, unsigned long n>
int RingBuffer<T, n>::size(std::string key)
{
    unsigned long tail = m_tails[key].load(std::memory_order_relaxed);
    if (m_head >= tail)
        return m_head - tail;
    else
        return (m_size - tail) + m_head;
}

template <typename T, unsigned long n>
void RingBuffer<T, n>::updateTail()
{
    unsigned long tail = m_tail.load(std::memory_order_relaxed);
    // std::cout << "Updating general tail: " << tail << std::endl;
    unsigned long currentMin = 2 * n;
    for(auto it = m_tails.cbegin(); it != m_tails.cend(); ++it ) {
        unsigned long tailPos = it->second.load(std::memory_order_relaxed);
        // std::cout << "Try tail: " << tailPos << std::endl;
        if (tailPos == tail)
        {
            // std::cout << "Now new tail" << std::endl;
            return;
        }

        if (tailPos < tail)
        {
            tailPos += m_size;
            // std::cout << "Tail is wrapped, thus adding size: " << tailPos << std::endl;
        }

        if (tailPos < currentMin)
        {
            currentMin = tailPos;
            // std::cout << "Found new min tail: " << currentMin << std::endl;
        }
    }
    if (currentMin == 2 * n)
    {
        // std::cout << "No tails" << currentMin << std::endl;
        return;
    }

    if (currentMin >= m_size)
    {
        currentMin = currentMin % m_size;
        // std::cout << "Min tail is wrapped, normalizing to: " << currentMin << std::endl;
    }
    // std::cout << "New tail is: " << currentMin << std::endl;
    m_tail.store(currentMin, std::memory_order_release);
    // std::cout << "New tail is: " << m_tail.load(std::memory_order_relaxed) << std::endl;
}

} // namespace glbinding