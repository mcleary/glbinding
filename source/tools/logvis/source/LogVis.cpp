#include <logvis/LogVis.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <glbinding/Binding.h>

using namespace gl;

namespace logvis
{

LogVis::LogVis(gl::GLuint logTexture)
{
    m_tailId = glbinding::logging::addTail();
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
    glbinding::logging::removeTail(m_tailId);
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

    auto i = glbinding::logging::cbegin(m_tailId);
    while(glbinding::logging::valid(m_tailId, i))
    {
        std::string command = (*i)->function->name();
        std::string category = glbinding::Meta::getCategory(command);
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
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);

    GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));

    // Draw
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, 22*18, GL_UNSIGNED_INT, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    for (std::string category : m_categories)
    {
        unsigned int maxValue = 40;
        unsigned int parts = 10;
        float blockSize = 2.0f / parts;
        int now = std::min(maxValue, m_lastStats.back().at(category));
        int avg = std::min(maxValue, averageCount(category));
        int max = std::min(maxValue, m_maxStats.at(category));
        // now = 4 + catNumber * 4;
        // avg = 37;
        // max = 40;
        float height = -1.0f + ceil(now / 4.0f) * blockSize;
        float heightAvg = -1.0f + ceil(avg / 4.0f) * blockSize;
        float heightMax = -1.0f + ceil(max / 4.0f) * blockSize;

        // std::cout << category << ": " << now << " - " << height << "      " << max << " - " << heightMax << std::endl;


        float start = -1.0f + margin + catNumber * (width + margin);
        float end = -1.0f + margin + catNumber * (width + margin) + width;

        std::vector<float> color = ColorByCategory.at(category);

        vertices[0+(catNumber * numVerts)] = start; vertices[1+(catNumber * numVerts)] = height; vertices[2+(catNumber * numVerts)] = color[0]; vertices[3+(catNumber * numVerts)] = color[1]; vertices[4+(catNumber * numVerts)] = color[2];
        vertices[5+(catNumber * numVerts)] = end; vertices[6+(catNumber * numVerts)] = height; vertices[7+(catNumber * numVerts)] = color[0]; vertices[8+(catNumber * numVerts)] = color[1]; vertices[9+(catNumber * numVerts)] = color[2];
        vertices[10+(catNumber * numVerts)] = end; vertices[11+(catNumber * numVerts)] = -1.0f; vertices[12+(catNumber * numVerts)] = color[0]; vertices[13+(catNumber * numVerts)] = color[1]; vertices[14+(catNumber * numVerts)] = color[2];
        vertices[15+(catNumber * numVerts)] = start; vertices[16+(catNumber * numVerts)] = -1.0f; vertices[17+(catNumber * numVerts)] = color[0]; vertices[18+(catNumber * numVerts)] = color[1]; vertices[19+(catNumber * numVerts)] = color[2];

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
