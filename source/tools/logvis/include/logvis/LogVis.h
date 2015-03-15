#pragma once

#include <logvis/logvis_api.h>
#include <glbinding/Logging.h>
#include <glbinding/Meta.h>
#include <glbinding/gl/gl.h>

#include <array>
#include <chrono>
#include <list>
#include <map>

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
    void renderLabel();
    void renderTime();

    std::list<int> getAvgTime();
    int getDigit(int number, int divisor);
    std::array<float, 4> getNumberPosition(int number);
    void createShaderProgram(const std::string vertSrcLocation, const std::string fragSrcLocation, gl::GLuint & vertexShader, gl::GLuint & fragmentShader, gl::GLuint & shaderProgram);
    bool readFile(const std::string & filePath, std::string & content);
    std::string readFile(const std::string & filePath);

protected:
    unsigned int m_tailId;
    std::list<CategoryStats> m_lastStats;
    CategoryStats m_maxStats;

    gl::GLuint m_logFrameBuffer;
    gl::GLuint m_vaos[3];
    gl::GLuint m_vbos[3];
    gl::GLuint m_ebos[3];
    gl::GLuint m_label_texture;
    gl::GLuint m_time_texture;
    gl::GLuint m_cat_vs;
    gl::GLuint m_cat_fs;
    gl::GLuint m_cat_program;
    gl::GLuint m_time_vs;
    gl::GLuint m_time_fs;
    gl::GLuint m_time_program;
    gl::GLuint m_label_vs;
    gl::GLuint m_label_fs;
    gl::GLuint m_label_program;

    std::chrono::high_resolution_clock::time_point m_lastTime;
    std::list<long long> m_lastTimes;

    std::chrono::high_resolution_clock::time_point m_lastUpdate;
    int m_frames;

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
        "Utility"
        // "Uncategorized"
    };

    const std::map<std::string, std::vector<float>> ColorByCategory
    {
        { "Draw", { 228.0f/255.0f, 26.0f/255.0f, 28.0f/255.0f }},
        { "Render Target", { 55.0f/255.0f, 126.0f/255.0f, 184.0f/255.0f }},
        { "Texture", { 77.0f/255.0f, 175.0f/255.0f, 74.0f/255.0f }},
        { "UBO Binding", { 152.0f/255.0f, 78.0f/255.0f, 163.0f/255.0f }},
        { "Program", { 255.0f/255.0f, 127.0f/255.0f, 0.0f/255.0f }},
        { "Uniform Updates", { 255.0f/255.0f, 255.0f/255.0f, 51.0f/255.0f }},
        { "ROP", { 166.0f/255.0f, 86.0f/255.0f, 40.0f/255.0f }},
        { "Vertex Format", { 247.0f/255.0f, 129.0f/255.0f, 191.0f/255.0f }},
        { "Utility", { 153.0f/255.0f, 153.0f/255.0f, 153.0f/255.0f }}
    };

};

} // namespace logvis
