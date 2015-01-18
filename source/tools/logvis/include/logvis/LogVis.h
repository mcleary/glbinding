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
    using TailIdentifier = unsigned int;

    glbinding::Logging::FunctionCallBuffer& log;
    TailIdentifier tailId;
};

} // namespace logvis
