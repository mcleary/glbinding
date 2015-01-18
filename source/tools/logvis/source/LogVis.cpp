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
    std::map<std::string, unsigned int> categoryCount;
    for (std::string category : glbinding::Meta::getCategories())
    {
        categoryCount[category] = 0;
    };

    std::vector<glbinding::Logging::BufferType> logEntries = log.pullTail(tailId);
    for (auto entry : logEntries)
    {
        int begin = entry.find(" ") + 1;
        int end = entry.find("(");
        std::string command = entry.substr(begin, end - begin);
        // std::cout << command << ": " << glbinding::Meta::getCategory(command) << std::endl;
        ++categoryCount[glbinding::Meta::getCategory(command)];
    }

    lastStats[head++ % 5] = categoryCount;

    for(auto it = categoryCount.cbegin(); it != categoryCount.cend(); ++it)
    {
        std::cout << it->first << ": " << it->second << " - " << averageCount(it->first)/lastStats.size() << std::endl;
    }
    std::cout << std::endl;
}

int LogVis::averageCount(std::string category)
{
    int count = 0;
    for (auto stats : lastStats)
    {
        count += stats[category];
    }
    return count;
}

} // namespace logvis
