#pragma once

#include <logvis/logvis_api.h>
#include <glbinding/Logging.h>
#include <glbinding/Meta.h>
#include <../../glbinding/source/RingBuffer.h>

namespace logvis
{

class LOGVIS_API LogVis
{
public:
    LogVis();
    ~LogVis();

    void update();

protected:
    int averageCount(std::string category);

protected:
    using TailIdentifier = unsigned int;

    glbinding::Logging::FunctionCallBuffer& log;
    TailIdentifier tailId;
    std::array<std::map<std::string, unsigned int>, 5> lastStats;
    unsigned short head = 0;
};

} // namespace logvis
