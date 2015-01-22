#include <logvis/LogVis.h>
#include <iostream>

using namespace gl;

namespace logvis
{

LogVis::LogVis(gl::GLuint logTexture)
: m_log(glbinding::Logging::getBuffer())
{
    m_tailId = m_log.addTail();
    for (std::string category : glbinding::Meta::getCategories())
    {
        m_maxStats[category] = 0;
    };

    glGenFramebuffers(1, &m_logFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_logFrameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, logTexture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

LogVis::~LogVis()
{
    m_log.removeTail(m_tailId);
}

void LogVis::update()
{
    LogVis::CategoryStats categoryCount = getCurrentLogPart();

    updateMax(categoryCount);
    updateLast(categoryCount);

    // for(auto it = categoryCount.cbegin(); it != categoryCount.cend(); ++it)
    // {
    //     std::cout << it->first << ": " << it->second << " - " << averageCount(it->first) << " - " << m_maxStats[it->first] << std::endl;
    // }
    // std::cout << std::endl;

    renderLogTexture();
}

LogVis::CategoryStats LogVis::getCurrentLogPart()
{
    LogVis::CategoryStats categoryCount;
    for (std::string category : glbinding::Meta::getCategories())
    {
        categoryCount[category] = 0;
    };

    std::vector<glbinding::Logging::BufferType> logEntries = m_log.pullTail(m_tailId);
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
    if (m_lastStats.size() > 5)
        m_lastStats.pop_front();
}

int LogVis::averageCount(std::string category)
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
    glbinding::Logging::pause();
    glBindFramebuffer(GL_FRAMEBUFFER, m_logFrameBuffer);

    renderCats();

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
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posAttrib);

    // Draw
    glDrawElements(GL_TRIANGLES, m_maxStats.size()*6, GL_UNSIGNED_INT, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glbinding::Logging::resume();
}

void LogVis::renderCats()
{
    float margin = 0.025f;
    int catCount = m_maxStats.size();
    float width = (2.0f - (margin * (catCount-1))) / catCount;

    float vertices[catCount*4*2];
    // GLuint elements[catCount * 6];

    int catNumber = 0;
    for (std::string category : glbinding::Meta::getCategories())
    {
        int avg = 10;
        float height = -1.0f + avg * 0.08f;

        float start = -1.0f + catNumber * (width + margin);
        float end = -1.0f + catNumber * (width + margin) + width;

        vertices[0+(catNumber * 8)] = start; vertices[1+(catNumber * 8)] = height;
        vertices[2+(catNumber * 8)] =  end; vertices[3+(catNumber * 8)] = height;
        vertices[4+(catNumber * 8)] =  end; vertices[5+(catNumber * 8)] = -1.0f;
        vertices[6+(catNumber * 8)] = start; vertices[7+(catNumber * 8)] = -1.0f;

        // elements[0+(catNumber * 6)] = static_cast<GLuint>(0+(catNumber * 4)); elements[1+(catNumber * 6)] = static_cast<GLuint>(1+(catNumber * 4)); elements[2+(catNumber * 6)] = static_cast<GLuint>(2+(catNumber * 4));
        // elements[3+(catNumber * 6)] = static_cast<GLuint>(2+(catNumber * 4)); elements[4+(catNumber * 6)] = static_cast<GLuint>(3+(catNumber * 4)); elements[5+(catNumber * 6)] = static_cast<GLuint>(0+(catNumber * 4));
        catNumber++;
    };

    GLuint vbo;
    glGenBuffers(1, &vbo); // Generate 1 buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // GLuint elements[] = {
    //     //1
    //     0, 1, 2,
    //     2, 3, 0
        //2
        // 4, 5, 6,
        // 6, 7, 4,
        // //3
        // 8, 9, 10,
        // 10, 11, 8,
        // //4
        // 12, 13, 14,
        // 14, 15, 12,
        // //5
        // 16, 17, 18,
        // 18, 19, 16,
        // //6
        // 20, 21, 22,
        // 22, 23, 20,
        // //7
        // 24, 25, 26,
        // 26, 27, 24,
        // //8
        // 28, 29, 30,
        // 30, 31, 28,
        // //9
        // 32, 33, 34,
        // 34, 35, 32,
        // //10
        // 36, 37, 38,
        // 38, 39, 36,
        // //11
        // 40, 41, 42,
        // 42, 43, 40,
        // //12
        // 44, 45, 46,
        // 46, 47, 44,
        // //13
        // 48, 49, 50,
        // 50, 51, 48,
        // //14
        // 52, 53, 54,
        // 54, 55, 52,
        // //15
        // 56, 57, 58,
        // 58, 59, 56,
        // //16
        // 60, 61, 62,
        // 62, 63, 60,
        // //17
        // 64, 65, 66,
        // 66, 67, 64,
        // //18
        // 68, 69, 70,
        // 70, 71, 68,
        // //19
        // 72, 73, 74,
        // 74, 75, 72,
        // //20
        // 76, 77, 78,
        // 78, 79, 76,
        // //21
        // 80, 81, 82,
        // 82, 83, 80,
        // //22
        // 84, 85, 86,
        // 86, 87, 84

    GLuint elements[] = {
        0, 1, 2,
        2, 3, 0,
        4, 5, 6,
        6, 7, 4,
        8, 9, 10,
        10, 11, 8,
        12, 13, 14,
        14, 15, 12,
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20,
        24, 25, 26,
        26, 27, 24,
        28, 29, 30,
        30, 31, 28,
        32, 33, 34,
        34, 35, 32,
        36, 37, 38,
        38, 39, 36,
        40, 41, 42,
        42, 43, 40,
        44, 45, 46,
        46, 47, 44,
        48, 49, 50,
        50, 51, 48,
        52, 53, 54,
        54, 55, 52,
        56, 57, 58,
        58, 59, 56,
        60, 61, 62,
        62, 63, 60,
        64, 65, 66,
        66, 67, 64,
        68, 69, 70,
        70, 71, 68,
        72, 73, 74,
        74, 75, 72,
        76, 77, 78,
        78, 79, 76,
        80, 81, 82,
        82, 83, 80,
        84, 85, 86,
        86, 87, 84
    };

    GLuint ebo;
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
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
