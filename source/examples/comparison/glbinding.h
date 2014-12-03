#pragma once

#include <glbinding/Ringbuffer.hpp>

void glbinding_init();
void glbinding_test();

void glbinding_error(bool enable);
void glbinding_log(bool enable, glbinding::RingBuffer<std::string, 10> &buffer);
