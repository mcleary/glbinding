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
    bool consumerSet;
    int consumerThreshold;
    bool consumerWorking;
    std::function<void (T result)> consume;

    unsigned long next(unsigned long current) {
        return (current + 1) % m_size;
    }

    public:
    RingBuffer()
    {
        m_head = 0;
        m_tail = 0;
        consumerSet = false;
        consumerWorking = false;
    };
    bool push(T);
    bool pull(T*);
    int size();

    void setConsumer(std::function<void (T result)> func, int start = 75);
    void startConsumer();
    bool consumerRunning();

    bool isFull();
    bool isEmpty();

    protected:
    bool overThreshold();
};

template <typename T, unsigned long n> bool RingBuffer<T, n>::push(const T object) {
    unsigned long head = m_head.load(std::memory_order_relaxed);
    unsigned long nextHead = next(head);
    if(consumerSet && !consumerWorking && overThreshold())
    {
      startConsumer();
    }
    if (nextHead == m_tail.load(std::memory_order_acquire)) {
        return false;
    }
    m_ringBuffer.at(head) = object;
    m_head.store(nextHead, std::memory_order_release);
    return true;
}

template <typename T, unsigned long n> bool RingBuffer<T, n>::pull(T * object) {
    unsigned long tail = m_tail.load(std::memory_order_relaxed);
    if (tail == m_head.load(std::memory_order_acquire)) {
        return false;
    }
    *object = m_ringBuffer.at(tail);
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

template <typename T, unsigned long n> void RingBuffer<T, n>::setConsumer(std::function<void (T result)> func, int start)
{
    consumerSet = true;
    consumerThreshold = start;
    consume = func;
}

template <typename T, unsigned long n> void RingBuffer<T, n>::startConsumer()
{
    if(!consumerWorking)
    {
        consumerWorking = true;
        std::thread consumer([&]()
        {
            T result;
            while(pull(&result))
            {
                consume(result);
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                // std::cout << "Consumed:" << result << std::endl;
            }

            consumerWorking = false;
            // std::cout << "Finished consuming -> " << consumerWorking << std::endl;
        });
        // std::cout << "Started consumer" << std::endl;
        consumer.detach();
    }
}

template <typename T, unsigned long n> bool RingBuffer<T, n>::overThreshold()
{
    int cap = size() * 100 / m_size;
    return cap >= consumerThreshold;
}

template <typename T, unsigned long n> bool RingBuffer<T, n>::consumerRunning()
{
    return consumerWorking;
}

} // namespace glbinding