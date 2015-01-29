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
    unsigned int averageCount(std::string category);
    void renderLogTexture();
    void renderCats();

protected:
    using TailIdentifier = unsigned int;

    TailIdentifier m_tailId;
    std::list<std::map<std::string, unsigned int>> m_lastStats;
    std::map<std::string, unsigned int> m_maxStats;
    gl::GLuint m_logFrameBuffer;

    bool readFile(const std::string & filePath, std::string & content);
    std::string readFile(const std::string & filePath);

    const std::list<std::string> m_categories
    {
        "Draw",
        "Render Target",
        "Texture",
        "UBO Binding",
        "Program",
        "Uniform Updates",
        "ROP",
        "Vertex Format",
        "Utility",
        "Uncategorized"
    };

    const std::map<std::string, std::vector<float>> ColorByCategory
    {
        { "Draw", {141.0f/255.0f,211.0f/255.0f,199.0f/255.0f}},
        { "Render Target", {255.0f/255.0f,255.0f/255.0f,179.0f/255.0f}},
        { "Texture", {190.0f/255.0f,186.0f/255.0f,218.0f/255.0f}},
        { "UBO Binding", {251.0f/255.0f,128.0f/255.0f,114.0f/255.0f}},
        { "Program", {128.0f/255.0f,177.0f/255.0f,211.0f/255.0f}},
        { "Uniform Updates", {253.0f/255.0f,180.0f/255.0f,98.0f/255.0f}},
        { "ROP", {179.0f/255.0f,222.0f/255.0f,105.0f/255.0f}},
        { "Vertex Format", {252.0f/255.0f,205.0f/255.0f,229.0f/255.0f}},
        { "Utility", {217.0f/255.0f,217.0f/255.0f,217.0f/255.0f}},
        { "Uncategorized", {188.0f/255.0f,128.0f/255.0f,189.0f/255.0f}}
    };

};

} // namespace logvis
