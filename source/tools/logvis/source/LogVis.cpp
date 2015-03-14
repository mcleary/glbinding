#include <logvis/LogVis.h>

#include <iostream>
#include <fstream>
#include <math.h>

#include <glbinding/Binding.h>

#include "RawFile.h"

using namespace gl;

namespace logvis
{

LogVis::LogVis(gl::GLuint logTexture)
: m_tailId(glbinding::logging::addTail())
, m_lastTime(std::chrono::high_resolution_clock::now())
{
    for (std::string category : glbinding::Meta::getCategories())
    {
        m_maxStats[category] = 0;
    };

    glbinding::logging::pause();
    // glGenFramebuffers(1, &m_logFrameBuffer);
    // glBindFramebuffer(GL_FRAMEBUFFER, m_logFrameBuffer);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, logTexture, 0);
    
    // Label set up

    float label_vertices[] = {
    //  Position    Texcoords
    -1.0f,  -0.5, 0.0f, 1.0f, // Top-left
     1.0f,  -0.5f, 1.0f, 1.0f, // Top-right
     1.0f, -1.0f, 1.0f, 0.0f, // Bottom-right
    -1.0f, -1.0f, 0.0f, 0.0f  // Bottom-left
    };

    GLuint label_elements[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &m_vaos);
    glBindVertexArray(m_vaos);

    glGenBuffers(1, &m_vbos);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(label_vertices), label_vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &m_ebos);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebos);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(label_elements), label_elements, GL_STATIC_DRAW);

    // Shaders
    GLuint label_vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint label_fs = glCreateShader(GL_FRAGMENT_SHADER);

    std::string vertexSource   = readFile("data/log/label.vert");
    std::string fragmentSource = readFile("data/log/label.frag");

    const char * vertSource = vertexSource.c_str();
    const char * fragSource = fragmentSource.c_str();

    glShaderSource(label_vs, 1, &vertSource, nullptr);
    glCompileShader(label_vs);

    glShaderSource(label_fs, 1, &fragSource, nullptr);
    glCompileShader(label_fs);

    m_label_program = glCreateProgram();
    glAttachShader(m_label_program, label_vs);
    glAttachShader(m_label_program, label_fs);

    glBindFragDataLocation(m_label_program, 0, "outColor");

    glLinkProgram(m_label_program);

    GLint posAttrib = glGetAttribLocation(m_label_program, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);

    GLint texAttrib = glGetAttribLocation(m_label_program, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));

    // Texture
    glGenTextures(1, &m_textures);
    glBindTexture(GL_TEXTURE_2D, m_textures);

    glUniform1i(glGetUniformLocation(m_label_program, "tex"), 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<int>(GL_REPEAT));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<int>(GL_REPEAT));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<int>(GL_LINEAR));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<int>(GL_LINEAR));

    {
        RawFile label("data/log/label.1200.100.rgb.ub.raw");
        if (!label.isValid())
            std::cout << "warning: loading texture from " << label.filePath() << " failed.";

        glTexImage2D(GL_TEXTURE_2D, 0, static_cast<int>(GL_RGB8), 1200, 100, 0, GL_RGB, GL_UNSIGNED_BYTE, label.data());
    }

    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glbinding::logging::resume();
}

LogVis::~LogVis()
{
    glbinding::logging::removeTail(m_tailId);
}

void LogVis::update()
{
    // Update time
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( now - m_lastTime ).count();

    m_lastTimes.push_back(duration);
    if (m_lastTimes.size() > 10)
        m_lastTimes.pop_front();

    long long summed_duration = 0;
    for (auto time : m_lastTimes)
    {
        summed_duration += time;
    }
    auto avg_duration = summed_duration/static_cast<long long>(m_lastTimes.size());

    // std::cout << 1000000/avg_duration << " - duration: " << duration << std::endl;

    // Update categories
    LogVis::CategoryStats categoryCount = getCurrentLogPart();
    updateMax(categoryCount);
    updateLast(categoryCount);

    // for(auto it = categoryCount.cbegin(); it != categoryCount.cend(); ++it)
    // {
    //     std::cout << it->first << ": " << it->second << " - " << averageCount(it->first) << " - " << m_maxStats[it->first] << std::endl;
    // }
    // std::cout << std::endl;

    renderLogTexture();

    m_lastTime = now;
}

LogVis::CategoryStats LogVis::getCurrentLogPart()
{
    LogVis::CategoryStats categoryCount;
    for (std::string category : glbinding::Meta::getCategories())
    {
        categoryCount[category] = 0;
    };

    auto i = glbinding::logging::cbegin(m_tailId);
    while(glbinding::logging::valid(m_tailId, i))
    {
        std::string command = (*i)->function->name();
        std::string category = glbinding::Meta::getCategory(command);
        // std::cout << category << ": " << command << std::endl;
        if (category == "Uncategorized") {
            std::cout << command << std::endl;
        }
        ++categoryCount[category];

        i = glbinding::logging::next(m_tailId, i);
    }

    return categoryCount;
}

void LogVis::updateMax(LogVis::CategoryStats currentCounts)
{
    for (auto it = m_maxStats.cbegin(); it != m_maxStats.cend(); ++it)
    {
        if (it->second < currentCounts[it->first])
        {
            m_maxStats[it->first] = currentCounts[it->first];
        }
    }
}

void LogVis::updateLast(LogVis::CategoryStats currentCounts)
{
    m_lastStats.push_back(currentCounts);
    if (m_lastStats.size() > 500)
        m_lastStats.pop_front();
}

unsigned int LogVis::averageCount(std::string category)
{
    int count = 0;
    for (auto stats : m_lastStats)
    {
        count += stats[category];
    }
    return static_cast<float>(count)/static_cast<float>(m_lastStats.size());
}

void LogVis::renderLogTexture()
{   
    glbinding::logging::pause();
    // glBindFramebuffer(GL_FRAMEBUFFER, m_logFrameBuffer);

    // Draw
    // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    renderCats();
    // renderTime();
    renderLabel();


    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glbinding::logging::resume();
}

void LogVis::renderCats()
{
    float margin = 0.025f;
    int catCount = m_categories.size();
    float width = (2.0f - (margin * (catCount+1))) / catCount;

    int numVerts = 4 * 5 * 3;
    int numElem = 6 * 3;

    float vertices[catCount * numVerts];
    GLuint elements[catCount * numElem];

    int catNumber = 0;
    unsigned int maxValue = 40;
    unsigned int parts = 10;
    float blockSize = 1.5f / parts;
    for (std::string category : m_categories)
    {
        int now = std::min(maxValue, m_lastStats.back().at(category));
        int avg = std::min(maxValue, averageCount(category));
        int max = std::min(maxValue, m_maxStats.at(category));
        // now = 4 + catNumber * 4;
        // avg = 20;
        // max = 40;
        float height = -0.5f + ceil(now / 4.0f) * blockSize;
        float heightAvg = -0.5f + ceil(avg / 4.0f) * blockSize;
        float heightMax = -0.5f + ceil(max / 4.0f) * blockSize;

        // std::cout << category << ": " << now << " - " << height << "      " << max << " - " << heightMax << std::endl;


        float start = -1.0f + margin + catNumber * (width + margin);
        float end = -1.0f + margin + catNumber * (width + margin) + width;

        std::vector<float> color = ColorByCategory.at(category);

        vertices[0+(catNumber * numVerts)] = start; vertices[1+(catNumber * numVerts)] = height; vertices[2+(catNumber * numVerts)] = color[0]; vertices[3+(catNumber * numVerts)] = color[1]; vertices[4+(catNumber * numVerts)] = color[2];
        vertices[5+(catNumber * numVerts)] = end; vertices[6+(catNumber * numVerts)] = height; vertices[7+(catNumber * numVerts)] = color[0]; vertices[8+(catNumber * numVerts)] = color[1]; vertices[9+(catNumber * numVerts)] = color[2];
        vertices[10+(catNumber * numVerts)] = end; vertices[11+(catNumber * numVerts)] = -0.5f; vertices[12+(catNumber * numVerts)] = color[0]; vertices[13+(catNumber * numVerts)] = color[1]; vertices[14+(catNumber * numVerts)] = color[2];
        vertices[15+(catNumber * numVerts)] = start; vertices[16+(catNumber * numVerts)] = -0.5f; vertices[17+(catNumber * numVerts)] = color[0]; vertices[18+(catNumber * numVerts)] = color[1]; vertices[19+(catNumber * numVerts)] = color[2];

        vertices[20+(catNumber * numVerts)] = start - 0.005; vertices[21+(catNumber * numVerts)] = heightAvg; vertices[22+(catNumber * numVerts)] = color[0]; vertices[23+(catNumber * numVerts)] = color[1]; vertices[24+(catNumber * numVerts)] = color[2];
        vertices[25+(catNumber * numVerts)] = end + 0.005; vertices[26+(catNumber * numVerts)] = heightAvg; vertices[27+(catNumber * numVerts)] = color[0]; vertices[28+(catNumber * numVerts)] = color[1]; vertices[29+(catNumber * numVerts)] = color[2];
        vertices[30+(catNumber * numVerts)] = end + 0.005; vertices[31+(catNumber * numVerts)] = heightAvg - blockSize; vertices[32+(catNumber * numVerts)] = color[0]; vertices[33+(catNumber * numVerts)] = color[1]; vertices[34+(catNumber * numVerts)] = color[2];
        vertices[35+(catNumber * numVerts)] = start - 0.005; vertices[36+(catNumber * numVerts)] = heightAvg - blockSize; vertices[37+(catNumber * numVerts)] = color[0]; vertices[38+(catNumber * numVerts)] = color[1]; vertices[39+(catNumber * numVerts)] = color[2];

        vertices[40+(catNumber * numVerts)] = start; vertices[41+(catNumber * numVerts)] = heightMax; vertices[42+(catNumber * numVerts)] = color[0]; vertices[43+(catNumber * numVerts)] = color[1]; vertices[44+(catNumber * numVerts)] = color[2];
        vertices[45+(catNumber * numVerts)] = end; vertices[46+(catNumber * numVerts)] = heightMax; vertices[47+(catNumber * numVerts)] = color[0]; vertices[48+(catNumber * numVerts)] = color[1]; vertices[49+(catNumber * numVerts)] = color[2];
        vertices[50+(catNumber * numVerts)] = end; vertices[51+(catNumber * numVerts)] = heightMax - blockSize; vertices[52+(catNumber * numVerts)] = color[0]; vertices[53+(catNumber * numVerts)] = color[1]; vertices[54+(catNumber * numVerts)] = color[2];
        vertices[55+(catNumber * numVerts)] = start; vertices[56+(catNumber * numVerts)] = heightMax - blockSize; vertices[57+(catNumber * numVerts)] = color[0]; vertices[58+(catNumber * numVerts)] = color[1]; vertices[59+(catNumber * numVerts)] = color[2];


        elements[0+(catNumber * numElem)] = static_cast<GLuint>(0+(catNumber * 4 * 3)); elements[1+(catNumber * numElem)] = static_cast<GLuint>(1+(catNumber * 4 * 3)); elements[2+(catNumber * numElem)] = static_cast<GLuint>(2+(catNumber * 4 * 3));
        elements[3+(catNumber * numElem)] = static_cast<GLuint>(2+(catNumber * 4 * 3)); elements[4+(catNumber * numElem)] = static_cast<GLuint>(3+(catNumber * 4 * 3)); elements[5+(catNumber * numElem)] = static_cast<GLuint>(0+(catNumber * 4 * 3));

        elements[6+(catNumber * numElem)] = static_cast<GLuint>(4+(catNumber * 4 * 3)); elements[7+(catNumber * numElem)] = static_cast<GLuint>(5+(catNumber * 4 * 3)); elements[8+(catNumber * numElem)] = static_cast<GLuint>(6+(catNumber * 4 * 3));
        elements[9+(catNumber * numElem)] = static_cast<GLuint>(6+(catNumber * 4 * 3)); elements[10+(catNumber * numElem)] = static_cast<GLuint>(7+(catNumber * 4 * 3)); elements[11+(catNumber * numElem)] = static_cast<GLuint>(4+(catNumber * 4 * 3));

        elements[12+(catNumber * numElem)] = static_cast<GLuint>(8+(catNumber * 4 * 3)); elements[13+(catNumber * numElem)] = static_cast<GLuint>(9+(catNumber * 4 * 3)); elements[14+(catNumber * numElem)] = static_cast<GLuint>(10+(catNumber * 4 * 3));
        elements[15+(catNumber * numElem)] = static_cast<GLuint>(10+(catNumber * 4 * 3)); elements[16+(catNumber * numElem)] = static_cast<GLuint>(11+(catNumber * 4 * 3)); elements[17+(catNumber * numElem)] = static_cast<GLuint>(8+(catNumber * 4 * 3));

        catNumber++;
    };

    GLuint vbo;
    glGenBuffers(1, &vbo); // Generate 1 buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint ebo;
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    // Shaders
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

    std::string vertexSource   = readFile("data/log/logvis.vert");
    std::string fragmentSource = readFile("data/log/logvis.frag");

    const char * vertSource = vertexSource.c_str();
    const char * fragSource = fragmentSource.c_str();

    glShaderSource(vs, 1, &vertSource, nullptr);
    glCompileShader(vs);

    glShaderSource(fs, 1, &fragSource, nullptr);
    glCompileShader(fs);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);

    glBindFragDataLocation(shaderProgram, 0, "outColor");

    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);

    GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));

    // 9 * 18: 9 categories * 3 values * 2 triangles * 3 vertices
    glDrawElements(GL_TRIANGLES, 9*18, GL_UNSIGNED_INT, 0);

}

void LogVis::renderTime()
{
}

void LogVis::renderLabel()
{
    glBindVertexArray(m_vaos);
    glUseProgram(m_label_program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textures);
    glUniform1i(glGetUniformLocation(m_label_program, "tex"), 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

bool LogVis::readFile(const std::string & filePath, std::string & content)
{
    // http://insanecoding.blogspot.de/2011/11/how-to-read-in-file-in-c.html

    std::ifstream in(filePath, std::ios::in | std::ios::binary);

    if (!in)
        return false;

    content = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    return true;
}

// convenience
std::string LogVis::readFile(const std::string & filePath)
{
    std::string content;
    readFile(filePath, content);

    return content;
}

} // namespace logvis
