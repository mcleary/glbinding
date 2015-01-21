#pragma once

#include <logvis/logvis_api.h>
#include <glbinding/Logging.h>
#include <glbinding/Meta.h>
#include <../../glbinding/source/RingBuffer.h>

#include <list>

namespace logvis
{

class LOGVIS_API LogVis
{
public:
    LogVis();
    ~LogVis();

    void update();

protected:
    using CategoryStats = std::map<std::string, unsigned int>;

    CategoryStats getCurrentLogPart();
    void updateMax(CategoryStats currentCounts);
    void updateLast(CategoryStats currentCounts);
    int averageCount(std::string category);

protected:
    using TailIdentifier = unsigned int;

    glbinding::Logging::FunctionCallBuffer& log;
    TailIdentifier tailId;
    std::list<std::map<std::string, unsigned int>> lastStats;
    std::map<std::string, unsigned int> maxStats;
    unsigned short head = 0;
};

} // namespace logvis
