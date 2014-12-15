#include <glbinding/Logging.hpp>

namespace glbinding 
{
    bool Logging::s_active = false;
    bool Logging::s_stop = false;
    bool Logging::s_finished = false; 
    glbinding::RingBuffer<std::string, 100> Logging::s_buffer;
    std::mutex Logging::s_lockfinish;
    std::condition_variable Logging::s_finishcheck;


} // namespace glbinding
