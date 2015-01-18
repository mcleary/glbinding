#include <logvis/LogVis.h>
#include <iostream>

namespace logvis
{

LogVis::LogVis()
: log(glbinding::Logging::getBuffer())
{
    tailId = log.addTail();
}

LogVis::~LogVis()
{
    log.removeTail(tailId);
}

void LogVis::update()
{
    std::vector<glbinding::Logging::BufferType> logEntries = log.pullTail(tailId);
    for (auto entry : logEntries)
    {
        int begin = entry.find(" ") + 1;
        int end = entry.find("(");
        std::string command = entry.substr(begin, end - begin);
        std::cout << command << ": " << glbinding::Meta::getCategory(command) << std::endl;
    }
}

} // namespace logvis
