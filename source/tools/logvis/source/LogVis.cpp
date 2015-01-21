#include <logvis/LogVis.h>
#include <iostream>

namespace logvis
{

LogVis::LogVis()
: log(glbinding::Logging::getBuffer())
{
    tailId = log.addTail();
    for (std::string category : glbinding::Meta::getCategories())
    {
        maxStats[category] = 0;
    };
}

LogVis::~LogVis()
{
    log.removeTail(tailId);
}

void LogVis::update()
{
    LogVis::CategoryStats categoryCount = getCurrentLogPart();

    updateMax(categoryCount);
    updateLast(categoryCount);

    for(auto it = categoryCount.cbegin(); it != categoryCount.cend(); ++it)
    {
        std::cout << it->first << ": " << it->second << " - " << averageCount(it->first) << " - " << maxStats[it->first] << std::endl;
    }
    std::cout << std::endl;
}

LogVis::CategoryStats LogVis::getCurrentLogPart()
{
    LogVis::CategoryStats categoryCount;
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
        ++categoryCount[glbinding::Meta::getCategory(command)];
    }

    return categoryCount;
}

void LogVis::updateMax(LogVis::CategoryStats currentCounts)
{
    for (auto it = maxStats.cbegin(); it != maxStats.cend(); ++it)
    {
        if (it->second < currentCounts[it->first])
        {
            maxStats[it->first] = currentCounts[it->first];
        }
    }
}

void LogVis::updateLast(LogVis::CategoryStats currentCounts)
{
    lastStats.push_back(currentCounts);
    if (lastStats.size() > 5)
        lastStats.pop_front();
}

int LogVis::averageCount(std::string category)
{
    int count = 0;
    for (auto stats : lastStats)
    {
        count += stats[category];
    }
    return static_cast<float>(count)/static_cast<float>(lastStats.size());
}

} // namespace logvis
