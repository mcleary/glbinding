#include <logvis/LogVis.h>

#include <iostream>
#include <fstream>
#include <math.h>

#include <glbinding/Binding.h>
#include "../../glbinding/source/logging_private.h"

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
    glGenFramebuffers(1, &m_logFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_logFrameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, logTexture, 0);
    
    glGenVertexArrays(3, m_vaos);
    glGenBuffers(3, m_vbos);
    glGenBuffers(3, m_ebos);

    // Cat set up
    int catCount = m_categories.size();
    int numVerts = 4 * 5 * 3;
    int numElem = 6 * 3;

    float cat_vertices[catCount * numVerts];
    GLuint cat_elements[catCount * numElem];

    for (int i = 0; i < catCount; ++i)
    {
        cat_elements[0+(i * numElem)] = static_cast<GLuint>(0+(i * 4 * 3)); cat_elements[1+(i * numElem)] = static_cast<GLuint>(1+(i * 4 * 3)); cat_elements[2+(i * numElem)] = static_cast<GLuint>(2+(i * 4 * 3));
        cat_elements[3+(i * numElem)] = static_cast<GLuint>(2+(i * 4 * 3)); cat_elements[4+(i * numElem)] = static_cast<GLuint>(3+(i * 4 * 3)); cat_elements[5+(i * numElem)] = static_cast<GLuint>(0+(i * 4 * 3));

        cat_elements[6+(i * numElem)] = static_cast<GLuint>(4+(i * 4 * 3)); cat_elements[7+(i * numElem)] = static_cast<GLuint>(5+(i * 4 * 3)); cat_elements[8+(i * numElem)] = static_cast<GLuint>(6+(i * 4 * 3));
        cat_elements[9+(i * numElem)] = static_cast<GLuint>(6+(i * 4 * 3)); cat_elements[10+(i * numElem)] = static_cast<GLuint>(7+(i * 4 * 3)); cat_elements[11+(i * numElem)] = static_cast<GLuint>(4+(i * 4 * 3));

        cat_elements[12+(i * numElem)] = static_cast<GLuint>(8+(i * 4 * 3)); cat_elements[13+(i * numElem)] = static_cast<GLuint>(9+(i * 4 * 3)); cat_elements[14+(i * numElem)] = static_cast<GLuint>(10+(i * 4 * 3));
        cat_elements[15+(i * numElem)] = static_cast<GLuint>(10+(i * 4 * 3)); cat_elements[16+(i * numElem)] = static_cast<GLuint>(11+(i * 4 * 3)); cat_elements[17+(i * numElem)] = static_cast<GLuint>(8+(i * 4 * 3));        
    }

    glBindVertexArray(m_vaos[0]);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cat_vertices), cat_vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebos[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cat_elements), cat_elements, GL_STATIC_DRAW);

    // Shaders

    createShaderProgram("data/log/logvis.vert", "data/log/logvis.frag", m_cat_vs, m_cat_fs, m_cat_program);

    GLint cat_posAttrib = glGetAttribLocation(m_cat_program, "position");
    glEnableVertexAttribArray(cat_posAttrib);
    glVertexAttribPointer(cat_posAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);

    GLint colAttrib = glGetAttribLocation(m_cat_program, "color");
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));

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

    glBindVertexArray(m_vaos[1]);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbos[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(label_vertices), label_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(label_elements), label_elements, GL_STATIC_DRAW);

    // Shaders
    createShaderProgram("data/log/label.vert", "data/log/label.frag", m_label_vs, m_label_fs, m_label_program);

    GLint posAttrib = glGetAttribLocation(m_label_program, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);

    GLint texAttrib = glGetAttribLocation(m_label_program, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));

    // Texture
    glGenTextures(1, &m_label_texture);
    glBindTexture(GL_TEXTURE_2D, m_label_texture);

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

    // Number set up
    float time_vertices[9 * 4 * 4];
    GLuint time_elements[9 * 6];

    for (int i = 0; i < 9; ++i)
    {
        time_elements[0+(i * 6)] = static_cast<GLuint>(0+(i * 4)); time_elements[1+(i * 6)] = static_cast<GLuint>(1+(i * 4)); time_elements[2+(i * 6)] = static_cast<GLuint>(2+(i * 4));
        time_elements[3+(i * 6)] = static_cast<GLuint>(2+(i * 4)); time_elements[4+(i * 6)] = static_cast<GLuint>(3+(i * 4)); time_elements[5+(i * 6)] = static_cast<GLuint>(0+(i * 4));
    }

    glBindVertexArray(m_vaos[2]);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbos[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(time_vertices), time_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebos[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(time_elements), time_elements, GL_STATIC_DRAW);

    // Shaders
    createShaderProgram("data/log/label.vert", "data/log/label.frag", m_time_vs, m_time_fs, m_time_program);

    GLint time_posAttrib = glGetAttribLocation(m_time_program, "position");
    glEnableVertexAttribArray(time_posAttrib);
    glVertexAttribPointer(time_posAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);

    GLint time_texAttrib = glGetAttribLocation(m_time_program, "texcoord");
    glEnableVertexAttribArray(time_texAttrib);
    glVertexAttribPointer(time_texAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));

    // Texture
    glGenTextures(1, &m_time_texture);
    glBindTexture(GL_TEXTURE_2D, m_time_texture);

    glUniform1i(glGetUniformLocation(m_time_program, "tex"), 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<int>(GL_REPEAT));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<int>(GL_REPEAT));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<int>(GL_LINEAR));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<int>(GL_LINEAR));

    {
        RawFile time("data/log/time.512.512.rgb.ub.raw");
        if (!time.isValid())
            std::cout << "warning: loading texture from " << time.filePath() << " failed.";

        glTexImage2D(GL_TEXTURE_2D, 0, static_cast<int>(GL_RGB8), 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, time.data());
    }    

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glbinding::logging::resume();
}

LogVis::~LogVis()
{
    glbinding::logging::removeTail(m_tailId);
    glDeleteFramebuffers(1, &m_logFrameBuffer);

    glDeleteTextures(1, &m_label_texture);
    glDeleteTextures(1, &m_time_texture);

    glDeleteBuffers(3, m_vaos);
    glDeleteBuffers(3, m_vbos);
    glDeleteBuffers(3, m_ebos);

    glDeleteProgram(m_cat_program);
    glDeleteShader(m_cat_fs);
    glDeleteShader(m_cat_vs);
    glDeleteProgram(m_label_program);
    glDeleteShader(m_label_fs);
    glDeleteShader(m_label_vs);
    glDeleteProgram(m_time_program);
    glDeleteShader(m_time_fs);
    glDeleteShader(m_time_vs);
}

void LogVis::update()
{
    // Update time
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( now - m_lastTime ).count();

    m_lastTimes.push_back(duration);
    if (m_lastTimes.size() > 60)
        m_lastTimes.pop_front();

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
    if (m_lastStats.size() > 60)
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
    glBindFramebuffer(GL_FRAMEBUFFER, m_logFrameBuffer);
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( m_lastTime - m_lastUpdate  ).count();
    if (duration >= 100000)
    {
        // Draw
        glClear(GL_COLOR_BUFFER_BIT);

        renderCats();
        renderLabel();
        renderTime();

        m_lastUpdate = m_lastTime;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glbinding::logging::resume();
}

void LogVis::renderCats()
{
    float margin = 0.05f;
    int catCount = m_categories.size();
    float width = (2.0f - (margin * (catCount+1))) / catCount;

    int numVerts = 4 * 5 * 3;

    float vertices[catCount * numVerts];

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

        vertices[20+(catNumber * numVerts)] = start - 0.015; vertices[21+(catNumber * numVerts)] = heightAvg; vertices[22+(catNumber * numVerts)] = color[0]; vertices[23+(catNumber * numVerts)] = color[1]; vertices[24+(catNumber * numVerts)] = color[2];
        vertices[25+(catNumber * numVerts)] = end + 0.015; vertices[26+(catNumber * numVerts)] = heightAvg; vertices[27+(catNumber * numVerts)] = color[0]; vertices[28+(catNumber * numVerts)] = color[1]; vertices[29+(catNumber * numVerts)] = color[2];
        vertices[30+(catNumber * numVerts)] = end + 0.015; vertices[31+(catNumber * numVerts)] = heightAvg - blockSize; vertices[32+(catNumber * numVerts)] = color[0]; vertices[33+(catNumber * numVerts)] = color[1]; vertices[34+(catNumber * numVerts)] = color[2];
        vertices[35+(catNumber * numVerts)] = start - 0.015; vertices[36+(catNumber * numVerts)] = heightAvg - blockSize; vertices[37+(catNumber * numVerts)] = color[0]; vertices[38+(catNumber * numVerts)] = color[1]; vertices[39+(catNumber * numVerts)] = color[2];

        vertices[40+(catNumber * numVerts)] = start; vertices[41+(catNumber * numVerts)] = heightMax; vertices[42+(catNumber * numVerts)] = color[0]; vertices[43+(catNumber * numVerts)] = color[1]; vertices[44+(catNumber * numVerts)] = color[2];
        vertices[45+(catNumber * numVerts)] = end; vertices[46+(catNumber * numVerts)] = heightMax; vertices[47+(catNumber * numVerts)] = color[0]; vertices[48+(catNumber * numVerts)] = color[1]; vertices[49+(catNumber * numVerts)] = color[2];
        vertices[50+(catNumber * numVerts)] = end; vertices[51+(catNumber * numVerts)] = heightMax - blockSize; vertices[52+(catNumber * numVerts)] = color[0]; vertices[53+(catNumber * numVerts)] = color[1]; vertices[54+(catNumber * numVerts)] = color[2];
        vertices[55+(catNumber * numVerts)] = start; vertices[56+(catNumber * numVerts)] = heightMax - blockSize; vertices[57+(catNumber * numVerts)] = color[0]; vertices[58+(catNumber * numVerts)] = color[1]; vertices[59+(catNumber * numVerts)] = color[2];

        catNumber++;
    };

    glBindVertexArray(m_vaos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glUseProgram(m_cat_program);

    // 9 * 18: 9 categories * 3 values * 2 triangles * 3 vertices
    glDrawElements(GL_TRIANGLES, 9*18, GL_UNSIGNED_INT, 0);

}

void LogVis::renderLabel()
{
    glBindVertexArray(m_vaos[1]);
    glUseProgram(m_label_program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_label_texture);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void LogVis::renderTime()
{
    float a = 0.05f;
    float time_vertices[9 * 4 * 4];

    time_vertices[0] = 1.0f - 2 * a; time_vertices[1] = 1.0f; time_vertices[2] = 0.0f; time_vertices[3] = 0.38f;
    time_vertices[4] = 1.0f; time_vertices[5] = 1.0f; time_vertices[6] = 0.5f; time_vertices[7] = 0.38f;
    time_vertices[8] = 1.0f; time_vertices[9] = 0.98f - 3*a; time_vertices[10] = 0.5f; time_vertices[11] = 0.07f;
    time_vertices[12] = 1.0f - 2 * a; time_vertices[13] = 0.98f - 3*a; time_vertices[14] = 0.0f; time_vertices[15] = 0.07f;       

    auto digits = getAvgTime();

    for (int i = 1; i < 9; ++i)
    {
        auto texPos = getNumberPosition(digits.front());
        time_vertices[0+(i*16)] = 1.0f - (i+2) * a; time_vertices[1+(i*16)] = 1.0f; time_vertices[2+(i*16)] = texPos[0]; time_vertices[3+(i*16)] = texPos[2];
        time_vertices[4+(i*16)] = 1.0f - (i+1) * a; time_vertices[5+(i*16)] = 1.0f; time_vertices[6+(i*16)] = texPos[1]; time_vertices[7+(i*16)] = texPos[2];
        time_vertices[8+(i*16)] = 1.0f - (i+1) * a; time_vertices[9+(i*16)] = 1.0f - 3*a; time_vertices[10+(i*16)] = texPos[1]; time_vertices[11+(i*16)] = texPos[3];
        time_vertices[12+(i*16)] = 1.0f - (i+2) * a; time_vertices[13+(i*16)] = 1.0f - 3*a; time_vertices[14+(i*16)] = texPos[0]; time_vertices[15+(i*16)] = texPos[3];       
        digits.pop_front();
    }

    glBindVertexArray(m_vaos[2]);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbos[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(time_vertices), time_vertices, GL_DYNAMIC_DRAW);

    glUseProgram(m_time_program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_time_texture);

    glDrawElements(GL_TRIANGLES, 9*6, GL_UNSIGNED_INT, 0);
}

std::list<int> LogVis::getAvgTime()
{
    long long summed_duration = 0;
    for (auto time : m_lastTimes)
    {
        summed_duration += time;
    }
    auto avg_duration = static_cast<int>(summed_duration)/m_lastTimes.size();
    // std::cout << 1000000/avg_duration << " - duration: " << avg_duration << std::endl;

    std::list<int> numbers;
    numbers.push_back(getDigit(avg_duration, 1));
    numbers.push_back(getDigit(avg_duration, 10));
    numbers.push_back(getDigit(avg_duration, 100));
    numbers.push_back(getDigit(avg_duration, 1000));
    numbers.push_back(getDigit(avg_duration, 10000));
    numbers.push_back(getDigit(avg_duration, 100000));
    numbers.push_back(getDigit(avg_duration, 1000000));
    numbers.push_back(getDigit(avg_duration, 10000000));

    return numbers;
}

int LogVis::getDigit(int number, int divisor)
{
    if (divisor > number)
        return -1;
    else
        return (number / divisor % 10);
}

std::array<float, 4> LogVis::getNumberPosition(int number)
{
    std::array<float, 4> vertices;
    switch (number)
    {
        case 0:
            vertices = {{0.0f, 0.2f, 1.0f, 0.74f}};
            break;
        case 1:
            vertices = {{0.2f, 0.4f, 1.0f, 0.74f}};
            break;
        case 2:
            vertices = {{0.4f, 0.6f, 1.0f, 0.74f}};
            break;
        case 3:
            vertices = {{0.6f, 0.8f, 1.0f, 0.74f}};
            break;
        case 4:
            vertices = {{0.8f, 1.0f, 1.0f, 0.74f}};
            break;
        case 5:
            vertices = {{0.0f, 0.2f, 0.69f, 0.43f}};
            break;
        case 6:
            vertices = {{0.2f, 0.4f, 0.69f, 0.43f}};
            break;
        case 7:
            vertices = {{0.4f, 0.6f, 0.69f, 0.43f}};
            break;
        case 8:
            vertices = {{0.6f, 0.8f, 0.69f, 0.43f}};
            break;
        case 9:
            vertices = {{0.8f, 1.0f, 0.69f, 0.43f}};
            break;
        case 10:
            vertices = {{0.0f, 0.5f, 0.4f, 0.0f}};
            break;
        default:
            vertices = {{0.5f, 1.0f, 0.4f, 0.0f}};
    }
    return vertices;
}

void LogVis::createShaderProgram(const std::string vertSrcLocation, const std::string fragSrcLocation, GLuint & vertexShader, GLuint & fragmentShader, GLuint & shaderProgram)
{
    std::string vertexSource   = readFile(vertSrcLocation);
    std::string fragmentSource = readFile(fragSrcLocation);

    const char * vertSource = vertexSource.c_str();
    const char * fragSource = fragmentSource.c_str();

    // Create and compile the vertex shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertSource, NULL);
    glCompileShader(vertexShader);

    // Create and compile the fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragSource, NULL);
    glCompileShader(fragmentShader);

    // Link the vertex and fragment shader into a shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
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
