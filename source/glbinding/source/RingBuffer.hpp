#pragma once

#include <iostream>
#include <iomanip>
#include <thread>
#include <iterator>

#include "RingBuffer.h"

namespace glbinding
{

template <typename T>
RingBuffer<T>::RingBuffer(const unsigned int maxSize)
:   m_size(maxSize)
,   m_head(0)
,   m_tail(0)
{

}

template <typename T>
bool RingBuffer<T>::push(T && object)
{
    uint64_t head = m_head.load(std::memory_order_relaxed);
    uint64_t nextHead = next(head);
    if (nextHead == m_tail.load(std::memory_order_acquire))
    {
        return false;
    }

    // 
    assert(head < m_size);
    if (m_buffer.size() <= head)
        m_buffer.push_back(object);
    else 
        m_buffer[head] = object;

    m_head.store(nextHead, std::memory_order_release);
    return true;
}

template <typename T>
typename RingBuffer<T>::TailIdentifier RingBuffer<T>::addTail()
{
    TailIdentifier i = 0;
    while(true)
    {
        if ( m_tails.find(i) == m_tails.end() ) {
          break;
        }
        i++;
    }
    m_tails[i] = m_tail.load(std::memory_order_acquire);
    return i;
}

template <typename T>
void RingBuffer<T>::removeTail(TailIdentifier key)
{
    m_tails.erase(key);
    updateTail();
}

template <typename T>
const typename std::vector<T>::const_iterator RingBuffer<T>::cbegin(TailIdentifier key)
{
    uint64_t tail = m_tails[key].load(std::memory_order_relaxed);
    auto i = m_buffer.cbegin();
    std::advance(i, tail);
    return i;
}

template <typename T>
bool RingBuffer<T>::valid(TailIdentifier key, const typename std::vector<T>::const_iterator & it)
{
    uint64_t tail = std::distance(m_buffer.cbegin(), it);
    uint64_t head = m_head.load(std::memory_order_acquire);
    return tail != head;
}

// Invalidates the old task
template <typename T>
const typename std::vector<T>::const_iterator RingBuffer<T>::next(TailIdentifier key, const typename std::vector<T>::const_iterator & it)
{
    uint64_t newTail = std::distance(m_buffer.cbegin(), it);
    m_tails[key].store(newTail, std::memory_order_release);
    updateTail();

    if (next(newTail) == 0)
        return m_buffer.cbegin();
    auto i = it;
    std::advance(i, 1);
    return i;
}

template <typename T>
void RingBuffer<T>::release(TailIdentifier key, const typename std::vector<T>::const_iterator & it)
{
    uint64_t newTail = std::distance(m_buffer.begin(), it);
    m_tails[key].store(newTail, std::memory_order_release);
    updateTail();    
}

template <typename T>
uint64_t RingBuffer<T>::sizeTail(TailIdentifier key)
{
    uint64_t head = m_head.load(std::memory_order_acquire);
    uint64_t tail = m_tails[key].load(std::memory_order_acquire);
    return size(head, tail);
}

template <typename T>
unsigned int RingBuffer<T>::maxSize() {
    return m_size;
}

template <typename T>
unsigned int RingBuffer<T>::size()
{
    uint64_t head = m_head.load(std::memory_order_acquire);
    uint64_t tail = m_tail.load(std::memory_order_acquire);
    return size(head, tail);
}

template <typename T>
bool RingBuffer<T>::isFull()
{
    uint64_t head = m_head.load(std::memory_order_relaxed);
    uint64_t nextHead = next(head);
    if (nextHead == m_tail.load(std::memory_order_acquire)) {
        return true;
    }
    return false;
}

template <typename T>
bool RingBuffer<T>::isEmpty()
{
    uint64_t tail = m_tail.load(std::memory_order_relaxed);
    if (tail == m_head.load(std::memory_order_acquire)) {
        return true;
    }
    return false;
}


//protected

template <typename T>
uint64_t RingBuffer<T>::next(uint64_t current) {
        return (current + 1) % m_size;
}

template <typename T>
void RingBuffer<T>::updateTail()
{
    m_tail_mutex.lock();
    uint64_t tail = m_tail.load(std::memory_order_acquire);
    uint64_t currentMin = 2 * m_size;

    for(auto it = m_tails.cbegin(); it != m_tails.cend(); ++it ) {
        uint64_t tailPos = it->second.load(std::memory_order_acquire);
        if (tailPos == tail)
        {
            m_tail_mutex.unlock();
            return;
        }

        if (tailPos < tail)
        {
            tailPos += m_size;
        }

        if (tailPos < currentMin)
        {
            currentMin = tailPos;
        }
    }

    if (currentMin == 2 * m_size)
    {
        m_tail_mutex.unlock();
        return;
    }

    if (currentMin >= m_size)
    {
        currentMin = currentMin % m_size;
    }

    m_tail.store(currentMin, std::memory_order_release);
    m_tail_mutex.unlock();
}

template <typename T>
uint64_t RingBuffer<T>::size(uint64_t head, uint64_t tail)
{
    if (head < tail)
    {
        return m_size - tail + head;
    }
    else
    {
        return head - tail;
    }
}

template <typename T>
std::vector<T*> RingBuffer<T>::pullBlock(uint64_t begin, uint64_t end) {
    std::vector<T*> result;

    for (uint64_t i = begin; i < end; i++)
    {
        result.push_back(&m_buffer.at(i % m_size));
    }

    return result;
}

} // namespace glbinding