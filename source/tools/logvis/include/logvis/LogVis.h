#pragma once

#include <logvis/logvis_api.h>
#include <glbinding/Logging.h>
#include <glbinding/Meta.h>
#include <../../glbinding/source/RingBuffer.h>
#include <glbinding/gl/gl.h>

#include <list>

namespace logvis
{

class LOGVIS_API LogVis
{
public:
    LogVis(gl::GLuint logTexture);
    ~LogVis();

    void update();

protected:
    using CategoryStats = std::map<std::string, unsigned int>;

    CategoryStats getCurrentLogPart();
    void updateMax(CategoryStats currentCounts);
    void updateLast(CategoryStats currentCounts);
    int averageCount(std::string category);
    void renderLogTexture();
    void renderCats();

protected:
    using TailIdentifier = unsigned int;

    glbinding::Logging::FunctionCallBuffer& m_log;
    TailIdentifier m_tailId;
    std::list<std::map<std::string, unsigned int>> m_lastStats;
    std::map<std::string, unsigned int> m_maxStats;
    gl::GLuint m_logFrameBuffer;

    bool readFile(const std::string & filePath, std::string & content);
    std::string readFile(const std::string & filePath);

    const std::map<std::string, std::vector<float>> ColorByCategory
    {
        { "Program Pipelines", {102.0f/255.0f,194.0f/255.0f,165.0f/255.0f}},
        { "Debug", {252.0f/255.0f,141.0f/255.0f,98.0f/255.0f}},
        { "Vertex Array Objects", {141.0f/255.0f,160.0f/255.0f,203.0f/255.0f}},
        { "Transform Feedback", {231.0f/255.0f,138.0f/255.0f,195.0f/255.0f}},
        { "State Management", {166.0f/255.0f,216.0f/255.0f,84.0f/255.0f}},
        { "Uncategorized", {255.0f/255.0f,217.0f/255.0f,47.0f/255.0f}},
        { "Textures", {229.0f/255.0f,196.0f/255.0f,148.0f/255.0f}},
        { "Buffer Objects", {179.0f/255.0f,179.0f/255.0f,179.0f/255.0f}},
        { "Rendering", {102.0f/255.0f,194.0f/255.0f,165.0f/255.0f}},
        { "Fixed Function", {252.0f/255.0f,141.0f/255.0f,98.0f/255.0f}},
        { "Frame Buffers", {141.0f/255.0f,160.0f/255.0f,203.0f/255.0f}},
        { "GL2 Textures", {231.0f/255.0f,138.0f/255.0f,195.0f/255.0f}},
        { "Immediate Mode", {166.0f/255.0f,216.0f/255.0f,84.0f/255.0f}},
        { "Samplers", {255.0f/255.0f,217.0f/255.0f,47.0f/255.0f}},
        { "Shaders", {229.0f/255.0f,196.0f/255.0f,148.0f/255.0f}},
        { "Queries", {179.0f/255.0f,179.0f/255.0f,179.0f/255.0f}},
        { "Matrix State", {102.0f/255.0f,194.0f/255.0f,165.0f/255.0f}},
        { "Syncing", {252.0f/255.0f,141.0f/255.0f,98.0f/255.0f}},
        { "Client Arrays", {141.0f/255.0f,160.0f/255.0f,203.0f/255.0f}},
        { "Call Lists", {231.0f/255.0f,138.0f/255.0f,195.0f/255.0f}},
        { "Utility", {166.0f/255.0f,216.0f/255.0f,84.0f/255.0f}},
        { "GL2 Rasterization", {255.0f/255.0f,217.0f/255.0f,47.0f/255.0f }}
   };

};

} // namespace logvis
