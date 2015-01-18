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
    std::map<std::string, unsigned int> categoryCount
    {
        { "Program Pipelines", 0},
        { "Debug", 0},
        { "Vertex Array Objects", 0},
        { "Transform Feedback", 0},
        { "State Management", 0},
        { "Uncategorized", 0},
        { "Textures", 0},
        { "Buffer Objects", 0},
        { "Rendering", 0},
        { "Fixed Function", 0},
        { "Frame Buffers", 0},
        { "GL2 Textures", 0},
        { "Immediate Mode", 0},
        { "Samplers", 0},
        { "Shaders", 0},
        { "Queries", 0},
        { "Matrix State", 0},
        { "Syncing", 0},
        { "Client Arrays", 0},
        { "Call Lists", 0},
        { "Utility", 0},
        { "GL2 Rasterization", 0}
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

    for(auto it = categoryCount.cbegin(); it != categoryCount.cend(); ++it)
    {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << std::endl;

}

} // namespace logvis
